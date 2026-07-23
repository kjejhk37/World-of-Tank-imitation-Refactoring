#pragma once

#include <memory>
#include <vector>

#include "model_import/ModelMesh.h"
#include "model_import/Node.h"

// Author: Claude
// Description: 모델 파일 하나를 로드한 결과 전체. 고유 지오메트리 목록(meshes)과 이를 참조하는
//              노드 계층(rootNode)으로 구성된다.
// Input: (해당 없음 - 데이터 구조체)
// Output: (해당 없음 - 데이터 구조체)
// Notes: meshes는 ModelMesh 값 복사가 아니라 shared_ptr<const ModelMesh> 핸들 목록이다 - Node가 같은
//        인덱스를 참조하면 그 Mesh는 메모리상 단 한 번만 존재한다(docs/brainstorming/
//        모델임포터_Mesh저장_20260723_2123.md "원본 Mesh는 어디에 저장되는가" 절 참고).
//        MeshManager를 거치지 않고 ModelLoader를 직접 호출해 얻은 Model은 이 파일 내부의
//        인스턴싱만 보존하며, 다른 파일과의 교차 중복 제거(IMeshDeduplicator)는 적용되지 않는다.
// Date: 2026-07-23
struct Model
{
    std::vector<std::shared_ptr<const ModelMesh>> meshes;
    Node rootNode;
};
