#pragma once

#include "TypeIndex.h"
#include "ComponentStore.h"
#include "EntityStore.h"
#include "AnyVector.h"

#include <memory>

namespace Expanse::ecs
{
	namespace details
	{
		/* Checks if all elements in tuple are not null */
		template<typename Tuple>
		bool HasAll(const Tuple& tup) noexcept {
			return std::apply([](auto&&... v) { return (... && v); }, tup);
		}

		/* Checks if all component indices in tuple are not null */
		template<typename IndexTuple>
		bool HasAllComponents(const IndexTuple& comp_idx) noexcept {
			return std::apply([](auto&&... v) { return (... && (v != NullComponentIndex)); }, comp_idx);
		}
	}

	struct _ComponentsTypeFamily {};
	struct _GlobalsTypeFamily {};

	using Globals = AnyVector<_GlobalsTypeFamily>;

	class World
	{
	public:
		Entity CreateEntity();

		void DestroyEntity(Entity entity);

		template<typename EntityRange>
		void DestroyEntities(EntityRange&& entities)
		{
			for (auto ent : entities) {
				DestroyEntity(ent);
			}
		}

		bool HasEntity(Entity entity) const;

		template<typename Comp, typename... Args>
		Comp* AddComponent(Entity entity, Args&&... args)
		{
			assert(HasEntity(entity));

			auto store = GetOrCreateStore<Comp>();
			const auto comp_idx = store->Create(entity, std::forward<Args>(args)...);
			entities[entity.Index()].SetComponent(CompTypeIndex<Comp>, comp_idx);
			return store->Get(comp_idx);
		}

		template<typename Comp>
		bool RemoveComponent(Entity entity)
		{
			return RemoveComponent(entity, CompTypeIndex<Comp>);
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

		template<typename... Comps>
		auto GetComponents(Entity entity)
		{
			return std::make_tuple((GetComponent<Comps>(entity))...);
		}

		template<typename... Comps>
		auto GetComponents(Entity entity) const
		{
			return std::make_tuple((GetComponent<Comps>(entity))...);
		}

		template<typename Comp>
		const std::vector<Comp>& GetComponentArray() const
		{
			auto store = GetStore<Comp>();

			static std::vector<Comp> empty{};
			return store ? store->GetAll() : empty;
		}

		template<typename Comp>
		bool HasComponent(Entity entity) const
		{
			assert(HasEntity(entity));

			return entities[entity.Index()].HasComponent(CompTypeIndex<Comp>);
		}

		template<typename Comp>
		bool HasComponents() const
		{
			const auto store = GetStore<Comp>();
			return store && !store->entities.empty();
		}

		template<typename Comp>
		const auto& GetEntitiesWith() const
		{		
			const auto store = GetStore<Comp>();

			static decltype(store->entities) empty{};
			return store ? store->entities : empty;
		}

		template<typename Comp, typename... Comps, typename Func>
		void ForEach(Func func)
		{
			// first, find stores for all mentioned components
			auto stores = GetComponentStores<Comp, Comps...>();
			if (!details::HasAll(stores)) return;

			for (const auto ent : GetEntitiesWith<Comp>())
			{
				const auto indices = GetComponentIndices<Comp, Comps...>(ent);
				if (details::HasAllComponents(indices)) {
					std::apply(func, GetComponentsTuple(ent, stores, indices));
				}
			}
		}
	protected:

		template<typename Comp>
		ComponentStore<Comp>* GetOrCreateStore()
		{
			const auto type_index = CompTypeIndex<Comp>;

			comp_stores.resize(std::max(comp_stores.size(), type_index + 1));

			auto& store = comp_stores[type_index];
			if (!store) {
				store = std::make_unique<ComponentStore<Comp>>();
			}

			return static_cast<ComponentStore<Comp>*>(store.get());
		}

		template<typename Comp>
		ComponentStore<Comp>* GetStore() const
		{
			const auto type_index = CompTypeIndex<Comp>;
			ComponentStoreBase* store = (type_index < comp_stores.size()) ? comp_stores[type_index].get() : nullptr;
			return static_cast<ComponentStore<Comp>*>(store);
		}

		template<typename Comp>
		Comp* GetComponentImpl(Entity entity) const
		{
			assert(HasEntity(entity));

			const auto type_index = CompTypeIndex<Comp>;

			auto& comps = entities[entity.Index()].components;
			if (type_index >= comps.size()) return nullptr;

			const auto comp_idx = comps[type_index];
			if (comp_idx == NullComponentIndex) return nullptr;

			auto store = GetStore<Comp>();
			assert(store);
			return store ? store->Get(comp_idx) : nullptr;
		}


		template<typename... Comps>
		auto GetComponentIndices(Entity entity) const noexcept
		{
			const auto& ent = entities[entity.Index()];
			return std::make_tuple( (ent.GetComponentIndex(CompTypeIndex<Comps>))... );
		}

		template<typename... Comps>
		auto GetComponentStores() const
		{
			return std::make_tuple( (GetStore<Comps>())... );
		}

		template<class Stores, class Indices, size_t... I>
		auto GetComponentsTupleImpl(Entity entity, Stores&& stores, Indices&& indices, std::index_sequence<I...>)
		{
			return std::make_tuple(entity, (std::ref(*(std::get<I>(stores)->Get(std::get<I>(indices)))))...);
		}

		// Returns a tuple consisting of entity and references to components, specified by component stores and component indices
		template<class Stores, class Indices>
		auto GetComponentsTuple(Entity entity, Stores&& stores, Indices&& indices)
		{
			return GetComponentsTupleImpl(entity,
				std::forward<Stores>(stores),
				std::forward<Indices>(indices),
				std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Stores>>>());
		}

		bool RemoveComponent(Entity entity, size_t comp_type);
		void RemoveAllComponents(Entity entity);
		void RemoveComponentFromStore(size_t comp_type, ComponentIndex comp_idx);

	protected:
		std::vector<std::unique_ptr<ComponentStoreBase>> comp_stores;
		std::vector<EntityStore> entities;
		std::vector<size_t> free_entities;

		template<class CompType>
		static const size_t CompTypeIndex = GetNextTypeIndex<_ComponentsTypeFamily>();
	};
}