#include "pch.h"

#include "ShaderProgram.h"
#include "VertexAttributes.h"

#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include <fstream>
#include <sstream>

namespace Expanse::Render::GL
{
	class ShaderProgramBuilder
	{
	public:
		~ShaderProgramBuilder()
		{
			for (auto shader : shaders) {
				glDeleteShader(shader);
			}
		}

		void AddShader(GLenum type, const std::string& source_file)
		{
			// load shader source code
			const std::string source = File::LoadContents(source_file);
			if (source.empty()) {
				Log::message("Shader source not found: {}", source_file);
				return;
			}

			// compile shader
			auto handle = glCreateShader(type);
			const char* source_ptr = source.data();
			const auto source_length = static_cast<GLint>(source.size());
			glShaderSource(handle, 1, &source_ptr, &source_length);
			glCompileShader(handle);

			// check compile status
			if (!CheckShaderCompileStatus(handle))
				return;

			shaders.push_back(handle);
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

		bool CheckLinkStatus(GLuint program)
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

		bool Link()
		{
			program = glCreateProgram();

			// attach all shaders
			for (auto shader : shaders) {
				glAttachShader(program, shader);
			}

			// setup attribute locations
			for (auto [loc, name] : VertexAttributes) {
				glBindAttribLocation(program, loc, name);
			}

			// setup uniform locations

			// link
			glLinkProgram(program);
			const bool linked = CheckLinkStatus(program);

			// detach shaders
			for (auto shader : shaders) {
				glDetachShader(program, shader);
			}

			// cleanup if link unsuccessful
			if (!linked) {
				glDeleteProgram(program);
				program = 0;
			}

			return linked;
		}

		GLuint GetProgram() const { return program; }
	private:
		GLuint program;
		std::vector<GLuint> shaders;
	};

	/**********************************************************************************/

	ShaderProgram::ShaderProgram(const std::string& vs_file, const std::string& fs_file)
	{
		ShaderProgramBuilder builder;

		builder.AddShader(GL_VERTEX_SHADER, vs_file);
		builder.AddShader(GL_FRAGMENT_SHADER, fs_file);
		builder.Link();

		program = builder.GetProgram();
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