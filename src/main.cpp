#include <iostream>
#include <NekiVK/NekiVK.h>

#include "Managers/Application.h"



int main()
{
	glfwInit();

	{
		Application app{};
		app.Start();
	}
	
	glfwTerminate();

	return 0;
}