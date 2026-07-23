#include "model_import/ModelLoader.h"

#include <utility>

#include "model_import/CgltfWrapper.h"
#include "model_import/TinyObjWrapper.h"
#include "model_import/UfbxWrapper.h"

namespace ModelLoader
{

Model LoadOBJ(const std::string& path)
{
    const TinyObjWrapper wrapper;
    ModelMesh mesh = wrapper.Load(path);

    Model model;
    model.meshes.push_back(std::make_shared<const ModelMesh>(std::move(mesh)));
    model.rootNode.name = "root";
    model.rootNode.meshIndex = 0;
    return model;
}

Model LoadFBX(const std::string& path)
{
    const UfbxWrapper wrapper;
    return wrapper.Load(path);
}

Model LoadGLB(const std::string& path)
{
    const CgltfWrapper wrapper;
    return wrapper.Load(path);
}

Model LoadGLTF(const std::string& path)
{
    const CgltfWrapper wrapper;
    return wrapper.Load(path);
}

}  // namespace ModelLoader
