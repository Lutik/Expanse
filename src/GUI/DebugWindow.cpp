#include "DebugWindow.h"

#include "Game/World.h"

#include "imgui.h"

namespace Expanse
{
	void DebugWindowSystem::Update()
	{
		const auto fps = static_cast<int>(1.0f / world.dt);


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::Begin("DebugInfo", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar);
		ImGui::Text("FPS: %d", fps);
		ImGui::End();
	}
}