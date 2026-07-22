#include <gtest/gtest.h>

#include "entity_component/EntityManager.h"

TEST(EntityManagerTest, CreateEntityReturnsRetrievableEntity)
{
    EntityManager manager;

    IEntity& created = manager.CreateEntity();
    IEntity* found = manager.FindEntity(created.GetId());

    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found, &created);
}

TEST(EntityManagerTest, EachCreatedEntityReceivesAUniqueId)
{
    EntityManager manager;

    IEntity& first = manager.CreateEntity();
    IEntity& second = manager.CreateEntity();

    EXPECT_NE(first.GetId(), second.GetId());
}

TEST(EntityManagerTest, FindEntityReturnsNullptrForUnknownId)
{
    EntityManager manager;
    const EntityId unknownId = static_cast<EntityId>(static_cast<std::uint64_t>(-1));
    EXPECT_EQ(manager.FindEntity(unknownId), nullptr);
}

TEST(EntityManagerTest, DestroyEntityRemovesItFromSubsequentLookups)
{
    EntityManager manager;

    IEntity& created = manager.CreateEntity();
    const EntityId id = created.GetId();

    manager.DestroyEntity(id);

    EXPECT_EQ(manager.FindEntity(id), nullptr);
}

TEST(EntityManagerTest, DestroyEntityOnUnknownIdIsNoOp)
{
    EntityManager manager;
    const EntityId unknownId = static_cast<EntityId>(static_cast<std::uint64_t>(-1));
    manager.DestroyEntity(unknownId);
}
