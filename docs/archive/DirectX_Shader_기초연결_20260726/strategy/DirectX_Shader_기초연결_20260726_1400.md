../brainstorming/DirectX_Shader_기초연결_20260726_1230.md

# Strategy: DirectX Shader 기초 연결

## 구현 진행 상황 (세션 인계용 — 다음 Claude가 이어서 개발할 때 여기부터 읽을 것)

**현재 상태: 체크리스트 1~11 전부 완료(전체 빌드/281개 테스트 통과). Step 5(코드 리뷰 → 커밋 리포트 → 아카이브 → git push) 진행 중.**

- 이번 작업은 `docs/task/DirectX_Shader_기초연결_20260726_1200.md` → `docs/brainstorming/DirectX_Shader_기초연결_20260726_1230.md`(승인 완료) → 이 전략 문서(승인 완료) 순으로 진행된 사이클이며, Step 4(구현)를 마치고 Step 5로 넘어간 상태다.
- 반복해서 남는 미해결 사항 하나: 큐브/인스턴싱이 화면에 실제로 올바르게 보이는지의 픽셀 단위 육안 확인은 Claude가 창을 볼 수 없어 수행하지 못했다 — 커밋 리포트에 종합 기록.

### 발견된 문제와 제안된 수정 (재승인 완료, 2026-07-28)

체크리스트 3번의 원래 계획(`std::array<T,2>` + `std::atomic<int> frontIndex`로 front/back을 스왑)은 `InstanceSnapshot`처럼 `std::vector<Matrix4x4>`를 담는 비-POD 타입에는 안전하지 않다.

- **경합 시나리오**: Consumer가 `AcquireReadSnapshot()`으로 slot 0 참조를 받아 `worldMatrices`를 순회하며 읽는 도중, Producer가 slot 1에 다 쓰고 `Publish()`(front→1)한 뒤 곧바로 다음 루프에서 back(`1-1=0`)에 다시 쓰기 시작하면, **Consumer가 아직 읽고 있는 slot 0을 Producer가 동시에 덮어쓰게 된다.** 슬롯이 2개뿐이면 "producer가 consumer보다 정확히 한 바퀴 더 돌면" 바로 발생하며, Engine 스레드가 계속 빠르게 도는 이번 설계상 실제로 자주 일어날 수 있다. `std::vector`를 한 스레드가 읽는 도중 다른 스레드가 재할당하면 UB(크래시 가능성 있음).
- **제안된 수정**: 슬롯을 **3개**로 늘리고, "지금 아무도 안 쓰는 중간 슬롯"의 인덱스를 producer/consumer가 각각 원자적 `exchange`로 주고받는 **락-프리 트리플 버퍼** 알고리즘으로 교체한다 — producer의 write 슬롯과 consumer의 read 슬롯이 항상 서로 다른 두 슬롯이 되도록 알고리즘이 보장해서, 위 경합이 구조적으로 불가능해진다. 클래스/파일 이름은 `DoubleBufferPublisher`로 그대로 유지(공개 인터페이스 관점에선 여전히 "읽기 슬롯 하나·쓰기 슬롯 하나"로 동작) — 내부 저장소만 3슬롯으로 바뀐다.
- **다음 Claude가 할 일**: 재승인 완료됨. `src/engine/DoubleBufferPublisher.h`를 트리플 버퍼 알고리즘(write/read/spare 슬롯 3개, producer/consumer가 원자적 `exchange`로 spare 인덱스 교환)으로 다시 작성하고, 단위 테스트(동시성 stress test 포함)부터 이어서 작성한다. 그다음 체크리스트 1, 2, 4~11 순서로 계속 진행.

---

## 적용 방식 요약

브레인스토밍에서 확정된 내용을 아래 순서로 코드에 반영한다 — 셰이더 빌드 파이프라인 → 정책 seam(`IFrameDataPublisher`) → Engine Update producer → 백엔드별 Baseline 드로우(하드코딩 삼각형) → `MeshManager` 모델 교체 → Instancing → Compute Shader 데모 → `main.cpp` 배선 → 빌드/테스트.

