#include "TimeManager.h"



double TimeManager::lastTime = 0;
double TimeManager::dt = 0;

void TimeManager::NewFrame()
{
	double currentTime{ glfwGetTime() };
	dt = currentTime - lastTime;
	lastTime = currentTime;
}