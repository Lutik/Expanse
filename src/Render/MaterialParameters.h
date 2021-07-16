#pragma once

#include <variant>

#include "Utils/Math.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace Expanse::Render
{
	using MaterialParameterValue = std::variant<float, glm::vec2, glm::vec3, glm::vec4>;
}