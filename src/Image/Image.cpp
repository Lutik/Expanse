#include "Image.h"

#include "png.h"

#include "Utils/Logger/Logger.h"

namespace Expanse::Image
{
	namespace PNG
	{
		ColorFormat ConvertPNGColorFormat(int png_color_type, int bit_depth)
		{
			switch (png_color_type) {
			case PNG_COLOR_TYPE_GRAY:
				break;
			case PNG_COLOR_TYPE_RGB:
				// RGB
				if (bit_depth == 8) return ColorFormat::RGB_8;
				if (bit_depth == 16) return ColorFormat::RGB_16;
				break;
			case PNG_COLOR_TYPE_PALETTE:
				break;
			case PNG_COLOR_TYPE_GRAY_ALPHA:
				break;
			case PNG_COLOR_TYPE_RGBA:
				if (bit_depth == 8) return ColorFormat::RGBA_8;
				if (bit_depth == 16) return ColorFormat::RGBA_16;
				break;
			}
			return ColorFormat::Unsupported;
		}


		ImageData Load(const std::string& filename)
		{
			ImageData image;

			FILE* file;
			fopen_s(&file, filename.c_str(), "rb");

			png_byte signature[8];
			fread(signature, 1, 8, file);
			if (!png_check_sig(signature, 8))
				return image;

			// create libpng structures
			auto read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
			auto info_ptr = png_create_info_struct(read_ptr);

			// libpng's error handling
			if (setjmp(png_jmpbuf(read_ptr))) {
				png_destroy_read_struct(&read_ptr, &info_ptr, nullptr);
				return image;
			}

			// init input stream for
			png_init_io(read_ptr, file);

			// tell that we handled signature
			png_set_sig_bytes(read_ptr, 8);

			// read png headers
			png_read_info(read_ptr, info_ptr);

			// read image info
			int bit_depth;
			int color_type;
			png_get_IHDR(read_ptr, info_ptr, &image.width, &image.height, &bit_depth, &color_type, nullptr, nullptr, nullptr);
			image.format = ConvertPNGColorFormat(color_type, bit_depth);

			// read image information
			const auto row_size = png_get_rowbytes(read_ptr, info_ptr);

			// allocate memory for our image, setup pointers to rows in this memory
			image.data.reset(new uint8_t[image.height * row_size]);
			std::vector<png_bytep> row_pointers;
			row_pointers.reserve(image.height);
			for (size_t row = image.height; row > 0; --row) {
				row_pointers.push_back(image.data.get() + (row - 1) * row_size);
			}

			// when reading 16 bit images, convert endianness from BE to LE
			if (bit_depth == 16) {
				png_set_swap(read_ptr);
			}

			// read image data
			png_read_image(read_ptr, row_pointers.data());

			// cleanup
			png_destroy_read_struct(&read_ptr, &info_ptr, nullptr);
			fclose(file);

			return image;
		}
	}



	ImageData Load(const std::string& file)
	{
		return PNG::Load(file);
	}
}