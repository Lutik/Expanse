#include "pch.h"

#include "ShaderProgram.h"
#include "VertexTypes.h"
#include "Utils.h"

#include "Utils/FileUtils.h"
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
			{ VertexElementUsage::POSITION, "a_position" },
			{ VertexElementUsage::COLOR, "a_color" },
			{ VertexElementUsage::TEXCOORD0, "a_uv" },
			{ VertexElementUsage::TEXCOORD0, "a_uv0" },
			{ VertexElementUsage::TEXCOORD1, "a_uv1" },
			{ VertexElementUsage::NORMAL, "a_normal" },
		};

		GLenum GLShaderTypeFromString(std::string_view type_str)
		{
			GLenum shader_type = GL_VERTEX_SHADER;
			if (type_str == "vertex")
				shader_type = GL_VERTEX_SHADER;
			else if (type_str == "fragment")
				shader_type = GL_FRAGMENT_SHADER;
			else if (type_str == "geometry")
				shader_type = GL_GEOMETRY_SHADER;
			else
				Log::message("Unknown shader type '{}'", type_str);
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

		// Splits whole shader program source into parts, representing different shader stages.
		// Beginning of each part must be marked with $<shader type> in source
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

	ShaderManager::~ShaderManager()
	{
		for (const auto& shader : shaders)
		{
			glDeleteProgram(shader.id);
		}
	}

	void ShaderManager::Bind(Shader program)
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
			const size_t index = GetFreeIndexInVector(shaders, [](const auto& res) { return res.use_count == 0; });
			auto& res = shaders[index];
			res.id = LoadShaderProgramFromFile(file);
			res.use_count = 1;
			res.name = file;
			return { index };
		}
	}

	void ShaderManager::Use(Shader handle)
	{
		if (!handle.IsValid()) return;

		shaders[handle.index].use_count++;
	}

	void ShaderManager::Free(Shader handle)
	{
		if (!handle.IsValid()) return;

		auto& res = shaders[handle.index];
		res.use_count--;
		if (res.use_count == 0)
		{
			if (current_shader == res.id) {
				current_shader = 0;
			}
			glDeleteProgram(res.id);
			res.id = 0;
			res.name.clear();
		}
	}

	std::vector<ShaderUniformInfo> ShaderManager::GetShaderUniformsInfo(Shader shader) const
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

			// some uniforms will have negative location and are unassignable (those belonging to uniform blocks for example)
			if (loc >= 0)
			{
				auto& uniform_info = result.emplace_back();
				uniform_info.name = std::string(buffer.data(), name_length);;
				uniform_info.location = glGetUniformLocation(program, buffer.data());
			}
		}

		return result;
	}

	std::vector<std::string> ShaderManager::GetShaderUniformBlocks(Shader shader) const
	{
		std::vector<std::string> result;

		if (!shader.IsValid()) return result;

		const GLuint program = shaders[shader.index].id;

		GLint block_count = 0;
		glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &block_count);
		result.reserve(block_count);

		GLint max_block_name_length = 0;
		glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &max_block_name_length);

		std::vector<char> buffer(max_block_name_length + 1, 0);

		for (GLint idx = 0; idx < block_count; ++idx)
		{
			GLsizei name_length = 0;
			glGetActiveUniformBlockName(program, idx, max_block_name_length, &name_length, buffer.data());

			result.emplace_back(buffer.data(), name_length);
		}

		return result;
	}

	void ShaderManager::BindUniformBlock(Shader shader, GLuint binding_point, const std::string& name)
	{
		if (!shader.IsValid()) return;

		const GLuint program = shaders[shader.index].id;

		const GLuint block_index = glGetUniformBlockIndex(program, name.c_str());
		if (block_index != GL_INVALID_INDEX)
		{
			glUniformBlockBinding(program, block_index, binding_point);
		}
	}
}