#pragma once

#include <string>

#include "platform/model_import/Model.h"

// Author: Claude
// Description: 모델 파일 임포트의 공개 API. 상태 없는 네임스페이스 정적 함수로 구성되며, 실제
//              라이브러리 호출은 각 포맷의 Wrapper 클래스(TinyObjWrapper/UfbxWrapper/CgltfWrapper)에
//              위임하는 얇은 퍼사드다.
// Input: 각 함수 - 모델 파일 경로
// Output: 각 함수 - 파싱된 Model
// Notes: 이 네임스페이스는 IEntity/IComponent/EntityManager를 전혀 참조하지 않는다 - ECS와 완전히
//        독립된 에셋 로딩 계층이다. 캐싱/저장 책임도 갖지 않는다 - 필요하면 MeshManager를 통해 호출한다.
// Date: 2026-07-23
namespace ModelLoader
{
Model LoadOBJ(const std::string& path);
Model LoadFBX(const std::string& path);
Model LoadGLB(const std::string& path);
Model LoadGLTF(const std::string& path);
}  // namespace ModelLoader
