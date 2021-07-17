#pragma once

#include "Render/ResourceHandles.h"

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
		Shader Create(const std::string& file);
		void Free(Shader program);
		void Use(Shader program);

		std::vector<ShaderUniformInfo> GetShaderUnifromsInfo(Shader shader);
	private:
		struct ShaderResource {
			GLuint id = 0;
			size_t use_count = 0;
			std::string name;

			bool IsFree() const { return use_count == 0; }
		};
		std::vector<ShaderResource> shaders;
		GLuint current_shader = 0;
	};
}