- 대상 백엔드: DX9/DX11/DX12 모두. Compute Shader만 DX9 제외(하드 API 제약).
- HLSL은 오프라인 컴파일(`fxc.exe`, CMake 커스텀 빌드 스텝)로 `.cso`를 생성한다. DX9(`vs_3_0`/`ps_3_0`)부터 DX12(`vs_5_0`/`ps_5_0`/`cs_5_0`)까지 `fxc.exe` 하나로 커버되므로 별도 컴파일러(`dxc`)를 추가로 들이지 않는다.
- 셰이더 소스는 백엔드별 폴더(`src/renderer/directx9/shaders/` 등)에 두고, 각 백엔드는 자기 폴더의 셰이더만 참조한다(기존 "각 렌더러는 자기 D3D 헤더만 안다" 원칙과 동일한 결로 셰이더도 분리).
- 멀티스레드 아키텍처는 `IFrameDataPublisher<T>` 인터페이스 + `DoubleBufferPublisher<T>` 구현체 하나만 만든다. `Fork-Join`/큐 정책은 이번 사이클에 만들지 않는다(YAGNI, OCP로 여지만 남김).
- 인스턴스 데이터의 "Engine Update" producer는 실제 게임플레이 데이터가 아직 없으므로, 데모용 워커(`InstanceUpdateWorker`)가 N개 인스턴스를 계속 움직이는 합성 데이터를 만든다 — 이후 물리/애니메이션 시스템이 생기면 이 워커를 교체한다.
- `IRenderer::RenderFrame()`은 시그니처를 `RenderFrame(const InstanceSnapshot& snapshot)`으로 변경한다 — 렌더러는 `IFrameDataPublisher`/스레딩을 전혀 모르고, `main.cpp`가 `publisher.AcquireReadSnapshot()`으로 얻은 순수 데이터만 넘겨받는다(SRP: 렌더러는 "동기화 방식"을 몰라도 됨).
- 그리는 대상은 하드코딩 삼각형(파이프라인 스모크 테스트) → `tests/fixtures/model_import/cube.obj`를 `MeshManager`로 로드한 실제 모델(순서대로 교체).

## 구현 체크리스트

### 1. 셰이더 빌드 파이프라인 (CMake + `fxc.exe`)

- [x] CMake 함수 `wot_add_hlsl_shader(<출력 .cso 변수> <소스 .hlsl> <진입점> <프로파일>)` 작성 완료 — `add_custom_command`으로 `fxc.exe /nologo /T <프로파일> /E <진입점> /Fo <출력> <소스>` 실행, `DEPENDS <소스>`.
- [x] 컴파일된 `.cso`를 `${CMAKE_BINARY_DIR}/$<CONFIG>/shaders/<backend>/`(실행 파일과 같은 위치 — `$<TARGET_FILE_DIR:main>`은 VS 멀티 컨피그 생성기의 `add_custom_command` OUTPUT에서 "No target main" 생성 오류를 내는 것을 실제로 확인해 대신 이 방식 채택, 아래 Notes 참고)에 출력하도록 경로 구성 완료.
- [x] 위 함수로 생성된 모든 `.cso` 출력을 모아 `shaders` 커스텀 타겟(`add_custom_target(shaders DEPENDS ...)`)으로 묶고, `main`/`tests`가 이 타겟에 의존하게 함(`add_dependencies`) 완료.
- [x] `find_program(FXC_EXECUTABLE fxc PATHS ...)`로 설치된 모든 Windows SDK 버전의 `fxc.exe`를 검색, 못 찾으면 `message(FATAL_ERROR ...)`로 중단 — 완료.
- 빌드 검증: `cmake --build . --target shaders`로 DX9(`vs_3_0`/`ps_3_0`)/DX11/DX12(`vs_5_0`/`ps_5_0`) Baseline 셰이더 6개가 전부 `.cso`로 컴파일됨을 확인.

### 2. `ShaderBytecodeLoader` (신규, 백엔드 공용)

- [x] `src/renderer/ShaderBytecodeLoader.h/.cpp` 작성 완료 — `std::vector<uint8_t> Load(const std::string& csoPath)` 네임스페이스 함수. 파일 없음/읽기 실패/빈 파일 시 `std::runtime_error`.
- [x] 단위 테스트: `tests/ShaderBytecodeLoaderTest.cpp` — 실제 빌드된 `directx11/Baseline.vs.cso`를 로드해 비어있지 않은 바이트열 반환 확인 + 존재하지 않는 파일에 대한 예외 발생 확인, 2건 전부 통과.

### 3. `IFrameDataPublisher<T>` / `DoubleBufferPublisher<T>` (신규, `src/engine/`)

