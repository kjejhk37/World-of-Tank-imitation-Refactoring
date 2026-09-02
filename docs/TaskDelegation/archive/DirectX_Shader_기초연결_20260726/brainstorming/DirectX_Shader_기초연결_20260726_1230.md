../task/DirectX_Shader_기초연결_20260726_1200.md

# Brainstorming: DirectX Shader 기초 연결

## Details

### 1. 대상 DirectX 백엔드 범위

`DirectX9Renderer`/`DirectX11Renderer`/`DirectX12Renderer` 세 백엔드 모두 대상으로 진행한다.

- Vertex/Pixel(baseline) Shader와 Instancing Shader는 Shader Model 2.0/3.0 문법으로 DX9에서도 작성 가능하다 — 언어/파이프라인 스테이지 자체에는 문제가 없다.
- DX9의 하드웨어 인스턴싱은 Direct3D 9 API의 정식 기능이 아니라 `D3DSTREAMSOURCE_INDEXEDDATA` + 특수 텍스처 포맷("INST" 트릭) 같은 드라이버 확장에 의존한다. 즉 언어/구조 문제가 아니라 "드라이버가 이 확장을 지원하는가"의 문제 — 대부분의 실제 GPU 드라이버는 지원하지만 100% 보장은 아니라서, 미지원 환경을 위한 비인스턴싱 폴백 경로를 함께 마련한다(best-effort).
- **Compute Shader만 DX9에서 제외한다.** 이는 드라이버 지원 여부의 문제가 아니라, Direct3D 9 API 자체에 Compute 파이프라인 스테이지(`Dispatch`, UAV, `RWStructuredBuffer` 등)가 존재하지 않기 때문이다 — 즉, 우회할 수 없는 하드 API 제약이다. Compute Shader는 DX11(feature level 11_0+)/DX12에서만 구현한다.

### 2. 멀티스레드 아키텍처 — 비동기 스냅샷(Producer-Consumer) 모델, join 없음

`Fork-Join`(매 프레임 블로킹 대기)으로 정정했던 것은 사용자 의도와 달랐다. 실제로 의도한 구조는 다음과 같다.

- **Engine 스레드**: Render와 무관하게 **자기 속도로 계속 순환**하며 데이터를 계산한다(물리/애니메이션/인스턴스 행렬 등). Render를 기다리지 않는다.
- **Render/메인 스레드**: 매 프레임 **자기 속도로 진행**하며, Engine이 "이번 프레임에 맞춰 끝났는지"는 신경 쓰지 않고, 그 순간 확보 가능한 **가장 최근에 완료된 상태**만 그대로 읽어 GPU에 올린다(뷰어처럼 진행 상태를 캡처).
- 즉 **둘 중 누구도 서로를 기다리며 블로킹하지 않는다** — Engine이 Render보다 느리면 Render는 같은 스냅샷을 여러 프레임 재사용하고, Engine이 더 빠르면 중간 결과 몇 개는 그냥 건너뛰어진다.

```
Engine Thread   : ──[tick]──●────────[tick]──●─────[tick]───────────●────────▶
                            │(commit)        │(commit)              │(commit)
                            ▼                ▼                      ▼
                         상태 A            상태 B                 상태 C
                 (Render를 기다리지 않고 자기 속도로 계속 진행 — join 없음)

Main/Render     : ──[Frame1]────[Frame2]────[Frame3]────[Frame4]────[Frame5]──▶
                      │ read A     │ read A     │ read B    │ read C    │ read C
                      ▼            ▼            ▼           ▼           ▼
                    Draw         Draw         Draw        Draw        Draw
        (Engine을 기다리지 않고, 매 프레임 "그 순간의 최신 완료본"만 그대로 사용 — join 없음)
```

