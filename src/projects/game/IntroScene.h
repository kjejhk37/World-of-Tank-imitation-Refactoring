#pragma once

#include "projects/game/Scene.h"

// Author: Claude
// Description: 게임 시작 화면(인트로)을 나타내는 씬. GameManager/SceneManager가 engine 스레드에서
//              돌리는 Scene 생명주기(Start/Update/End) 등록 대상 자리를 채우기 위한 의도적으로
//              빈 뼈대다.
// Input: (해당 없음)
// Output: (해당 없음)
// Notes: WOT-master를 더 이상 참고하지 않기로 하면서(TestScene 폐기 결정) PrototypeScene을
//        대체한다. 2D UI 위젯(Button/Progress/StringField/Label/Image)은 이 클래스에 두지 않는다 -
//        SetUiElementRegistry로 등록된 registry가 렌더 스레드(DirectX*Renderer::RenderFrame() 내부)
//        에서 RenderAll()로 호출되는데, IntroScene은 engine 스레드에서 살아 위젯/registry를 여기
//        두면 두 스레드가 동시에 같은 ImGui 위젯 객체를 건드리는 데이터 레이스가 생긴다(ImGui는
//        스레드 세이프하지 않고, 3D InstanceSnapshot과 달리 UI 위젯엔 DoubleBufferPublisher 같은
//        안전한 스레드 간 전달 장치가 없다). 그래서 위젯 5종 + UiElementRegistry는 렌더 스레드인
//        main.cpp가 직접 소유한다(docs/strategy/게임_기본기능_채우기_20260819_1928.md "설계 변경"
//        섹션 참고) - 앞으로도 계속 이 상태로 남을 수 있다(엔진 스레드 로직이 실제로 필요해지기
//        전까지).
// Date: 2026-08-19
class IntroScene : public Scene
{
public:
    void Update(float deltaTime) override;
};
