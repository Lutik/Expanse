#pragma once

#include "ResourceHandles.h"

namespace Expanse::Render::GL
{
	class TextureManager
	{
	public:
		Texture Create(const std::string& file);
		void Free(Texture texture);
		void Bind(Texture texture, int unit);

	private:
		struct TextureResource
		{
			GLuint id = 0;

			std::string name;
			size_t use_count = 0;


			void Create(const std::string& file);
			void Free();
			void Bind(int texture_unit);
		};

		std::vector<TextureResource> textures;
	};
}