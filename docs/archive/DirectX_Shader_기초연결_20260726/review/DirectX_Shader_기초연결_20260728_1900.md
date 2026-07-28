../strategy/DirectX_Shader_기초연결_20260726_1400.md

# Code Review: DirectX Shader 기초 연결

## Stage 1 — 품질 점검

체크리스트 1~11(셰이더 빌드 파이프라인, `IFrameDataPublisher`/`DoubleBufferPublisher`, `InstanceSnapshot`/`InstanceUpdateWorker`, `IRenderer` 시그니처 변경, DX9/11/12 Baseline·Instancing·Compute Demo, `main.cpp` 배선) 전부를 SOLID/라이브러리 격리/테스트/주석 규칙 기준으로 점검했다.

### 발견된 이슈

- [MEDIUM] `DirectX9/11/12Renderer` 각 클래스가 디바이스·스왑체인 수명주기 + Baseline 파이프라인 + Instancing 파이프라인을 전부 한 클래스 안에 갖고 있어 SRP가 다소 늘어났다(백엔드별 `.cpp`가 300줄 이상).
  - 이번 사이클 전략 문서가 "Baseline과 Instancing이 같은 렌더러 클래스 안에 공존"하는 구조를 명시적으로 채택했기 때문에 발생한 것으로, 버그는 아니다.
  - 지금 당장 고치려면 각 파이프라인(Baseline/Instancing)을 별도 소유 객체(예: `DirectX11BaselinePipeline`, `DirectX11InstancingPipeline`)로 뽑아내는 구조 변경이 필요한데, 이는 이번 사이클 전략 문서가 정한 범위를 넘어서는 새 작업이다.
  - **[해결 방안]**
    - 지금 고치지 않고, 다음 셰이더 고도화(라이팅/머티리얼) 사이클에서 파이프라인별 클래스 분리를 함께 검토하는 것을 제안한다.
- [LOW] 테스트 전용 접근자 `DebugReadBackInstanceBuffer()`가 `DirectX9/11/12Renderer`의 공개(public) API에 그대로 노출되어 있다.
  - 프로덕션 호출부가 전혀 없는 메서드가 클래스의 공개 인터페이스에 섞여 있어 ISP 관점에서 약간의 노이즈가 된다.
  - **[해결 방안]**
    - friend 테스트 픽스처나 별도 테스트 전용 헤더로 옮기는 방법이 있지만, 렌더러 3개 + 테스트 3개 파일을 다 건드려야 하는 구조 변경이라 비용 대비 효과가 크지 않다고 판단해 지금은 그대로 두는 것을 제안한다.
- [LOW] 카메라/투영 시스템이 없어 3개 백엔드의 Baseline/Instancing 파이프라인 모두 WVP/ViewProj 상수가 항상 Identity다.
  - 이번 사이클 Task/Brainstorming 단계에서 반복적으로 범위 밖으로 확정된 사항이며 버그가 아니다 — 참고용으로만 다시 기록한다.
  - **[해결 방안]**
    - 다음 사이클에서 카메라/뷰-투영 시스템을 별도 Task로 진행.
- [LOW] "인스턴싱된 큐브가 화면에 실제로 올바르게 보이는지"의 픽셀 단위 육안 확인을 수행하지 못했다.
  - Claude는 렌더링된 창을 직접 볼 수 없어, 크래시 없이 실행되는지(자동 확인 완료)까지만 검증했다.
  - **[해결 방안]**
    - 사용자가 편할 때 `main.exe --renderer=directx9`/`directx11`/`directx12`를 직접 실행해 육안으로 확인 권장.

### 즉시 해소함

- [RESOLVED] `main.cpp`가 `InstanceUpdateWorker::Start()` 내부 `std::thread` 생성 실패(`std::system_error`, OS 자원 고갈 등 드문 상황)를 처리하지 않고 있었다.
  - `try/catch` + `Log::Error(ErrorCode::EngineWorkerStartFailed, ...)` 추가로 처리 완료 — 실패해도 앱 전체가 죽지 않고, 렌더러는 이미 빈 스냅샷일 때 Baseline 단일 드로우로 폴백하므로 계속 동작한다.
  - `ErrorCode.h`에 4000번대(엔진) 카테고리 신설.
  - 수정 후 `main`/`tests` 재빌드 및 `ctest -C Debug` 281건 전부 통과 확인.

### 통과 확인된 항목

