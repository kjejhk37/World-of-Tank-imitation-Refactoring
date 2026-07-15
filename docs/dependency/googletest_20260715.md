Brainstorming: [`../brainstorming/렌더러_분기_20260715_2205.md`](../brainstorming/렌더러_분기_20260715_2205.md)

# Step 1 — Evaluation

## Alternatives

| | GoogleTest | Catch2 (v3) | doctest |
|---|---|---|---|
| 기능 범위 | xUnit 스타일 + `gmock` 내장 | BDD 스타일(`SECTION`), mock 없음 | 최소 기능, mock 없음 |
| 커뮤니티/인지도 | 업계 사실상 표준 | 게임/오픈소스권 인기 | 상대적으로 마이너 |
| 유지보수 | 활발 (Google 공식 관리) | 활발 | 활발하지만 상대적으로 소규모 |

## Maintenance Status

- 최신 릴리스: 1.17.0. 1.17.x부터 C++17 이상 요구 — 이 프로젝트의 `CMAKE_CXX_STANDARD 17`과 일치.
- 이슈 트래커에 2026-01까지도 신규 이슈가 등록되고 있어 활발히 유지보수 중임을 확인.
- Google이 공식으로 관리하는 저장소로, 저장소 자체가 아카이브되거나 방치된 상태 아님.

## License

- BSD-3-Clause. Permissive, 카피레프트 없음.
- 이 프로젝트(개인 학습/포트폴리오, 상업적 재배포 없음)와 라이선스 충돌 없음.

## Risks

- 컴파일 시간이 Catch2/doctest 대비 가장 느림 — 다만 현재 테스트 대상 로직이 argv 파싱 하나뿐이라 체감 영향은 미미할 것으로 예상.
- CMake `FetchContent`로 소스를 받아오므로 최초 빌드 시 네트워크 필요 — 오프라인 빌드가 필요해지면 별도 vendoring 검토 필요(현재 범위 아님).
- 알려진 보안 취약점 없음(테스트 전용 도구라 프로덕션 바이너리에 포함되지 않음 — 배포용 `main.exe`에는 링크되지 않음).

---

# Step 2 — Wrapper Design Draft

**일반 원칙과의 차이를 먼저 밝힘 — 사용자 확인 필요.**

CLAUDE.md 핵심 원칙은 "외부 라이브러리는 반드시 내부 wrapper 클래스로 감싸고, 다른 모듈이 외부 라이브러리를 직접 참조하지 않게 한다"임.
이 원칙은 DirectX/OpenGL처럼 **프로덕션 코드가 의존하는** 라이브러리에는 그대로 적용됨(`IRenderer` 래퍼로 이미 반영 중).

GoogleTest는 성격이 다름.

- 테스트 코드의 본질적인 목적 자체가 `TEST(...)`, `EXPECT_EQ(...)` 같은 프레임워크 매크로를 **직접** 표현하는 것 — 이를 또 다른 추상화로 감싸면 표준 관용구를 잃고 가독성만 떨어짐.
- "외부 라이브러리 교체 시 변경 범위 최소화"라는 wrapper의 목적도 테스트 프레임워크에는 약하게 적용됨 — 프레임워크를 바꾸면 테스트 코드 자체를 다시 써야 하는 건 wrapper 유무와 무관.
- 실제로 격리해야 할 지점은 "프로덕션 코드가 GoogleTest를 참조하지 않는 것"이며, 이는 클래스 래퍼가 아니라 **빌드 타겟 경계**로 달성 가능함.

**제안하는 격리 방식 (클래스 래퍼 대신 타겟 분리)**

- 신규 CMake 실행 타겟 `tests`를 만들어 `gtest`/`gtest_main`을 여기에만 링크.
- `main` 타겟(`src/main.cpp` 및 프로덕션 코드)은 GoogleTest에 대한 의존성이 전혀 없음 — `add_executable(main ...)`에는 GoogleTest가 링크되지 않음.
- `tests` 타겟은 프로덕션 코드 중 테스트 대상 단위(예: argv 파싱 함수)만 링크해서 검증.

이 방식으로 진행하는 것에 동의하는지 확인 필요 — 만약 클래스 래퍼 형태를 그대로 원한다면 별도로 논의.

---

# Step 3 — Verification Status

- `[x] Verified` — CMake `FetchContent`로 GoogleTest 1.17.0 소스를 받아 빌드에 통합 (`tests` 타겟에만 링크, `main`/`app_lib`은 무의존 확인)
- `[x] Verified` — `enable_testing()` + `gtest_discover_tests`로 CTest 연동, `tests.exe` 직접 실행으로 4개 케이스 전부 통과 확인
- `[x] Verified` — 기본 assertion 매크로(`TEST`, `EXPECT_EQ`, `EXPECT_THROW`) 컴파일 및 통과
- `[x] Verified` — 현재 프로젝트 툴체인(MSVC 19.37, Ninja)에서 정상 빌드 (`/utf-8` 플래그 추가로 한글 주석 관련 C4819 경고까지 해소)

검증은 `docs/strategy/렌더러_분기_20260715_2248.md` 구현 단계에서 수행 완료.
