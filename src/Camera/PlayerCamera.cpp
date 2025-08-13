#include "PlayerCamera.h"
#include "../Managers/InputManager.h"
#include "../Managers/TimeManager.h"



PlayerCamera::PlayerCamera(float _movementSpeed, float _mouseSensitivity, Neki::VulkanSwapchain& _swapchain, glm::vec3 _pos, glm::vec3 _up, float _yaw, float _pitch, float _nearPlaneDist, float _farPlaneDist, float _fov)
						  : Camera(_swapchain, _pos, _up, _yaw, _pitch, _nearPlaneDist, _farPlaneDist, _fov)
{
	movementSpeed = _movementSpeed;
	mouseSensitivity = _mouseSensitivity;
}



void PlayerCamera::Update()
{
	const float speed{ movementSpeed * static_cast<float>(TimeManager::dt) };
	glm::vec3 movementDirection{};
	if (std::get<KEY_INPUT_STATE>(InputManager::GetInputState(INPUT_ACTION::MOVE_FORWARDS))		== KEY_INPUT_STATE::HELD) { movementDirection += forward; }
	if (std::get<KEY_INPUT_STATE>(InputManager::GetInputState(INPUT_ACTION::MOVE_BACKWARDS))	== KEY_INPUT_STATE::HELD) { movementDirection -= forward; }
	if (std::get<KEY_INPUT_STATE>(InputManager::GetInputState(INPUT_ACTION::MOVE_LEFT))			== KEY_INPUT_STATE::HELD) { movementDirection -= right; }
	if (std::get<KEY_INPUT_STATE>(InputManager::GetInputState(INPUT_ACTION::MOVE_RIGHT))		== KEY_INPUT_STATE::HELD) { movementDirection += right; }
	if (movementDirection != glm::vec3(0))
	{
		pos += glm::normalize(movementDirection) * speed;
	}

	
	// //Process mouse movement
	const float oldMouseX{ static_cast<float>(std::get<MOUSE_INPUT_STATE>(InputManager::GetInputStateLastFrame(INPUT_ACTION::CAMERA_PITCH))) };
	const float oldMouseY{ static_cast<float>(std::get<MOUSE_INPUT_STATE>(InputManager::GetInputStateLastFrame(INPUT_ACTION::CAMERA_YAW))) };
	const float newMouseX{ static_cast<float>(std::get<MOUSE_INPUT_STATE>(InputManager::GetInputState(INPUT_ACTION::CAMERA_PITCH))) };
	const float newMouseY{ static_cast<float>(std::get<MOUSE_INPUT_STATE>(InputManager::GetInputState(INPUT_ACTION::CAMERA_YAW))) };
	const float pitchOffset{ (newMouseX - oldMouseX) * mouseSensitivity };
	const float yawOffset{ (newMouseY - oldMouseY) * mouseSensitivity };
	pitch += pitchOffset;
	yaw += yawOffset;

	//Constrain pitch for first person camera
	if		(pitch > 89.0f)	 { pitch = 89.0f; }
	else if (pitch < -89.0f) { pitch = -89.0f; }

	UpdateCameraVectors();
}