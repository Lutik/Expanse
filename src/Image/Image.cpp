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
			case 0:
				// grayscale
				break;
			case 2:
				// RGB
				if (bit_depth == 8) return ColorFormat::RGB_8;
				if (bit_depth == 16) return ColorFormat::RGB_16;
				break;
			case 3:
				// palette format
				break;
			case 4:
				// grayscale + alpha
				break;
			case 6:
				// RGBA
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

			image.data.reset(new uint8_t[image.height * row_size]); // alloc image

			std::vector<png_bytep> row_pointers; // setup row pointers (flipped upside-down for opengl)
			row_pointers.reserve(image.height);
			for (size_t row = image.height; row > 0; --row) {
				row_pointers.push_back(image.data.get() + (row - 1) * row_size);
			}		

			png_read_image(read_ptr, row_pointers.data());


			png_destroy_read_struct(&read_ptr, &info_ptr, nullptr);
			fclose(file);

			return image;
		}
	}



	ImageData Load(const std::string& file)
	{
		PNG::Load(file);
		return {};
	}
}