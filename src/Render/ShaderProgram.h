#pragma once

namespace Expanse::Render
{
	class ShaderProgram
	{
	public:
		ShaderProgram() = default;
		ShaderProgram(const std::string& vs_file, const std::string& fs_file);

		~ShaderProgram();

		ShaderProgram(const ShaderProgram& other) = delete;
		ShaderProgram& operator=(const ShaderProgram& other) = delete;
		ShaderProgram(ShaderProgram&& other);
		ShaderProgram& operator=(ShaderProgram&& other);

		bool IsValid() const { return program != 0; }

		void Bind();

	private:
		GLuint program = 0;
	};
}