- 이 모델에서는 join이 없으므로, "Engine이 쓰다 만 상태(torn read)"를 Render가 보지 않도록 막아줄 안전장치가 다시 필요하다 — 여기서 **더블 버퍼링(또는 원자적 포인터/인덱스 스왑)**이 정확히 이 역할을 한다: Engine은 back buffer에 쓰다가 완료되면 원자적으로 front/back을 교체(publish)하고, Render는 항상 현재 front만 읽는다. 락 없이도 Render가 절대 "쓰다 만" 데이터를 보지 않게 된다.
- 처음에 제안했던 더블 버퍼링이 맞는 방향이었다 — Fork-Join으로 정정하며 "필요 없다"고 한 부분이 잘못이었다.
- Compute Shader 데모(`RWStructuredBuffer` 변환)는 이번 사이클엔 렌더 루프와 독립된 최소 데모(단위 테스트)로 검증하므로, 이 비동기 스냅샷 모델에 바로 올라타지는 않는다 — 실제 연동 시점(다음 사이클)에 같은 발행/구독 방식을 재사용할 수 있다.

#### Unity/Unreal과의 비교 — 어느 쪽이 이 모델과 같은가

**① 이번 사이클 모델 — Engine과 Render가 서로 기다리지 않는 비동기 스냅샷**

(위 다이어그램 참고)

**② Unity Job System(일반적인 `Schedule()`+`Complete()` 사용 패턴) — 이것과는 다른 방식(Fork-Join, 블로킹)**

```
시간 ─────────────────────────────────────────────────────▶

Main Thread     : ██ Schedule ██──(다른 작업)──██ Complete() ██ Render/Culling ██
Job Worker 1    :      ░░░░ Job A ░░░░
Job Worker 2    :      ░░░░░░░░ Job B ░░░░░░░░
                                                  ▲
                                    Complete()는 블로킹 — 여기서는 참고용 대조 사례일 뿐,
                                    이번 모델(서로 안 기다림)과는 다른 방식이다.
```

**③ Unreal Game/Render/RHI Thread — 가장 가까운 실제 사례(서로 기다리지 않음)**

```
                 Frame N                Frame N+1              Frame N+2
Game Thread   : ██ Tick(N) ██         ██ Tick(N+1) ██        ██ Tick(N+2) ██
                       │ Render Command 큐에 push(대기 없이 바로 다음 프레임 진행)
                       ▼
Render Thread :        ██ Draw(N) ██         ██ Draw(N+1) ██        ██ Draw(N+2) ██
                              │ (실제 GPU 호출만 위임, 역시 대기 없음)
                              ▼
RHI Thread    :               ██ Submit(N) ██     ██ Submit(N+1) ██ ...

→ Render Thread는 Game Thread를 기다리지 않고 큐에 쌓인 걸 자기 속도로 소비한다
  (결과적으로 한 프레임 정도 뒤처진 데이터를 그림 — 우리 모델의 "최신 완료본만 읽는다"와 원리적으로 동일).
```

- **결론**: 이번 사이클 모델은 Unity Job System의 일반적 사용 패턴(Fork-Join, 블로킹)보다 **Unreal의 Game Thread/Render Thread 관계에 원리적으로 훨씬 가깝다** — 둘 다 "생산자(Engine/Game)는 소비자(Render)를 기다리지 않고 계속 진행하고, 소비자는 그 시점에 확보 가능한 가장 최근 결과만 사용한다"는 비동기 발행/구독 구조다. 차이는 Unreal이 "커맨드 큐"(여러 개를 순서대로 쌓아둠)를 쓰고 우리는 "최신 상태 하나만 덮어쓰는 더블 버퍼"를 쓴다는 정도 — 인스턴스 행렬처럼 "최신 값만 있으면 되는" 데이터엔 더블 버퍼가 더 단순하고 적합하다.

#### 정책을 교체 가능하게 만드는 seam — `IFrameDataPublisher<T>`

향후 Unity식(Fork-Join/블로킹)이나 Unreal식(순차 큐) 정책으로 바꾸고 싶어질 걸 대비해, "엔진 개발자가 데이터를 계산하는 공간"과 "그 데이터가 Render로 넘어가는 동기화 방식"을 인터페이스로 분리한다.

```
IFrameDataPublisher<T>            // Engine과 Render 둘 다 이 인터페이스만 안다
    T&       AcquireWriteSlot()   // producer(Engine)가 쓸 자리를 받는다
    void     Publish()            // 다 썼다고 알린다(구현체별로 동작이 다름)
    const T& AcquireReadSnapshot()// consumer(Render)가 최신 데이터를 받는다

구현체:
  - DoubleBufferPublisher<T>  — 이번 사이클에 실제로 구현 (더블 버퍼, join 없음, AcquireReadSnapshot은 즉시 반환)
  - (미구현, 향후 필요 시) BlockingJoinPublisher<T> — Fork-Join, AcquireReadSnapshot이 내부적으로 대기 후 반환
  - (미구현, 향후 필요 시) QueuePublisher<T>        — Unreal식 순차 큐 + 병합(coalesce) 정책
```

