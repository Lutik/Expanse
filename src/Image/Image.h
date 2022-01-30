#pragma once

#include <string>
#include <memory>

namespace Expanse::Image
{
	enum class ColorFormat
	{
		RGB_8,
		RGBA_8,
		Red_8,
		RGB_16,
		RGBA_16,
		Red_16,
		Unsupported
	};
	struct ImageData
	{
		uint32_t width = 0;
		uint32_t height = 0;
		ColorFormat format = ColorFormat::Unsupported;
		std::unique_ptr<uint8_t> data;
	};

	ImageData Load(const std::string& file);
}