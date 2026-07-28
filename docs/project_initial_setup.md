# 프로젝트 초기 설정

## 사용자 언어

한국어

## 개발 언어

C++ (DirectX 11 → 상위 버전 마이그레이션 대상)

## 개발 환경 / 빌드 시스템

- 에디터: VS Code
- 빌드 시스템: CMake

## 기본 아키텍처

`설정_저장로드_시스템` 사이클(`docs/strategy/설정_저장로드_시스템_20260719_1221.md`)에서 확정.

```
src/
  app/              # 진입점(main.cpp) 및 조립(composition root)
  config/           # LaunchConfig(argv) + AppConfig/ConfigManager(JSON 파일 기반)
  logging/          # 로그 등급/포맷/싱크(콘솔·파일) + 전역 Log 래퍼, ErrorCode
  math/             # 순수 수학(Vec3/Quaternion/Matrix4x4/Transform) — 프로젝트의 다른 모듈에 무의존
  geometry/         # 도형 데이터(AABB/OBB/Sphere/Capsule/Cylinder/Mesh) + 쌍별 충돌 판정(Intersections)
  platform/         # OS/플랫폼 종속 컴포넌트 (Win32Window 등)
  engine/           # 스레딩 정책 seam(IFrameDataPublisher/DoubleBufferPublisher) + Engine Update
                    # producer(InstanceUpdateWorker/InstanceSnapshot) — 렌더러는 이 페이로드
                    # 타입만 알고 동기화 방식은 모른다(DirectX_Shader_기초연결 사이클에서 도입)
  renderer/         # 렌더러 백엔드 선택/구현 (DirectX9/11/12, OpenGL)
  ui/               # UI 프레임워크(IUiManager) + 백엔드별 ImGui 구현 (DirectX9/11/12)
  persistence/       # Save/Load 시스템 (ISaveable, SaveLoadManager)
  serialization/     # 저장소 추상화 (IDataStore) + 구현체 (JsonDataStore, SqliteDataStore 스텁)
```

- 계층 간 의존 방향: `app` → `config`/`persistence`/`renderer`/`platform` → `serialization`. `renderer`와 `platform`은 서로 독립적(윈도우 생성과 렌더러 초기화가 분리됨, `main()`이 조립).
- `config`/`persistence`는 `serialization`의 `IDataStore` 인터페이스에만 의존하며, 구현체(JSON/SQLite)를 직접 알지 못한다 (의존성 역전).
- 외부 라이브러리(`nlohmann::json` 등)에 대한 직접 참조는 해당 라이브러리를 감싸는 구현체(.cpp) 내부로 한정한다 — CLAUDE.md 외부 라이브러리 wrapper 원칙. DirectX 9/11/12도 동일 원칙 적용 — `<d3d9.h>`/`<d3d11.h>`/`<d3d12.h>`를 아는 파일은 각각 `DirectX9Renderer`/`DirectX11Renderer`/`DirectX12Renderer` 하나씩으로 완전히 분리되며, 외부(`RendererFactory`/`main()`)는 `IRenderer` 인터페이스만 안다. ImGui도 동일하게 `<imgui.h>`/`<imgui_impl_*.h>`를 아는 파일은 `ImGuiManagerDX9`/`DX11`/`DX12` 하나씩으로 분리되고, 각 `DirectXnRenderer`가 자기 짝을 내부적으로만 소유한다(`IUiManager` 인터페이스로 공용 부분만 노출).
- 새로운 시스템(엔티티/월드, 오디오 등)이 추가될 때도 이 패턴(인터페이스 계층 + 구현체 격리)을 유지한다.
- `math`/`geometry`는 프로젝트의 다른 모듈(`config`/`renderer`/`logging` 등)을 전혀 참조하지 않는다 — 안정화되면 별도 저장소로 뽑아 git submodule화할 계획이라, 지금부터 의존성을 자기 자신과 표준 라이브러리로만 한정해둔다.
