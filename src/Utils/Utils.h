#pragma once

namespace Expanse::utils
{
	template<std::ranges::input_range Rng1, std::ranges::input_range Rng2, typename Func>
	auto for_each_zipped(Rng1&& rng1, Rng2&& rng2, Func&& func)
	{
		auto itr1 = std::ranges::begin(rng1);
		auto itr2 = std::ranges::begin(rng2);
		const auto last1 = std::ranges::end(rng1);
		const auto last2 = std::ranges::end(rng2);
		while (itr1 != last1 && itr2 != last2) {
			std::invoke(func, *itr1, *itr2);
			++itr1;
			++itr2;
		}
		return std::make_pair(itr1, itr2);
	}

	template<std::ranges::input_range Rng, typename T>
	bool contains(Rng&& rng, const T& value)
	{
		return std::ranges::find(rng, value) != std::ranges::end(rng);
	}
}