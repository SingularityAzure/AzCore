/*
	File: Gamepad.cpp
	Author: Philip Haynes
*/

#include "Gamepad.hpp"
#include "../keycodes.hpp"

namespace AzCore {

namespace io {

ButtonState* Gamepad::GetButtonState(u8 keyCode) {
	if (keyCode >= KC_GP_AXIS_LS_RIGHT && keyCode <= KC_GP_AXIS_H0_UP) {
		return &axisPush[keyCode - KC_GP_AXIS_LS_RIGHT];
	}
	else if (keyCode >= KC_GP_BTN_A && keyCode <= KC_GP_BTN_THUMBR) {
		return &button[keyCode - KC_GP_BTN_A];
	}
	else if (keyCode >= KC_GP_AXIS_H0_UP_RIGHT && keyCode <= KC_GP_AXIS_H0_UP_LEFT) {
		return &hat[keyCode - KC_GP_AXIS_H0_UP_RIGHT];
	} else {
		return nullptr;
	}
}

bool Gamepad::Repeated(u8 keyCode) {
	ButtonState* state = GetButtonState(keyCode);
	if (state) {
		return state->Repeated();
	} else {
		return false;
	}
}

bool Gamepad::Pressed(u8 keyCode) {
	ButtonState* state = GetButtonState(keyCode);
	if (state) {
		return state->Pressed();
	} else {
		return false;
	}
}

bool Gamepad::Down(u8 keyCode) {
	ButtonState* state = GetButtonState(keyCode);
	if (state) {
		return state->Down();
	} else {
		return false;
	}
}

bool Gamepad::Released(u8 keyCode) {
	ButtonState* state = GetButtonState(keyCode);
	if (state) {
		return state->Released();
	} else {
		return false;
	}
}

} // namespace io

} // namespace AzCore
