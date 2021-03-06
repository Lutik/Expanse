#include "gtest/gtest.h"

#include "ECS/Entity.h"
#include "ECS/World.h"

namespace Expanse::Tests
{
	TEST(ECS, EntityVersionIncAndOverflow)
	{
		const ecs::Entity::BaseType Index = 7;
		ecs::Entity ent{ Index };

		const auto mv = ecs::Entity::MaxVersion + 1;

		for (size_t v = 0; v <= mv; ++v)
		{
			EXPECT_EQ(Index, ent.Index());
			EXPECT_EQ(v % mv, ent.Version());

			ent.IncVersion();
		}
	}

	TEST(ECS, EntityAddAndRemove)
	{
		ecs::Entity ent;
		ecs::World world;

		EXPECT_FALSE(world.HasEntity(ent));

		ent = world.CreateEntity();

		EXPECT_TRUE(world.HasEntity(ent));

		world.DestroyEntity(ent);

		EXPECT_FALSE(world.HasEntity(ent));

		auto ent2 = world.CreateEntity();

		EXPECT_FALSE(world.HasEntity(ent));
	}


	struct CompA {
		int x = 0;
	};

	struct CompB {
		float v = 0.0f;
	};

	TEST(ECS, ComponentAddAndRemove)
	{
		ecs::World world;
		auto ent = world.CreateEntity();

		auto* a = world.AddComponent<CompA>(ent);
		a->x = 17;

		auto* b = world.GetComponent<CompA>(ent);

		EXPECT_EQ(a, b);
		EXPECT_EQ(17, b->x);

		world.RemoveComponent<CompA>(ent);

		auto* c = world.GetComponent<CompA>(ent);
		EXPECT_EQ(nullptr, c);
	}

	TEST(ECS, MultiComponentGet)
	{
		ecs::World world;
		auto ent = world.CreateEntity();

		auto* a = world.AddComponent<CompA>(ent);
		auto* b = world.AddComponent<CompB>(ent);

		auto [a1, b1] = world.GetComponents<CompA, CompB>(ent);

		const auto& cworld = world;
		auto [a2, b2] = cworld.GetComponents<CompA, CompB>(ent);

		EXPECT_EQ(a, a1);
		EXPECT_EQ(b, b1);
		EXPECT_EQ(a, a2);
		EXPECT_EQ(b, b2);
	}

	TEST(ECS, ForEach)
	{
		ecs::World world;
		auto ent0 = world.CreateEntity();
		auto ent1 = world.CreateEntity();
		auto ent2 = world.CreateEntity();

		world.AddComponent<CompA>(ent0, 1);

		world.AddComponent<CompA>(ent1, 2);
		world.AddComponent<CompB>(ent1, 2.0f);

		world.AddComponent<CompA>(ent2, 3);
		world.AddComponent<CompB>(ent2, 3.0f);


		world.ForEach<CompA, CompB>([](ecs::Entity ent, CompA& a, CompB& b) {
			a.x = 0;
			b.v = 0.0f;
		});

		auto* a = world.GetComponent<CompA>(ent0);
		EXPECT_EQ(1, a->x);

		auto [a1, b1] = world.GetComponents<CompA, CompB>(ent1);
		EXPECT_EQ(0, a1->x);
		EXPECT_EQ(0.0f, b1->v);

		auto [a2, b2] = world.GetComponents<CompA, CompB>(ent1);
		EXPECT_EQ(0, a2->x);
		EXPECT_EQ(0.0f, b2->v);
	}
}