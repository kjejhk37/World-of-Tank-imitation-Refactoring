../strategy/DirectX_Shader_기초연결_20260726_1400.md

# Commit: DirectX Shader 기초 연결

## 구현 결과 요약

전략 문서 체크리스트 1~11 전부 완료.

- 셰이더 빌드 파이프라인(CMake + `fxc.exe`), `ShaderBytecodeLoader`.
- `IFrameDataPublisher<T>` / `DoubleBufferPublisher<T>`(락-프리 트리플 버퍼) + 동시성 테스트.
- `InstanceSnapshot` / `InstanceUpdateWorker`(Engine producer 데모).
- `IRenderer::RenderFrame(const InstanceSnapshot&)` 시그니처 변경, 3개 백엔드 + `OpenGLRenderer` + 호출부 전부 갱신.
- DX9/DX11/DX12 Baseline(하드코딩 삼각형 → `MeshManager`로 로드한 실제 큐브) 파이프라인.
- DX9(하드웨어 인스턴싱 + CPU 폴백)/DX11/DX12 Instancing 파이프라인.
- DX11/DX12 Compute Shader 최소 데모(렌더 루프와 독립).
- `main.cpp`에 `InstanceUpdateWorker` 배선.
- `tests` 타겟 전체 281건 `ctest -C Debug` 통과, 3개 백엔드 모두 `main.exe` 크래시 없이 실행 확인.

## 이슈 (심각도별)

- [RESOLVED] `main.cpp`가 `InstanceUpdateWorker::Start()` 내부 `std::thread` 생성 실패(`std::system_error`)를 처리하지 않고 있었다.
  → 코드 리뷰 단계에서 즉시 수정: `try/catch` + `Log::Error(ErrorCode::EngineWorkerStartFailed, ...)` 추가, `ErrorCode.h`에 4000번대(엔진) 카테고리 신설. 수정 후 재빌드 및 `ctest` 281건 재확인.
- [DEFERRED] `DirectX9/11/12Renderer` 각 클래스가 디바이스 수명주기 + Baseline 파이프라인 + Instancing 파이프라인을 한 클래스 안에 전부 갖고 있어 SRP가 다소 늘어났다(MEDIUM).
  → 이번 사이클 전략 문서가 명시적으로 채택한 구조라 버그는 아니다. 다음 셰이더 고도화(라이팅/머티리얼) 사이클에서 파이프라인별 클래스 분리(예: `DirectX11BaselinePipeline`/`DirectX11InstancingPipeline`)를 함께 검토하는 것을 제안 — 사용자가 이번 대화에서 "즉시 해소 가능한 것 외에는 별도 처리"로 사전 승인함에 따라 새 Task 사이클로 미룸.
- [DEFERRED] 테스트 전용 접근자 `DebugReadBackInstanceBuffer()`가 `DirectX9/11/12Renderer`의 공개 API에 노출되어 있다(LOW).
  → friend 테스트 픽스처 등으로 옮길 수 있으나 렌더러 3개 + 테스트 3개 파일을 다 건드려야 해 비용 대비 효과가 낮다고 판단, 다음 리팩터링 사이클 후보로 미룸.
- [DEFERRED] 카메라/투영 시스템이 없어 3개 백엔드 전부 WVP/ViewProj 상수가 항상 Identity다(LOW).
  → Task/Brainstorming 단계에서 반복 확정된 범위 밖 사항. 카메라/뷰-투영 시스템은 별도 Task로 진행 필요.
- [DEFERRED] "인스턴싱된 큐브가 화면에 실제로 올바르게 보이는지"의 픽셀 단위 육안 확인을 Claude가 수행하지 못했다(LOW).
  → Claude는 렌더링된 창을 직접 볼 수 없어 크래시 없이 실행되는지까지만 자동 확인했다. 사용자가 편할 때 `main.exe --renderer=directx9`/`directx11`/`directx12`를 직접 실행해 육안으로 확인 권장 — 코드 결함이 아니라 검증 수단의 한계.

## 참고

- 코드 리뷰(Stage 1+2): `../review/DirectX_Shader_기초연결_20260728_1900.md`
- Refactor Inspection: 사용자가 건너뛰기로 결정(2026-07-29).