- [x] `src/engine/IFrameDataPublisher.h` 작성 — 템플릿 인터페이스: `T& AcquireWriteSlot()`, `void Publish()`, `const T& AcquireReadSnapshot() const`.
- [x] `src/engine/DoubleBufferPublisher.h` 작성 — 슬롯 2개+원자적 인덱스 스왑안 폐기, **슬롯 3개짜리 락-프리 트리플 버퍼**(producer/consumer가 원자적 `exchange`로 spare 인덱스를 주고받는 방식)로 구현 완료 — public 이름은 `DoubleBufferPublisher` 유지.
- [x] `Publish()`가 back buffer에 쓰기가 "완전히 끝난 뒤"에만 호출된다는 계약을 클래스 주석(Notes)에 명시 완료.
- [x] 단위 테스트: `tests/DoubleBufferPublisherTest.cpp`의 `ConcurrentProducerConsumerNeverObservesTornRead` — producer/consumer 스레드를 100ms 동시 실행하며 매 읽기마다 `b == a*2` 검증, torn read 없이 통과.
- [x] 단위 테스트(결정적 검증용): `ManualModeReflectsPublishedValueOnSingleThread`/`ManualModeReflectsMultiplePublishesInOrder` — 단일 스레드에서 write→publish→read 순서로 스왑 동작 검증, 통과.

### 4. `InstanceSnapshot` / `InstanceUpdateWorker` (신규, `src/engine/`)

- [x] `src/engine/InstanceSnapshot.h` 작성 완료 — `struct InstanceSnapshot { std::vector<Matrix4x4> worldMatrices; };`
- [x] `src/engine/InstanceUpdateWorker.h/.cpp` 작성 완료 — 생성자에서 인스턴스 개수(N, 기본 상수 `kDefaultInstanceCount=16`)를 받고, `Start()`로 `std::thread` 시작, `Stop()`으로 원자적 종료 플래그 설정 후 `join()`(중복 호출 안전, 소멸자에서도 방어적으로 호출). 내부 루프는 16ms 간격으로 N개 인스턴스를 격자 배치 + 시간에 따른 회전/상하 이동으로 `Matrix4x4::FromTRS` 계산해 `DoubleBufferPublisher<InstanceSnapshot>`에 커밋.
- [x] `DoubleBufferPublisher<InstanceSnapshot>`을 멤버로 소유하고 `const IFrameDataPublisher<InstanceSnapshot>& GetPublisher() const`로 노출 완료.
- [x] 단위 테스트: `tests/InstanceUpdateWorkerTest.cpp` 3건(개수 반영 확인/Stop 중복 호출 안전성/시간에 따라 스냅샷이 계속 바뀌는지) 전부 통과.

### 5. `IRenderer` 인터페이스 변경

- [x] `IRenderer.h`의 `RenderFrame()`을 `RenderFrame(const InstanceSnapshot& snapshot)`으로 변경 완료(`InstanceSnapshot.h` include 추가).
- [x] `DirectX9/11/12Renderer`, `OpenGLRenderer`, `main.cpp`, `DirectX9/11/12RendererTest.cpp` 호출부를 새 시그니처에 맞춰 갱신 완료(현재는 빈 `InstanceSnapshot{}` 전달 — 실제 워커 연결은 체크리스트 10에서). 빌드(`main`/`tests`) 통과, 기존 테스트 273건 전부 회귀 없이 통과 확인.

### 6. Baseline Vertex/Pixel Shader — 하드코딩 삼각형 스모크 테스트 (DX9/DX11/DX12)

