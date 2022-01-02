#pragma once

#include "TypeIndex.h"

#include <memory>

namespace Expanse::ecs
{
	namespace details
	{
		struct AnyValueBase
		{
			virtual ~AnyValueBase() = default;
		};

		template<class T>
		struct AnyValue : public AnyValueBase
		{
			template<typename... Args>
			AnyValue(Args&&... args) : value(std::forward<Args>(args)...) {}

			T value;
		};
	}

	template<typename TypeFamily>
	class AnyVector final
	{
	public:
		template<typename T, typename... Args>
		T* Set(Args&&... args)
		{
			const auto type_index = TypeIndex<T>;
			values.resize(std::max(values.size(), type_index + 1));
			auto any_val = std::make_unique<details::AnyValue<T>>(std::forward<Args>(args)...);
			auto result = &(any_val->value);
			values[type_index] = std::move(any_val);
			return result;
		}

		template<typename T>
		T* Get()
		{
			const auto type_index = TypeIndex<T>;
			if (type_index >= values.size()) return nullptr;

			auto any_val = static_cast<details::AnyValue<T>*>(values[type_index].get());
			return any_val ? &(any_val->value) : nullptr;
		}

		template<typename T, typename... Args>
		T* GetOrCreate(Args&&... args)
		{
			T* val = Get<T>();
			return val ? val : Set<T>(std::forward<Args>(args)...);
		}

		template<typename T>
		void Reset()
		{
			const auto type_index = TypeIndex<T>;
			if (type_index < values.size()) {
				values[type_index].reset();
			}
		}

	private:
		template<class T>
		static const auto TypeIndex = GetNextTypeIndex<TypeFamily>();

		std::vector<std::unique_ptr<details::AnyValueBase>> values;
	};
}