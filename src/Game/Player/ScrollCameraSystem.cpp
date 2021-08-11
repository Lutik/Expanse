#include "ScrollCameraSystem.h"

#include "Game/World.h"

namespace Expanse::Game::Player
{
	ScrollCamera::ScrollCamera(World& w)
		: ISystem(w)
	{
	}

	void ScrollCamera::Update()
	{
		UpdateMovement();
		UpdateZoom();
	}

	void ScrollCamera::UpdateMovement()
	{
		static constexpr std::pair<Input::KeyCode, FPoint> MoveBindings[] = {
			{ Input::Key::Left, { -1.0f, 0.0f}},
			{ Input::Key::Right, { 1.0f, 0.0f}},
			{ Input::Key::Up, { 0.0f, 1.0f}},
			{ Input::Key::Down, { 0.0f, -1.0f}},
		};

		FPoint cam_offset = { 0.0f, 0.0f };
		for (auto [key, dir] : MoveBindings) {
			if (world.input.IsKeyDown(key)) {
				cam_offset += dir;
			}
		}

		const float scene_cam_speed = camera_speed / world.camera_scale;
		world.camera_pos += cam_offset * (scene_cam_speed * world.dt);
	}

	void ScrollCamera::UpdateZoom()
	{
		if (world.input.mouse_wheel > 0) {
			world.camera_scale *= 1.5f;
		}
		if (world.input.mouse_wheel < 0) {
			world.camera_scale /= 1.5f;
		}
	}
}