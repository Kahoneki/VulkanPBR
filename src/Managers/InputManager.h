#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <GLFW/glfw3.h>
#include <unordered_map>
#include <variant>



enum class INPUT_ACTION
{
	MOVE_FORWARDS,
	MOVE_BACKWARDS,
	MOVE_LEFT,
	MOVE_RIGHT,
	CAMERA_PITCH,
	CAMERA_YAW,
};

enum class KEY_INPUT_STATE
{
	// PRESSED, //Set for the first frame key is pressed
	HELD,
	// RELEASED, //Set for the first frame the key is released
	NOT_HELD,
};

typedef double MOUSE_INPUT_STATE;

using INPUT_STATE = std::variant<MOUSE_INPUT_STATE, KEY_INPUT_STATE>;

class InputManager
{
public:
	static void UpdateInput(GLFWwindow* _window);
	[[nodiscard]] static INPUT_STATE GetInputState(INPUT_ACTION _action);
	[[nodiscard]] static INPUT_STATE GetInputStateLastFrame(INPUT_ACTION _action);
	
private:
	static std::unordered_map<INPUT_ACTION, INPUT_STATE> actionStates;
	static std::unordered_map<INPUT_ACTION, INPUT_STATE> actionStatesLastFrame;

	//Used to stop big mouse pos delta on first frame
	static bool firstFrame;
};



#endif
