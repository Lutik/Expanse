#pragma once

#include "Render/RenderTypes.h"

namespace Expanse::Render::GL
{
	struct Shader : public Handle {};

	struct ShaderUniformInfo
	{
		std::string name;
		GLint location;
	};

	class ShaderManager
	{
	public:
		ShaderManager() = default;
		~ShaderManager();
		ShaderManager(const ShaderManager&) = delete;
		ShaderManager& operator=(const ShaderManager&) = delete;

		Shader Create(const std::string& file);
		void Free(Shader program);
		void Use(Shader program);

		// Retrieve names and locations of all uniforms in shader program
		std::vector<ShaderUniformInfo> GetShaderUniformsInfo(Shader shader) const;

		// Retrieve names of all uniform blocks in shader program
		std::vector<std::string> GetShaderUniformBlocks(Shader shader) const;

		void BindUniformBlock(Shader shader, GLuint binding_point, const std::string& name);
	private:
		struct ShaderResource
		{
			GLuint id = 0;
			size_t use_count = 0;
			std::string name;
		};
		std::vector<ShaderResource> shaders;
		GLuint current_shader = 0;
	};
}