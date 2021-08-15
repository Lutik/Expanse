#include "pch.h"

#include "MaterialManager.h"

#include "Common/ResourceDescriptions.h"

#include "Utils.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include "glm/gtc/type_ptr.hpp"

namespace Expanse::Render::GL
{
	struct UniformSetterVisitor
	{
		TextureManager& tex_mgr;
		GLint loc;
		GLint tex_unit = 0;	

		void operator()(NoValue value) {}
		void operator()(float value) { glUniform1f(loc, value); }
		void operator()(const glm::vec2& value) { glUniform2fv(loc, 1, glm::value_ptr(value)); }
		void operator()(const glm::vec3& value) { glUniform3fv(loc, 1, glm::value_ptr(value)); }
		void operator()(const glm::vec4& value) { glUniform4fv(loc, 1, glm::value_ptr(value)); }
		void operator()(const glm::mat4& value) { glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value)); }
		void operator()(Texture tex) { glUniform1i(loc, tex_unit); }
		void operator()(const TextureName& tex_name) { operator()(tex_mgr.Create(tex_name)); }
	};

	void MaterialManager::Init()
	{
		gl_state.Init();
	}

	Material MaterialManager::CreateEmpty()
	{
		const auto index = GetFreeIndexInVector(materials, [](const auto& mat) { return mat.IsFree(); });
		return { index };
	}

	Material MaterialManager::Create(const std::string& file)
	{
		auto desc = LoadMaterialDescription(file);
		if (!desc) {
			return Material{};
		}

		const auto handle = CreateEmpty();

		auto& mat = materials[handle.index];

		// create shader
		mat.shader = shaders.Create(desc->shader_file);

		// fill valid parameter names and locations from shader
		for (const auto& [name, loc] : shaders.GetShaderUniformsInfo(mat.shader))
		{		
			auto& param = mat.parameters.emplace_back();
			param.name = name;
			param.location = loc;
		}

		// setup uniform blocks bindings
		for (size_t bp = 0; bp < globals.size(); ++bp)
		{
			shaders.BindUniformBlock(mat.shader, static_cast<GLuint>(bp), globals[bp].name);
		}

		// read and set initial parameter values
		for (auto& [name, value] : desc->params)
		{
			SetParameter(handle, name, value);
		}

		// copy properties
		mat.properties = desc->properties;

		return handle;
	}

	Material MaterialManager::Create(Material material)
	{
		if (!material.IsValid()) return material;

		const auto handle = CreateEmpty();

		auto& mat = materials[handle.index];

		// copy another material
		mat = materials[material.index];

		// Inc reference count for used shader and textures
		shaders.Use(mat.shader);
		for (const auto& param : mat.parameters) {
			UseParam(param.value);
		}

		return handle;
	}

	void MaterialManager::Free(Material material)
	{
		if (!material.IsValid()) return;

		auto& mat = materials[material.index];

		// free shader and textures
		shaders.Free(mat.shader);
		for (auto& param : mat.parameters) {
			FreeParam(param.value);
		}

		mat = {};
	}

	void MaterialManager::SetParameter(Material material, std::string_view name, const MaterialParameterValue& value)
	{
		if (!material.IsValid()) return;

		auto& mat = materials[material.index];

		auto itr = std::ranges::find_if(mat.parameters, [name](const auto& param) {
			return param.name == name;
		});

		if (itr == mat.parameters.end()) {
			// invalid material parameter name
			return;
		}

		// free previously used texture
		FreeParam(itr->value);

		if (auto tex_name = std::get_if<TextureName>(&value))
		{
			// texture names should be converted to texture handles
			itr->value = textures.Create(*tex_name);
		}
		else
		{
			itr->value = value;
			UseParam(itr->value);
		}
	}

	void MaterialManager::Bind(Material material)
	{
		if (!material.IsValid()) return;

		const auto& mat = materials[material.index];

		// bind shader
		shaders.Bind(mat.shader);

		// set uniforms and bind textures
		UniformSetterVisitor set_param{ textures };
		for (const auto& param : mat.parameters)
		{
			set_param.loc = param.location;		
			std::visit(set_param, param.value);

			// strings should be converted to texture handles in advance
			assert(std::get_if<TextureName>(&param.value) == nullptr);

			// if parameter is texture, bind it and increment texture unit
			if (auto* tex = std::get_if<Texture>(&param.value))
			{
				textures.Bind(*tex, set_param.tex_unit);
				set_param.tex_unit++;
			}
		}

		// set opengl state
		gl_state.Set(mat.properties);
	}

	Texture MaterialManager::CreateTexture(const std::string& file)
	{
		return textures.Create(file);
	}

	Texture MaterialManager::CreateTexture(std::string_view name, const TextureDescription& tex_info)
	{
		return textures.Create(name, tex_info);
	}

	void MaterialManager::FreeTexture(Texture texture)
	{
		textures.Free(texture);
	}

	void MaterialManager::SetGlobalParam(std::string_view name, const void* data_ptr, size_t data_size)
	{
		auto itr = std::ranges::find_if(globals, [=](const auto& gp) { return gp.name == name; });
		if (itr == globals.end())
		{
			auto& param = globals.emplace_back();
			param.name = name;

			glGenBuffers(1, &param.buffer);
			glBindBuffer(GL_UNIFORM_BUFFER, param.buffer);
			glBufferData(GL_UNIFORM_BUFFER, data_size, data_ptr, GL_STATIC_DRAW);

			glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(globals.size() - 1), param.buffer);
		}
		else
		{
			glBindBuffer(GL_UNIFORM_BUFFER, itr->buffer);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, data_size, data_ptr);
		}
	}

	MaterialManager::~MaterialManager()
	{
		for (const auto& gp : globals)
		{
			glDeleteBuffers(1, &gp.buffer);
		}
	}

	void MaterialManager::UseParam(const MaterialParameterValue& param)
	{
		if (auto* tex = std::get_if<Texture>(&param)) {
			textures.Use(*tex);
		}
	}

	void MaterialManager::FreeParam(const MaterialParameterValue& param)
	{
		if (auto* tex = std::get_if<Texture>(&param)) {
			textures.Free(*tex);
		}
	}
}