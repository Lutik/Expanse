#pragma once

#include "Render/IRenderer.h"

#include "imgui.h"

namespace Expanse
{
	class ImGuiRenderer
	{
	public:
		explicit ImGuiRenderer(Render::IRenderer* render);
		~ImGuiRenderer();

		void StartFrame();
		void EndFrame();

	private:
		Render::IRenderer* renderer = nullptr;

		Render::Texture font_texture;
		Expanse::Render::Material gui_material;
		Expanse::Render::Mesh gui_mesh;

		void CreateFontTexture();
		void FreeFontTexture();

		void SetupRenderState(ImDrawData* draw_data, int fb_width, int fb_height);
		void RenderDrawData(ImDrawData* draw_data);
	};
}