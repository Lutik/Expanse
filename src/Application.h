#pragma once

#include <vector>
#include <string>

#include <GL/glew.h>

#include "Render/IRenderer.h"
#include "Render/VertexTypes.h"

#include "Render/OpenGL/ShaderProgram.h"
#include "Render/OpenGL/VertexArrayObject.h"

namespace Expanse
{
    class Application
    {
    public:
        void Init();

        void Tick();

    private:
        std::unique_ptr<Render::IRenderer> renderer;

        Render::GL::ShaderProgram shader;
        Render::GL::VertexArray vao;
    };
}