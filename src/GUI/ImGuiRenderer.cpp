
#include "ImGuiRenderer.h"

#include <GL/glew.h>

#include "glm/mat4x4.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace Expanse
{
    static const Render::VertexLayout ImGuiVertexFormat = { sizeof(ImDrawVert),
    {
        { Render::VertexElementUsage::POSITION, sizeof(ImDrawVert::pos), offsetof(ImDrawVert, pos), 4, false, false },
        { Render::VertexElementUsage::TEXCOORD0, sizeof(ImDrawVert::uv), offsetof(ImDrawVert, uv), 4, false, false },
        { Render::VertexElementUsage::COLOR, sizeof(ImDrawVert::col), offsetof(ImDrawVert, col), 1, true, false },
    } };

    ImGuiRenderer::ImGuiRenderer(Render::IRenderer* render)
        : renderer(render)
    {
        CreateFontTexture();

        gui_material = renderer->CreateMaterial("content/materials/imgui.json");

        gui_mesh = renderer->CreateMesh();
        renderer->SetMeshPrimitiveType(gui_mesh, Expanse::Render::PrimitiveType::Triangles);

        ImGuiIO& io = ImGui::GetIO();
        io.BackendRendererName = "imgui_impl_expanse_gl330";
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    }

    ImGuiRenderer::~ImGuiRenderer()
    {
        renderer->FreeMaterial(gui_material);
        renderer->FreeMesh(gui_mesh);

        FreeFontTexture();
    }

    void ImGuiRenderer::StartFrame()
    {
        ImGui::NewFrame();
    }

    void ImGuiRenderer::EndFrame()
    {
        ImGui::Render();

        RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiRenderer::SetupRenderState(ImDrawData* draw_data, int fb_width, int fb_height)
    {
        // Setup viewport, orthographic projection matrix
        renderer->SetViewport({ 0, 0, fb_width, fb_height });

        float L = draw_data->DisplayPos.x;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
        float T = draw_data->DisplayPos.y;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
        const auto projection = glm::orthoLH_NO(L, R, B, T, -1.0f, 1.0f);

        renderer->SetViewProjection(glm::mat4{ 1.0f }, projection);
    }

    void ImGuiRenderer::RenderDrawData(ImDrawData* draw_data)
    {
        // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
        int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
        int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
        if (fb_width <= 0 || fb_height <= 0)
            return;

        // Setup desired GL state
        SetupRenderState(draw_data, fb_width, fb_height);

        // Will project scissor/clipping rectangles into framebuffer space
        const ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
        const ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

        // Render command lists
        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];

            // Upload vertex/index buffers
            Expanse::Render::BufferData vertex_data{ cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert) };
            Expanse::Render::BufferData index_data{ cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx) };

            renderer->SetMeshVertices(gui_mesh, vertex_data, ImGuiVertexFormat);
            renderer->SetMeshIndices(gui_mesh, index_data, 2);

            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
            {
                const ImDrawCmd& cmd = cmd_list->CmdBuffer[cmd_i];
                if (cmd.UserCallback)
                {
                    // User callback, registered via ImDrawList::AddCallback()
                    // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                    if (cmd.UserCallback == ImDrawCallback_ResetRenderState)
                        SetupRenderState(draw_data, fb_width, fb_height);
                    else
                        cmd.UserCallback(cmd_list, &cmd);
                }
                else
                {
                    // Project scissor/clipping rectangles into framebuffer space
                    ImVec4 clip_rect;
                    clip_rect.x = (cmd.ClipRect.x - clip_off.x) * clip_scale.x;
                    clip_rect.y = (cmd.ClipRect.y - clip_off.y) * clip_scale.y;
                    clip_rect.z = (cmd.ClipRect.z - clip_off.x) * clip_scale.x;
                    clip_rect.w = (cmd.ClipRect.w - clip_off.y) * clip_scale.y;

                    if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                    {
                        // Apply scissor/clipping rectangle
                        Rect scissor_rect{ (int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y) };
                        renderer->SetScissor(scissor_rect);

                        // Bind texture and draw
                        renderer->SetMaterialParameter(gui_material, "tex", Render::Texture{ (size_t)cmd.GetTexID() });
                        renderer->DrawIndexRange(gui_mesh, gui_material, cmd.IdxOffset, cmd.ElemCount, cmd.VtxOffset);
                    }
                }
            }
        }

        // Restore default render state
        renderer->ResetViewport();
        renderer->ResetScissor();
    }

    void ImGuiRenderer::CreateFontTexture()
    {
        // Build texture atlas
        ImGuiIO& io = ImGui::GetIO();
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        // Upload texture to graphics system
        Render::TextureDescription tex_data;

        tex_data.image.data.reset(new uint8_t[width * height * 4]);
        std::copy(pixels, pixels + width * height * 4, tex_data.image.data.get());

        tex_data.image.width = width;
        tex_data.image.height = height;
        tex_data.image.format = Image::ColorFormat::RGBA_8;
        tex_data.filter_type = Render::TextureFilterType::Linear;
        tex_data.address_mode = Render::TextureAddressMode::Repeat;
        tex_data.use_mipmaps = false;

        font_texture = renderer->CreateTexture("imgui_font", tex_data);

        io.Fonts->SetTexID((ImTextureID)font_texture.index);
    }

    void ImGuiRenderer::FreeFontTexture()
    {
        renderer->FreeTexture(font_texture);
        font_texture = Render::Texture{};

        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->SetTexID((ImTextureID)font_texture.index);
    }
}