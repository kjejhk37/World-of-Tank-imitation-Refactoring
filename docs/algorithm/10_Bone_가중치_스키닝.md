# Bone 가중치와 스키닝(Skinning) — 정점이 뼈대를 따라 움직이는 방법

**분류**: 메쉬 애니메이션 데이터 구조 (충돌 감지 알고리즘은 아니지만, 아래 "충돌 검사와의 연결" 절 때문에 이 폴더에 둔다).
**선행 지식**: [00_Geometry_공용입력타입.md](00_Geometry_공용입력타입.md)의 `Support()`/`GetBounds()`, `Geometry` variant.
**관련 사이클**: `docs/brainstorming/모델임포터_Mesh저장_20260723_2123.md` — 이번 사이클에서는 스코프 제외, 이후 스켈레탈 애니메이션 도입 시 다시 참조하기 위해 미리 작성.

## 스키닝이 푸는 문제

탱크의 차체·포탑·포신처럼 "부품 전체가 강체로 통째로 움직이는" 애니메이션은 어렵지 않다 — 부품마다 `Node`(로컬 Transform)를 하나씩 두고, 그 Transform을 매 프레임 바꾸면 된다 (예: 포탑 노드의 회전값만 바꾸면 포탑 전체가 같이 돈다). 이 프로젝트의 `Model{meshes[], rootNode}` 구조가 이미 이 방식을 전제로 설계되어 있다.

문제는 **부품 하나가 여러 방향으로 동시에 구부러져야 하는 경우**다. 예를 들어 사람 팔뚝이 팔꿈치에서 접힐 때, 팔뚝 메쉬의 정점들은 "팔뚝 뼈에 100% 붙어서" 움직이지 않는다 — 팔꿈치 근처 정점은 팔뚝 뼈와 위팔 뼈 **양쪽 영향을 섞어서** 움직여야 접히는 부분이 매끄럽다(한쪽 뼈에만 100% 붙이면 관절이 꺾이는 지점에서 메쉬가 찢어지거나 뾰족하게 튀어나오는 "캔디래퍼(candy-wrapper)" 아티팩트가 생긴다). **스키닝(Skinning)**은 "정점 하나가 여러 뼈의 영향을 가중 평균으로 섞어서 따라가게 만드는" 기법이다.

## 스켈레톤과 바인드 포즈

- **스켈레톤(Skeleton)**: 뼈(Bone/Joint)들이 부모-자식 계층으로 연결된 구조. 이 프로젝트의 `Node` 계층과 개념적으로 같다(다만 스켈레톤의 Node는 렌더링 대상 Mesh 대신 "관절"을 표현한다).
- **바인드 포즈(Bind Pose)**: 메쉬를 처음 스켈레톤에 "입힐" 때의 기준 자세(보통 T-포즈). 이 순간의 각 뼈의 월드 Transform을 `BindPoseMatrix`라고 하자.
- **역바인드 행렬(Inverse Bind Matrix)**: `BindPoseMatrix`의 역행렬. 정점의 월드 좌표를 "그 뼈 기준의 로컬 좌표"로 되돌리는 데 쓴다 — 스키닝 계산에 반드시 필요하다(아래 공식 참고).
- 애니메이션이 재생되면 각 뼈의 현재 월드 Transform(`CurrentBoneMatrix`)이 매 프레임 바뀐다. 스키닝은 "바인드 포즈에서 지금 자세로 얼마나 움직였는가"를 정점에 적용하는 과정이다.

## Bone 가중치 데이터 구조

정점 하나마다 다음 정보를 저장한다.

```cpp
struct VertexBoneData {
    uint32_t boneIndices[4];  // 이 정점에 영향을 주는 뼈의 인덱스 (최대 4개)
    float    boneWeights[4];  // 각 뼈가 이 정점에 얼마나 영향을 주는지 (합 = 1.0)
};
```

- `boneWeights`의 합은 항상 1.0으로 **정규화**되어야 한다. 합이 1.0이 아니면 스케일이 깨지거나(합 < 1: 메쉬가 뼈대 쪽으로 쪼그라듦) 메쉬가 부풀어 보이는 버그가 생긴다.
- 영향이 4개보다 적은 정점(대부분의 정점은 뼈 1~2개면 충분)은 남는 슬롯의 weight를 0으로 채운다.

## 계산 공식 — Linear Blend Skinning (LBS)

가장 널리 쓰이는 스키닝 방식이다 (Unity/Unreal 모두 기본값으로 이 방식을 쓴다).

```
SkinnedPosition = Σ_{i=0}^{3} ( weight[i] * CurrentBoneMatrix[boneIndices[i]] * InverseBindMatrix[boneIndices[i]] * OriginalPosition )
```

읽는 순서:

1. `InverseBindMatrix[i]`로 정점의 월드 좌표를 "그 뼈 기준 로컬 좌표"로 되돌린다.
2. `CurrentBoneMatrix[i]`로 "그 뼈가 지금 어디에 있는지"를 곱해 다시 월드 좌표로 가져온다.
3. 이걸 영향을 주는 뼈 개수만큼(최대 4개) 계산해서 `weight`로 가중 평균한다.

노멀도 같은 방식으로 변환하되, 이동(translation) 성분은 제외하고 회전 성분만 적용한다(노멀은 방향이지 위치가 아니므로).

## 정점당 본 개수를 몇 개로 할 것인가 — 4 vs 8 vs 256

실사용 엔진 조사 결과(Unity/Unreal 공식 문서 기준):

