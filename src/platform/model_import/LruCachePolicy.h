#pragma once

#include <list>
#include <mutex>
#include <unordered_map>
#include <utility>

#include "platform/model_import/IMeshCachePolicy.h"

// Author: Claude
// Description: 최근 사용된 N개(생성자 maxEntries)를 shared_ptr로 강하게 붙잡아두는 캐시 정책.
//              아무도 안 쓰는 Model도 잠깐 살아있어 재스폰 시 재파싱을 피할 수 있다.
// Input: 생성자 - maxEntries(강하게 유지할 최대 엔트리 수)
// Output: TryGet - 캐시 항목의 shared_ptr(없으면 nullptr)
// Notes: 내부에 두 자료구조를 같이 둔다 - weak_ptr 기반 registry(정체성 조회, 모든 키 대상)와
//        최근 사용 순서로 정렬된 shared_ptr 목록(LRU, maxEntries개까지만 강하게 유지).
//        LRU에서 밀려나도 외부에서 아직 참조 중이면 registry의 weak_ptr을 통해 계속 조회 가능하다 -
//        완전히 사라지는 건 모든 외부 참조도 끊겼을 때뿐이다.
// Date: 2026-07-23
class LruCachePolicy : public IMeshCachePolicy
{
public:
    explicit LruCachePolicy(size_t maxEntries);

    std::shared_ptr<const Model> TryGet(const std::string& key) override;
    void Store(const std::string& key, std::shared_ptr<const Model> model) override;
    void Unload(const std::string& key) override;

private:
    using RecencyList = std::list<std::pair<std::string, std::shared_ptr<const Model>>>;

    void Touch(const std::string& key, const std::shared_ptr<const Model>& model);

    size_t m_maxEntries;
    std::mutex m_mutex;
    std::unordered_map<std::string, std::weak_ptr<const Model>> m_registry;
    RecencyList m_recency;
    std::unordered_map<std::string, RecencyList::iterator> m_recencyIndex;
};
