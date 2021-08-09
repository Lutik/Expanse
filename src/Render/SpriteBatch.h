#pragma once

#include "IRenderer.h"

#include <ranges>

namespace Expanse::Render
{
	template<typename Vertex, typename Index = uint16_t>
	class SpriteBatch
	{
	public:
		explicit SpriteBatch(IRenderer* r)
			: renderer(r)
		{
			assert(renderer);
			mesh = renderer->CreateMesh();
			renderer->SetMeshPrimitiveType(mesh, PrimitiveType::Triangles);
		}

		template<typename VRange, typename IRange>
		void Draw(VRange&& verts, IRange&& inds, Render::Material material)
		{
			if (material.index != current_material.index) {
				Flush();
				current_material = material;
			}

			auto rebase_idx = [base = static_cast<Index>(vertices.size())](Index idx){ return base + idx; };
			auto rebased_indices = inds | std::views::transform(rebase_idx);

			vertices.insert(vertices.end(), verts.begin(), verts.end());
			indices.insert(indices.end(), rebased_indices.begin(), rebased_indices.end());
		}

		template<typename VRange, typename IRange>
		void DrawWithOffset(FPoint offset, VRange&& verts, IRange&& inds, Render::Material material)
		{
			auto offset_verts = verts | std::views::transform([offset](auto vert) { vert.position += offset; return vert; });
			Draw(offset_verts, inds, material);
		}

		void Flush()
		{
			if (current_material.IsValid())
			{
				renderer->SetMeshVertices(mesh, vertices);
				renderer->SetMeshIndices(mesh, indices);
				renderer->Draw(mesh, current_material);
			}

			vertices.clear();
			indices.clear();
			current_material = Render::Material{};
		}
	//private:
		IRenderer* renderer = nullptr;

		std::vector<Vertex> vertices;
		std::vector<Index> indices;
		Render::Material current_material;
		Render::Mesh mesh;
	};
}