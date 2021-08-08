
#include "Utils/Random.h"

namespace Expanse
{
	/*
	* Distributions
	*/
	
	float CanonicalFloat(uint32_t x)
	{
		// based on algorithm from
		// https://xor0110.wordpress.com/2010/09/24/how-to-generate-floating-point-random-numbers-efficiently/
		x = x & 0x007FFFFF | 0x3F800000;
		float f;
		std::memcpy(&f, &x, sizeof(f));
		return f - 1.0f;
	}
	
	int UniformInt(uint32_t x, int a, int b)
	{
		return a + x % (b - a + 1);
	}

	float UniformFloat(uint32_t x, float a, float b)
	{
		return a + CanonicalFloat(x) * (b - a);
	}

	/*
	* Pseudo-random number generators
	*/

	uint32_t Xorshift32(uint32_t x)
	{
		x ^= x << 13;
		x ^= x >> 17;
		x ^= x << 5;
		return x;
	}

	/*
	* Noise functions
	*/
	uint32_t Squirrel3(int x, uint32_t seed)
	{
		static constexpr unsigned int BITS_NOISE[3] = { 0xB5297A4D, 0x68E31DA4, 0x1B56C4E9 };

		unsigned int v = x;
		v *= BITS_NOISE[0];
		v += seed;
		v ^= (v >> 8);
		v *= BITS_NOISE[1];
		v ^= (v << 8);
		v *= BITS_NOISE[2];
		v ^= (v >> 8);
		return v;
	}

	uint32_t Squirrel3(Point pt, uint32_t seed)
	{
		static constexpr int BIG_PRIME = 198491317;
		return Squirrel3(pt.x + BIG_PRIME * pt.y, seed);
	}

	/*
	* Utilities
	*/

	uint32_t GetRandomSeed()
	{
		std::random_device rnd_device;
		std::seed_seq sseq{ rnd_device(), rnd_device(), rnd_device(), rnd_device() };
		uint32_t seed;
		sseq.generate(&seed, &seed + 1);
		return seed;
	}

	/*
	* Convenience functions
	*/

	static uint32_t debug_rng = GetRandomSeed();

	float RandomFloat(float a, float b)
	{
		debug_rng = Xorshift32(debug_rng);
		return UniformFloat(debug_rng, a, b);
	}

	int RandomInt(int a, int b)
	{
		debug_rng = Xorshift32(debug_rng);
		return UniformInt(debug_rng, a, b);
	}
}