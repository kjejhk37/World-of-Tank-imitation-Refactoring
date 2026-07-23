#pragma once

#include <functional>
#include <memory>
#include <string>

#include "model_import/IMeshCachePolicy.h"
#include "model_import/IMeshDeduplicator.h"
#include "model_import/Model.h"

// Author: Claude
// Description: Model의 opt-in 캐싱을 제공하는 진입점. ModelLoader(상태 없는 파싱 함수)와 별개로,
//              "저장할지 여부"를 사용자가 이 클래스를 통해 선택할 수 있게 한다.
// Input: 생성자 - IMeshCachePolicy(필수, 정책 선택) + IMeshDeduplicator(선택, 기본 nullptr)
// Output: GetOrLoad - 캐시된(또는 새로 로드한) Model의 shared_ptr
// Notes: 캐시 키는 파일 경로로 좁게 고정하지 않고 고유 식별자 문자열로 취급한다 - 향후 절차적
//        프리미티브(Sphere 등)를 "primitive:sphere:r=1.0:sub=16" 같은 합성 키로 같은 매니저에
//        올릴 수 있게 열어두기 위함이다.
//        GetOrLoad는 캐시 미스 시 loader()를 락 없이 호출한다 - 각 캐시 정책/Deduplicator가
//        내부적으로 스레드 안전하므로, 서로 다른 키를 동시에 로드하는 것을 막지 않기 위함이다.
//        단, 같은 키를 여러 스레드가 동시에 처음 로드하면 파싱이 중복 실행될 수 있다(마지막
//        Store만 캐시에 남음) - 데이터 손상은 없지만 낭비가 있을 수 있는 알려진 단순화다.
//        MeshManager를 거치지 않고 ModelLoader를 직접 호출하면 이 캐싱/중복제거는 전혀 적용되지 않는다.
// Date: 2026-07-23
class MeshManager
{
public:
    explicit MeshManager(std::unique_ptr<IMeshCachePolicy> cachePolicy,
                         std::unique_ptr<IMeshDeduplicator> deduplicator = nullptr);

    std::shared_ptr<const Model> GetOrLoad(const std::string& key, const std::function<Model()>& loader);
    void Unload(const std::string& key);

private:
    std::unique_ptr<IMeshCachePolicy> m_cachePolicy;
    std::unique_ptr<IMeshDeduplicator> m_deduplicator;
};
