#include "pch.h"

#include "TextureManager.h"

#include "Utils.h"

#include "Image/Image.h"

#include "Utils/Logger/Logger.h"

#include <optional>

namespace Expanse::Render::GL
{
	Texture TextureManager::Create(const std::string& file)
	{
		// look if texture already exists
		auto itr = std::ranges::find_if(textures, [&file](const auto& tex) { return tex.name == file; });
		if (itr != textures.end())
		{
			itr->use_count++;
			const size_t index = itr - textures.begin();
			return { index };
		}
		else
		{
			const size_t index = GetFreeIndexInVector(textures, [](const auto& tex) { return tex.use_count == 0; });
			auto& tex = textures[index];

			tex.Create(file);
			tex.name = file;
			tex.use_count = 1;
			return { index };
		}
	}

	void TextureManager::Free(Texture texture)
	{
		if (!texture.IsValid()) return;

		auto& tex = textures[texture.index];
		tex.use_count--;
		if (tex.use_count == 0)
		{
			tex.Free();
			tex.name.clear();
		}
	}

	void TextureManager::Bind(Texture texture, int unit)
	{
		if (!texture.IsValid()) return;

		textures[texture.index].Bind(unit);
	}

	struct TextureFormat
	{
		GLint texture_format; // format to use to store texture on OpenGL side
		GLenum pixel_format;   // image data pixel format (rgb, rgba, etc.)
		GLenum component_type; // image data single pixel component type (byte, int, float, etc.)
	};

	std::optional<TextureFormat> ImageFormatToGL(Image::ColorFormat format)
	{
		using namespace Image;

		static constexpr std::pair<ColorFormat, TextureFormat> Formats[] = {
			{ ColorFormat::RGB_8,  { GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE }},
			{ ColorFormat::RGBA_8,  { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE }},
			{ ColorFormat::RGB_16,  { GL_RGB16, GL_RGB, GL_UNSIGNED_SHORT }},
			{ ColorFormat::RGBA_16, { GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT }},
		};

		auto itr = std::ranges::find_if(Formats, [=](const auto& p) { return p.first == format; });
		
		std::optional<TextureFormat> result;
		if (itr != std::end(Formats)) {
			result = itr->second;
		}
		return result;
	}

	void TextureManager::TextureResource::Create(const std::string& file)
	{
		glGenTextures(1, &id);

		const auto img = Image::Load(file);

		if (!img.data) {
			Log::message("Texture could not be loaded ({})", file);
			return;
		}

		auto format = ImageFormatToGL(img.format);
		if (!format) {
			Log::message("Unsupported texture format ({})", file);
			return;
		}

		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, format->texture_format, img.width, img.height, 0, format->pixel_format, format->component_type, img.data.get());

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	void TextureManager::TextureResource::Free()
	{
		glDeleteTextures(1, &id);
	}

	void TextureManager::TextureResource::Bind(int texture_unit)
	{
		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glBindTexture(GL_TEXTURE_2D, id);
	}

}