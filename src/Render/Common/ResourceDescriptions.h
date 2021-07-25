#pragma once

#include <optional>

#include "Render/RenderTypes.h"

#include "Image/Image.h"

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


	struct TextureDescription
	{
		Image::ImageData image;

		TextureFilterType filter_type = TextureFilterType::Linear;
		TextureAddressMode address_mode = TextureAddressMode::Repeat;
		bool use_mipmaps = false;
	};

	std::optional<TextureDescription> LoadTextureDescription(const std::string& file);
}