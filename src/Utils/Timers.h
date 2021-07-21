#pragma once

#include <chrono>

namespace Expanse
{
	template<class Clock>
		requires std::chrono::is_clock_v<Clock>
	class TTimer
	{
	public:
		TTimer() : start_time(Clock::now()) {}

		float Elapsed(bool reset = false)
		{
			const auto now_time = Clock::now();
			const std::chrono::duration<float> diff = now_time - start_time;
			if (reset) {
				start_time = now_time;
			}
			return diff.count();
		}

	protected:
		Clock::time_point start_time;
	};

	using Timer = TTimer<std::chrono::steady_clock>;
}