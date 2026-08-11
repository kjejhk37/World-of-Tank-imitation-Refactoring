#pragma once

#include <cstddef>

#include "projects/game/SceneManager.h"

// Author: Claude
// Description: 이 프로젝트의 씬 구성을 아는 조립 지점(composition root). SceneManager를 소유하고,
//              생성 시 PrototypeScene을 등록·활성화한다. 원본 WOT-master의 GameManager와 달리
//              자기 스스로 루프를 돌지 않는다 — 매 프레임 Update() 호출은 InstanceUpdateWorker(engine
//              스레드)가 담당한다.
// Input: 생성자 - instanceCount(PrototypeScene에 전달할 Entity 개수)
// Output: (해당 없음 — GetSceneManager()로 조회)
// Notes: SceneManager/Scene은 InstanceSnapshot을 모르므로, 프레임 데이터를 실제로 만드는 건
//        SceneSnapshotBuilder(별도 클래스, SRP)다 — 이 클래스는 씬 갱신만 책임진다.
// Date: 2026-08-11
class GameManager
{
public:
    explicit GameManager(std::size_t instanceCount);

    void Update(float deltaTime);

    const SceneManager& GetSceneManager() const;

private:
    SceneManager m_sceneManager;
};
