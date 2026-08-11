#pragma once

#include "graphics/renderer/InstanceSnapshot.h"
#include "projects/game/SceneManager.h"

// Author: Claude
// Description: SceneManager가 관리하는 활성 씬들의 TransformComponent를 모아 렌더 스레드가 소비할
//              InstanceSnapshot으로 변환한다.
// Input: sceneManager — 순회 대상 / outSnapshot — 채워질 스냅샷(기존 내용은 지워짐)
// Output: (해당 없음 — outSnapshot을 통해 결과 반환)
// Notes: GameManager::Update()(씬 갱신)와 책임을 분리했다(SRP) — GameManager는 InstanceSnapshot을
//        몰라도 되고, 이 클래스는 씬 갱신 로직을 몰라도 된다. TransformComponent가 없는 Entity는
//        건너뛴다(이번 사이클엔 모든 Entity가 하나씩 갖지만, 향후 다른 Component만 가진 Entity가
//        생겨도 안전하도록).
// Date: 2026-08-11
class SceneSnapshotBuilder
{
public:
    static void Build(const SceneManager& sceneManager, InstanceSnapshot& outSnapshot);
};
