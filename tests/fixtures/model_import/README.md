# model_import 테스트 픽스처

- `cube.obj` — 이번 사이클에서 직접 작성한 8정점 큐브(OBJ). `TinyObjWrapper`/`ModelLoader::LoadOBJ` 테스트용.
- `cube.fbx` — [ufbx](https://github.com/ufbx/ufbx) 프로젝트의 자체 테스트 데이터(`data/blender_279_default_6100_ascii.fbx`, MIT License)에서 그대로 가져온 샘플. `UfbxWrapper`/`ModelLoader::LoadFBX` 테스트용. FBX ASCII 포맷을 직접 손으로 작성하는 것은 신뢰도가 낮아, 실제 Blender가 내보낸 검증된 샘플을 재사용했다.
- `triangle.gltf` — 이번 사이클에서 직접 작성한 삼각형 1개짜리 glTF(임베디드 base64 버퍼). `CgltfWrapper`/`ModelLoader::LoadGLB`·`LoadGLTF` 기본 동작 테스트용.
- `instancing.gltf` — 같은 메쉬(mesh index 0)를 서로 다른 위치의 노드 2개가 참조하는 glTF. 씬 그래프 내부 인스턴싱(flatten 없이 보존되는지) 검증용.
