#pragma once

#include <string_view>
#include <memory>
#include <vector>
#include <format>

namespace Expanse::Log
{
	struct ILogSink
	{
		virtual ~ILogSink() = default;
		virtual void Write(const std::string& msg) = 0;
	};

	// Init logging functionality
	void init();

	// Add/remove additional log output sinks
	void add_sink(std::shared_ptr<ILogSink> sink);
	void remove_sink(std::shared_ptr<ILogSink> sink);

	// Output unformatted message to log
	void message(const std::string& msg);

	// Output formatted message to log
	template<typename... Args>
	void message(std::string_view format_str, const Args&... args)
	{
		message(std::format(format_str, args...));
	}
}