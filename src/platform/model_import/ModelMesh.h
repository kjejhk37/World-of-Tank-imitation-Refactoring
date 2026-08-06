#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "platform/math/Vec2.h"
#include "platform/math/Vec3.h"

// Author: Claude
// Description: 포맷(OBJ/FBX/GLTF/GLB)에 무관한 공통 지오메트리 데이터. 정점/인덱스/노멀/UV만 담는
//              순수 데이터 구조이며, ECS/렌더링/외부 라이브러리 어느 것도 참조하지 않는다.
// Input: (해당 없음 - 데이터 구조체)
// Output: (해당 없음 - 데이터 구조체)
// Notes: 탄젠트/바이탄젠트는 저장하지 않는다 - 노멀맵이 필요해지는 시점에 런타임 계산한다
//        (docs/algorithm/10_Bone_가중치_스키닝.md와 같은 이유로, 본 가중치 필드도 이번 사이클엔 없다).
//        uvChannels는 최대 8채널(Unity/Unreal과 동일 상한)까지 확장 가능하도록 벡터의 벡터로 두되,
//        이번 사이클은 채널 0(uvChannels[0])만 채운다.
//        materialRef는 실제 텍스처 로딩을 하지 않는 참조(이름)일 뿐이다 - 여러 머티리얼을 쓰는
//        원본 지오메트리(glTF의 멀티 프리미티브 메쉬 등)는 이번 사이클에서 첫 번째 머티리얼 이름으로
//        단순화한다(CgltfWrapper 참고).
// Date: 2026-07-23
struct ModelMesh
{
    std::vector<Vec3> positions;
    std::vector<uint32_t> indices;
    std::vector<Vec3> normals;
    std::vector<std::vector<Vec2>> uvChannels;
    std::optional<std::string> materialRef;
};
