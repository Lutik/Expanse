#include "pch.h"
#include "RenderStateManager.h"

namespace Expanse::Render::GL
{
	void RenderStateManager::Init()
	{
		SetBlendMode(current.blend_mode);
		SetCulling(current.culling);
	}

	void RenderStateManager::Set(const MaterialProperties& props)
	{
		if (current.blend_mode != props.blend_mode)
			SetBlendMode(props.blend_mode);

		if (current.culling != props.culling)
			SetCulling(props.culling);
	}

	void RenderStateManager::SetBlendMode(BlendMode blend_mode)
	{
		current.blend_mode = blend_mode;

		if (blend_mode == BlendMode::None)
		{
			glDisable(GL_BLEND);
		}
		else
		{
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);

			switch (blend_mode)
			{
			case BlendMode::Alpha:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case BlendMode::Add:
				glBlendFunc(GL_ONE, GL_ONE);
				break;
			case BlendMode::Multiply:
				glBlendFunc(GL_DST_COLOR, GL_ZERO);
				break;
			case BlendMode::ImGui:
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			}
		}
	}

	void RenderStateManager::SetCulling(bool culling)
	{
		if (culling) {
			glEnable(GL_CULL_FACE);
		} else {
			glDisable(GL_CULL_FACE);
		}
	}
}