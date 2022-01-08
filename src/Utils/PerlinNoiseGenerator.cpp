#include "Utils/PerlinNoiseGenerator.h"
#include "Utils/Utils.h"

namespace Expanse
{
	float PerlinNoiseGenerator::Get(FPoint pos) const
	{
		float value = 0.0f;
		utils::for_each_zipped(seeds, harmonics, [&value, pos](uint32_t seed, const auto& h)
		{
			value += Lerp(h.min, h.max, PerlinNoise(pos * h.freq, seed));
		});
		return value;
	}
}