#include "Utils/FileUtils.h"

#include <fstream>

namespace Expanse
{
	namespace File
	{
		std::string LoadContents(const std::string& path)
		{
			std::string contents;
			std::ifstream file{ path.data() };

			char buffer[256];
			while (file.read(buffer, std::size(buffer))) {
				contents.append(buffer, file.gcount());
			}
			contents.append(buffer, file.gcount());

			return contents;
		}

		void WriteContents(const std::string& path, std::string_view content)
		{
			std::ofstream file{ path };
			if (file) {
				file << content;
				file.close();
			}
		}
	}
}