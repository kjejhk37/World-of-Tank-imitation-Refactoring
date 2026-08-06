#pragma once

#include <string>

#include "platform/model_import/Model.h"

// Author: Claude
// Description: cgltf(외부 라이브러리)에 대한 유일한 직접 참조 지점. GLTF/GLB 파일을 읽어 공통
//              Model 구조(고유 ModelMesh 목록 + 노드 계층)로 변환한다.
// Input: Load - GLTF 또는 GLB 파일 경로
// Output: Load - 파싱된 Model (실패 시 예외 std::runtime_error)
// Notes: cgltf.h를 include하는 곳은 이 .cpp 하나뿐이다 (CGLTF_IMPLEMENTATION 매크로도 여기서만 정의).
//        glTF는 씬(cgltf_scene)이 여러 루트 노드를 가질 수 있으나, 이 프로젝트의 Model::rootNode는
//        단일 루트를 전제로 하므로 루트 노드가 여럿이면 identity Transform의 합성 루트 노드로 감싼다.
//        glTF의 하나의 mesh가 여러 primitive(프리미티브별 별도 머티리얼)를 가질 수 있는데, 이번
//        사이클은 한 mesh의 모든 primitive를 정점/인덱스 버퍼 하나로 합치고 materialRef는 첫
//        primitive의 머티리얼 이름으로 단순화한다(ModelMesh.h의 동일한 제약과 일관).
//        내부적으로 라이브러리 객체(cgltf_data 등)를 함수 호출마다 지역 변수로만 생성하므로
//        전역/정적 mutable state가 없고, 여러 스레드에서 서로 다른 파일을 동시에 Load해도 안전하다.
//        `has_matrix` 노드는 shear와 미러링(반사, 음수 행렬식)이 없다고 가정한 간이 분해로 처리한다 -
//        반사가 포함된 노드는 반사가 빠진 잘못된 회전으로 조용히 변환된다(이번 사이클 스코프 밖).
// Date: 2026-07-23
class CgltfWrapper
{
public:
    Model Load(const std::string& path) const;
};
