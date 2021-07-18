#include "pch.h"

#include "ShaderProgram.h"
#include "VertexTypes.h"
#include "Utils.h"

#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include <fstream>
#include <sstream>

namespace Expanse::Render::GL
{
	namespace
	{
		struct VertexAttributePair
		{
			VertexElementUsage usage;
			const char* name;
		};

		constexpr VertexAttributePair VertexAttributes[] = {
			{ VertexElementUsage::POSITION, "position" },
			{ VertexElementUsage::COLOR, "color" },
			{ VertexElementUsage::TEXCOORD0, "uv" },
			{ VertexElementUsage::TEXCOORD0, "uv0" },
			{ VertexElementUsage::TEXCOORD1, "uv1" },
		};

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
			for (auto [usage, name] : VertexAttributes) {
				const auto loc = static_cast<int>(usage);
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

			// free shaders after program is built
			for (auto shader : shaders) {
				glDeleteShader(shader);
			}

			return program;
		}
	}

	/**********************************************************************************/

	void ShaderManager::Use(Shader program)
	{
		if (!program.IsValid()) return;

		const GLuint id = shaders[program.index].id;
		if (id != current_shader) {
			glUseProgram(id);
			current_shader = id;
		}
	}

	Shader ShaderManager::Create(const std::string& file)
	{
		auto itr = std::find_if(shaders.begin(), shaders.end(), [&file](const auto& res) {
			return res.name == file;
		});

		if (itr != shaders.end())
		{
			itr->use_count++;
			const size_t index = itr - shaders.begin();
			return { index };
		}
		else
		{
			const size_t index = GetFreeIndexInVector(shaders, [](const auto& res) { return res.IsFree(); });
			auto& res = shaders[index];
			res.id = LoadShaderProgramFromFile(file);
			res.use_count = 1;
			res.name = file;
			return { index };
		}
	}

	void ShaderManager::Free(Shader handle)
	{
		if (!handle.IsValid()) return;

		auto& res = shaders[handle.index];
		res.use_count--;
		if (res.use_count == 0)
		{
			glDeleteShader(res.id);
			res.id = 0;
			res.name.clear();
		}
	}

	std::vector<ShaderUniformInfo> ShaderManager::GetShaderUnifromsInfo(Shader shader)
	{
		std::vector<ShaderUniformInfo> result;

		if (!shader.IsValid()) return result;

		const GLuint program = shaders[shader.index].id;

		GLint uniform_count = 0;
		glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniform_count);
		result.reserve(uniform_count);

		GLint max_uniform_name_length = 0;
		glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_uniform_name_length);

		std::vector<char> buffer(max_uniform_name_length + 1, 0);

		for (GLint idx = 0; idx < uniform_count; ++idx)
		{
			GLsizei name_length = 0;
			GLint size = 0;
			GLenum type;
			glGetActiveUniform(program, idx, max_uniform_name_length, &name_length, &size, &type, buffer.data());

			const GLint loc = glGetUniformLocation(program, buffer.data());
			
			auto& uniform_info = result.emplace_back();
			uniform_info.name = std::string(buffer.data(), name_length);
			uniform_info.location = loc;
		}

		return result;
	}
}