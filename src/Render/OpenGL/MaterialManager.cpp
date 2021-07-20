#include "pch.h"

#include "MaterialManager.h"

#include "Utils.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include "glm/gtc/type_ptr.hpp"

namespace Expanse::Render::GL
{
	struct UniformSetterVisitor
	{
		GLint loc;
		GLint tex_unit = 0;

		void operator()(NoValue value) {}
		void operator()(float value) { glUniform1f(loc, value); }
		void operator()(const glm::vec2& value) { glUniform2fv(loc, 1, glm::value_ptr(value)); }
		void operator()(const glm::vec3& value) { glUniform3fv(loc, 1, glm::value_ptr(value)); }
		void operator()(const glm::vec4& value) { glUniform4fv(loc, 1, glm::value_ptr(value)); }
		void operator()(const glm::mat4& value) { glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value)); }
		void operator()(Texture tex) { glUniform1i(loc, tex_unit); }
	};


	MaterialParameterValue FloatVectorValueFromJson(nlohmann::json jvalue)
	{
		MaterialParameterValue value;

		float buf[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		const size_t value_size = std::min(std::size(buf), jvalue.size());
		for (size_t i = 0; i < value_size; ++i) {
			buf[i] = jvalue[i].get<float>();
		}

		switch (value_size) {
			case 1: value = buf[0]; break;
			case 2: value = glm::vec2{ buf[0], buf[1] }; break;
			case 3: value = glm::vec3{ buf[0], buf[1], buf[2] }; break;
			case 4: value = glm::vec4{ buf[0], buf[1], buf[2], buf[3] }; break;
			default: value = 0.0f;
		}

		return value;
	}

	MaterialParameterValue MaterialManager::ParamValueFromJson(nlohmann::json jvalue)
	{
		MaterialParameterValue value;

		if (jvalue.is_array())
		{			
			if (jvalue[0].is_number_float())
			{
				// must be some kind of vector
				value = FloatVectorValueFromJson(jvalue);
			}
			else if (jvalue[0].is_array())
			{
				// TODO: read matrix values
			}
		}
		else if (jvalue.is_string())
		{
			// must be texture
			value = CreateTexture(jvalue.get<std::string>());
		}

		return value;
	}

	Material MaterialManager::CreateEmpty()
	{
		const auto index = GetFreeIndexInVector(materials, [](const auto& mat) { return mat.IsFree(); });
		return { index };
	}

	Material MaterialManager::Create(const std::string& file)
	{
		const auto file_content = File::LoadContents(file);
		if (file_content.empty()) {
			Log::message("Could not load material '{}', file not found", file);
			return Material{};
		}

		auto json_mat = nlohmann::json::parse(file_content);

		const auto handle = CreateEmpty();

		auto& mat = materials[handle.index];

		// create shader
		const auto shader_file = json_mat["shader"].get<std::string>();
		mat.shader = shaders.Create(shader_file);

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
		const auto jparams = json_mat["parameters"];
		for (auto& [name, jvalue] : jparams.items())
		{
			SetParameter(handle, name, ParamValueFromJson(jvalue));
		}

		return handle;
	}

	Material MaterialManager::Create(Material material)
	{
		if (!material.IsValid()) return material;

		const auto handle = CreateEmpty();

		materials[handle.index] = materials[material.index];

		return handle;
	}

	void MaterialManager::Free(Material material)
	{
		if (!material.IsValid()) return;

		auto& mat = materials[material.index];

		// free shader
		shaders.Free(mat.shader);

		// free textures
		for (auto& param : mat.parameters) {
			if (auto* tex = std::get_if<Texture>(&param.value)) {
				textures.Free(*tex);
			}
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

		itr->value = value;
	}

	void MaterialManager::Bind(Material material)
	{
		if (!material.IsValid()) return;

		const auto& mat = materials[material.index];

		// bind shader
		shaders.Use(mat.shader);

		// set uniforms and bind textures
		UniformSetterVisitor set_param;
		for (const auto& param : mat.parameters)
		{
			set_param.loc = param.location;		
			std::visit(set_param, param.value);

			// if parameter is texture, bind it and increment texture unit
			if (auto* tex = std::get_if<Texture>(&param.value))
			{
				textures.Bind(*tex, set_param.tex_unit);
				set_param.tex_unit++;
			}
		}
	}

	Texture MaterialManager::CreateTexture(const std::string& file)
	{
		return textures.Create(file);
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
}