#include "Utils/Utils.h"

#include <fstream>

namespace Expanse
{
	namespace File
	{
		std::string LoadContents(const std::string& path)
		{
			std::string contents;
			std::ifstream in{ path, std::ios::in };
			if (in)
			{
				in.seekg(0, std::ios::end);
				contents.resize(in.tellg());
				in.seekg(0, std::ios::beg);
				in.read(contents.data(), contents.size());
				in.close();			
			}
			return contents;
		}
	}
}