#ifndef VKAPP_H
#define VKAPP_H

#include <NekiVK/NekiVK.h>
#include <memory>

#include "Camera/PlayerCamera.h"


struct CamData
{
	glm::mat4 view;
	glm::mat4 proj;
};

class VKApp
{
	friend class Application;


public:
	explicit VKApp();
	~VKApp() = default;


private:
	//Sub-classes
	std::unique_ptr<Neki::VKLogger> logger;
	std::unique_ptr<Neki::VKDebugAllocator> instDebugAllocator;
	std::unique_ptr<Neki::VKDebugAllocator> deviceDebugAllocator;
	std::unique_ptr<Neki::VulkanDevice> vulkanDevice;
	std::unique_ptr<Neki::VulkanCommandPool> vulkanCommandPool;
	std::unique_ptr<Neki::VulkanDescriptorPool> vulkanDescriptorPool;
	std::unique_ptr<Neki::BufferFactory> bufferFactory;
	std::unique_ptr<Neki::ImageFactory> imageFactory;
	std::unique_ptr<Neki::ModelFactory> modelFactory;
	std::unique_ptr<Neki::VulkanSwapchain> vulkanSwapchain;
	std::unique_ptr<Neki::VulkanRenderManager> vulkanRenderManager;
	std::unique_ptr<Neki::VulkanGraphicsPipeline> vulkanGraphicsPipeline;

	//Init sub-functions
	void InitialiseRenderManager();
	void InitialiseSampler();
	void LoadModel();
	void InitialiseCamData();
	void CreateCamDescriptorSet();
	void BindCamDescriptorSet();
	void CreatePipeline();

	//Per-frame functions
	void UpdateCamData(PlayerCamera& _playerCamera);
	void DrawFrame(PlayerCamera& _playerCamera);

	std::uint32_t clearValueCount;
	VkClearValue* clearValues;

	//Model
	Neki::GPUModel model;
	glm::mat4 modelModelMatrix; //as in.... the model matrix for the mode
	VkSampler modelTextureSampler;

	//Camera
	VkDescriptorSetLayout camDescriptorSetLayout;
	VkDescriptorSet camDescriptorSet;
	VkBuffer camDataUBO;
	void* camDataUBOMap;
};


#endif