- Engine 쪽 계산 로직과 `DirectX*Renderer::RenderFrame()` 둘 다 `IFrameDataPublisher<T>`(구체적으로는 `IFrameDataPublisher<InstanceSnapshot>`)에만 의존한다 — 어떤 정책이 꽂혀 있는지 모른 채로 동작한다(DIP).
- 이번 사이클은 실제로 필요한 `DoubleBufferPublisher<T>` 구현체 **하나만** 만든다. `BlockingJoinPublisher`/`QueuePublisher`는 실제로 그 정책이 필요한 서브시스템(예: 일회성 이벤트 큐가 필요해지는 시점)이 생기기 전까지 만들지 않는다 — "가상의 미래 요구사항을 위해 설계하지 않는다" 원칙에 따름. 인터페이스만 있으면 나중에 구현체를 추가하는 건 기존 코드를 안 건드리고 가능하다(OCP).
- 완전히 "정책 교체 비용 0"은 아니다 — 예를 들어 큐 정책은 연속 갱신 데이터에 병합(coalesce) 로직이 추가로 필요해, 인터페이스가 그 개념까지 다 흡수하지는 못할 수 있다. 다만 "계산 로직/렌더링 코드를 다시 쓰지 않고 정책만 교체"하는 목표는 충분히 달성된다.

### 3. HLSL 컴파일 방식 — 구조적 차이 설명

"구조가 많이 다른가?"에 대한 답: **셰이더 소스 코드나 렌더러 클래스 설계에는 차이가 없다.** 차이는 딱 하나 — "언제, 어떤 코드가 컴파일을 수행하느냐"뿐이다.

| 구분 | 런타임 컴파일 (`D3DCompile`) | 오프라인 컴파일 (`dxc`/`fxc`) |
|---|---|---|
| `.hlsl` 소스 | 동일 | 동일 |
| 컴파일 시점 | 앱 실행 중, `D3DCompile()` 호출 | 빌드 시점, CMake 커스텀 빌드 스텝 |
| 런타임 코드 | 컴파일 함수 호출 + 에러 처리 필요 | `.cso` 바이너리 파일 읽기만 하면 됨 |
| `CreateVertexShader` 등에 넘기는 것 | 컴파일 결과 blob (동일) | 미리 컴파일된 blob (동일) |
| 배포 의존성 | `D3DCompiler_47.dll` 필요 | 없음 |
| 문법 오류 발견 시점 | 실행 중(런타임 에러) | 빌드 중(컴파일 에러) |

- 결과적으로 `CreateVertexShader`/`CreatePixelShader`/`CreateComputeShader`에 바이트코드 blob을 넘기는 지점은 완전히 동일 — 오프라인이든 런타임이든 렌더러 쪽 코드는 "블롭을 어디서 받아오는지"만 다르다(내부 `ShaderBytecodeLoader` wrapper가 그 차이를 감춘다).
- 배포 목표(패키징된 `.exe`, 컴파일러 DLL 의존성 없음)에 부합하므로 **오프라인 컴파일로 확정**.

### 4. Compute Shader 데모 범위

- 이번 사이클은 렌더 루프와 완전히 독립된 최소 데모만 작성한다 — `RWStructuredBuffer`에 대한 간단한 변환(예: 값 2배) Compute Shader를 Dispatch하고 CPU로 리드백해 단위 테스트로 검증.
- 실제 `Model`/`ModelMesh` 정점 데이터와 어떻게 연동할지(GPU 스키닝, 파티클, 포스트프로세싱 등)는 이번 사이클에서 결정하지 않고 다음 사이클에서 별도로 논의한다.

## Trade-offs

- **DX9 포함(Compute 제외, Instancing best-effort)**
  - 장점: 3개 백엔드 간 기능 격차를 만들지 않음, 사용자가 명시한 "기술적 문제가 아니면 포함" 기준에 부합.
  - 단점: DX9 인스턴싱이 드라이버 미지원 환경에서는 폴백 경로(비인스턴싱 루프)까지 별도로 구현/테스트해야 해서 작업량이 늘어남.

