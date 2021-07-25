#pragma once

#include <optional>

#include "Render/RenderTypes.h"

namespace Expanse::Render
{
	struct MaterialParameterPair
	{
		std::string name;
		MaterialParameterValue value;
	};

	struct MaterialDescription
	{
		std::string shader_file;
		std::vector<MaterialParameterPair> params;
	};

	std::optional<MaterialDescription> LoadMaterialDescription(const std::string& file);
}