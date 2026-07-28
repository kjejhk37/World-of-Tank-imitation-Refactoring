# Task: DirectX Shader 기초 연결

## Purpose

DirectX 렌더러에 여러 종류 Shader의 기초 형태(skeleton)를 연결한다 — 단일 Vertex/Pixel Shader 하나만이 아니라, 이후 확장의 기준점이 될 대표 Shader 유형들을 최소 형태로 갖춘다.

- HLSL Vertex/Pixel Shader 컴파일 및 로드, Input Layout 구성, 최소 Constant Buffer(WVP 등) 배선.
- Instancing Shader — 동일 메쉬를 다수 인스턴스로 그리는 기초 경로(인스턴스 버퍼/`SV_InstanceID` 등).
- Compute Shader — 그래픽 파이프라인 밖에서 GPU 연산을 수행하는 기초 경로(디스패치, UAV 바인딩 등).
- `MeshManager`가 반환하는 `Model`/`ModelMesh`의 정점 데이터를 GPU 버퍼(Vertex/Index Buffer)에 업로드해, 실제로 화면에 그려지는 최소 draw call 경로를 구축한다.
- **멀티스레드 환경을 고려한 설계** — Shader 리소스 컴파일/로드, Command List 기록(DX12), 리소스 업로드 등을 단일 스레드 전제로 짜지 않고 병렬화 여지를 구조적으로 남긴다.
- 라이팅, 머티리얼/텍스처링, PBR 등 고급 셰이더 기능은 이번 사이클 범위에서 제외하고 다음 사이클로 미룬다 (사용자가 명시적으로 범위를 한정함).

## Author

(생략)

## User Draft

> 기본적으로 DirectX 내에서 기초적인 Shader 연결을 진행하는 것을 목표로 하는 것이고 이를 더 개발하는 것은 다음 목표로 진행할 꺼야

> 중요한것은 Compute Shader 와 Instance Shader 등 여러 Shader의 기본 형태를 만드는 것이고 멀티 스레드 환경을 고려해야해

- 이번 사이클은 "기초 연결"까지만 — 셰이더 고도화(라이팅/머티리얼 등)는 별도 사이클.
- 다만 "기초 연결"의 범위 안에 Vertex/Pixel뿐 아니라 Instancing, Compute Shader의 최소 형태가 포함됨.
- 멀티스레드 환경을 고려한 구조가 필수 요구사항.

## Requirements

- 대상 DirectX 백엔드 범위: **DX9/DX11/DX12 모두 포함**.
  - Vertex/Pixel(baseline) + Instancing Shader는 DX9(Shader Model 2.0/3.0)에서도 문법적으로 작성 가능 — DX9에도 포함.
  - 단, DX9의 하드웨어 인스턴싱은 API가 아니라 드라이버 확장 기능(`D3DSTREAMSOURCE_INDEXEDDATA` 트릭)이라 모든 DX9 디바이스가 지원을 보장하지 않음 — best-effort로 시도하고, 미지원 시 폴백(비인스턴싱 루프) 여지를 남긴다.
  - **Compute Shader는 DX9에서 제외** — 드라이버 문제가 아니라 Direct3D 9 API 자체에 Compute 파이프라인 스테이지가 없어 100% 불가능한 하드 제약. DX11(feature level 11_0+)/DX12에서만 구현.
- HLSL 컴파일 방식: **오프라인 컴파일**(`dxc`/`fxc`로 빌드 시점에 `.cso` 생성, CMake 커스텀 빌드 스텝) — 셰이더 소스(.hlsl) 자체나 렌더러 설계에는 런타임 컴파일과 차이가 없고, "컴파일을 언제/어디서 수행하느냐"(빌드 타임 vs 런타임)와 배포 시 `D3DCompiler_47.dll` 의존성 여부만 다르다. 배포 목표(패키징된 `.exe`)에 맞춰 오프라인으로 확정.
- 그리는 대상: 하드코딩 삼각형으로 파이프라인 스모크 테스트 → `MeshManager`로 로드한 실제 모델로 교체하는 2단계.
- **멀티스레드 아키텍처**: "Shader 스레드를 여러 개로 쪼갠다"가 아니라, **엔진 연산부(물리/애니메이션/인스턴스 데이터 계산 등)를 멀티스레드 워커로 분리하고, Shader/렌더링은 메인 스레드가 그 결과 스냅샷을 캡처해 GPU에 제출하는 역할**을 맡는 구조.
  - 워커 스레드: 매 프레임 인스턴스 월드 행렬 등 GPU에 업로드할 데이터를 계산.
  - 메인 스레드(Shader/Viewer 역할): 워커가 완성한 최신 스냅샷을 스레드 안전하게 읽어와 GPU 버퍼 업로드 + Draw/Dispatch 수행. 렌더링 자체를 여러 스레드로 쪼개지 않음.
  - 동기화 방식(더블 버퍼링 vs 뮤텍스 등 구체 방식)은 Strategy 단계에서 확정.
- Compute Shader 데모: 이번 사이클은 렌더 루프와 독립된 최소 데모(`RWStructuredBuffer` 변환 + CPU 리드백 단위 테스트)만 작성. 실제 `Model` 데이터와의 연동 방식은 다음 사이클에서 별도로 결정.

## Checklist

(Step 3 전략 문서에서 구체화 예정)

## 참고

- 직전 사이클 결과: `docs/archive/DirectX_초기설정_20260719/commit/DirectX_초기설정_20260719_1459.md` — DX9/DX11/DX12 렌더러가 `Clear→Present`만 수행하는 상태(셰이더/버텍스버퍼/드로우콜 없음)에서 시작.

### 이전 계획 메모 원문 (`DirectX_Shader_다음작업_20260723_2330.md`, 병합 후 삭제됨)

이 문서는 정식 Task 문서(Step 1)가 아니라, 다음 Task Cycle이 무엇인지 남겨두는 메모였다.

**배경**

`docs/archive/모델임포터_Mesh저장_20260723/brainstorming/모델임포터_Mesh저장_20260723_2123.md`에서 확정한 전체 로드맵은 다음과 같다.

- Importer (완료 — `docs/archive/모델임포터_Mesh저장_20260723/`)
- DirectX 세팅 (완료 — `docs/archive/DirectX_초기설정_20260719/`)
- Shader 개발 (이번 사이클)

**다음 Task의 대략적 범위 (메모 작성 시점 기준)**

- DirectX 9/11/12 세팅 (기존 `src/renderer/directx9`/`directx11`/`directx12`, `src/ui/directx9`/`directx11`/`directx12`와의 관계 정리 필요).
- 이후 Shader 컴파일/로드 파이프라인, `MeshManager`가 반환하는 `Model`/`ModelMesh`를 실제로 GPU에 업로드해 렌더링하는 경로와의 연결.

**확인이 필요한 사항 (메모 작성 시점 기준)**

- DirectX 세팅 사이클과 Shader 사이클을 하나로 묶을지, 별도로 나눌지. → DirectX 세팅은 `DirectX_초기설정_20260719` 사이클로 이미 별도 완료됨.
- 기존 `src/renderer/directx*` 코드가 이미 어느 정도 되어 있는지 먼저 파악 필요. → 위 "직전 사이클 결과" 참고.
