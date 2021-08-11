#pragma once

#include <string>

namespace Expanse
{
	namespace File
	{
		// Loads whole text file contents into a string
		// Returns empty string if the file doesn't exists
		//
		std::string LoadContents(const std::string& path);

		void WriteContents(const std::string& path, std::string_view content);
	}
}