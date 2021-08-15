#include "LogWindow.h"

#include "Game/World.h"

#include "imgui.h"

#include <ranges>

namespace Expanse
{
	GUILogSink::GUILogSink(size_t max_msg)
		: max_messages(max_msg)
	{}

	void GUILogSink::Write(const std::string& msg)
	{
		messages.push_back(msg);

		while (messages.size() > max_messages) {
			messages.pop_front();
		}
	}

	/**********************************************************/


	LogWindowSystem::LogWindowSystem(Game::World& w)
		: Game::ISystem(w)
	{
		log_sink = std::make_shared<GUILogSink>(16);
		Log::add_sink(log_sink);
	}

	LogWindowSystem::~LogWindowSystem()
	{
		Log::remove_sink(log_sink);
	}

	void LogWindowSystem::Update()
	{
		ImGui::Begin("Log", nullptr);

		for (const auto& msg : log_sink->GetMessages() | std::views::reverse)
		{
			ImGui::Text("%s", msg.c_str());
		}

		ImGui::End();
	}
}