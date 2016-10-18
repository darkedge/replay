// Usage:
// memset this class to 0
// optionally, populate MJControls::keyboard with platform macro's
// call ChangeKeyState(int, bool) in your platform callback functions

#pragma once
#include <map>
#include <string>

#define NUM_KEYBOARD_KEYS 0x100
#define NUM_KEYS_PER_BUTTON 3

class MJControls {
public:
	bool AssociateKey(int key, const std::string& name) {
		int k = keyboard[key];
		if (k >= 0 && k < NUM_KEYBOARD_KEYS) {
			configuration[name] = k;
			return true;
		} else {
			return false;
		}
	}
	// TODO: Change to output array (multiple values)
	int GetAssociatedKeys(const std::string& name) {
		if (configuration.count(name) == 0) {
			return -1;
		} else {
			return configuration.at(name);
		}
	}
	bool GetKey(int key) {
		int k = keyboard[key];
		if (!k) {
			return false;
		}
		return keys[k];
	}
	bool GetButton(const std::string& name) {
		if (configuration.count(name) == 0) {
			return false;
		}
		return keys[configuration.at(name)];
	}
	bool GetButtonDown(const std::string& name) {
		if (configuration.count(name) == 0) {
			return false;
		}
		return down[configuration.at(name)];
	}
	bool GetButtonUp(const std::string& name) {
		if (configuration.count(name) == 0) {
			return false;
		}
		return up[configuration.at(name)];
	}
	// Returns the index of a key pressed this frame, otherwise 0.
	int AnyKey() {
		return anyKey;
	}

	void ChangeKeyState(int key, bool pressed) {
		keys[key] = pressed;
	}

	void BeginFrame() {
		// Keyboard
		anyKey = 0;
		{
			bool changes[NUM_KEYBOARD_KEYS];
			for (unsigned int i = 0; i < NUM_KEYBOARD_KEYS; i++)
			{
				changes[i] = keys[i] ^ prev[i];
				down[i] = changes[i] & keys[i];
				if (down[i]) {
					anyKey = i;
				}
				up[i] = changes[i] & !keys[i];
			}
		}
	}

	void EndFrame() {
		memcpy(prev, keys, NUM_KEYBOARD_KEYS * sizeof(bool));
	}

	enum Key {
		A,
		B,
		C,
		D,
		Down,
		E,
		Escape,
		F,
		F1,
		F10,
		F11,
		F12,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		G,
		H,
		I,
		J,
		K,
		L,
		LAlt,
		LCtrl,
		Left,
		LShift,
		M,
		N,
		Num0,
		Num1,
		Num2,
		Num3,
		Num4,
		Num5,
		Num6,
		Num7,
		Num8,
		Num9,
		O,
		P,
		Q,
		R,
		RAlt,
		RCtrl,
		Right,
		RShift,
		S,
		Space,
		T,
		Tab,
		U,
		Up,
		V,
		W,
		X,
		Y,
		Z,

		NumKeys
	};

	int keyboard[NumKeys];

private:
	int anyKey;
	bool prev[NUM_KEYBOARD_KEYS];
	bool keys[NUM_KEYBOARD_KEYS];
	bool down[NUM_KEYBOARD_KEYS];
	bool up[NUM_KEYBOARD_KEYS];

	std::map<std::string, int> configuration;
};
