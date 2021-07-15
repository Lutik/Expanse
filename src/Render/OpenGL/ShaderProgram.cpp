#include "pch.h"

#include "ShaderProgram.h"
#include "VertexAttributes.h"

#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include <fstream>
#include <sstream>

namespace Expanse::Render::GL
{
	namespace
	{
		GLenum GLShaderTypeFromString(std::string_view type_str)
		{
			GLenum shader_type = GL_VERTEX_SHADER;
			if (type_str == "vertex")
				shader_type = GL_VERTEX_SHADER;
			else if (type_str == "fragment")
				shader_type = GL_FRAGMENT_SHADER;
			return shader_type;
		}

		bool CheckShaderCompileStatus(GLuint handle)
		{
			GLint compileStatus = GL_FALSE;
			glGetShaderiv(handle, GL_COMPILE_STATUS, &compileStatus);
			GLint compileLogLength = 0;
			glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &compileLogLength);
			if (compileLogLength > 0) {
				std::string compileLog;
				compileLog.resize(compileLogLength);
				glGetShaderInfoLog(handle, compileLogLength, nullptr, compileLog.data());
				Log::message("Shader log: {}", compileLog);
			}
			return (compileStatus == GL_TRUE);
		}

		bool CheckProgramLinkStatus(GLuint program)
		{
			GLint linkStatus = GL_FALSE;
			glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
			GLint linkLogLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &linkLogLength);
			if (linkLogLength > 0) {
				std::string linkLog;
				linkLog.resize(linkLogLength);
				glGetProgramInfoLog(program, linkLogLength, nullptr, linkLog.data());
				Log::message("Shader program log: {}", linkLog);
			}
			return (linkStatus == GL_TRUE);
		}

		struct ShaderSourceBlock {
			std::string_view type;
			std::string_view source;
		};

		ShaderSourceBlock SplitShaderProgramSourceBlock(std::string_view block)
		{
			const auto type_end = block.find(' ', 1);
			if (type_end != std::string_view::npos) {
				const auto type = block.substr(1, type_end - 1);
				const auto source = block.substr(type_end + 1);
				return { type, source };
			}
			return {};
		}

		std::vector<ShaderSourceBlock> SplitShaderProgramSource(std::string_view program_source)
		{
			std::vector<ShaderSourceBlock> shader_sources;

			auto block_start = program_source.find('$', 0);
			while (block_start != std::string_view::npos)
			{
				const auto block_end = program_source.find('$', block_start + 1);
				const auto block_view = program_source.substr(block_start, block_end - block_start);

				shader_sources.push_back(SplitShaderProgramSourceBlock(block_view));

				block_start = block_end;
			}

			return shader_sources;
		}

		GLuint CreateShader(GLenum type, std::string_view source)
		{
			// compile shader
			auto handle = glCreateShader(type);
			const char* source_ptr = source.data();
			const auto source_length = static_cast<GLint>(source.size());
			glShaderSource(handle, 1, &source_ptr, &source_length);
			glCompileShader(handle);

			// check compile status
			if (CheckShaderCompileStatus(handle))
			{
				return handle;
			}
			else
			{
				glDeleteShader(handle);
				return 0;
			}
		}

		GLuint LinkProgram(const std::vector<GLuint>& shaders)
		{
			GLuint program = glCreateProgram();

			// attach all shaders
			for (auto shader : shaders) {
				glAttachShader(program, shader);
			}

			// setup attribute locations
			for (auto [loc, name] : VertexAttributes) {
				glBindAttribLocation(program, loc, name);
			}

			glLinkProgram(program);

			for (auto shader : shaders) {
				glDetachShader(program, shader);
			}

			if (!CheckProgramLinkStatus(program))
			{
				glDeleteProgram(program);
				program = 0;
			}

			return program;
		}

		GLuint LoadShaderProgramFromFile(const std::string& source_file)
		{
			const std::string program_source = File::LoadContents(source_file);
			if (program_source.empty()) {
				Log::message("Shader program source not found: {}", source_file);
				return 0;
			}

			// split program source
			const auto shader_sources = SplitShaderProgramSource(program_source);

			// compile individual shaders
			std::vector<GLuint> shaders;
			for (auto [type, source] : shader_sources)
			{
				GLuint shader = CreateShader(GLShaderTypeFromString(type), source);
				if (shader) {
					shaders.push_back(shader);
				}
			}

			// create and link program
			GLuint program = LinkProgram(shaders);

			for (auto shader : shaders) {
				glDeleteShader(shader);
			}

			return program;
		}
	}


	/**********************************************************************************/

	ShaderProgram::ShaderProgram(const std::string& source_file)
	{
		program = LoadShaderProgramFromFile(source_file);
	}

	ShaderProgram::~ShaderProgram()
	{
		glDeleteProgram(program);
	}

	ShaderProgram::ShaderProgram(ShaderProgram&& other)
	{
		std::swap(program, other.program);
	}

	ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other)
	{
		std::swap(program, other.program);
		return *this;
	}

	void ShaderProgram::Bind()
	{
		glUseProgram(program);
	}
}