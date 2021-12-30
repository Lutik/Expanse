#include "Input.h"

namespace Expanse::Input
{
	InputState::InputState()
	{
		keyboard_state.fill(ButtonState::Up);
		mouse_state.fill(ButtonState::Up);
	}

	bool InputState::KeyPressed(KeyCode key) const
	{
		return keyboard_state[key] == ButtonState::Pressed;
	}

	bool InputState::KeyReleased(KeyCode key) const
	{
		return keyboard_state[key] == ButtonState::Released;
	}

	bool InputState::IsKeyDown(KeyCode key) const
	{
		return (keyboard_state[key] == ButtonState::Down) || (keyboard_state[key] == ButtonState::Pressed);
	}

	bool InputState::MousePressed(int button) const
	{
		return mouse_state[button] == ButtonState::Pressed;
	}

	bool InputState::MouseReleased(int button) const
	{
		return mouse_state[button] == ButtonState::Released;
	}

	bool InputState::IsMouseDown(int button) const
	{
		return (mouse_state[button] == ButtonState::Down) || (mouse_state[button] == ButtonState::Pressed);
	}

	void InputState::Update()
	{
		mouse_wheel = 0;
		mouse_pos_rel = { 0, 0 };

		auto update_key_state = [](auto& key_state)
		{
			if (key_state == ButtonState::Pressed) {
				key_state = ButtonState::Down;
			}
			else if (key_state == ButtonState::Released) {
				key_state = ButtonState::Up;
			}
		};

		if (key_state_changed) {
			std::ranges::for_each(keyboard_state, update_key_state);
			key_state_changed = false;
		}
		if (mouse_key_state_changed) {
			std::ranges::for_each(mouse_state, update_key_state);
			mouse_key_state_changed = false;
		}
	}


	InputState g_input_state;
}