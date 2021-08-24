#pragma once

#include "Entity.h"

#include <vector>

namespace Expanse::ecs
{
	struct ComponentStoreBase
	{
		virtual ~ComponentStoreBase() = default;

		virtual Entity Remove(ComponentIndex comp_idx) = 0;
	};

	template<class Comp>
	class ComponentStore : public ComponentStoreBase
	{
	public:
		template<class... Args>
		ComponentIndex Create(Entity entity, Args&&... args)
		{
			if constexpr (std::is_constructible_v<Comp, Args...>) {
				components.emplace_back(std::forward<Args>(args)...);
			} else {
				components.push_back({ std::forward<Args>(args)... });
			}
			entities.push_back(entity);
			return static_cast<ComponentIndex>(entities.size() - 1);
		}

		Comp* Get(ComponentIndex comp_idx) {
			return &components[comp_idx];
		}

		const Comp* Get(ComponentIndex comp_idx) const {
			return &components[comp_idx];
		}

		const std::vector<Comp>& GetAll() const {
			return components;
		}

		Entity Remove(ComponentIndex comp_idx) override
		{
			assert(!components.empty());

			const auto idx = static_cast<size_t>(comp_idx);
			const auto last = components.size() - 1;

			assert(idx <= last);

			if (idx != last)
			{
				components[idx] = std::move(components[last]);
				components.pop_back();

				entities[idx] = std::move(entities[last]);
				entities.pop_back();

				return entities[idx];
			}
			else
			{
				components.pop_back();
				entities.pop_back();
				return Entity{};
			}
		}

	public:
		std::vector<Comp> components;
		std::vector<Entity> entities;
	};
}