#pragma once

#include <mutex>
#include <vector>

#include "model_import/IMeshDeduplicator.h"

// Author: Claude
// Description: 회전/이동/균등 스케일에 불변한 저비용 근사 시그니처로 ModelMesh 합동 여부를 판정하는
//              IMeshDeduplicator 기본 구현.
// Input: (해당 없음 - 상태는 내부 시그니처 registry)
// Output: FindOrRegister - 합동인 기존 ModelMesh 핸들 또는 candidate 자신
// Notes: **정확한 형상 정합(shape registration)이 아니라 휴리스틱이다.** 시그니처는
//        (1) 정점 개수, (2) 무게중심 기준 정점 거리를 정렬한 뒤 최댓값으로 정규화한 리스트,
//        (3) 삼각형 표면적 합을 최댓값 거리의 제곱으로 정규화한 값으로 구성된다 - 전부 회전/이동/
//        균등 스케일에 불변하지만, 비균일 스케일이나 미러링(반사)은 다른 것으로 오판할 수 있다.
//        registry는 벡터에 순차 비교(O(n))로 구현했다 - 이번 사이클의 목적이 정확도나 성능이
//        아니라 향후 교체를 위한 인터페이스 경계 확정이기 때문에 의도적으로 단순하게 유지했다.
//        registry에 축출(eviction) 정책이 없어 프로세스 생애 동안 등록된 모든 고유 지오메트리
//        시그니처가 계속 쌓인다 - 실사용 규모가 커지면 재검토가 필요하다.
// Date: 2026-07-23
class NaiveSignatureDeduplicator : public IMeshDeduplicator
{
public:
    std::shared_ptr<const ModelMesh> FindOrRegister(std::shared_ptr<const ModelMesh> candidate) override;

private:
    struct Signature
    {
        size_t vertexCount = 0;
        float normalizedSurfaceArea = 0.0f;
        std::vector<float> sortedNormalizedDistances;
    };

    static Signature ComputeSignature(const ModelMesh& mesh);
    static bool AreCongruent(const Signature& a, const Signature& b);

    std::mutex m_mutex;
    std::vector<std::pair<Signature, std::shared_ptr<const ModelMesh>>> m_registry;
};
