#pragma once

#include "Render/Common/ResourceDescriptions.h"

namespace Expanse::Render::GL
{
	class RenderStateManager
	{
	public:
		void Init();
		void Set(const MaterialProperties& props);

	private:
		void SetBlendMode(BlendMode blend_mode);
		void SetCulling(bool two_sided);

		MaterialProperties current;
	};
}