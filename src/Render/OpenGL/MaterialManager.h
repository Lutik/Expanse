#pragma once

#include "ShaderProgram.h"
#include "TextureManager.h"
#include "Render/ResourceHandles.h"
#include "Render/MaterialParameters.h"


namespace Expanse::Render::GL
{
	class MaterialManager
	{
	public:		
		Material Create(const std::string& file);
		Material Create(Material material);
		void Free(Material material);

		void SetParameter(Material material, std::string_view name, const MaterialParameterValue& value);

		void Bind(Material material);


		Texture CreateTexture(const std::string& file);
		void FreeTexture(Texture texture);

	private:
		ShaderManager shaders;
		TextureManager textures;

		struct MaterialParameter
		{
			std::string name;
			GLint location;
			MaterialParameterValue value;

			GLenum type;
		};

		struct MaterialResource
		{
			Shader shader;
			std::vector<MaterialParameter> parameters;

			bool IsFree() const { return !shader.IsValid(); }
		};

		std::vector<MaterialResource> materials;


		Material CreateEmpty();
	};
}