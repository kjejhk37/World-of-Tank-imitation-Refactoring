#pragma once

#include <memory>

#include "model_import/ModelMesh.h"

// Author: Claude
// Description: 서로 다른 Model(파일)에서 온 Mesh가 기하학적으로 합동(같은 지오메트리를 회전/이동/
//              균등 스케일만 다르게 배치한 것)인지 판단해, 합동이면 기존 핸들을 재사용하게 하는 인터페이스.
// Input: (해당 없음 - 인터페이스)
// Output: (해당 없음 - 인터페이스)
// Notes: 같은 파일 내부의 씬 그래프 인스턴싱(예: 바퀴 6개)은 이 인터페이스 없이 파싱 단계에서
//        이미 해결된다 - 이 인터페이스는 오직 "서로 다른 파일" 간의 우연한 중복만을 다룬다
//        (docs/brainstorming/모델임포터_Mesh저장_20260723_2123.md 참고).
//        이 기능의 실제 가치는 메모리 절감보다 향후 별도 "mesh 전처리" 프로젝트/submodule이
//        더 정교한 구현으로 교체할 수 있도록 인터페이스 경계를 지금 확정해두는 데 있다.
// Date: 2026-07-23
class IMeshDeduplicator
{
public:
    virtual ~IMeshDeduplicator() = default;

    // candidate와 합동인 기존 Mesh가 등록되어 있으면 그 핸들을 반환하고, 없으면 candidate 자신을
    // 등록한 뒤 그대로 반환한다. find와 register를 하나로 묶어 동시성 문제를 피한다.
    virtual std::shared_ptr<const ModelMesh> FindOrRegister(std::shared_ptr<const ModelMesh> candidate) = 0;
};
