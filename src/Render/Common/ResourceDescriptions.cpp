#include "pch.h"

#include "ResourceDescriptions.h"

#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"

namespace Expanse::Render
{
	namespace
	{
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

		MaterialParameterValue ParamValueFromJson(nlohmann::json jvalue)
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
				value = jvalue.get<std::string>();
			}

			return value;
		}
	}

	std::optional<MaterialDescription> LoadMaterialDescription(const std::string& file)
	{
		const auto file_content = File::LoadContents(file);
		if (file_content.empty()) {
			Log::message("Could not load material '{}', file not found", file);
			return std::nullopt;
		}


		auto json_mat = nlohmann::json::parse(file_content);

		MaterialDescription desc;

		// shader file
		desc.shader_file = json_mat["shader"].get<std::string>();

		// material parameters
		const auto jparams = json_mat["parameters"];
		for (auto& [name, jvalue] : jparams.items())
		{
			desc.params.push_back({ name, ParamValueFromJson(jvalue) });
		}

		return desc;
	}
}