- **비동기 스냅샷 모델(Engine 스레드 ↔ Render 스레드, 더블 버퍼링, join 없음)**
  - 장점: 사용자가 제시한 아키텍처를 그대로 반영, 어느 쪽도 서로를 기다리며 스톨되지 않음(Engine이 느려도 Render는 이전 스냅샷으로 계속 프레임을 그림), Unreal의 Game/Render Thread 관계와 원리적으로 동일해 실제 엔진 아키텍처와의 정합성이 높음.
  - 단점: Render가 항상 "약간 과거의" 상태를 그릴 수 있음(Engine의 최신 tick이 아직 커밋 전이면 이전 스냅샷 재사용) — 즉시 일관성이 아니라 최종 일관성(eventual consistency)을 받아들여야 함. 더블 버퍼 발행/구독(publish) 시점의 원자성(atomic swap)을 정확히 구현하지 않으면 torn read 버그가 생길 수 있어 설계/검증이 필요.

- **`IFrameDataPublisher<T>` seam 도입(정책 교체 가능한 구조)**
  - 장점: 계산 로직/렌더링 코드가 구체적인 동기화 정책(더블 버퍼/Fork-Join/큐)을 몰라도 되어(DIP), 향후 정책을 바꾸거나 추가할 때(OCP) 기존 코드를 다시 쓸 필요가 없음.
  - 단점: 인터페이스 설계 자체에 약간의 선행 비용이 들고, 정책마다 성격이 달라(예: 큐는 병합 로직 필요) 인터페이스가 모든 정책의 요구사항을 완벽히 흡수하지 못할 수 있음 — "정책 교체 비용 0"이 아니라 "크게 낮춤" 정도로 기대치를 잡아야 함.

- **오프라인 셰이더 컴파일**
  - 장점: 배포 시 컴파일러 DLL 불필요, 빌드 시점에 셰이더 문법 오류 발견.
  - 단점: CMake에 셰이더별 커스텀 빌드 스텝(entry point, target profile 등)을 등록해야 하는 초기 설정 비용, 셰이더 수정 후 재빌드 필요.

## Q&A Log

- Q: 백엔드에서 DX9을 제외하는 이유가 뭐야? 기술적 문제가 아니라면 가능하면 진행해줘.
  A: Compute Shader는 Direct3D 9 API 자체에 파이프라인 스테이지가 없어 100% 불가능한 하드 제약이라 DX9에서 제외. 반면 Vertex/Pixel(baseline)과 Instancing은 언어/파이프라인상 DX9에서도 가능(단, 인스턴싱은 드라이버 확장 기능이라 100% 보장은 아니고 best-effort + 폴백 필요) — 이에 따라 DX9도 대상에 포함하고 Compute Shader만 제외하는 것으로 확정.
- Q: 멀티스레드에 대해서는 엔진 연산부와 Shader 연산부를 나누는 것을 생각하고 있다 — 엔진 연산부는 멀티스레드로 데이터를 연산하고, Shader가 메인 스레드 역할을 하며 전체 진행 상태를 캡처하는 뷰어 느낌.
  A: "DX12 Command List를 여러 스레드로 분산 기록"하는 기존 Option A/B 안을 폐기하고, "엔진 연산부(워커 스레드) → Shader/메인 스레드(결과 소비 후 GPU 제출)" 구조로 확정.
- Q: 컴파일 방식(오프라인 vs 런타임)에 대해서는 구조가 많이 다른가?
  A: 셰이더 소스/렌더러 설계는 동일하고, 차이는 "컴파일을 언제 수행하는지"(빌드 타임 vs 런타임)와 배포 의존성뿐. 배포 목표에 맞춰 오프라인 컴파일로 확정(위 Details 3번 표 참고).
- Q: Compute Shader는 우선 최소 데모만 작성해달라, 실제 Model 연동 방식은 그다음에 정한다.
  A: 확정 — 이번 사이클은 독립 데모(`RWStructuredBuffer` 변환 + 단위 테스트)까지만.
- Q: Author 필드는?
  A: 생략.
