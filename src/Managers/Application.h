#ifndef APPLICATION_H
#define APPLICATION_H
#include "../Camera/PlayerCamera.h"
#include "../VKApp.h"



class Application final
{
public:
	explicit Application();
	~Application();

	//Start the main frame loop
	void Start();
	
private:
	void RunFrame();
	
	std::unique_ptr<VKApp> vkApp;
	std::unique_ptr<PlayerCamera> camera;
};



#endif