- [x] 백엔드별 `shaders/Baseline.vs.hlsl`/`Baseline.ps.hlsl` 작성 완료 — 최소 Constant Buffer(World-View-Projection, DX9는 `SetVertexShaderConstantF`, DX11/12는 `cbuffer`)로 정점을 변환, 단색 픽셀 셰이더. 카메라/투영 시스템은 범위 밖이라 WVP=Identity, 삼각형 정점 자체를 클립 공간(-1~1)에 맞춰 하드코딩.
- [x] `DirectX9Renderer`: `IDirect3DVertexShader9`/`IDirect3DPixelShader9` 로드 + `IDirect3DVertexDeclaration9` + `D3DPOOL_MANAGED` 정점 버퍼, `RenderFrame`에서 `SetVertexShaderConstantF` + `DrawPrimitive` 완료.
- [x] `DirectX11Renderer`: `ID3D11VertexShader`/`ID3D11PixelShader`/`ID3D11InputLayout` + `D3D11_USAGE_IMMUTABLE` 정점 버퍼 + `D3D11_USAGE_DYNAMIC` 상수 버퍼, `RenderFrame`에서 뷰포트 설정 + `Draw` 완료(뷰포트는 기존 코드에 누락돼 있어 이번에 추가).
- [x] `DirectX12Renderer`: 루트 CBV 파라미터 1개짜리 루트 시그니처 + PSO(`ID3D12PipelineState`), 업로드 힙 커밋 리소스로 정점/상수 버퍼, `RenderFrame`에서 뷰포트/시저 설정 + `DrawInstanced` 완료.
- [x] 3개 백엔드 모두 단위 테스트(기존 `forceWarp`/HAL 패턴과 동일하게) `Initialize()` 후 `RenderFrame(빈 InstanceSnapshot)` 호출이 크래시/false 없이 동작하는지 확인 — 실제 HAL/WARP 디바이스로 셰이더/버퍼/PSO 생성과 Draw 호출까지 전부 통과.
- [x] 수동 검증: `--renderer=directx9`/`directx11`/`directx12` 각각 실행 시 1.5초간 크래시 없이 유지되는지 확인. **다만 화면에 삼각형이 실제로 보이는지(픽셀 단위 시각 확인)는 Claude가 창을 눈으로 볼 수 없어 직접 확인하지 못했다 — 사용자가 여유될 때 육안으로 확인 권장(LOW, 커밋 리포트에 기록 예정).**

### 7. `MeshManager` 모델로 교체 (하드코딩 삼각형 → 실제 모델)

- [x] `tests/fixtures/model_import/cube.obj`를 런타임 에셋 경로(`assets/models/cube.obj`)로 복사 완료 — `main`/`tests` 둘 다 `POST_BUILD` 커스텀 커맨드로 자기 출력 디렉터리 옆에 복사(`$<TARGET_FILE_DIR:...>`는 OUTPUT 절과 달리 `TARGET POST_BUILD`에서는 안전하게 지원됨).
- [x] 백엔드별로 하드코딩 삼각형 정점 버퍼 생성 코드를, `MeshManager::GetOrLoad`로 로드한 `Model`의 첫 번째 `ModelMesh`(positions/indices/normals)를 Vertex/Index Buffer로 업로드하는 코드로 교체 완료(position/normal을 별도 입력 슬롯 2개로 바인딩, index는 `uint32_t`/`R32_UINT`/`D3DFMT_INDEX32`).
- [x] Input Layout/Declaration을 `ModelMesh`의 실제 정점 레이아웃(position + normal)에 맞춰 갱신 완료 — NORMAL은 라이팅이 범위 밖이라 셰이더가 읽지는 않지만 레이아웃엔 선언.
- [x] 수동 검증: 세 백엔드 모두 단위 테스트(HAL/WARP 실제 디바이스로 모델 로드+버퍼 생성+DrawIndexed 계열 호출까지) 통과, `main.exe --renderer=...` 각각 1.5초간 크래시 없이 유지 확인. **큐브가 실제로 화면에 보이는지는 체크리스트 6과 동일하게 Claude가 직접 육안 확인하지 못함(LOW, 커밋 리포트에 기록 예정).**

### 8. Instancing Shader (DX9 best-effort+폴백, DX11/DX12)

