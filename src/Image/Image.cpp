#include "Image.h"

#include "Utils/Logger/Logger.h"

namespace stb_utils
{
	inline void* malloc(size_t size)
	{
		return new uint8_t[size];
	}

	void* realloc_sized(void* ptr, size_t old_size, size_t new_size)
	{
		auto* src = static_cast<uint8_t*>(ptr);
		auto* dst = new uint8_t[new_size];
		std::copy(src, src + std::min(old_size, new_size), dst);
		delete[] src;
		return dst;
	}

	inline void free(void* ptr)
	{
		delete[] static_cast<uint8_t*>(ptr);
	}
}

#define STBI_MALLOC(sz)                     stb_utils::malloc(sz)
#define STBI_REALLOC_SIZED(p, oldsz, newsz) stb_utils::realloc_sized(p, oldsz, newsz)
#define STBI_FREE(p)                        stb_utils::free(p)

#define STBI_MAX_DIMENSIONS 4096

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Expanse::Image
{
	ColorFormat FormatFromChannels(int channels)
	{
		switch (channels) {
			case 3: return ColorFormat::RGB_8;
			case 4: return ColorFormat::RGBA_8;
			default: return ColorFormat::Unsupported;
		}
	}

	ImageData Load(const std::string& file)
	{
		ImageData image;

		int width, height, channels;
		unsigned char* data = stbi_load(file.c_str(), &width, &height, &channels, 0);

		if (data)
		{
			image.width = width;
			image.height = height;
			image.format = FormatFromChannels(channels);

			// We can just own this pointer, because we overrided STB memory
			// functions, otherwise we would need to copy the data
			image.data.reset(static_cast<uint8_t*>(data));
		}
		else
		{
			const std::string error_msg{ stbi_failure_reason() };
			Log::message(error_msg);
		}

		return image;
	}
}