- Q: (사용자) 확정된 내용, 특히 스냅샷 구조에 대해 이견은 없는지 — Unity/Unreal과 비교해서 평가해달라.
  A: 방향 자체는 두 엔진과 일치(엔진 연산부가 데이터를 만들고, 렌더링 담당 스레드가 그 결과를 소비하는 구조). 다만 "더블 버퍼링"이라는 표현은 부정확 — Unreal은 Render Command Queue + 파이프라이닝, Unity는 Job System 의존성 그래프로 구현하며, 단순 더블 버퍼 스왑은 워커가 하나뿐인 지금 범위엔 맞지만 이후 서브시스템이 늘어나면 한계가 있음을 지적.
- Q: (사용자) 사실은 "Engine Update(멀티스레드) → Render Update(단일 스레드)"를 말한 것 — 이 이해가 맞는지.
  A: (오답 정정 전) 이걸 "Fork-Join"으로 잘못 정정 — join이 순서를 보장하므로 더블 버퍼링이 필요 없다고 잘못 답함.
- Q: (사용자) 기본적으로 스레딩은 join을 안 쓰는 게 기본값 아닌가 — 스레드 종료 여부만 폴링해서 이벤트로 넘기는 방식이 기본값 아닌지, 아니면 Unreal이 이걸 시스템화한 것인지.
  A: join(블로킹 대기)은 naive 구현의 기본값이고, 논블로킹+이벤트는 프로덕션 스케줄러의 고도화라고 답함(이 답 자체는 유효하나, 아래에서 밝혀지듯 애초에 사용자가 원한 건 Fork-Join 계열이 아니었음).
- Q: (사용자) 그렇다면 내가 말한 게 엔진의 기본 타입이 맞고, Unreal/Unity는 이걸 고도화한 것이 맞는지 — Thread 사용 다이어그램을 아스키 아트로 문서에 추가해달라.
  A: (오답 정정 전) Fork-Join 기준으로 다이어그램 3종을 작성 — 이후 사용자가 이 다이어그램이 본인 의도와 다르다고 정정함.
- Q: (사용자) `Main Thread : ██ Fork ██──(join 대기)──██ Render Update ██` 다이어그램이 잘못됐다 — join으로 대기하면 Fork의 모든 스레드가 종료돼야 Render가 시작되는데, 그게 아니라 Engine Update는 따로(자기 속도로) 진행되고, Render Update는 Engine 스레드가 끝났는지 여부와 무관하게 매 프레임 "지금 상태"만 그대로 view에 올린다는 뜻이었다.
  A: 정정 — 이건 Fork-Join이 아니라 **비동기 스냅샷(Producer-Consumer) 모델**이었다(대화 맨 처음 제안했던 더블 버퍼링이 맞는 방향). Engine/Render 둘 다 서로 기다리지 않고 각자 속도로 진행하며, Render는 항상 "가장 최근 완료된 스냅샷"만 읽는다. 이 경우 join이 없으므로 torn read를 막을 더블 버퍼링(원자적 publish)이 다시 필요함 — Details 2번/Trade-offs 전면 정정.
- Q: 이 모델을 진행하면 나중에 어떤 문제가 생길 것 같은지?
  A: (1) 서브시스템이 늘어나면 producer별 스냅샷 시점이 어긋나는 문제(가장 유력), (2) 입력 반응성이 중요한 데이터가 실수로 이 경로를 타는 문제, (3) Engine 스레드가 멈춰도 조용히 정체되는 문제(워치독 필요), (4) "제자리 수정" 유혹이 락 안전성을 깨뜨리는 문제, (5) 인스턴스 개수 변화 시 버퍼 크기 불일치, (6) 비동기로 인한 테스트 재현성 저하. 1번/6번은 지금 설계에 반영, 나머지는 체크리스트에 가볍게 반영 제안.
- Q: 1번(producer가 여러 개면 시점이 어긋나는 문제)에서 producer가 "app의 프로젝트"를 말하는 거냐?
  A: producer는 앱(엔진) 내부의 개별 연산 서브시스템/스레드(예: 물리 스레드, 애니메이션 스레드, 인스턴싱 스레드) 하나하나를 가리키는 것이지, 외부 프로세스나 앱 전체가 아니다.
