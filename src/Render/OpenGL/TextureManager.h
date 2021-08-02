#pragma once

#include "Render/Common/ResourceDescriptions.h"

namespace Expanse::Render::GL
{
	class TextureManager
	{
	public:
		TextureManager() = default;
		~TextureManager();
		TextureManager(const TextureManager&) = delete;
		TextureManager& operator=(const TextureManager&) = delete;

		Texture Create(const std::string& file);
		Texture Create(std::string_view name, const TextureDescription& tex_data);

		void Free(Texture texture);
		void Bind(Texture texture, int unit);

	private:
		struct TextureResource
		{
			GLuint id = 0;
			std::string name;
			size_t use_count = 0;

			void Create(const TextureDescription& desc);
			void Bind(int texture_unit);
		};

		std::vector<TextureResource> textures;

		Texture GetOrCreateHandle(std::string_view name);
	};
}