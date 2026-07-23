#include <gtest/gtest.h>

#include "math/Quaternion.h"
#include "model_import/NaiveSignatureDeduplicator.h"

namespace
{

std::shared_ptr<const ModelMesh> MakeTriangle(const Vec3& p0, const Vec3& p1, const Vec3& p2)
{
    auto mesh = std::make_shared<ModelMesh>();
    mesh->positions = {p0, p1, p2};
    mesh->indices = {0, 1, 2};
    mesh->normals = {Vec3::Zero(), Vec3::Zero(), Vec3::Zero()};
    mesh->uvChannels.resize(1);
    mesh->uvChannels[0] = {Vec2(0.0f, 0.0f), Vec2(0.0f, 0.0f), Vec2(0.0f, 0.0f)};
    return mesh;
}

// baseTriangle을 회전/이동/균등 스케일한, 기하학적으로 합동인 삼각형을 만든다.
std::shared_ptr<const ModelMesh> MakeTransformedCopy(const std::shared_ptr<const ModelMesh>& base, const Quaternion& rotation,
                                                 const Vec3& translation, float scale)
{
    auto mesh = std::make_shared<ModelMesh>(*base);
    for (Vec3& p : mesh->positions)
    {
        p = rotation.Rotate(p) * scale + translation;
    }
    return mesh;
}

}  // namespace

TEST(NaiveSignatureDeduplicatorTest, FirstRegistrationReturnsTheSameCandidate)
{
    NaiveSignatureDeduplicator dedup;
    const std::shared_ptr<const ModelMesh> triangle = MakeTriangle(Vec3(0, 0, 0), Vec3(1, 0, 0), Vec3(0, 1, 0));

    EXPECT_EQ(dedup.FindOrRegister(triangle), triangle);
}

TEST(NaiveSignatureDeduplicatorTest, RotatedTranslatedScaledCopyIsRecognizedAsCongruent)
{
    NaiveSignatureDeduplicator dedup;
    const std::shared_ptr<const ModelMesh> original = MakeTriangle(Vec3(0, 0, 0), Vec3(1, 0, 0), Vec3(0, 1, 0));
    dedup.FindOrRegister(original);

    const std::shared_ptr<const ModelMesh> transformedCopy =
        MakeTransformedCopy(original, Quaternion::FromAxisAngle(Vec3(0, 0, 1), 37.0f), Vec3(5, -3, 2), 2.5f);

    EXPECT_EQ(dedup.FindOrRegister(transformedCopy), original);
}

TEST(NaiveSignatureDeduplicatorTest, GeometricallyDifferentMeshIsNotConsideredCongruent)
{
    NaiveSignatureDeduplicator dedup;
    const std::shared_ptr<const ModelMesh> triangle = MakeTriangle(Vec3(0, 0, 0), Vec3(1, 0, 0), Vec3(0, 1, 0));
    dedup.FindOrRegister(triangle);

    const std::shared_ptr<const ModelMesh> differentShape =
        MakeTriangle(Vec3(0, 0, 0), Vec3(4, 0, 0), Vec3(0, 1, 0));

    EXPECT_EQ(dedup.FindOrRegister(differentShape), differentShape);
}
