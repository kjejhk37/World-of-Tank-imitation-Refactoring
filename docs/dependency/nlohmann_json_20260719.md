Brainstorming: [`../brainstorming/설정_저장로드_시스템_20260719_1221.md`](../brainstorming/설정_저장로드_시스템_20260719_1221.md)

# Step 1 — Evaluation

## Alternatives

| | nlohmann::json | RapidJSON | Boost.JSON |
|---|---|---|---|
| 기능 범위 | 직관적인 API (`json["key"]`), STL 컨테이너처럼 사용 가능 | SAX/DOM 둘 다 지원, API가 저수준 | Boost 생태계 통합, API가 nlohmann과 유사 |
| 성능 | 상대적으로 느림 (편의성 우선 설계) | 매우 빠름 (파싱 성능 벤치마크 상위권) | 준수한 성능 |
| 통합 난이도 | header-only 단일 헤더, CMake `FetchContent`로 즉시 사용 가능 | header-only지만 API가 장황함 | Boost 전체 또는 개별 라이브러리 의존성 필요 |
| 커뮤니티/인지도 | C++ 진영 사실상 표준 JSON 라이브러리, 예제/레퍼런스 풍부 | 게임/임베디드권에서 성능 이유로 채택 | Boost 사용자 대비 상대적으로 마이너 |

이번 사이클의 Config/Save 데이터는 사람이 직접 읽고 수정할 수 있는 크기(설정값 수십 개, 세이브 데이터 수백 개 필드 이내)이므로, 파싱 성능보다 **API 편의성과 코드 가독성**이 우선순위임 — `nlohmann::json` 선택.

## Maintenance Status

- 최신 릴리스: v3.11.3 (CMake `FetchContent`로 받아오는 버전).
- GitHub 저장소가 활발히 유지보수 중이며, C++ 진영에서 가장 널리 쓰이는 JSON 라이브러리 중 하나로 이슈/PR이 지속적으로 처리됨.
- 저장소 자체가 아카이브되거나 방치된 상태 아님.

## License

- MIT License. Permissive, 카피레프트 없음.
- 이 프로젝트(개인 학습/포트폴리오, 상업적 재배포 없음)와 라이선스 충돌 없음.

## Risks

- header-only 라이브러리라 대형 헤더(`json.hpp`) 포함 시 컴파일 시간이 늘어날 수 있음 — 다만 이 프로젝트에서는 `JsonDataStore.cpp` 단 하나의 번역 단위에서만 포함하므로 영향 범위가 제한적.
- CMake `FetchContent`로 소스를 받아오므로 최초 빌드 시 네트워크 필요 — `googletest` 도입 때와 동일한 패턴/리스크이며, 이미 프로젝트에서 수용한 방식.
- 파싱 성능이 RapidJSON 대비 느림 — 현재 Config/Save 데이터 규모(사람이 직접 편집 가능한 수준)에서는 체감 영향 없음. 이후 데이터 규모가 커지면(예: 대용량 세이브) `SqliteDataStore`로 전환하는 것이 이번 사이클에서 이미 설계된 대응 경로임.

---

# Step 2 — Wrapper Design Draft

CLAUDE.md 원칙("외부 라이브러리는 반드시 내부 wrapper 클래스로 감싼다")을 그대로 적용.

- `nlohmann::json`에 대한 직접 참조(`#include <nlohmann/json.hpp>`)는 `src/serialization/JsonDataStore.cpp` 단 한 곳에만 존재한다.
- `JsonDataStore.h`는 `nlohmann::json`을 전혀 노출하지 않고, 프로젝트 공용 타입인 `DataRecord`(`std::unordered_map<std::string, std::string>`)만 공개 인터페이스에 사용한다.
- `ConfigManager`, `SaveLoadManager` 등 상위 모듈은 `IDataStore` 인터페이스만 알고, `nlohmann::json`의 존재 자체를 모른다.

**빌드 타겟 격리 (GoogleTest 사례와 동일 원칙)**

- `nlohmann_json::nlohmann_json`은 `app_lib`에 `PRIVATE`로 링크됨 — `app_lib`을 소비하는 `main`/`tests` 타겟의 include 경로에는 `nlohmann::json` 헤더가 노출되지 않는다.
- `tests` 타겟은 `JsonDataStoreTest.cpp`에서 `JsonDataStore.h`(공개 인터페이스)만 include하며, `nlohmann/json.hpp`를 직접 include하지 않는다.

---

# Step 3 — Verification Status

- `[x] Verified` — CMake `FetchContent`로 `nlohmann::json` v3.11.3 소스를 받아 빌드에 통합 (`app_lib`에만 `PRIVATE` 링크, `main`/`tests`는 include 경로 상으로 무의존 확인)
- `[x] Verified` — `JsonDataStore::Save`/`Load` 왕복 테스트(`JsonDataStoreTest`) 통과
- `[x] Verified` — 존재하지 않는 파일 Load 시 예외 없이 `false` 반환 확인
- `[x] Verified` — 손상된(문법 오류) JSON 파일에 대해 `main.exe` 실행 시 크래시 없이 하드코딩 기본값으로 정상 기동 확인 (수동 검증)
- `[x] Verified` — 현재 프로젝트 툴체인(MSVC 19.37, Ninja)에서 정상 빌드, `ctest` 기준 전체 21개 테스트 케이스 통과

검증은 `docs/strategy/설정_저장로드_시스템_20260719_1221.md` 구현 단계에서 수행 완료.
