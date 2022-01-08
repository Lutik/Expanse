
#include "Utils/Random.h"

#include <numbers>

namespace Expanse
{
	/*
	* Distributions
	*/
	
	float CanonicalFloat(uint32_t x)
	{
		// based on algorithm from
		// https://xor0110.wordpress.com/2010/09/24/how-to-generate-floating-point-random-numbers-efficiently/
		const float f = std::bit_cast<float>(x & 0x007FFFFF | 0x3F800000);
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
		static constexpr uint32_t BITS_NOISE[3] = { 0xB5297A4D, 0x68E31DA4, 0x1B56C4E9 };

		uint32_t v = x;
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

	float PerlinNoise(FPoint pos, uint32_t seed)
	{
		auto ease = [](float t) {
			return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
		};

		auto get_grad = [=](int px, int py){
			const auto noise = Squirrel3(Point{ px, py }, seed);
			const auto angle = UniformFloat(noise, 0.0f, 2.0f * std::numbers::pi_v<float>);
			return FPoint{ std::sin(angle), std::cos(angle) };
		};

		const auto fx = std::floor(pos.x);
		const auto fy = std::floor(pos.y);
		const auto x = static_cast<int>(fx);
		const auto y = static_cast<int>(fy);

		const FPoint grads[4] = {
			get_grad(x, y),
			get_grad(x, y+1),
			get_grad(x+1, y),
			get_grad(x+1, y+1),
		};

		const FPoint offsets[4] = {
			{pos.x - fx, pos.y - fy},
			{pos.x - fx, pos.y - fy - 1.0f},
			{pos.x - fx - 1.0f, pos.y - fy},
			{pos.x - fx - 1.0f, pos.y - fy - 1.0f},
		};

		const float dots[4] = {
			DotProduct(grads[0], offsets[0]),
			DotProduct(grads[1], offsets[1]),
			DotProduct(grads[2], offsets[2]),
			DotProduct(grads[3], offsets[3]),
		};

		const float sx = ease(pos.x - fx);
		const float sy = ease(pos.y - fy);

		const float v0 = Lerp(dots[0], dots[1], sy);
		const float v1 = Lerp(dots[2], dots[3], sy);
		const float v = Lerp(v0, v1, sx);

		return std::clamp((v + 0.55f) / 1.1f, 0.0f, 1.0f);
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