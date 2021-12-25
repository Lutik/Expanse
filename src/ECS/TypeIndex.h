#pragma once

namespace Expanse::ecs
{
	namespace details
	{
		template<class TypeFamily>
		struct TypeIndexCounter final
		{
			[[nodiscard]] static size_t Next() noexcept {
				static size_t counter = 0;
				return counter++;
			}
		};
	}

	template<class TypeFamily>
	[[nodiscard]] size_t GetNextTypeIndex() { return details::TypeIndexCounter<TypeFamily>::Next(); }
}