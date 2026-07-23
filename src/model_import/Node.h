#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "math/Transform.h"

// Author: Claude
// Description: 씬 그래프의 한 노드. 로컬 Transform과 (있다면) 참조하는 Mesh의 인덱스, 자식 노드
//              목록을 갖는다. ModelMesh 자체는 들고 있지 않다 - Model::meshes의 인덱스만 참조한다.
// Input: (해당 없음 - 데이터 구조체)
// Output: (해당 없음 - 데이터 구조체)
// Notes: 같은 meshIndex를 여러 Node가 동시에 참조할 수 있다 - 이게 씬 그래프 내부 인스턴싱
//        (예: 탱크 바퀴 6개가 같은 지오메트리를 참조)을 파싱 단계에서 flatten 없이 보존하는 지점이다.
//        OBJ처럼 씬 그래프가 없는 포맷은 노드 1개(자식 없음, meshIndex=0)로 감싸 반환한다.
// Date: 2026-07-23
struct Node
{
    std::string name;
    Transform localTransform;
    std::optional<size_t> meshIndex;
    std::vector<Node> children;
};
