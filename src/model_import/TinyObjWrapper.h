#pragma once

#include <string>

#include "model_import/ModelMesh.h"

// Author: Claude
// Description: tinyobjloader(외부 라이브러리)에 대한 유일한 직접 참조 지점. OBJ 파일을 읽어
//              공통 ModelMesh 구조로 변환한다.
// Input: Load - OBJ 파일 경로
// Output: Load - 파싱된 ModelMesh (실패 시 예외 std::runtime_error)
// Notes: tiny_obj_loader.h를 include하는 곳은 이 .cpp 하나뿐이다 - 다른 모듈은 이 클래스를 통해서만
//        OBJ를 읽는다(agent_harness/CLAUDE.md의 wrapper 클래스 원칙).
//        내부적으로 라이브러리 객체(attrib_t/shape_t 등)를 함수 호출마다 지역 변수로만 생성하므로
//        전역/정적 mutable state가 없고, 여러 스레드에서 서로 다른 파일을 동시에 Load해도 안전하다.
//        OBJ는 씬 그래프가 없으므로 ModelMesh 하나만 반환한다 - Model로 감싸는 것은 ModelLoader::LoadOBJ의 책임이다.
// Date: 2026-07-23
class TinyObjWrapper
{
public:
    ModelMesh Load(const std::string& path) const;
};
