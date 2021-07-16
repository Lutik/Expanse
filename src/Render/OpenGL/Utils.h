#pragma once

namespace Expanse::Render::GL
{
	template<class Res, class Pred>
	size_t GetFreeIndexInVector(std::vector<Res>& vec, Pred is_free)
	{
		const auto itr = std::ranges::find_if(vec, is_free);
		if (itr != vec.end()) {
			return itr - vec.begin();
		} else {
			vec.emplace_back();
			return vec.size() - 1;
		}
	}
}