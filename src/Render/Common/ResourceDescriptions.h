#pragma once

#include <optional>

#include "Render/RenderTypes.h"

#include "Image/Image.h"

namespace Expanse::Render
{
	struct MaterialProperties
	{
		BlendMode blend_mode = BlendMode::Alpha;
		bool culling = false;
	};

	struct MaterialParameterPair
	{
		std::string name;
		MaterialParameterValue value;
	};

	struct MaterialDescription
	{
		// shader program
		std::string shader_file;

		// uniforms and textures
		std::vector<MaterialParameterPair> params;

		// render state (blend mode, etc.)
		MaterialProperties properties;
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