#pragma once

#include <mutex>
#include <unordered_map>

#include "platform/model_import/IMeshCachePolicy.h"

// Author: Claude
// Description: weak_ptr 기반 캐시 정책. 마지막 shared_ptr 소유자가 사라지는 순간 자동으로 메모리가
//              해제된다 - 캐시 자체는 Model의 생사를 좌우하지 않는다.
// Input: (해당 없음 - 상태는 내부 캐시 맵)
// Output: TryGet - 살아있는 캐시 항목의 shared_ptr(없으면 nullptr)
// Notes: 같은 키를 반복 로드/해제(예: 엔티티 스폰/디스폰 반복)하면 매번 재파싱된다는 트레이드오프가
//        있다 - 이걸 완화하려면 LruCachePolicy를 대신 선택한다(둘을 동시에 쓰지 않는다).
// Date: 2026-07-23
class RefCountingCachePolicy : public IMeshCachePolicy
{
public:
    std::shared_ptr<const Model> TryGet(const std::string& key) override;
    void Store(const std::string& key, std::shared_ptr<const Model> model) override;
    void Unload(const std::string& key) override;

private:
    std::mutex m_mutex;
    std::unordered_map<std::string, std::weak_ptr<const Model>> m_entries;
};
