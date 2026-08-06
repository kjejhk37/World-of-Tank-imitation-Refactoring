#pragma once

#include <memory>
#include <string>

#include "platform/model_import/Model.h"

// Author: Claude
// Description: MeshManager가 Model을 어떻게/언제까지 캐싱할지 결정하는 전략 인터페이스.
// Input: (해당 없음 - 인터페이스)
// Output: (해당 없음 - 인터페이스)
// Notes: 구현체(RefCountingCachePolicy/LruCachePolicy/HotReloadCachePolicy)는 서로 배타적으로
//        하나만 선택해 MeshManager에 주입한다(동시 조합 없음) - weak_ptr 기반 자동 해제와 LRU의
//        강제 보존은 근본적으로 상충하는 전략이라 하나의 정책으로 묶지 않는다
//        (docs/brainstorming/모델임포터_Mesh저장_20260723_2123.md 참고).
//        Unload()는 어떤 정책을 쓰든 항상 제공되는 명시적 해제 수단("2중 안전장치")이다.
// Date: 2026-07-23
class IMeshCachePolicy
{
public:
    virtual ~IMeshCachePolicy() = default;

    virtual std::shared_ptr<const Model> TryGet(const std::string& key) = 0;
    virtual void Store(const std::string& key, std::shared_ptr<const Model> model) = 0;
    virtual void Unload(const std::string& key) = 0;
};
