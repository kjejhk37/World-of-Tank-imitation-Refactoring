#pragma once

#include <cstddef>

#include "projects/game/Scene.h"

// Author: Claude
// Description: 씬 전환·생명주기 뼈대를 검증하기 위한 1차 활성 씬. WOT-master의 TestScene(원본 GameManager가
//              "Start" 키로 등록해 실제로 활성화했던 씬)의 후신이지만, 모델/충돌 디테일은 옮기지 않고
//              기존 InstanceUpdateWorker가 만들던 격자 배치+상하 bob+회전 데모를 Scene/Entity 경유로
//              재현한다 — 로직이 Scene으로 옮겨져도 동작이 그대로인지 회귀로 검증하는 게 목적이다.
// Input: 생성자 - instanceCount(생성할 Entity 개수)
// Output: (해당 없음 — GetEntities()로 조회)
// Notes: Entity 생성은 생성자가 아니라 Start()에서 한다 — Scene의 생명주기 훅(Start가 활성화 시점에
//        호출됨)이 실제로 의미 있게 동작하는지 검증하기 위함이다. Tank 모델/충돌은 다음 사이클.
// Date: 2026-08-11
class PrototypeScene : public Scene
{
public:
    explicit PrototypeScene(std::size_t instanceCount);

    void Start() override;
    void Update(float deltaTime) override;

private:
    std::size_t m_instanceCount;
    float m_elapsedSeconds = 0.0f;
};
