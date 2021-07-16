#pragma once

#include "Render/ResourceHandles.h"

namespace Expanse::Render::GL
{
	struct ShaderProgram : public Handle {};

	struct ShaderUniformInfo
	{
		std::string name;
		GLint location;
	};

	class ShaderProgramsManager
	{
	public:
		ShaderProgram Create(const std::string& file);
		void Free(ShaderProgram program);
		void Use(ShaderProgram program);

		std::vector<ShaderUniformInfo> GetShaderUnifromsInfo(ShaderProgram shader);
	private:
		struct ShaderProgramResource {
			GLuint id = 0;
			size_t use_count = 0;
			std::string name;

			bool IsFree() const { return use_count == 0; }
		};
		std::vector<ShaderProgramResource> shader_programs;
		GLuint current_shader_program = 0;
	};
}