- **Unity**: 과거엔 정점당 최대 4개 고정(`BoneWeight` 구조체, `Mesh.boneWeights`). 최신 버전은 `BoneWeight1` + `Mesh.GetAllBoneWeights()`로 정점당 최대 256개까지 가변 지원.
- **Unreal**: "Default Bone Influences" 설정으로 플랫폼에 따라 정점당 4개 또는 8개.

**왜 4개가 오랫동안 표준이었나**: GPU 버텍스 셰이더가 정점마다 본 인덱스/가중치를 상수 레지스터나 텍스처에서 읽어와야 하는데, 이 개수가 늘어날수록 셰이더 연산량과 대역폭이 늘어난다. 대부분의 캐릭터 관절(팔꿈치, 무릎 등)은 뼈 2~3개의 영향만으로도 충분히 매끄럽고, 4개면 거의 모든 실사용 케이스를 커버한다. Unity가 최근 256개까지 늘린 건 얼굴 애니메이션처럼 아주 세밀한 근육 시뮬레이션 등 특수한 경우를 위해서다.

## 실전에서 흔히 만나는 문제

- **가중치 정규화 누락**: 아티스트 툴에서 내보낸 원본 가중치 합이 정확히 1.0이 아닌 경우가 흔하다(부동소수점 오차, 툴 버그). 임포트 시 반드시 재정규화(`weight[i] /= sum(weights)`)해야 한다.
- **바인드 포즈 불일치**: 메쉬를 내보낼 때의 바인드 포즈와 애니메이션 데이터의 바인드 포즈가 다르면(예: 다른 소프트웨어에서 재수출) 스키닝 결과가 뒤틀린다.
- **영향 개수 초과 시 잘림**: 5개 이상의 뼈 영향을 받는 정점을 4개로 자르면, 잘린 뼈의 영향이 사라지면서 그 부분만 이상하게 움직일 수 있다(주로 자동 리깅 툴이 만든 메쉬에서 발생) — 잘라낸 뒤 반드시 남은 가중치를 재정규화해야 한다.

## 이 프로젝트에서의 설계 방침

- 이번 "모델 Importer" 사이클에서는 **본/스키닝 데이터를 Mesh 필드에 포함하지 않기로 결정**했다 (`docs/task/모델임포터_Mesh저장_20260723_2123.md` 참고). 이유:
  - 탱크의 차체/포탑/포신 같은 강체 부품은 `Node` + Transform만으로 충분하다.
  - 캐터필러(무한궤도)는 UV 스크롤(텍스처 좌표를 시간에 따라 이동시키는 셰이더 기법, 지오메트리는 전혀 건드리지 않음)로 구현하기로 했으므로, 이 경우도 본 데이터가 필요 없다.
- 향후 진짜 스켈레탈 애니메이션이 필요해지면, `VertexBoneData`(정점당 최대 **4개** 고정 — Unity의 가변 256개 방식 대신 단순한 고정 크기 채택)를 `Mesh`에 필드로 추가한다. 이건 기존 필드에 멤버를 더하는 수준의 낮은 리스크 변경이다.

## 충돌 검사와의 연결 (왜 미리 적어두는가)

이 프로젝트의 충돌 감지 파이프라인([00_Geometry_공용입력타입.md](00_Geometry_공용입력타입.md))은 `Support()`/`GetBounds()`가 **정적인** 도형(Sphere/AABB/OBB/Capsule/Cylinder/Mesh)을 전제로 설계되어 있다. `Mesh` 케이스의 `Support()`는 "모든 정점 중 특정 방향과의 내적이 가장 큰 정점"을 찾는데, 스키닝된 메쉬는 애니메이션에 따라 정점 위치가 매 프레임 바뀌므로 이 가정이 깨진다.

두 가지 대응 방법이 있다.

1. **매 프레임 스키닝 계산 후 `Support()`에 최신 정점을 넘긴다** — 정확하지만 정점 수만큼 매 프레임 스키닝 연산을 해야 해서 비용이 크고, GJK/EPA 같은 반복 알고리즘이 프레임당 여러 번 `Support()`를 호출하는 것과 맞물려 더 비싸진다.
2. **뼈마다 단순 도형(Capsule/Sphere)을 붙이고, 그 도형들로 충돌 판정한다** — Unity/Unreal의 래그돌(ragdoll) 물리가 실제로 쓰는 방식이다. 각 뼈의 `CurrentBoneMatrix`로 Capsule의 Transform만 갱신하면 되고, **이 프로젝트에 이미 있는 `Geometry` variant와 `Support()`/`GetBounds()`를 그대로 재사용할 수 있다** — `Mesh` 케이스를 애니메이션 인식하게 고칠 필요가 전혀 없다.

즉 스켈레탈 애니메이션과 충돌 검사를 나중에 연결할 때는, 이 문서의 `VertexBoneData`/`CurrentBoneMatrix`를 이용해 뼈마다 Capsule/Sphere 프록시를 만들고 그 Transform을 매 프레임 갱신하는 방식을 우선 검토해야 한다는 것을 미리 기록해둔다.

## 참고

- Unity 공식 문서 — [Mesh vertex data](https://docs.unity3d.com/6000.4/Documentation/Manual/mesh-vertex-data.html), [Mesh.boneWeights](https://docs.unity3d.com/ScriptReference/Mesh-boneWeights.html)
- Unreal Engine 공식 문서 — [Skeletal Mesh Rendering Paths](https://dev.epicgames.com/documentation/unreal-engine/skeletal-mesh-rendering-paths-in-unreal-engine)
