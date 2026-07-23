Brainstorming: [`../brainstorming/모델임포터_Mesh저장_20260723_2123.md`](../brainstorming/모델임포터_Mesh저장_20260723_2123.md)

# Step 1 — Evaluation

## Alternatives

| | tinyobjloader | Assimp(OBJ 지원) | 자체 파서 |
|---|---|---|---|
| 기능 범위 | OBJ/MTL 전용, 삼각화·머티리얼 지원 | OBJ 포함 40+ 포맷, 풀 씬 그래프 | 필요한 것만 직접 구현 |
| 통합 난이도 | 단일 헤더+.cc, CMakeLists 제공(FetchContent 용이) | 대형 의존성(zlib/rapidjson 등 벤더링) | 통합은 쉽지만 파싱 버그를 직접 책임 |
| 유지보수 | 활발(2026-06 C11 로더 추가 등) | 활발하나 프로젝트 규모가 큼 | 유지보수 주체가 이 프로젝트 자신 |
| 배포 크기 | 작음 | 큼 | 가장 작음(대신 신뢰성 리스크) |

이 프로젝트는 포맷별 경량 라이브러리 방향(방향 B)을 이미 결정했으므로, OBJ 전용으로 가장 널리 쓰이고 활발히 유지보수되는 tinyobjloader를 선택했다.

## Maintenance Status

- 최신 안정 릴리스: v1.0.7 (v2.0 계열은 아직 RC 단계라 배제).
- 2026-06-19에 C11 로더(`tiny_obj_c`) 추가 등 활발히 유지보수 중.
- GitHub 저장소가 아카이브되거나 방치된 상태 아님.

## License

- MIT 계열. Permissive, 카피레프트 없음. 이 프로젝트(개인 학습/포트폴리오)와 라이선스 충돌 없음.

## Risks

- v1.0.7 API(`tinyobj::LoadObj` 자유 함수 + `attrib_t`/`shape_t`)는 이후 릴리스(v2.0 계열의 `ObjReader` 클래스)와 API가 다르다 — 향후 라이브러리 버전을 올릴 때 `TinyObjWrapper` 내부만 다시 쓰면 되고 다른 코드는 영향 없음(wrapper 격리 원칙 덕분).
- 정점을 인덱스 튜플별로 비-중복 제거(non-deduplicated) 방식으로 변환한다 — 정점 재사용 최적화(vertex welding)는 하지 않는다. 이번 사이클의 우선순위(정확성 > 메모리 최적화)에 따른 의도적 단순화.

---

# Step 2 — Wrapper Design Draft

CLAUDE.md 원칙("외부 라이브러리는 반드시 내부 wrapper 클래스로 감싼다")을 그대로 적용.

- `tiny_obj_loader.h`에 대한 직접 참조는 `src/model_import/TinyObjWrapper.cpp` 단 한 곳에만 존재한다.
- `TinyObjWrapper.h`는 `tinyobj::` 네임스페이스의 어떤 타입도 노출하지 않고, 프로젝트 공용 타입인 `Mesh`만 공개 인터페이스에 사용한다.
- `ModelLoader::LoadOBJ`(상위 모듈)는 `TinyObjWrapper`만 알고, tinyobjloader의 존재 자체를 모른다.

**빌드 타겟 격리**

- `tinyobjloader`는 `app_lib`에 `PRIVATE`로 링크된다 — `app_lib`을 소비하는 `main`/`tests` 타겟의 include 경로에는 tinyobjloader 헤더가 노출되지 않는다.
- `tests` 타겟은 `ModelLoaderTest.cpp`에서 `ModelLoader.h`(공개 인터페이스)만 include하며, `tiny_obj_loader.h`를 직접 include하지 않는다.

---

# Step 3 — Verification Status

- [x] Verified — CMake `FetchContent`로 tinyobjloader v1.0.7 소스를 받아 빌드에 통합 (`app_lib`에만 `PRIVATE` 링크)
- [x] Verified — `cube.obj`(8정점 큐브) 로드 후 삼각화된 36개 정점/인덱스로 변환됨을 `ModelLoaderTest`로 확인
- [~] In Progress — 존재하지 않는 파일/손상된 OBJ에 대한 예외 처리 견고성 (기본 예외 던지기만 구현, 세밀한 에러 메시지 검증은 후속 사이클)

검증은 `docs/strategy/모델임포터_Mesh저장_20260723_2240.md` 구현 단계에서 수행.