- Q: 각 producer(app)는 독립된 기능이니 그 걱정은 없어야 한다.
  A: 코드/모듈이 독립적인 것과, 화면에 그려질 때 결과가 합성되는지는 별개 — 예를 들어 전차의 차체(물리)+포탑(애니메이션)처럼 같은 렌더 대상에 합성되면 시점 어긋남이 시각적으로 드러난다(디싱크). 서로 합성되지 않는 대상끼리는 걱정 없음. → 이번 사이클은 producer가 인스턴싱 하나뿐이라 해당 없음, 두 번째 producer 추가 시 재검토.
- Q: (참고) `src/app`를 나중에 `projects`로 이름을 바꿀 생각이다.
  A: 향후 구조 개편 시점의 계획으로 기록(메모리에 별도 저장). 이번 사이클과 무관.
- Q: Render가 바로 그리지 않고 큐에 한 번 더 감싼 뒤 순차 적용하면 그게 Unreal 방식인지?
  A: 맞음 — 이게 오히려 "최신 값만 덮어쓰는 더블 버퍼"보다 Unreal의 Render Command Queue에 더 가까운 방식. 다만 Unreal도 무제한 큐가 아니라 fence로 깊이를 제한하는 파이프라인이고, 연속 갱신 데이터(인스턴스 행렬)엔 병합 로직 없이는 큐가 계속 자라거나 "느리게 뒤쫓는" 렌더링이 되므로, 이번 사이클엔 더블 버퍼 유지, 큐 패턴은 향후 일회성 이벤트(이펙트 스폰 등)에 적용 제안.
- Q: 구조를 바꾸는 데 어느 정도 수정이 일어나는지 확인해줄 수 있는지?
  A: 코드 확인 결과, `main.cpp`(Engine 스레드 시작/정지 추가)와 `IRenderer`+3개 렌더러(RenderFrame이 스냅샷을 읽도록 변경)가 가장 침습적인 부분이고, 더블 버퍼 클래스/Engine 스레드 클래스는 작고 격리된 신규 코드. 실제 작업량 대부분은 스레딩 모델과 무관하게 어차피 필요한 셰이더 파이프라인 구축(HLSL 4종, 버퍼 바인딩)이 차지함.
- Q: 엔진 시스템 하나를 만들고 그 안에 개발 공간을 두고 정책만 교체하면 Unity/Unreal/내 방식 전환 공수가 크지 않을 것 같은데 가능한지?
  A: 가능 — `IFrameDataPublisher<T>` 인터페이스로 "계산 로직"과 "동기화 정책"을 분리(Strategy 패턴 + DIP). 이번 사이클은 `DoubleBufferPublisher<T>`만 구현하고, Fork-Join/큐 구현체는 필요해질 때 추가(OCP) — 위 Details "정책을 교체 가능하게 만드는 seam" 절 참고. 완전히 비용 0은 아니지만(정책별로 세부 요구사항이 다를 수 있음), 계산/렌더링 코드를 다시 쓰지 않고 정책만 바꾸는 목표는 달성됨.

### Step 4(구현) 중 발견된 설계 결함과 정정 (2026-07-28)

- Q: (구현 도중 Claude가 발견) 슬롯 2개 + 원자적 인덱스 스왑안은 `InstanceSnapshot`처럼 `std::vector`를 담는 비-POD 타입에서 안전한가?
  A: 아니다 — Consumer가 slot 0을 읽는 도중 Producer가 slot 1을 publish하고 곧바로 다음 루프에서 slot 0(직전 front)에 다시 쓰기 시작하면, Consumer가 읽고 있는 slot 0을 Producer가 동시에 덮어쓰는 torn read가 발생할 수 있다(Engine이 Render보다 정확히 한 바퀴 더 돌면 바로 발생). 슬롯을 3개로 늘리고, "지금 아무도 안 쓰는 중간(spare) 슬롯" 인덱스를 producer/consumer가 원자적 `exchange`로 주고받는 락-프리 트리플 버퍼로 교체 제안 — public 이름(`DoubleBufferPublisher`)은 유지.
