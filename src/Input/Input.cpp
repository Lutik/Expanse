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

	void ProcessEvent(const SDL_Event& event, InputState& input)
	{
		if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.scancode < input.keyboard_state.size()) {
				input.keyboard_state[event.key.keysym.scancode] = ButtonState::Pressed;
				input.key_state_changed = true;
			}
		}
		else if (event.type == SDL_KEYUP)
		{
			if (event.key.keysym.scancode < input.keyboard_state.size()) {
				input.keyboard_state[event.key.keysym.scancode] = ButtonState::Released;
				input.key_state_changed = true;
			}
		}
		else if (event.type == SDL_MOUSEWHEEL)
		{
			input.mouse_wheel = event.wheel.y;
		}
		else if (event.type == SDL_MOUSEMOTION)
		{
			input.mouse_pos = Point{ event.motion.x, event.motion.y };
			input.mouse_pos_rel += Point{ event.motion.xrel, -event.motion.yrel };
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			input.mouse_state[event.button.button] = ButtonState::Pressed;
			input.mouse_key_state_changed = true;
		}
		else if (event.type == SDL_MOUSEBUTTONUP)
		{
			input.mouse_state[event.button.button] = ButtonState::Released;
			input.mouse_key_state_changed = true;
		}
	}

	void UpdateState(InputState& input)
	{
		input.mouse_wheel = 0;
		input.mouse_pos_rel = { 0, 0 };

		auto update_key_state = [](auto& key_state)
		{
			if (key_state == ButtonState::Pressed) {
				key_state = ButtonState::Down;
			}
			else if (key_state == ButtonState::Released) {
				key_state = ButtonState::Up;
			}
		};

		if (input.key_state_changed) {
			std::ranges::for_each(input.keyboard_state, update_key_state);
			input.key_state_changed = false;
		}
		if (input.mouse_key_state_changed) {
			std::ranges::for_each(input.mouse_state, update_key_state);
			input.mouse_key_state_changed = false;
		}
	}
}