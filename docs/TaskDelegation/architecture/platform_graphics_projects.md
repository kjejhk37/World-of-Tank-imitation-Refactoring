# platform / graphics / projects 계층 정의

`docs/strategy/src구조재편_20260806_2232.md`에서 확정된 내용을 영구 참조용으로 옮겨 적은 문서.
Phase B(각 계층의 git submodule 추출) 이후 platform/graphics/projects가 별도 저장소로 갈라져도,
"이 기능/타입이 어느 저장소로 가야 하는가"를 판단하는 공통 기준으로 쓴다.
task/brainstorming/strategy/commit 문서와 달리 사이클 종료 후에도 archive하지 않고 이 위치에 영구 보관한다
(`docs/algorithm/`과 동일한 취급).

---

## 핵심 질문

**"이 타입/기능의 모양(shape)을 누가 결정하는가?"**

| 답 | 소속 |
|---|---|
| 누구든 재사용 가능한 일반 원칙 (수학, 알고리즘, 동시성 메커니즘, 파일 포맷) | **platform** |
| 특정 그래픽스 API/렌더링 파이프라인의 계약(인터페이스, 그 인터페이스가 주고받는 데이터 모양) | **graphics** |
| 이 프로젝트(WOT)의 콘텐츠·규칙·의사결정 | **projects** |

---

## platform

검증된, 완전히 추상화된 최소 단위 기능/데이터 라이브러리.
주요 키워드: **최소단위 기능 / 추상화 / 검증**.

- graphics/projects 둘 다 몰라야 한다 (어떤 헤더도 include하지 않는다). `platform_lib`는 어떤 상위 타겟도 링크하지 않는 것으로 강제한다.
- 데이터 타입이라도, "특정 타입이 primitive해 보인다"는 것만으로는 platform 소속 근거가 안 된다 — 그 타입이 존재하는 이유가 특정 소비자(예: `IRenderer`)의 계약을 만족시키기 위해서라면, 필드가 아무리 단순해도 그 소비자 쪽 소속이다.
- `T`에 대해 완전히 무지한 제네릭 메커니즘(예: `DoubleBufferPublisher<T>`)은 `T`가 나중에 무엇으로 쓰이든 항상 platform.
- **메커니즘과 스키마 인스턴스를 구분한다.** 같은 폴더에 "범용 메커니즘"과 "그 메커니즘이 다루는 구체적 데이터"가 같이 있으면, 메커니즘만 platform으로 가고 데이터는 projects로 남는다.

## graphics

platform을 사용해 DirectX/OpenGL 등 특정 그래픽스 API를 캡슐화하는 계층. Unity/Unreal 같은 "엔진"에 해당.
주요 키워드: **시각화(View) / engine 프로세스**.

- projects를 몰라야 한다. platform에는 의존해도 된다.
- 자신의 공개 인터페이스(`IRenderer` 등)가 주고받는 데이터 타입의 모양은 graphics 자신이 정의하고 소유한다 — 그 타입이 지금 순수 데이터처럼 보여도, 모양을 결정하는 주체가 렌더링 파이프라인이면 graphics 소속.
- WOT 고유의 개념(탱크, 맵 이름 등)은 여기 있으면 안 된다 — graphics는 이 프로젝트가 아닌 다른 프로젝트에서도 재사용 가능해야 한다.
- **"engine 프로세스" 키워드가 곧 "메인 루프를 graphics가 가져간다"는 뜻은 아니다.** 병렬 producer/consumer 구조(engine/renderer 분리)를 쓸지, 어떻게 루프를 구성할지는 projects의 선택이다 — 간단한 프로젝트는 그 구조 자체가 필요 없을 수 있다. graphics는 `RenderFrame(snapshot)`처럼 프레임 단위 계약만 제공하고 루프 오케스트레이션에 대해 아무 가정도 하지 않는다.

## projects

platform과 graphics를 모두 사용해 이번 프로젝트(WOT)의 실제 동작/콘텐츠를 구현하는 최하위 계층.
Unity/Unreal(graphics)로 실제 게임(WOT)을 만드는 것에 해당.

- platform, graphics 어느 쪽도 projects에 의존해서는 안 된다(역방향 금지).
- 지금은 데모/자리 표시자로 보이는 코드(예: `InstanceUpdateWorker`)라도, 나중에 실제 게임플레이 시스템(물리/애니메이션)으로 교체될 성격이면 지금부터 projects 소속 — 나중에 다시 옮기는 왕복 비용을 피한다.
- main.cpp의 루프 오케스트레이션(창 생성 → 렌더러 초기화 → 메시지 펌프 → RenderFrame 호출)은 projects 소속 — graphics로 추출하지 않는다.

---

## 실제 판단 사례

### 사례 1 — `InstanceSnapshot` (engine/renderer 경계)

`IFrameDataPublisher<T>`/`DoubleBufferPublisher<T>`는 `T`에 대해 완전히 무지한 제네릭 메커니즘이라 **platform**.
반면 `InstanceSnapshot`은 `IRenderer::RenderFrame(const InstanceSnapshot&)`의 파라미터 타입 — renderer 자신의 "프레임 입력 계약"이므로, 필드가 `Matrix4x4` 배열뿐인 순수 데이터처럼 보여도 **graphics** 소속.
`InstanceUpdateWorker`(그 계약을 채우는 WOT 데모 시뮬레이션 producer, 추후 실제 물리/애니메이션으로 교체 예정)는 **projects**.

### 사례 2 — `AppConfig` / `PlayerProgress` (메커니즘 vs 스키마)

`ConfigManager`, `LaunchConfigParser`, `LaunchConfigStore`(범용 JSON 설정 로딩 메커니즘)와 `SaveLoadManager`, `ISaveable`, `DataRecord`(범용 저장/복원 메커니즘)는 **platform**.
반면 `AppConfig`(창 크기·타이틀 등 WOT 설정 스키마)와 `PlayerProgress`(재화·해금한 탱크 ID를 담은 WOT 세이브 스키마)는 필드 내용 자체가 WOT 고유 콘텐츠라 **projects**.
"renderer/config를 include하는가"라는 의존성 검사만으로는 이 구분이 안 걸러진다 — 필드 내용까지 봐야 한다.
