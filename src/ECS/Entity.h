#pragma once

#include <cstdint>
#include <limits>
#include <cassert>

namespace Expanse::ecs
{
	class Entity final
	{
	public:
		// Basic settings
		using BaseType = uint16_t;
		static constexpr size_t VersionBits = 2;

		// Derived constants
		static constexpr BaseType NullValue = std::numeric_limits<BaseType>::max();
		static constexpr size_t IndexBits = sizeof(BaseType) * 8 - VersionBits;
		static constexpr BaseType IndexMask = NullValue >> VersionBits;
		static constexpr BaseType MaxVersion = (1 << VersionBits) - 1;
		static constexpr BaseType MaxIndex = (1 << IndexBits) - 1;

	public:

		constexpr Entity() = default;
		constexpr explicit Entity(BaseType index) : value(index) {
			assert(index <= MaxIndex);
		}
		constexpr explicit Entity(size_t index) : value(static_cast<BaseType>(index)) {
			assert( index <= static_cast<size_t>(MaxIndex));
		}

		operator bool () const noexcept { return value != NullValue; }

		bool operator==(const Entity& ent) const noexcept { return value == ent.value; }
		bool operator!=(const Entity& ent) const noexcept { return value != ent.value; }

		[[nodiscard]] BaseType Index() const noexcept { return value & IndexMask; }
		[[nodiscard]] BaseType Version() const noexcept { return value >> IndexBits; }

		void IncVersion() {
			value = Index() | ((Version() + 1) << IndexBits);
		}

	private:
		BaseType value = NullValue;
	};

	// Type that is used to store component indices in component pools
	using ComponentIndex = Entity::BaseType;
	inline constexpr auto NullComponentIndex = std::numeric_limits<ComponentIndex>::max();
}