#ifndef PLAYERCAMERA_H
#define PLAYERCAMERA_H

#include <glm/glm.hpp>

#include "Camera.h"



class PlayerCamera : public Camera
{
public:
	PlayerCamera(float _movementSpeed, float _mouseSensitivity, Neki::VulkanSwapchain& _swapchain, glm::vec3 _pos, glm::vec3 _up, float _yaw, float _pitch, float _nearPlaneDist, float _farPlaneDist, float _fov);

	void Update();


private:
	float movementSpeed;
	float mouseSensitivity;
};



#endif