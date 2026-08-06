#include "platform/collision/gjk_epa/Epa.h"

#include <algorithm>
#include <utility>

#include "platform/collision/gjk_epa/MinkowskiSupport.h"

// Author: Claude
// Description: RunEpa()의 구현 — 폴리토프(다면체) 확장 기반 EPA.
// Input/Output: (Epa.h 참고)
// Notes: MakeFace()는 항상 원점이 폴리토프 내부에 있다는 불변식(GJK가 보장)을 이용해, 계산한 법선이
//        원점 쪽을 향하면 정점 순서를 뒤집어 바깥쪽을 향하도록 보정한다.
// Date: 2026-07-20

namespace
{
    struct Face
    {
        int a, b, c;
        Vec3 normal;
        float distance;
    };

    Face MakeFace(const std::vector<Vec3>& vertices, int ia, int ib, int ic)
    {
        Vec3 normal = Cross(vertices[ib] - vertices[ia], vertices[ic] - vertices[ia]).Normalized();
        float distance = Dot(normal, vertices[ia]);
        if (distance < 0.0f)
        {
            std::swap(ib, ic);
            normal = -normal;
            distance = -distance;
        }
        return Face{ia, ib, ic, normal, distance};
    }

    void AddOrRemoveEdge(std::vector<std::pair<int, int>>& edges, int a, int b)
    {
        const auto it = std::find(edges.begin(), edges.end(), std::pair<int, int>(b, a));
        if (it != edges.end())
        {
            edges.erase(it);
        }
        else
        {
            edges.push_back({a, b});
        }
    }

    size_t FindClosestFaceIndex(const std::vector<Face>& faces)
    {
        size_t minIndex = 0;
        for (size_t i = 1; i < faces.size(); ++i)
        {
            if (faces[i].distance < faces[minIndex].distance)
            {
                minIndex = i;
            }
        }
        return minIndex;
    }

    bool IsDuplicateVertex(const std::vector<Vec3>& vertices, const Vec3& p, float epsilon)
    {
        for (const Vec3& v : vertices)
        {
            if ((v - p).LengthSquared() < epsilon * epsilon)
            {
                return true;
            }
        }
        return false;
    }
}

PenetrationInfo RunEpa(const Geometry& a, const Geometry& b, const GjkSimplex& startingSimplex)
{
    std::vector<Vec3> vertices = startingSimplex.points;
    std::vector<Face> faces = {
        MakeFace(vertices, 0, 1, 2),
        MakeFace(vertices, 0, 1, 3),
        MakeFace(vertices, 0, 2, 3),
        MakeFace(vertices, 1, 2, 3),
    };

    constexpr int kMaxIterations = 64;
    constexpr float kTolerance = 1e-4f;

    for (int iter = 0; iter < kMaxIterations; ++iter)
    {
        // AABB/OBB처럼 지지 함수가 한 옥탄트 전체에서 같은 점(모서리)을 반환하는 평평한 도형은, 가장 가까운
        // 면의 법선으로 다시 지지점을 구해도 그 면 자신의 정점과 같은 점이 나올 수 있다 — 이 경우 "개선 없음"이
        // 진짜 수렴이 아니라 이미 아는 점을 다시 찾은 것뿐이므로, 다음으로 가까운 면으로 넘어간다.
        std::vector<size_t> order(faces.size());
        for (size_t i = 0; i < faces.size(); ++i)
        {
            order[i] = i;
        }
        std::sort(order.begin(), order.end(),
                  [&](size_t l, size_t r) { return faces[l].distance < faces[r].distance; });

        size_t minIndex = order.front();
        Vec3 p = MinkowskiSupport(a, b, faces[minIndex].normal);
        for (size_t candidate : order)
        {
            const Vec3 candidateP = MinkowskiSupport(a, b, faces[candidate].normal);
            if (!IsDuplicateVertex(vertices, candidateP, kTolerance))
            {
                minIndex = candidate;
                p = candidateP;
                break;
            }
        }

        const Face minFace = faces[minIndex];
        const float d = Dot(minFace.normal, p);

        if (d - minFace.distance < kTolerance)
        {
            return PenetrationInfo{minFace.normal, minFace.distance};
        }

        vertices.push_back(p);
        const int newIndex = static_cast<int>(vertices.size()) - 1;

        std::vector<std::pair<int, int>> horizonEdges;
        std::vector<Face> keptFaces;
        for (const Face& face : faces)
        {
            if (Dot(face.normal, p - vertices[face.a]) > 0.0f)
            {
                AddOrRemoveEdge(horizonEdges, face.a, face.b);
                AddOrRemoveEdge(horizonEdges, face.b, face.c);
                AddOrRemoveEdge(horizonEdges, face.c, face.a);
            }
            else
            {
                keptFaces.push_back(face);
            }
        }

        for (const auto& edge : horizonEdges)
        {
            keptFaces.push_back(MakeFace(vertices, edge.first, edge.second, newIndex));
        }
        faces = std::move(keptFaces);
    }

    const size_t minIndex = FindClosestFaceIndex(faces);
    return PenetrationInfo{faces[minIndex].normal, faces[minIndex].distance};
}
