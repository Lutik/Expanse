#pragma once

#include "Render/ResourceHandles.h"

namespace Expanse::Render::GL
{
	struct ShaderProgram : public Handle {};

	class ShaderProgramsManager
	{
	public:
		ShaderProgram Create(const std::string& file);
		void Free(ShaderProgram program);
		void Use(ShaderProgram program);

	private:
		struct ShaderProgramResource {
			GLuint id = 0;
			size_t use_count = 0;
			std::string name;
		};
		std::vector<ShaderProgramResource> shader_programs;
		GLuint current_shader_program = 0;
	};
}