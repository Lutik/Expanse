#pragma once

#include <array>
#include <vector>

#include "KeyCodes.h"

#include "Utils/Math.h"

namespace Expanse::Input
{
	enum class ButtonState : uint8_t
	{
		Up = 0,
		Released = 1,
		Down = 2,
		Pressed = 3,
	};

	using KeyCode = std::underlying_type_t<Key::Scancode>;


	inline constexpr size_t MaxKeyboardKeys = 512;
	inline constexpr size_t MaxMouseKeys = 32;

	struct InputState
	{
		bool KeyPressed(KeyCode key) const;
		bool KeyReleased(KeyCode key) const;
		bool IsKeyDown(KeyCode key) const;

		bool MousePressed(int button) const;
		bool MouseReleased(int button) const;
		bool IsMouseDown(int button) const;


		// keyboard
		std::array<ButtonState, MaxKeyboardKeys> keyboard_state;

		// mouse		
		int mouse_wheel = 0;
		Point mouse_pos = { 0, 0 };
		Point mouse_pos_rel = { 0, 0 };
		std::array<ButtonState, MaxMouseKeys> mouse_state;

		// misc
		bool key_state_changed = false;
		bool mouse_key_state_changed = false;

		// constructor
		InputState();

		// Should be called every frame
		void Update();
	};

	extern InputState g_input_state;

	inline bool KeyPressed(KeyCode key) { return g_input_state.KeyPressed(key); }
	inline bool KeyReleased(KeyCode key) { return g_input_state.KeyReleased(key); }
	inline bool IsKeyDown(KeyCode key) { return g_input_state.IsKeyDown(key); }
	inline bool MousePressed(int button) { return g_input_state.MousePressed(button); }
	inline bool MouseReleased(int button) { return g_input_state.MouseReleased(button); }
	inline bool IsMouseDown(int button) { return g_input_state.IsMouseDown(button); }
	inline int MouseWheel() { return g_input_state.mouse_wheel; }
	inline void Update() { g_input_state.Update(); }
}