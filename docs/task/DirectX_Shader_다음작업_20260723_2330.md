# 다음 작업 예고 — DirectX 세팅 → Shader 개발

이 문서는 정식 Task 문서(Step 1)가 아니라, 다음 Task Cycle이 무엇인지 남겨두는 메모다.
실제 작업을 시작할 때는 이 메모를 참고해 정식 Task 문서(Purpose/Author/User Draft/Requirements/Checklist)를 새로 작성한다.

## 배경

`docs/archive/모델임포터_Mesh저장_20260723/brainstorming/모델임포터_Mesh저장_20260723_2123.md`에서 확정한 전체 로드맵은 다음과 같다.

- Importer (완료 — `docs/archive/모델임포터_Mesh저장_20260723/`)
- **DirectX 세팅 (다음)**
- Shader 개발 (그다음)

## 다음 Task의 대략적 범위

- DirectX 9/11/12 세팅 (기존 `src/renderer/directx9`/`directx11`/`directx12`, `src/ui/directx9`/`directx11`/`directx12`와의 관계 정리 필요).
- 이후 Shader 컴파일/로드 파이프라인, `MeshManager`가 반환하는 `Model`/`ModelMesh`를 실제로 GPU에 업로드해 렌더링하는 경로와의 연결.

## 확인이 필요한 사항 (다음 Task 시작 시)

- DirectX 세팅 사이클과 Shader 사이클을 하나로 묶을지, 별도로 나눌지.
- 기존 `src/renderer/directx*` 코드가 이미 어느 정도 되어 있는지 먼저 파악 필요 (`docs/archive/DirectX_초기설정_20260719/` 참고).
