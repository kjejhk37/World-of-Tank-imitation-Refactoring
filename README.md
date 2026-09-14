# World-of-Tank-imitation-Refactoring

`WOT-master` 전차 게임 코드베이스를 리팩토링하고 DirectX 버전을 업그레이드하는 프로젝트입니다.

## 목적

- `WOT-master`의 구조 및 코드 리팩토링, DirectX 버전 업.
- 과거에 공부했던 C++ / DirectX / 수학적 매커니즘에 대한 복습.
- 이직용 포트폴리오 재정비.

## 범위

- 기본 디렉토리 구조를 포함한 전체 프로젝트 구조 리팩토링.
- 코드 및 수학적 매커니즘(물리, 렌더링 수식 등) 업데이트.
- DirectX 11 → 상위 버전 마이그레이션.
- 빌드 시스템: `.sln`/`.vcxproj` → CMake.

## 개발 환경

- 에디터: VS Code.
- 빌드 시스템: CMake.

## 게임 리소스

- 게임 리소스(텍스처·사운드·모델·데이터)는 비공개 저장소 `WOT-assets`에 있으며 `assets/` submodule로 연결되어 있습니다.
- 리소스는 공개 배포하지 않으므로, 권한이 없으면 `git clone --recursive` 시 `assets` submodule만 받지 못하는 것이 정상입니다.
- 현재 빌드는 `assets/`를 사용하지 않으므로 리소스 없이도 빌드·테스트할 수 있습니다.

자세한 협업 방식과 워크플로우는 [`CLAUDE.md`](./CLAUDE.md)를 참고하세요.
