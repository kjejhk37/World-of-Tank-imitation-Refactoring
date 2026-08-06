# Geometry — 모든 충돌 감지 알고리즘의 공용 입력 타입

이 뒤에 나오는 모든 알고리즘(GJK/EPA, BVH, R-Tree, Voxel, Spatial Hashing, Octree, k-d Tree, MPR, Conservative Advancement, FCL)이 공통으로 의존하는 기반 타입이다.
알고리즘 각각의 문서를 읽기 전에 이 문서를 먼저 읽으면 "지지 함수(Support Function)"와 "경계 상자(Bounds)"라는 두 개념이 왜 반복해서 등장하는지 이해할 수 있다.

## 왜 필요한가

충돌 감지 알고리즘은 원래 "구 대 구", "박스 대 캡슐" 처럼 도형 쌍마다 전용 수식을 쓸 수도 있다.
하지만 그렇게 하면 도형이 6종(Sphere/AABB/OBB/Capsule/Cylinder/Mesh)일 때 쌍의 개수가 21개(6×7/2)나 되고, 알고리즘마다 이 21개를 전부 새로 구현해야 한다.
GJK 같은 범용 알고리즘은 "이 도형에서 특정 방향으로 가장 먼 점이 어디인가"라는 질문 하나만 답할 수 있으면 도형 종류에 상관없이 동작한다 — 이 질문에 답하는 함수가 지지 함수(Support Function)다.

## 핵심 타입과 함수 (`src/platform/geometry/Geometry.h`)

```cpp
using Geometry = std::variant<Sphere, AABB, OBB, Capsule, Cylinder, Mesh>;

Vec3 Support(const Geometry& geometry, const Vec3& direction);
AABB GetBounds(const Geometry& geometry);
```

- `Geometry`는 6종 도형 중 하나를 담는 `std::variant`다.
- `Support(geometry, direction)`은 `direction` 방향으로 그 도형 위에서 가장 먼 점을 반환한다.
- `GetBounds(geometry)`는 그 도형을 감싸는 축 정렬 경계 상자(AABB)를 반환한다.

## 지지 함수(Support)가 하는 일

각 도형마다 "그 방향으로 가장 먼 점"을 구하는 방법이 다르다.

- **Sphere**: 중심에서 `direction` 방향으로 반지름만큼 이동한 점.
- **AABB**: `direction`의 부호에 따라 각 축에서 min 또는 max를 고른 점(모서리).
- **OBB**: 로컬 축마다 `direction`과의 내적 부호로 half-extent를 더하거나 빼서 만든 점(모서리).
- **Capsule**: 두 끝점 중 `direction` 쪽에 가까운 것 + 반지름만큼 `direction`으로 이동.
- **Cylinder**: 축 방향 성분으로 캡(끝면)을 고르고, 축에 수직인 성분 방향으로 반지름만큼 확장.
- **Mesh**: 모든 정점 중 `direction`과의 내적이 가장 큰 정점.

GJK/EPA/MPR은 전부 이 `Support()` 하나만 호출해서 동작한다 — 그래서 새 도형이 추가돼도 GJK/EPA/MPR 코드는 한 줄도 안 바꿔도 된다(대신 `Geometry.cpp`의 `Overload` 목록에 그 도형의 지지 함수만 추가하면 됨).

## 경계 상자(GetBounds)가 하는 일

BVH/R-Tree/Voxel/Spatial Hashing/Octree/k-d Tree/Conservative Advancement는 도형의 정확한 형태 대신 "그 도형을 감싸는 상자"만 필요로 한다 — 진짜 형태를 알 필요 없이 "이 근처에 뭐가 있는지"만 빠르게 좁히면 되기 때문이다(broad-phase).
`GetBounds()`는 그 근사치를 만들어준다.

## 왜 `std::variant`로 만들었나 (설계 결정)

원래 흔한 방식은 `class IGeometry { virtual Vec3 Support(...) = 0; ... }`처럼 가상 함수 기반 추상 클래스를 만들고 6개 도형이 전부 상속받게 하는 것이다.
이 프로젝트는 대신 `std::variant` + 자유 함수(`Support`/`GetBounds`)를 선택했다.

| | 가상 클래스(IGeometry) | std::variant(이 프로젝트가 선택) |
|---|---|---|
| 저장 | 포인터/힙 할당 필요 | 값으로 저장(스택/컨테이너에 바로) |
| 호출 비용 | 가상 호출(vtable 조회) | 컴파일 타임에 분기(`std::visit`) |
| 새 도형 추가 | 새 파생 클래스만 추가하면 됨 | `Geometry` 별칭 + 모든 `Overload` 목록에 케이스 추가 필요 |
| 메모리 크기 | 도형 크기만큼(포인터는 항상 작음) | 가장 큰 대안(`Mesh`) 크기로 고정 |

두 방식 다 정답이 있는 게 아니라 트레이드오프다.
이 프로젝트는 "값으로 다루기 쉬움 + 기존 코드베이스가 이미 쓰던 데이터 구조체+자유 함수 패턴과 일관성"을 우선했다.

## 접근 규칙(중요)

**`Geometry.h`/`Geometry.cpp` 밖의 어떤 코드도 `std::visit`나 `std::get`을 직접 호출하지 않는다.**
모든 알고리즘은 반드시 `Support()`/`GetBounds()` 두 자유 함수로만 도형에 접근한다.
이렇게 해두면 나중에 정말로 가상 클래스 방식으로 바꿔야 할 일이 생겨도, 바뀌는 범위가 `Geometry.cpp` 파일 하나 + 도형을 컬렉션으로 저장하는 소수 지점으로 국한된다(전체 알고리즘 코드는 그대로 컴파일됨).

## 코드 위치

- `src/platform/geometry/Geometry.h`, `src/platform/geometry/Geometry.cpp`
- 테스트: `tests/GeometryTest.cpp`
