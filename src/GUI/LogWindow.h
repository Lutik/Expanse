#pragma once

#include "Utils/Logger/Logger.h"

#include "Game/ISystem.h"

#include <deque>

namespace Expanse
{
	class GUILogSink : public Log::ILogSink
	{
	public:
		GUILogSink(size_t max_msg);

		void Write(const std::string& msg);

		const auto& GetMessages() const { return messages; }
	private:
		std::deque<std::string> messages;
		size_t max_messages;
	};

	/**********************************************************/

	class LogWindowSystem final : public Game::ISystem
	{
	public:
		LogWindowSystem(Game::World& w);
		~LogWindowSystem();

		void Update() override;

	private:
		std::shared_ptr<GUILogSink> log_sink;
	};
}