- **라이브러리 격리**: `<d3d9.h>`/`<d3d11.h>`/`<d3d12.h>`를 동시에 include하는 파일 없음(grep 확인). 셰이더 소스도 백엔드별 폴더로 분리.
- **단위 테스트**: 이번 사이클에서 구현된 모든 기능(트리플 버퍼 동시성, InstanceUpdateWorker 생애주기, ShaderBytecodeLoader, 3개 백엔드 Baseline/Instancing 스모크+데이터 검증, DX11/DX12 Compute Demo)에 대응하는 테스트 존재. 전체 281건 통과.
- **주석 규칙**: 신규 클래스/헤더에 Author/Description/Input/Output/Notes/Date 포함(기존 코드베이스 관례대로 클래스당 1개 블록, 개별 private 메서드는 기존 파일들과 동일하게 블록 없음).
- **DIP/OCP**: `IFrameDataPublisher<T>`/`DoubleBufferPublisher<T>`는 `Fork-Join`/큐 정책을 모른다. `DirectX*Renderer`는 `IFrameDataPublisher`/스레딩 정책을 모르고 `InstanceSnapshot`(순수 데이터)만 안다.

---

## Stage 2 — 다이어그램

### Class Diagram

```
┌────────────────────────┐        ┌──────────────────────────────┐
│  IFrameDataPublisher<T> │◄──────│  DoubleBufferPublisher<T>     │
│  (interface)             │ impl  │  - m_slots[3], m_writeIndex   │
│  + AcquireWriteSlot()    │       │  - m_spareState(atomic)       │
│  + Publish()             │       │  + AcquireWriteSlot/Publish/  │
│  + AcquireReadSnapshot() │       │    AcquireReadSnapshot        │
└────────────────────────┘        └──────────────────────────────┘
            ▲ uses
            │
┌───────────────────────────┐      ┌───────────────────┐
│  InstanceUpdateWorker      │─────▶│  InstanceSnapshot  │
│  - m_publisher              │      │  worldMatrices[]   │
│  - m_thread                 │      └───────────────────┘
│  + Start()/Stop()           │
│  + GetPublisher()           │
└───────────────────────────┘
            │ produces
            ▼
┌────────────────────────┐
│  IRenderer (interface)   │
│  + Initialize()           │
│  + RenderFrame(snapshot)  │
│  + OnResize()/Shutdown()  │
└────────────────────────┘
     ▲            ▲            ▲             ▲
     │ impl        │ impl        │ impl         │ impl
┌─────────┐  ┌──────────┐  ┌──────────┐  ┌───────────┐
│DirectX9  │  │DirectX11  │  │DirectX12  │  │OpenGL     │
│Renderer  │  │Renderer   │  │Renderer   │  │Renderer   │
│(Baseline+│  │(Baseline+ │  │(Baseline+ │  │(스텁)      │
│Instancing│  │Instancing │  │Instancing │  └───────────┘
│ 폴백 포함)│  │)          │  │)          │
└─────────┘  └──────────┘  └──────────┘
                    │              │
                    │ 독립(연결 안 함) │ 독립(연결 안 함)
                    ▼              ▼
            ┌───────────────┐ ┌───────────────┐
            │DirectX11        │ │DirectX12        │
            │ComputeDemo      │ │ComputeDemo      │
            └───────────────┘ └───────────────┘

┌────────────────────────┐
│  ShaderBytecodeLoader    │  (네임스페이스 함수, 백엔드 공용)
│  + Load(csoPath)          │
└────────────────────────┘
     ▲ used by (전부)
DirectX9/11/12Renderer, DirectX11/12ComputeDemo
```

### Sequence Diagram — 한 프레임(인스턴싱 경로, DX11 기준)

```
main()          InstanceUpdateWorker   DoubleBufferPublisher   DirectX11Renderer      GPU(D3D11)
  │                     │                      │                      │                  │
  │  Start()            │                      │                      │                  │
  │────────────────────▶│ (백그라운드 스레드 시작)│                      │                  │
  │                     │  AcquireWriteSlot()  │                      │                  │
  │                     │─────────────────────▶│                      │                  │
  │                     │  (worldMatrices 채움) │                      │                  │
  │                     │  Publish()           │                      │                  │
  │                     │─────────────────────▶│ (spare 슬롯 교환)      │                  │
  │                     │        ... 반복(16ms 간격, Render와 무관) ...  │                  │
  │                     │                      │                      │                  │
  │  RenderFrame(       │                      │                      │                  │
  │   worker.GetPublisher()                    │                      │                  │
  │    .AcquireReadSnapshot())                 │                      │                  │
  │─────────────────────────────────────────────────────────────────▶│                  │
  │                     │                      │  AcquireReadSnapshot()│                  │
  │                     │                      │◀─────────────────────│                  │
  │                     │                      │ (필요시 swap-in)       │                  │
  │                     │                      │─────────────────────▶│                  │
  │                     │                      │                      │  Map 인스턴스 버퍼 │
  │                     │                      │                      │─────────────────▶│
  │                     │                      │                      │  DrawIndexedInstanced│
  │                     │                      │                      │─────────────────▶│
  │                     │                      │                      │◀─────────────────│
  │◀─────────────────────────────────────────────────────────────────│                  │
  │  (다음 프레임 반복, Engine과 Render 서로 안 기다림)                    │                  │
  │  Stop()             │                      │                      │                  │
  │────────────────────▶│ (join, 스레드 정지)   │                      │                  │
```
