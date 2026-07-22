#include <gtest/gtest.h>

#include "entity_component/ComponentBase.h"
#include "entity_component/Entity.h"

namespace
{
    class DummyComponent : public ComponentBase
    {
    public:
        explicit DummyComponent(int value = 0) : m_value(value)
        {
        }

        int Value() const
        {
            return m_value;
        }

    private:
        int m_value;
    };

    class OtherComponent : public ComponentBase
    {
    };

    class TrackedComponent : public ComponentBase
    {
    public:
        explicit TrackedComponent(int* destructionCounter) : m_destructionCounter(destructionCounter)
        {
        }

        ~TrackedComponent() override
        {
            if (m_destructionCounter != nullptr)
            {
                ++(*m_destructionCounter);
            }
        }

    private:
        int* m_destructionCounter;
    };
}

TEST(EntityTest, AddComponentReturnsUsableReferenceAndIsRetrievableByType)
{
    Entity entity;

    DummyComponent& added = AddComponent<DummyComponent>(entity, 42);
    EXPECT_EQ(added.Value(), 42);

    DummyComponent* found = GetComponent<DummyComponent>(entity);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->Value(), 42);
    EXPECT_EQ(found, &added);
}

TEST(EntityTest, GetComponentReturnsNullptrWhenTypeNotPresent)
{
    Entity entity;
    EXPECT_EQ(GetComponent<DummyComponent>(entity), nullptr);
}

TEST(EntityTest, EntitySupportsMultipleComponentsOfTheSameType)
{
    Entity entity;

    AddComponent<DummyComponent>(entity, 1);
    AddComponent<DummyComponent>(entity, 2);
    AddComponent<DummyComponent>(entity, 3);

    const std::vector<DummyComponent*> found = GetComponents<DummyComponent>(entity);
    ASSERT_EQ(found.size(), 3u);

    int sum = 0;
    for (const DummyComponent* component : found)
    {
        sum += component->Value();
    }
    EXPECT_EQ(sum, 6);
}

TEST(EntityTest, ComponentsOfDifferentTypesDoNotInterfere)
{
    Entity entity;

    AddComponent<DummyComponent>(entity, 10);
    AddComponent<OtherComponent>(entity);

    EXPECT_NE(GetComponent<DummyComponent>(entity), nullptr);
    EXPECT_NE(GetComponent<OtherComponent>(entity), nullptr);
    EXPECT_EQ(GetComponents<DummyComponent>(entity).size(), 1u);
    EXPECT_EQ(GetComponents<OtherComponent>(entity).size(), 1u);
}

TEST(EntityTest, RemoveComponentDeletesOnlyTheTargetedInstance)
{
    Entity entity;

    DummyComponent& first = AddComponent<DummyComponent>(entity, 1);
    AddComponent<DummyComponent>(entity, 2);

    EXPECT_TRUE(entity.RemoveComponent(first.GetId()));
    EXPECT_EQ(GetComponents<DummyComponent>(entity).size(), 1u);
    EXPECT_EQ(GetComponents<DummyComponent>(entity).front()->Value(), 2);
}

TEST(EntityTest, RemovingAllComponentsOfATypeLeavesTypeQueryableAndEmpty)
{
    Entity entity;

    DummyComponent& only = AddComponent<DummyComponent>(entity, 5);
    EXPECT_TRUE(entity.RemoveComponent(only.GetId()));

    EXPECT_EQ(GetComponent<DummyComponent>(entity), nullptr);
    EXPECT_TRUE(GetComponents<DummyComponent>(entity).empty());

    DummyComponent& readded = AddComponent<DummyComponent>(entity, 9);
    EXPECT_EQ(GetComponent<DummyComponent>(entity), &readded);
}

TEST(EntityTest, RemoveComponentReturnsFalseForUnknownId)
{
    Entity entity;
    AddComponent<DummyComponent>(entity);

    const ComponentId unknownId = static_cast<ComponentId>(static_cast<std::uint64_t>(-1));
    EXPECT_FALSE(entity.RemoveComponent(unknownId));
}

TEST(EntityTest, DestroyingEntityDestroysAllOwnedComponents)
{
    int destructionCount = 0;

    {
        Entity entity;
        AddComponent<TrackedComponent>(entity, &destructionCount);
        AddComponent<TrackedComponent>(entity, &destructionCount);
        EXPECT_EQ(destructionCount, 0);
    }

    EXPECT_EQ(destructionCount, 2);
}

TEST(EntityTest, EachAddedComponentReceivesAUniqueId)
{
    Entity entity;

    DummyComponent& first = AddComponent<DummyComponent>(entity);
    DummyComponent& second = AddComponent<DummyComponent>(entity);

    EXPECT_NE(first.GetId(), second.GetId());
}

TEST(EntityTest, EachEntityReceivesAUniqueId)
{
    Entity a;
    Entity b;
    EXPECT_NE(a.GetId(), b.GetId());
}
