#pragma once

namespace Expanse::utils
{
	template<typename Rng1, typename Rng2, typename Func>
	auto for_each_zipped(Rng1&& rng1, Rng2&& rng2, Func&& func)
	{
		auto itr1 = std::begin(rng1);
		auto itr2 = std::begin(rng2);
		const auto last1 = std::end(rng1);
		const auto last2 = std::end(rng2);
		while (itr1 != last1 && itr2 != last2) {
			func(*itr1, *itr2);
			++itr1;
			++itr2;
		}
		return std::make_pair(itr1, itr2);
	}

	template<typename Rng, typename T>
	bool contains(Rng&& rng, const T& value)
	{
		return std::ranges::find(rng, value) != std::ranges::end(rng);
	}
}