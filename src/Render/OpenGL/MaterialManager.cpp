#include "pch.h"

#include "MaterialManager.h"

#include "Utils.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include "glm/gtc/type_ptr.hpp"

namespace Expanse::Render::GL
{
	using namespace nlohmann;

	struct UniformSetterVisitor
	{
		GLint loc;

		void operator()(float value) { glUniform1f(loc, value); }
		void operator()(const glm::vec2& value) { glUniform2fv(loc, 1, glm::value_ptr(value)); }
		void operator()(const glm::vec3& value) { glUniform3fv(loc, 1, glm::value_ptr(value)); }
		void operator()(const glm::vec4& value) { glUniform4fv(loc, 1, glm::value_ptr(value)); }
	};


	MaterialParameterValue ParamValueFromJson(json jvalue)
	{
		MaterialParameterValue value = 0.0f;

		if (jvalue.is_array())
		{
			// must be some kind of vector (integer or floating point)
			if (jvalue[0].is_number_float())
			{
				float buf[4];
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
			}
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
		auto json_mat = json::parse(file_content);

		const auto handle = CreateEmpty();

		auto& mat = materials[handle.index];

		// create shader
		const auto shader_file = json_mat["shader"].get<std::string>();
		mat.shader = shaders.Create(shader_file);

		// fill valid parameter names and locations from shader
		for (const auto& [name, loc] : shaders.GetShaderUnifromsInfo(mat.shader))
		{		
			auto& param = mat.parameters.emplace_back();
			param.name = name;
			param.location = loc;
		}

		// TODO: read and set initial parameter values
		const auto jparams = json_mat["parameters"];
		for (auto& [name, jvalue] : jparams.items())
		{
			SetParameter(handle, name, ParamValueFromJson(jvalue));
		}

		return handle;
	}

	Material MaterialManager::Create(Material material)
	{
		const auto handle = CreateEmpty();

		materials[handle.index] = materials[material.index];

		return handle;
	}

	void MaterialManager::Free(Material material)
	{
		if (!material.IsValid()) return;

		auto& mat = materials[material.index];

		shaders.Free(mat.shader);	

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

		// set uniforms
		for (const auto& param : mat.parameters) {
			UniformSetterVisitor set_param{ param.location };
			std::visit(set_param, param.value);
		}
	}
}