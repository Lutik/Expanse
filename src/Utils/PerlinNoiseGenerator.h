#pragma once

#include "Utils/Math.h"
#include "Utils/Random.h"

#include <vector>

namespace Expanse
{
	struct NoiseHarmonics
	{
		float freq;
		float min;
		float max;
	};

	struct PerlinNoiseGenerator
	{
	public:
		PerlinNoiseGenerator(uint32_t seed, std::vector<NoiseHarmonics> h)
			: harmonics(std::move(h))
			, seeds(harmonics.size())
		{
			GenerateSeeds(seeds, seed);
		}

		float Get(FPoint pos) const;

	private:
		std::vector<NoiseHarmonics> harmonics;
		std::vector<uint32_t> seeds;
	};
}