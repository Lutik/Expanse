#pragma once

#include <string_view>
#include <memory>
#include <vector>
#include <format>

namespace Expanse::Log
{
	// Init logging functionality
	void init();

	// Output unformatted message to log
	void message(const std::string& msg);

	// Output formatted message to log
	template<typename... Args>
	void message(std::string_view format_str, const Args&... args)
	{
		message(std::format(format_str, args...));
	}
}