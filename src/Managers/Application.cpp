#include "Application.h"

#include "InputManager.h"
#include "TimeManager.h"



Application::Application()
{
	vkApp = std::make_unique<VKApp>();
	camera = std::make_unique<PlayerCamera>(30.0f, 0.05f, *(vkApp->vulkanSwapchain), glm::vec3(0,0,3), glm::vec3(0,1,0), 0.0f, 0.0f, 0.1f, 100.0f, 90.0f);
}



Application::~Application(){}



void Application::Start()
{
	while (!vkApp->vulkanSwapchain->WindowShouldClose())
	{
		RunFrame();
	}

	//Todo: this is a temp fix for a shutdownn bug - regular surface destruction results in a segfault, destroying swapchain (and hence surface) first fixes this, but it's a weird workaround.
	vkDeviceWaitIdle(vkApp->vulkanDevice->GetDevice());
	vkApp->vulkanSwapchain.reset();
}



void Application::RunFrame()
{
	glfwPollEvents();
	TimeManager::NewFrame();
	InputManager::UpdateInput(vkApp->vulkanSwapchain->GetWindow());
	camera->Update();
	vkApp->DrawFrame(*camera);
}