- Q: (사용자) spare 슬롯이 "Engine이 방금 발행했지만 Render가 아직 안 가져간 것"인지 "Render가 방금 다 읽고 반납한 것"인지 4가지로 정리해 확인 — (1) Engine 내에서 메인 업데이트를 진행, (2) 3개 슬롯 중 [Engine 연산 중·Render 연산 끝] / [Render 연산 중·Engine 연산 끝] / [Render 연산 후 대기 중이거나 Engine 연산 후 대기 중]의 세 상태로 접근을 확인, (3) 3개 슬롯 중 Render는 다음 프레임에 최신 슬롯을 사용, (4) 이를 통해 Render와 Engine이 멀티스레드 환경에서 같은 슬롯을 동시에 사용하는 것을 방지.
  A: 정확함 — 트리플 버퍼의 세 슬롯은 항상 (a) Engine이 현재 쓰고 있는 write 슬롯, (b) Render가 현재 읽고 있는 read 슬롯, (c) 어느 쪽도 지금 안 건드리는 spare 슬롯, 이렇게 서로 다른 역할로 나뉜다. spare 슬롯의 의미는 시점에 따라 두 국면을 번갈아 거친다 — Engine의 `Publish()` 직후부터 Render의 다음 swap-in 전까지는 "발행됐지만 아직 안 가져간 것"(new=true), Render의 swap-in 직후부터 Engine의 다음 `Publish()` 전까지는 "다 읽고 반납한 헌 것"(new=false). 두 표현은 서로 배타적인 상태가 아니라 같은 슬롯이 번갈아 거치는 두 국면이며, 알고리즘 자체가 write 슬롯과 read 슬롯이 항상 다른 두 슬롯이 되도록 보장하므로 Engine/Render가 같은 슬롯을 동시에 건드리는 경우가 구조적으로 없다. → 재승인 완료, `docs/strategy/DirectX_Shader_기초연결_20260726_1400.md` 체크리스트 3번의 트리플 버퍼 설계를 그대로 적용해 구현을 재개한다.

## Summary & Open Questions (사용자 결정 필요)

**현재까지 확정된 내용 (브레인스토밍 완료, 이견 없음 확인됨)**

- 대상 백엔드: DX9/DX11/DX12 모두 포함. Compute Shader만 DX9에서 제외(하드 API 제약). DX9 Instancing은 best-effort + 폴백.
- 멀티스레드 아키텍처: **비동기 스냅샷(Producer-Consumer) 모델, join 없음** — Engine 스레드는 Render와 무관하게 자기 속도로 계속 순환하며 데이터를 계산·커밋하고, Render/메인 스레드는 매 프레임 자기 속도로 진행하며 그 순간 확보 가능한 가장 최근 완료 스냅샷만 읽어 GPU에 올린다. Unreal Game/Render Thread 관계와 원리적으로 동일.
- 동기화 구현(2026-07-28 정정): 슬롯 2개 + 원자적 인덱스 스왑은 `std::vector`를 담는 비-POD 페이로드(`InstanceSnapshot`)에서 torn read 위험이 있어 폐기. **슬롯 3개짜리 락-프리 트리플 버퍼**(write 슬롯/read 슬롯/spare 슬롯을 producer·consumer가 원자적 `exchange`로 교환)로 교체 확정 — public 이름(`DoubleBufferPublisher`)은 유지. 자세한 내용은 위 "Step 4(구현) 중 발견된 설계 결함과 정정" 절 참고.
- **`IFrameDataPublisher<T>` seam 도입** — 계산 로직/렌더링 코드는 이 인터페이스에만 의존(DIP). 이번 사이클은 `DoubleBufferPublisher<T>` 구현체 하나만 만들고, Fork-Join/큐 구현체는 필요해질 때 추가(OCP).
- Instancing 데이터(이번 사이클 유일한 producer)의 시점 어긋남 리스크는 producer가 하나뿐이라 해당 없음 — 향후 물리/애니메이션처럼 같은 렌더 대상에 합성되는 두 번째 producer가 생기면 재검토.
- HLSL 컴파일: 오프라인(dxc/fxc + CMake 빌드 스텝)으로 확정.
- Compute Shader: 이번 사이클은 렌더 루프와 독립된 최소 데모까지만. 실제 Model 연동은 다음 사이클에서 별도 결정.
- Author 필드: 생략.

**남은 확인 사항**

- 없음 — Step 3(전략 문서 작성)로 진행.
