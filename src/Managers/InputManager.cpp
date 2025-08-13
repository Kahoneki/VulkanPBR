#include "InputManager.h"



bool InputManager::firstFrame = true;
std::unordered_map<INPUT_ACTION, INPUT_STATE> InputManager::actionStates
{
	{INPUT_ACTION::MOVE_FORWARDS,		KEY_INPUT_STATE::NOT_HELD},
	{INPUT_ACTION::MOVE_BACKWARDS,	KEY_INPUT_STATE::NOT_HELD},
	{INPUT_ACTION::MOVE_LEFT,			KEY_INPUT_STATE::NOT_HELD},
	{INPUT_ACTION::MOVE_RIGHT,		KEY_INPUT_STATE::NOT_HELD},
	{INPUT_ACTION::CAMERA_YAW,		0.0f},
	{INPUT_ACTION::CAMERA_PITCH,		0.0f},
};

std::unordered_map<INPUT_ACTION, INPUT_STATE> InputManager::actionStatesLastFrame = InputManager::actionStates;

void InputManager::UpdateInput(GLFWwindow* _window)
{
	actionStatesLastFrame = actionStates;
	
	actionStates[INPUT_ACTION::MOVE_FORWARDS]	= (glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS) ? KEY_INPUT_STATE::HELD : KEY_INPUT_STATE::NOT_HELD;
	actionStates[INPUT_ACTION::MOVE_BACKWARDS]	= (glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS) ? KEY_INPUT_STATE::HELD : KEY_INPUT_STATE::NOT_HELD;
	actionStates[INPUT_ACTION::MOVE_LEFT]		= (glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS) ? KEY_INPUT_STATE::HELD : KEY_INPUT_STATE::NOT_HELD;
	actionStates[INPUT_ACTION::MOVE_RIGHT]		= (glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS) ? KEY_INPUT_STATE::HELD : KEY_INPUT_STATE::NOT_HELD;
	
	//Don't process mouse movement on first frame
	if (!firstFrame)
	{
		double xPos;
		double yPos;
		glfwGetCursorPos(_window, &xPos, &yPos);
		actionStates[INPUT_ACTION::CAMERA_YAW] = xPos;
		actionStates[INPUT_ACTION::CAMERA_PITCH] = yPos;
	}
	else
	{
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		firstFrame = false;
	}
}



INPUT_STATE InputManager::GetInputState(INPUT_ACTION _action)
{
	return actionStates.at(_action);
}



INPUT_STATE InputManager::GetInputStateLastFrame(INPUT_ACTION _action)
{
	return actionStatesLastFrame.at(_action);
}