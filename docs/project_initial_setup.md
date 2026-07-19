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
  renderer/         # 렌더러 백엔드 선택/구현 (DirectX/OpenGL)
  persistence/       # Save/Load 시스템 (ISaveable, SaveLoadManager)
  serialization/     # 저장소 추상화 (IDataStore) + 구현체 (JsonDataStore, SqliteDataStore 스텁)
```

- 계층 간 의존 방향: `app` → `config`/`persistence`/`renderer` → `serialization`.
- `config`/`persistence`는 `serialization`의 `IDataStore` 인터페이스에만 의존하며, 구현체(JSON/SQLite)를 직접 알지 못한다 (의존성 역전).
- 외부 라이브러리(`nlohmann::json` 등)에 대한 직접 참조는 해당 라이브러리를 감싸는 구현체(.cpp) 내부로 한정한다 — CLAUDE.md 외부 라이브러리 wrapper 원칙.
- 새로운 시스템(엔티티/월드, 오디오 등)이 추가될 때도 이 패턴(인터페이스 계층 + 구현체 격리)을 유지한다.
