#include "model_import/NaiveSignatureDeduplicator.h"

#include <algorithm>
#include <cmath>

#include "math/MathConstants.h"

namespace
{
constexpr float kSignatureEpsilon = 0.01f;
}

NaiveSignatureDeduplicator::Signature NaiveSignatureDeduplicator::ComputeSignature(const ModelMesh& mesh)
{
    Signature signature;
    signature.vertexCount = mesh.positions.size();

    if (mesh.positions.empty())
    {
        return signature;
    }

    Vec3 centroid = Vec3::Zero();
    for (const Vec3& p : mesh.positions)
    {
        centroid += p;
    }
    centroid = centroid / static_cast<float>(mesh.positions.size());

    std::vector<float> distances;
    distances.reserve(mesh.positions.size());
    float maxDistance = 0.0f;
    for (const Vec3& p : mesh.positions)
    {
        const float d = (p - centroid).Length();
        distances.push_back(d);
        maxDistance = std::max(maxDistance, d);
    }

    if (maxDistance < MathConstants::kEpsilon)
    {
        signature.sortedNormalizedDistances.assign(distances.size(), 0.0f);
        return signature;
    }

    for (float& d : distances)
    {
        d /= maxDistance;
    }
    std::sort(distances.begin(), distances.end());
    signature.sortedNormalizedDistances = std::move(distances);

    float surfaceArea = 0.0f;
    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
    {
        const Vec3& p0 = mesh.positions[mesh.indices[i]];
        const Vec3& p1 = mesh.positions[mesh.indices[i + 1]];
        const Vec3& p2 = mesh.positions[mesh.indices[i + 2]];
        surfaceArea += 0.5f * Cross(p1 - p0, p2 - p0).Length();
    }
    signature.normalizedSurfaceArea = surfaceArea / (maxDistance * maxDistance);

    return signature;
}

bool NaiveSignatureDeduplicator::AreCongruent(const Signature& a, const Signature& b)
{
    if (a.vertexCount != b.vertexCount)
    {
        return false;
    }
    if (std::abs(a.normalizedSurfaceArea - b.normalizedSurfaceArea) > kSignatureEpsilon)
    {
        return false;
    }
    if (a.sortedNormalizedDistances.size() != b.sortedNormalizedDistances.size())
    {
        return false;
    }
    for (size_t i = 0; i < a.sortedNormalizedDistances.size(); ++i)
    {
        if (std::abs(a.sortedNormalizedDistances[i] - b.sortedNormalizedDistances[i]) > kSignatureEpsilon)
        {
            return false;
        }
    }
    return true;
}

std::shared_ptr<const ModelMesh> NaiveSignatureDeduplicator::FindOrRegister(std::shared_ptr<const ModelMesh> candidate)
{
    const Signature signature = ComputeSignature(*candidate);

    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& entry : m_registry)
    {
        if (AreCongruent(entry.first, signature))
        {
            return entry.second;
        }
    }

    m_registry.emplace_back(signature, candidate);
    return candidate;
}