- [x] 백엔드별 `shaders/Instancing.vs.hlsl` 작성 완료 — DX11/DX12는 INSTANCE_WORLD를 4개의 TEXCOORD 스타일 float4 원소로 분해해 별도 Input Slot(`D3D11_INPUT_PER_INSTANCE_DATA`/`D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA`)로 읽음. Pixel Shader는 Baseline.ps.cso 재사용(라이팅 없는 단색이라 중복 불필요).
- [x] DX9: `IDirect3DDevice9::SetStreamSourceFreq`(`D3DSTREAMSOURCE_INDEXEDDATA`/`D3DSTREAMSOURCE_INSTANCEDATA`)로 하드웨어 인스턴싱 시도 완료 — 실제 하드웨어(HAL)에서 성공 확인.
- [x] DX9 폴백: `SetStreamSourceFreq`/드로우 호출 실패 시(반환값 확인) Baseline 파이프라인을 인스턴스 개수만큼 반복 호출(`SetVertexShaderConstantF` + `DrawIndexedPrimitive`)하는 경로로 전환 완료 — 별도 셰이더 불필요(Baseline 재사용).
- [x] `RenderFrame(const InstanceSnapshot& snapshot)`에서 `snapshot.worldMatrices`를 인스턴스 버퍼(DX9는 동적 버텍스 버퍼, DX11은 `D3D11_USAGE_DYNAMIC`, DX12는 업로드 힙 상시 매핑)에 매 프레임 업로드 완료. `snapshot`이 비어있으면(레거시 호출) 체크리스트 6/7의 Baseline 단일 드로우로 폴백.
- [x] 단위 테스트: DX11/DX12는 `DebugReadBackInstanceBuffer`(테스트 전용, STAGING 복사/업로드 힙 직접 읽기)로 N개 행렬이 정확히 업로드됐는지 바이트 단위 검증 통과. DX9는 하드웨어 인스턴싱 성공 시 동일하게 바이트 단위 검증(실제 HAL에서 성공 확인) + `forceInstancingFallback` 테스트 전용 플래그로 강제한 폴백 경로도 크래시 없이 통과.
- [x] 수동 검증: `main.exe --renderer=...` 각각 1.5초간 크래시 없이 유지 확인. **N개 큐브가 실제로 다른 위치에서 계속 움직이는 게 화면에 보이는지는 체크리스트 6/7과 동일하게 Claude가 육안 확인하지 못함 — main.cpp가 아직 InstanceUpdateWorker를 연결하지 않아(체크리스트 10 완료 전) 지금은 빈 스냅샷만 전달되고 있어 어차피 인스턴싱 경로 자체가 실행되지 않는다는 점도 함께 기록.**

### 9. Compute Shader 최소 데모 (DX11/DX12만, 렌더 루프와 독립)

- [x] 백엔드별 `shaders/Transform.cs.hlsl` 작성 완료 — `RWStructuredBuffer<float>`에 대해 값을 2배로 만드는 최소 커널(`numthreads(64,1,1)`).
- [x] DX11: `DirectX11ComputeDemo` — `ID3D11ComputeShader` 로드, `D3D11_RESOURCE_MISC_BUFFER_STRUCTURED` 버퍼 + `ID3D11UnorderedAccessView` 생성, `Dispatch`, staging 버퍼로 리드백 완료.
- [x] DX12: `DirectX12ComputeDemo` — 루트 시그니처(UAV 디스크립터 테이블 1개) + `ID3D12PipelineState`(컴퓨트), UPLOAD/DEFAULT(UAV)/READBACK 3단 버퍼 + 리소스 배리어, 펜스 대기 후 리드백 완료.
- [x] 단위 테스트(양쪽 백엔드): 입력 배열 1024개(1~1024)를 업로드 → Dispatch → 리드백한 값이 전부 2배가 됐는지 검증, 통과.
- [x] 이 데모는 `IRenderer`/`RenderFrame` 경로에 연결하지 않는다 — `DirectX11ComputeDemo`/`DirectX12ComputeDemo`가 스스로 헤드리스 디바이스를 생성하는 완전히 독립된 클래스로 격리 완료.

### 10. `main.cpp` 배선

- [x] `InstanceUpdateWorker`를 `renderer->Initialize()` 성공 이후, 메시지 루프 진입 이전에 `Start()` 완료.
- [x] 메시지 루프를 `while (window->PumpMessages()) { renderer->RenderFrame(worker.GetPublisher().AcquireReadSnapshot()); }`로 변경 완료.
- [x] `renderer->Shutdown()` 이전에 `worker.Stop()` 호출 완료.
- [x] 조기 반환 경로 방어 — `InstanceUpdateWorker::Stop()`이 이미 `Start()` 미호출 시 no-op이라(체크리스트 4에서 구현) 별도 방어 코드 불필요. 빌드(`main`/`tests`) 통과, 3개 백엔드 모두 1.5초간 크래시 없이 실행 확인, 전체 테스트 281건 회귀 없이 통과.

### 11. 빌드/문서 마무리

