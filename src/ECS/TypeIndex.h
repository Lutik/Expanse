#pragma once

namespace Expanse::ecs
{
	namespace details
	{
		template<class TypeFamily = void>
		struct TypeIndexCounter final
		{
			[[nodiscard]] static size_t Next() noexcept {
				static size_t counter = 0;
				return counter++;
			}
		};
	}

	template<class T, class TypeFamily = void>
	inline const size_t TypeIndex = details::TypeIndexCounter<TypeFamily>::Next();
}