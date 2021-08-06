#pragma once

#include "TypeIndex.h"
#include "ComponentStore.h"
#include "EntityStore.h"

#include <memory>

namespace Expanse::ecs
{
	class World
	{
	public:
		Entity CreateEntity();

		void DestroyEntity(Entity entity);

		bool HasEntity(Entity entity) const;

		template<typename Comp, typename... Args>
		Comp* AddComponent(Entity entity, Args&&... args)
		{
			assert(HasEntity(entity));

			auto store = GetOrCreateStore<Comp>();
			const auto comp_idx = store->Create(entity, std::forward<Args>(args)...);
			entities[entity.Index()].SetComponent(TypeIndex<Comp>, comp_idx);
			return store->Get(comp_idx);
		}

		template<typename Comp>
		bool RemoveComponent(Entity entity)
		{
			return RemoveComponent(entity, TypeIndex<Comp>);
		}

		template<typename Comp>
		Comp* GetComponent(Entity entity)
		{
			return GetComponentImpl<Comp>(entity);
		}

		template<typename Comp>
		const Comp* GetComponent(Entity entity) const
		{
			return GetComponentImpl<Comp>(entity);
		}

		template<typename Comp, typename... Comps>
		auto GetComponents(Entity entity)
		{
			return GetComponentsImpl<decltype(this), Comp, Comps...>(this, entity);
		}

		template<typename Comp, typename... Comps>
		auto GetComponents(Entity entity) const
		{
			return GetComponentsImpl<decltype(this), Comp, Comps...>(this, entity);
		}

		template<typename Comp>
		bool HasComponent(Entity entity) const
		{
			assert(HasEntity(entity));

			return entities[entity.Index()].HasComponent(TypeIndex<Comp>);
		}

	protected:

		template<typename Comp>
		ComponentStore<Comp>* GetOrCreateStore()
		{
			comp_stores.resize(std::max(comp_stores.size(), TypeIndex<Comp> + 1));

			auto& store = comp_stores[TypeIndex<Comp>];
			if (!store) {
				store = std::make_unique<ComponentStore<Comp>>();
			}

			return static_cast<ComponentStore<Comp>*>(store.get());
		}

		template<typename Comp>
		ComponentStore<Comp>* GetStore() const
		{
			ComponentStoreBase* store = (TypeIndex<Comp> < comp_stores.size()) ? comp_stores[TypeIndex<Comp>].get() : nullptr;
			return static_cast<ComponentStore<Comp>*>(store);
		}

		template<typename Comp>
		Comp* GetComponentImpl(Entity entity) const
		{
			assert(HasEntity(entity));

			auto& comps = entities[entity.Index()].components;
			if (TypeIndex<Comp> >= comps.size()) return nullptr;

			const auto comp_idx = comps[TypeIndex<Comp>];
			if (comp_idx == NullComponentIndex) return nullptr;

			auto store = GetStore<Comp>();
			assert(store);
			return store ? store->Get(comp_idx) : nullptr;
		}

		template<typename W, typename Comp, typename... Comps>
		static auto GetComponentsImpl(W world, Entity entity)
		{
			auto comp_tup = std::make_tuple(world->GetComponent<Comp>(entity));
			if constexpr (sizeof...(Comps) == 0) {
				return comp_tup;
			} else {
				return std::tuple_cat(comp_tup, world->GetComponents<Comps...>(entity));
			}
		}

		bool RemoveComponent(Entity entity, size_t comp_type);
		void RemoveAllComponents(Entity entity);
		void RemoveComponentFromStore(size_t comp_type, ComponentIndex comp_idx);

	protected:
		std::vector<std::unique_ptr<ComponentStoreBase>> comp_stores;
		std::vector<EntityStore> entities;
		std::vector<size_t> free_entities;
	};
}