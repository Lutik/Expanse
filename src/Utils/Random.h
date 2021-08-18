#pragma once

#include <random>

#include "Utils/Math.h"

namespace Expanse
{
	/* Distributions */
	float CanonicalFloat(uint32_t x);
	int UniformInt(uint32_t x, int a, int b);
	float UniformFloat(uint32_t x, float a, float b);

	/* RNG */
	uint32_t Xorshift32(uint32_t x);

	/* Noise functions */
	uint32_t Squirrel3(int x, uint32_t seed = 0);
	uint32_t Squirrel3(Point pos, uint32_t seed = 0);

	inline int NoiseInt(Point pos, int a, int b, uint32_t seed = 0) {
		return UniformInt(Squirrel3(pos, seed), a, b);
	}

	float PerlinNoise(FPoint pos, uint32_t seed = 0);

	/* Misc */
	uint32_t GetRandomSeed();

	/* Convenience functions */
	float RandomFloat(float a, float b);
	int RandomInt(int a, int b);
}