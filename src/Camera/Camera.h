#ifndef CAMERA_H
#define CAMERA_H

#include <NekiVK/Core/VulkanSwapchain.h>
#include <glm/glm.hpp>


enum class PROJECTION_METHOD
{
	PERSPECTIVE,
	ORTHOGRAPHIC,
};


class Camera
{
public:
	Camera(Neki::VulkanSwapchain& _swapchain, glm::vec3 _pos, glm::vec3 _up, float _yaw, float _pitch, float _nearPlaneDist, float _farPlaneDist, float _fov);

	[[nodiscard]] glm::mat4 GetViewMatrix() const;
	[[nodiscard]] glm::mat4 GetProjectionMatrix(PROJECTION_METHOD _method = PROJECTION_METHOD::PERSPECTIVE) const;


protected:
	//Recalculate front vector based on updated euler angles
	void UpdateCameraVectors();

	//Dependency injections
	Neki::VulkanSwapchain& swapchain;

	//Camera data
	glm::vec3 pos;
	glm::vec3 up;
	glm::vec3 forward;
	glm::vec3 right;
	float yaw;
	float pitch;
	float nearPlaneDist;
	float farPlaneDist;
	float fov;
};



#endif