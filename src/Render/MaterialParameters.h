#pragma once

#include <variant>

#include "Utils/Math.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"

#include "ResourceHandles.h"

namespace Expanse::Render
{
	struct NoValue {};
	using MaterialParameterValue = std::variant<NoValue, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4, Texture>;
}