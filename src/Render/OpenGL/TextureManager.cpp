#include "pch.h"

#include "TextureManager.h"

#include "Utils.h"

#include "Image/Image.h"

#include "Utils/Logger/Logger.h"

#include <optional>

namespace Expanse::Render::GL
{
	TextureManager::~TextureManager()
	{
		for (const auto& tex : textures) {
			glDeleteTextures(1, &tex.id);
		}
	}

	Texture TextureManager::Create(const std::string& file)
	{
		Texture texture = GetOrCreateHandle(file);

		assert(texture.IsValid());

		auto& tex = textures[texture.index];
		tex.use_count++;

		if (tex.use_count == 1)
		{
			const auto desc = LoadTextureDescription(file);
			if (desc) {
				tex.Create(*desc);
			}
		}

		return texture;
	}

	Texture TextureManager::Create(std::string_view name, const TextureDescription& tex_info)
	{
		Texture texture = GetOrCreateHandle(name);

		assert(texture.IsValid());

		auto& tex = textures[texture.index];
		tex.use_count++;

		// will overwrite existing texture data
		tex.Create(tex_info);

		return texture;
	}

	void TextureManager::Free(Texture texture)
	{
		if (!texture.IsValid()) return;

		auto& tex = textures[texture.index];
		tex.use_count--;
		if (tex.use_count == 0)
		{
			glDeleteTextures(1, &tex.id);
			tex.id = 0;
			tex.name.clear();
		}
	}

	void TextureManager::Bind(Texture texture, int unit)
	{
		if (!texture.IsValid()) return;

		textures[texture.index].Bind(unit);
	}

	Texture TextureManager::GetOrCreateHandle(std::string_view name)
	{
		auto itr = std::ranges::find_if(textures, [name](const auto& tex) { return tex.name == name; });
		if (itr != textures.end())
		{
			const size_t index = itr - textures.begin();
			return { index };
		}
		else
		{
			const size_t index = GetFreeIndexInVector(textures, [](const auto& tex) { return tex.use_count == 0; });
			textures[index].name = name;
			return { index };
		}
	}

	struct TextureFormat
	{
		// format to use to store texture on OpenGL side
		GLint texture_format = GL_RGB; 

		// image data pixel format (rgb, rgba, etc.)
		GLenum pixel_format = GL_RGB;

		// image data single pixel component type (byte, int, float, etc.)
		GLenum component_type = GL_UNSIGNED_BYTE;
	};

	TextureFormat ImageFormatToGL(Image::ColorFormat format)
	{
		using namespace Image;

		switch (format)
		{
			case ColorFormat::RGB_8: return { GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE };
			case ColorFormat::RGBA_8: return { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE };
			case ColorFormat::RGB_16: return { GL_RGB16, GL_RGB, GL_UNSIGNED_SHORT };
			case ColorFormat::RGBA_16: return { GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT };
			default: return {};
		}
	}

	std::pair<GLint, GLint> FilterTypeToGL(TextureFilterType filter, bool use_mipmaps)
	{
		switch (filter)
		{
		case TextureFilterType::Nearest:
			return { GL_NEAREST, GL_NEAREST };
		case TextureFilterType::Linear:
			if (use_mipmaps)
				return { GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR };
			else
				return { GL_LINEAR, GL_LINEAR };
		default:
			return { GL_NEAREST, GL_NEAREST };
		}
	}

	GLint AddressModeToGL(TextureAddressMode mode)
	{
		switch (mode) {
			case TextureAddressMode::Clamp: return GL_CLAMP_TO_EDGE;
			case TextureAddressMode::Repeat: return GL_REPEAT;
			default: return GL_REPEAT;
		}
	}

	void TextureManager::TextureResource::Create(const TextureDescription& desc)
	{
		if (id == 0) {
			glGenTextures(1, &id);
		}

		auto format = ImageFormatToGL(desc.image.format);

		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, format.texture_format, desc.image.width, desc.image.height, 0, format.pixel_format, format.component_type, desc.image.data.get());

		if (desc.use_mipmaps) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		const auto [min_filter, mag_filter] = FilterTypeToGL(desc.filter_type, desc.use_mipmaps);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);

		const auto wrap_mode = AddressModeToGL(desc.address_mode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
	}

	void TextureManager::TextureResource::Bind(int texture_unit)
	{
		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glBindTexture(GL_TEXTURE_2D, id);
	}
}