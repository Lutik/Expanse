
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

    struct OpenGLState
    {
        GLenum last_active_texture;
        GLuint last_program;
        GLuint last_texture;
        GLuint last_array_buffer;
        GLuint last_vertex_array_object;
        GLint last_polygon_mode[2];
        GLint last_viewport[4];
        GLint last_scissor_box[4];
        GLenum last_blend_src_rgb;
        GLenum last_blend_dst_rgb;
        GLenum last_blend_src_alpha;
        GLenum last_blend_dst_alpha;
        GLenum last_blend_equation_rgb;
        GLenum last_blend_equation_alpha;
        GLboolean last_enable_blend;
        GLboolean last_enable_cull_face;
        GLboolean last_enable_depth_test;
        GLboolean last_enable_stencil_test;
        GLboolean last_enable_scissor_test;
        GLboolean last_enable_primitive_restart;

        OpenGLState()
        {
            glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
            glActiveTexture(GL_TEXTURE0);
            glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&last_program);
            glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&last_texture);
            glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint*)&last_array_buffer);
            glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&last_vertex_array_object);
    #ifdef GL_POLYGON_MODE
            glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
    #endif
            glGetIntegerv(GL_VIEWPORT, last_viewport);
            glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
            glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
            glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
            glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
            glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
            glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
            glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
            last_enable_blend = glIsEnabled(GL_BLEND);
            last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
            last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
            last_enable_stencil_test = glIsEnabled(GL_STENCIL_TEST);
            last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
            last_enable_primitive_restart = glIsEnabled(GL_PRIMITIVE_RESTART);
        }

        ~OpenGLState()
        {
            glUseProgram(last_program);
            glBindTexture(GL_TEXTURE_2D, last_texture);
            glActiveTexture(last_active_texture);
            glBindVertexArray(last_vertex_array_object);
            glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
            glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
            glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
            if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
            if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
            if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
            if (last_enable_stencil_test) glEnable(GL_STENCIL_TEST); else glDisable(GL_STENCIL_TEST);
            if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
            if (last_enable_primitive_restart) glEnable(GL_PRIMITIVE_RESTART); else glDisable(GL_PRIMITIVE_RESTART);

            glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
            glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
            glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
        }
    };

    ImGuiRenderer::ImGuiRenderer(Render::IRenderer* render)
        : renderer(render)
    {
        CreateFontTexture();

        gui_material = renderer->CreateMaterial("content/materials/imgui.json");

        gui_mesh = renderer->CreateMesh();
        renderer->SetMeshPrimitiveType(gui_mesh, Expanse::Render::PrimitiveType::Triangles);

        ImGuiIO& io = ImGui::GetIO();
        io.BackendRendererName = "imgui_impl_expanse_gl330";
        //io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
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
        // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glEnable(GL_SCISSOR_TEST);
        glDisable(GL_PRIMITIVE_RESTART);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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

        // Backup GL state
        OpenGLState push_state;

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

                    if (clip_rect.x < fb_width && clip_rect.
                        y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                    {
                        // Apply scissor/clipping rectangle
                        glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));

                        // Bind texture, Draw
                        Render::Texture tex{ (size_t)cmd.GetTexID() };
                        renderer->SetMaterialParameter(gui_material, "tex", tex);

                        renderer->Draw(gui_mesh, gui_material);

                        // glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)), (GLint)pcmd->VtxOffset);
                    }
                }
            }
        }
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