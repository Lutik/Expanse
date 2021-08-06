#pragma once

#include <vector>

#include "Entity.h"

namespace Expanse::ecs
{
	struct EntityStore
	{
		EntityStore(size_t index) : entity(index) {}

		// make sure this struct is never copied, only moved
		EntityStore(const EntityStore&) = delete;
		EntityStore(EntityStore&&) = default;
		EntityStore& operator=(const EntityStore&) = delete;
		EntityStore& operator=(EntityStore&&) = default;

		void SetComponent(size_t comp_type, ComponentIndex comp_index)
		{
			components.resize(std::max(components.size(), comp_type + 1), NullComponentIndex);
			components[comp_type] = comp_index;
		}

		void ResetComponent(size_t comp_type)
		{
			components[comp_type] = NullComponentIndex;
		}

		size_t GetComponentIndex(size_t comp_type) const noexcept
		{
			return (comp_type < components.size()) ? components[comp_type] : NullComponentIndex;
		}

		bool HasComponent(size_t comp_type) const noexcept
		{
			return (comp_type < components.size()) && components[comp_type] != NullComponentIndex;
		}

		Entity entity;
		std::vector<ComponentIndex> components;
	};
}