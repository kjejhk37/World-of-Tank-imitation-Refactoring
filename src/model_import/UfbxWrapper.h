#pragma once

#include <string>

#include "model_import/Model.h"

// Author: Claude
// Description: ufbx(외부 라이브러리)에 대한 유일한 직접 참조 지점. FBX 파일을 읽어 공통 Model
//              구조(고유 ModelMesh 목록 + 노드 계층)로 변환한다.
// Input: Load - FBX 파일 경로
// Output: Load - 파싱된 Model (실패 시 예외 std::runtime_error)
// Notes: ufbx.h를 include하는 곳은 이 .cpp 하나뿐이다.
//        FBX는 씬 그래프가 있으므로, 같은 ufbx_mesh를 참조하는 여러 ufbx_node를 flatten하지 않고
//        Model::meshes에 고유 지오메트리 하나로, Node 여러 개가 그 인덱스를 공유하는 형태로 변환한다.
//        내부적으로 라이브러리 객체(ufbx_scene 등)를 함수 호출마다 지역 변수로만 생성하므로
//        전역/정적 mutable state가 없고, 여러 스레드에서 서로 다른 파일을 동시에 Load해도 안전하다.
// Date: 2026-07-23
class UfbxWrapper
{
public:
    Model Load(const std::string& path) const;
};
