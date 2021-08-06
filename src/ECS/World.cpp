#include "World.h"

namespace Expanse::ecs
{
	Entity World::CreateEntity()
	{
		if (free_entities.empty())
		{
			entities.emplace_back(entities.size());
			return entities.back().entity;
		}
		else
		{
			const auto free_idx = free_entities.back();
			free_entities.pop_back();
			return entities[free_idx].entity;
		}
	}

	void World::DestroyEntity(Entity entity)
	{
		assert(HasEntity(entity));

		// TODO: remove all components

		const auto idx = entity.Index();
		entities[idx].entity.IncVersion();
		free_entities.push_back(idx);
	}

	bool World::HasEntity(Entity entity) const
	{
		const auto idx = entity.Index();
		return (idx < entities.size())
			&& (entities[idx].entity == entity)
			&& (std::ranges::find(free_entities, idx) == free_entities.end());
	}

	bool World::RemoveComponent(Entity entity, size_t comp_type)
	{
		assert(HasEntity(entity));

		auto& ent = entities[entity.Index()];

		if (comp_type >= ent.components.size()) return false;

		const auto comp_idx = ent.components[comp_type];
		if (comp_idx == NullComponentIndex) return false;

		RemoveComponentFromStore(comp_type, comp_idx);

		ent.components[comp_type] = NullComponentIndex;

		return true;
	}

	void World::RemoveAllComponents(Entity entity)
	{
		assert(HasEntity(entity));

		auto& ent = entities[entity.Index()];

		const size_t comp_count = ent.components.size();
		for (size_t comp_type = 0; comp_type < comp_count; ++comp_type)
		{
			const auto comp_idx = ent.components[comp_type];
			if (comp_idx != NullComponentIndex)
			{
				RemoveComponentFromStore(comp_type, comp_idx);
			}
		}

		ent.components.clear();
	}

	void World::RemoveComponentFromStore(size_t comp_type, ComponentIndex comp_idx)
	{
		assert(comp_type < comp_stores.size());

		auto& store = comp_stores[comp_type];

		assert(store);

		const Entity fix_ent = store->Remove(comp_idx);
		if (fix_ent) {
			entities[fix_ent.Index()].components[comp_type] = comp_idx;
		}
	}
}