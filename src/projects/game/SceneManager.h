#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "projects/game/Scene.h"

// Author: Claude
// Description: 씬을 key 문자열로 등록(Create)하고, 활성화(Add)/비활성화(Remove)하며, 활성 씬들의
//              Update를 매 프레임 위임하는 관리자.
// Input: (해당 없음 — 각 메서드 참고)
// Output: (해당 없음 — 각 메서드 참고)
// Notes: 원본 WOT-master의 SceneManager는 Singleton<SceneManager>였지만, 이 프로젝트는 main.cpp가
//        보여주듯 전역 싱글턴 대신 명시적 조립(DI) 스타일을 쓰므로 이 클래스는 싱글턴이 아니다 —
//        호출자(GameManager)가 인스턴스를 직접 소유한다.
// Date: 2026-08-11
class SceneManager
{
public:
    void Create(const std::string& key, std::unique_ptr<Scene> scene);
    void Add(const std::string& key);
    void Remove(const std::string& key);

    void Update(float deltaTime);

    const std::vector<Scene*>& GetActiveScenes() const;

private:
    std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
    std::vector<Scene*> m_activeScenes;
};
