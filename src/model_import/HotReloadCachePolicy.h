#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>

#include "model_import/IFileChangeDetector.h"
#include "model_import/IMeshCachePolicy.h"

// Author: Claude
// Description: 원본 파일이 바뀌면 캐시를 무효화해 재로드를 유도하는 캐시 정책.
// Input: 생성자 - IFileChangeDetector 구현체(소유권 이전)
// Output: TryGet - 파일이 안 바뀌었으면 캐시된 Model, 바뀌었으면 nullptr(강제 캐시미스)
// Notes: TryGet이 변경을 감지하면 그 자리에서 캐시를 지우고 nullptr을 반환한다 - 실제 재파싱은
//        이 클래스가 하지 않고, MeshManager::GetOrLoad가 캐시미스를 보고 loader를 다시 호출해
//        Store로 이어지는 흐름에 그대로 올라탄다(별도 재로드 로직 중복 없음).
//        엔트리를 weak_ptr이 아니라 shared_ptr로 강하게 유지한다 - 핫리로드 대상은 변경 감지가
//        의미 있으려면 계속 살아있어야 하므로 RefCountingCachePolicy와는 다른 선택이다.
// Date: 2026-07-23
class HotReloadCachePolicy : public IMeshCachePolicy
{
public:
    explicit HotReloadCachePolicy(std::unique_ptr<IFileChangeDetector> detector);

    std::shared_ptr<const Model> TryGet(const std::string& key) override;
    void Store(const std::string& key, std::shared_ptr<const Model> model) override;
    void Unload(const std::string& key) override;

private:
    std::unique_ptr<IFileChangeDetector> m_detector;
    std::mutex m_mutex;
    std::unordered_map<std::string, std::shared_ptr<const Model>> m_entries;
};