- [x] `CMakeLists.txt`에 신규 소스(`ShaderBytecodeLoader`, `InstanceUpdateWorker`, `DirectX11ComputeDemo`, `DirectX12ComputeDemo`) 추가 완료. `IFrameDataPublisher`/`DoubleBufferPublisher`/`InstanceSnapshot`은 헤더 전용이라 소스 목록에 없음(의도된 상태).
- [x] `CMakeLists.txt`에 셰이더 빌드 스텝(`wot_add_hlsl_shader` 호출 11건: DX9/11/12 Baseline VS+PS 6건 + DX9/11/12 Instancing VS 3건 + DX11/12 Transform CS 2건 — Instancing/Compute의 PS는 Baseline.ps.cso 재사용이라 별도 컴파일 불필요) 및 `shaders` 타겟 의존성(`add_dependencies(main/tests shaders)`) 추가 완료.
- [x] `main`, `tests` 타겟 전체 빌드 통과 확인.
- [x] `tests` 타겟 전체 케이스 통과 확인(`ctest -C Debug`) — 281건 전부 통과. (사이드 발견: `gtest_discover_tests`가 `WORKING_DIRECTORY`를 지정하지 않으면 멀티 컨피그 생성기에서 `tests.exe`가 실제로 있는 `build/Debug/`가 아니라 최상위 빌드 디렉터리를 실행 위치로 써서, 렌더러/셰이더 로더가 쓰는 상대 경로 자산을 못 찾는 문제가 있었음 — `WORKING_DIRECTORY "$<TARGET_FILE_DIR:tests>"`로 수정.)
- [x] `/verify` 수준의 수동 확인: 3개 백엔드 각각 `main.exe --renderer=...`가 1.5초간 크래시 없이 유지됨을 확인. 리사이즈 회귀는 `SurvivesResizeAndRenderFrame` 계열 단위 테스트로, 워커 스레드 정지는 `InstanceUpdateWorkerTest.StopReturnsWithoutHangingOrCrashing`으로 커버됨(둘 다 통과). **다만 "인스턴싱된 큐브들이 실제로 화면에 계속 움직이며 보이는지"는 체크리스트 6/7/8과 동일하게 Claude가 창을 육안으로 볼 수 없어 직접 확인하지 못함 — 커밋 리포트에 종합해서 기록.**
- [x] `docs/project_initial_setup.md`에 `src/engine/` 디렉터리 추가 반영 완료.
- [x] `<d3d9.h>`/`<d3d11.h>`/`<d3d12.h>`를 동시에 include하는 파일이 없음을 grep으로 확인.

## 최종 승인 및 검증 기준

- [x] CMake로 `main`, `tests` 타겟이 셰이더 빌드 스텝을 포함해 정상 컴파일·링크된다.
- [x] `tests` 타겟의 모든 케이스가 통과한다(`ShaderBytecodeLoader`, `DoubleBufferPublisher` 동시성 테스트, `InstanceUpdateWorker`, 백엔드별 렌더러 스모크 테스트, DX11/DX12 Compute Shader 리드백 테스트 포함) — `ctest -C Debug` 281건 전부 통과.
- [~] `main.exe --renderer=directx9`/`directx11`/`directx12` 각각 실행 시 인스턴싱된 큐브 여러 개가 계속 움직이며 그려진다(수동 확인) — **크래시 없이 실행됨은 확인, 화면 픽셀 육안 확인은 Claude가 수행 불가(커밋 리포트 기록).**
- [x] 창 리사이즈/전체화면이 이전 사이클과 동일하게 정상 동작한다(회귀 없음) — 단위 테스트로 확인.
- [x] 프로그램 종료 시 `InstanceUpdateWorker`가 크래시/행 없이 정지한다 — 단위 테스트로 확인.
- [x] `<d3d9.h>`/`<d3d11.h>`/`<d3d12.h>`를 동시에 include하는 파일이 없다(grep 확인, 셰이더 소스 폴더 분리 포함).
- [ ] `IFrameDataPublisher<T>`/`DoubleBufferPublisher<T>`가 `Fork-Join`/큐 정책을 몰라도 되고, `DirectX*Renderer`가 `IFrameDataPublisher`/스레딩을 몰라도 되는 계층 분리가 코드 리뷰로 확인된다(DIP/SRP) — 코드 리뷰 단계에서 확인 예정.
- [ ] 신규 작성된 모든 함수/클래스에 CLAUDE.md 주석 규칙(Author / 설명 / Input / Output / Notes / Date)이 반영되어 있다 — 코드 리뷰 단계에서 확인 예정.
