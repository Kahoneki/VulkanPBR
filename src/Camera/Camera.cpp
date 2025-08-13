#include "Camera.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <iostream>



Camera::Camera(Neki::VulkanSwapchain& _swapchain, glm::vec3 _pos, glm::vec3 _up, float _yaw, float _pitch, float _nearPlaneDist, float _farPlaneDist, float _fov)
: swapchain(_swapchain)
{
	pos = _pos;
	up = _up;
	yaw = _yaw;
	pitch = _pitch;
	nearPlaneDist = _nearPlaneDist;
	farPlaneDist = _farPlaneDist;
	fov = _fov;

	UpdateCameraVectors();
}



glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(pos, pos + forward, up); //glm::lookAt() normalises direction argument (pos + forward)
}



glm::mat4 Camera::GetProjectionMatrix(PROJECTION_METHOD _method) const
{
	if (_method == PROJECTION_METHOD::ORTHOGRAPHIC)
	{
		std::cerr << "ERR::CAMERA::GET_PROJECTION_MATRIX::ORTHOGRAPHIC_CAMERA_NOT_CURRENTLY_SUPPORTED";
		throw std::runtime_error("");
	}
	else
	{
		float aspectRatio{ static_cast<float>(swapchain.GetSwapchainExtent().width) / static_cast<float>(swapchain.GetSwapchainExtent().height) };
		return glm::perspective(glm::radians(fov), aspectRatio, nearPlaneDist, farPlaneDist);
	}
}



void Camera::UpdateCameraVectors()
{
	//Calculate new forward and up vectors
	forward.x = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	forward.y = sin(glm::radians(pitch));
	forward.z = -cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	forward = glm::normalize(forward);

	constexpr glm::vec3 worldUp{ glm::vec3(0, 1, 0) };
	right = glm::normalize(glm::cross(forward, worldUp));
	up = glm::normalize(glm::cross(right, forward));
}