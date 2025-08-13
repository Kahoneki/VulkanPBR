#include "VKApp.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>

#include "Managers/TimeManager.h"



VKApp::VKApp() : clearValueCount(0), clearValues(nullptr), modelModelMatrix(), modelTextureSampler(VK_NULL_HANDLE), camDescriptorSetLayout(VK_NULL_HANDLE), camDescriptorSet(VK_NULL_HANDLE), camDataUBO(VK_NULL_HANDLE), camDataUBOMap(nullptr)
{
	logger = std::make_unique<Neki::VKLogger>(Neki::VKLoggerConfig(true));
	instDebugAllocator = std::make_unique<Neki::VKDebugAllocator>(Neki::VK_ALLOCATOR_TYPE::DEBUG);
	deviceDebugAllocator = std::make_unique<Neki::VKDebugAllocator>(Neki::VK_ALLOCATOR_TYPE::DEBUG);

	const char* instLay[]{ "VK_LAYER_KHRONOS_validation" };
	const char* instExt[]{ "VK_KHR_surface" };
	const char* devLay[]{ "VK_LAYER_KHRONOS_validation" };
	const char* devExt[]{ "VK_KHR_swapchain" };
	vulkanDevice = std::make_unique<Neki::VulkanDevice>(*logger, *instDebugAllocator, *deviceDebugAllocator, VK_API_VERSION_1_4, "Vulkan PBR", 1, instLay, 1, instExt, 1, devLay, 1, devExt);

	vulkanCommandPool = std::make_unique<Neki::VulkanCommandPool>(*logger, *deviceDebugAllocator, *vulkanDevice, Neki::VK_COMMAND_POOL_TYPE::GRAPHICS);

	VkDescriptorPoolSize descriptorPoolSizes[]{ { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 }, { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 7 } };
	vulkanDescriptorPool = std::make_unique<Neki::VulkanDescriptorPool>(*logger, *deviceDebugAllocator, *vulkanDevice, 2, descriptorPoolSizes);

	bufferFactory = std::make_unique<Neki::BufferFactory>(*logger, *deviceDebugAllocator, *vulkanDevice, *vulkanCommandPool);
	imageFactory = std::make_unique<Neki::ImageFactory>(*logger, *deviceDebugAllocator, *vulkanDevice, *vulkanCommandPool, *bufferFactory);
	modelFactory = std::make_unique<Neki::ModelFactory>(*logger, *deviceDebugAllocator, *vulkanDevice, *bufferFactory, *imageFactory, *vulkanDescriptorPool);

	VkExtent2D winSize{ 1920, 1032 };
	vulkanSwapchain = std::make_unique<Neki::VulkanSwapchain>(*logger, *deviceDebugAllocator, *vulkanDevice, *imageFactory, winSize);

	//Clear values
	clearValueCount = 2;
	clearValues = new VkClearValue[clearValueCount];
	clearValues[0].depthStencil = { 1.0f, 0 };
	clearValues[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };

	InitialiseRenderManager();
	InitialiseSampler();
	LoadModel();
	InitialiseCamData();
	CreateCamDescriptorSet();
	BindCamDescriptorSet();
	CreatePipeline();
}



void VKApp::InitialiseRenderManager()
{
	logger->Log(Neki::VK_LOGGER_CHANNEL::HEADING, Neki::VK_LOGGER_LAYER::APPLICATION, "\n\n\n", Neki::VK_LOGGER_WIDTH::DEFAULT, false);
	logger->Log(Neki::VK_LOGGER_CHANNEL::HEADING, Neki::VK_LOGGER_LAYER::APPLICATION, "Initialising Render Manager", Neki::VK_LOGGER_WIDTH::DEFAULT, false);

	//Attachments
	VkAttachmentDescription attachments[]
	{
		Neki::VulkanRenderManager::GetDefaultOutputDepthAttachmentDescription(),
		Neki::VulkanRenderManager::GetDefaultOutputColourAttachmentDescription(),
	};
	VkFormat attachmentFormats[]{ VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED };
	Neki::FORMAT_TYPE attachmentTypes[]{ Neki::FORMAT_TYPE::DEPTH_NO_SAMPLING, Neki::FORMAT_TYPE::SWAPCHAIN };

	//Subpass
	VkAttachmentReference depthAttachmentRef{ 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
	VkAttachmentReference colourAttachmentRef{ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkSubpassDescription subpass{ Neki::VulkanRenderManager::GetDefaultSubpassDescription(&colourAttachmentRef, &depthAttachmentRef) };

	Neki::VKRenderPassCleanDesc renderPassDesc{};
	renderPassDesc.attachmentCount = 2;
	renderPassDesc.attachments = attachments;
	renderPassDesc.attachmentFormats = attachmentFormats;
	renderPassDesc.attachmentTypes = attachmentTypes;
	renderPassDesc.subpassCount = 1;
	renderPassDesc.subpasses = &subpass;
	renderPassDesc.dependencyCount = 0;
	renderPassDesc.dependencies = nullptr;

	vulkanRenderManager = std::make_unique<Neki::VulkanRenderManager>(*logger, *deviceDebugAllocator, *vulkanDevice, *vulkanSwapchain, *imageFactory, *vulkanCommandPool, 2, renderPassDesc);
}



void VKApp::InitialiseSampler()
{
	logger->Log(Neki::VK_LOGGER_CHANNEL::HEADING, Neki::VK_LOGGER_LAYER::APPLICATION, "\n\n\n", Neki::VK_LOGGER_WIDTH::DEFAULT, false);
	logger->Log(Neki::VK_LOGGER_CHANNEL::HEADING, Neki::VK_LOGGER_LAYER::APPLICATION, "Creating Sampler", Neki::VK_LOGGER_WIDTH::DEFAULT, false);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.pNext = nullptr;
	samplerInfo.flags = 0;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	modelTextureSampler = imageFactory->CreateSampler(samplerInfo);
}



void VKApp::LoadModel()
{
	logger->Log(Neki::VK_LOGGER_CHANNEL::HEADING, Neki::VK_LOGGER_LAYER::APPLICATION, "\n\n\n", Neki::VK_LOGGER_WIDTH::DEFAULT, false);
	logger->Log(Neki::VK_LOGGER_CHANNEL::HEADING, Neki::VK_LOGGER_LAYER::APPLICATION, "Loading Model", Neki::VK_LOGGER_WIDTH::DEFAULT, false);

	std::unordered_map<Neki::MODEL_TEXTURE_TYPE, VkSampler> textureSamplers;
	for (std::uint32_t i{ 0 }; i < static_cast<std::uint32_t>(Neki::MODEL_TEXTURE_TYPE::NUM_MODEL_TEXTURE_TYPES); ++i)
	{
		textureSamplers[static_cast<Neki::MODEL_TEXTURE_TYPE>(i)] = modelTextureSampler;
	}

	model = modelFactory->LoadModel("Resource Files/DamagedHelmet/DamagedHelmet.gltf", textureSamplers);

	modelModelMatrix = glm::mat4(1.0f);
	modelModelMatrix = glm::rotate(modelModelMatrix, glm::radians(30.0f), glm::vec3(0, -1, 0));
	modelModelMatrix = glm::rotate(modelModelMatrix, glm::radians(180.0f), glm::vec3(0, 0, 1));
	modelModelMatrix = glm::rotate(modelModelMatrix, glm::radians(70.0f), glm::vec3(1, 0, 0));
}



void VKApp::InitialiseCamData()
{
	logger->Log(Neki::VK_LOGGER_CHANNEL::HEADING, Neki::VK_LOGGER_LAYER::APPLICATION, "\n\n\n", Neki::VK_LOGGER_WIDTH::DEFAULT, false);
	logger->Log(Neki::VK_LOGGER_CHANNEL::HEADING, Neki::VK_LOGGER_LAYER::APPLICATION, "Initialising Camera Data", Neki::VK_LOGGER_WIDTH::DEFAULT, false);

	CamData cameraData{};
	cameraData.view = glm::lookAt(glm::vec3(0.0f, -3.0f, 6.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const float aspectRatio{ static_cast<float>(vulkanSwapchain->GetSwapchainExtent().width) / static_cast<float>(vulkanSwapchain->GetSwapchainExtent().height) };
	cameraData.proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
	constexpr VkDeviceSize bufferSize{ sizeof(CamData) };
	camDataUBO = bufferFactory->AllocateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	//Map buffer memory
	logger->Log(Neki::VK_LOGGER_CHANNEL::INFO, Neki::VK_LOGGER_LAYER::APPLICATION, "  Mapping UBO memory", Neki::VK_LOGGER_WIDTH::SUCCESS_FAILURE);
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), camDataUBO, &memRequirements);
	VkResult result{ vkMapMemory(vulkanDevice->GetDevice(), bufferFactory->GetMemory(camDataUBO), 0, memRequirements.size, 0, &camDataUBOMap) };
	logger->Log(result == VK_SUCCESS ? Neki::VK_LOGGER_CHANNEL::SUCCESS : Neki::VK_LOGGER_CHANNEL::ERROR, Neki::VK_LOGGER_LAYER::APPLICATION, result == VK_SUCCESS ? "success\n" : "failure", Neki::VK_LOGGER_WIDTH::DEFAULT, false);
	if (result != VK_SUCCESS)
	{
		logger->Log(Neki::VK_LOGGER_CHANNEL::ERROR, Neki::VK_LOGGER_LAYER::APPLICATION, " (" + std::to_string(result) + ")\n", Neki::VK_LOGGER_WIDTH::DEFAULT, false);
		throw std::runtime_error("");
	}
	memcpy(camDataUBOMap, &cameraData, static_cast<std::size_t>(bufferSize));
	vkUnmapMemory(vulkanDevice->GetDevice(), bufferFactory->GetMemory(camDataUBO));
}



void VKApp::CreateCamDescriptorSet()
{
	logger->Log(Neki::VK_LOGGER_CHANNEL::HEADING, Neki::VK_LOGGER_LAYER::APPLICATION, "\n\n\n", Neki::VK_LOGGER_WIDTH::DEFAULT, false);
	logger->Log(Neki::VK_LOGGER_CHANNEL::HEADING, Neki::VK_LOGGER_LAYER::APPLICATION, "Creating Camera Descriptor Set", Neki::VK_LOGGER_WIDTH::DEFAULT, false);

	//Define descriptor binding 0 as a uniform buffer accessible from the vertex shader
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	//Create the descriptor set layout
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.pNext = nullptr;
	layoutInfo.flags = 0;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	//Create the descriptor set
	logger->Log(Neki::VK_LOGGER_CHANNEL::INFO, Neki::VK_LOGGER_LAYER::APPLICATION, "  Creating descriptor set layout", Neki::VK_LOGGER_WIDTH::SUCCESS_FAILURE);
	VkResult result{ vkCreateDescriptorSetLayout(vulkanDevice->GetDevice(), &layoutInfo, static_cast<const VkAllocationCallbacks*>(*deviceDebugAllocator), &camDescriptorSetLayout) };
	logger->Log(result == VK_SUCCESS ? Neki::VK_LOGGER_CHANNEL::SUCCESS : Neki::VK_LOGGER_CHANNEL::ERROR, Neki::VK_LOGGER_LAYER::APPLICATION, result == VK_SUCCESS ? "success\n" : "failure", Neki::VK_LOGGER_WIDTH::DEFAULT, false);
	if (result != VK_SUCCESS)
	{
		logger->Log(Neki::VK_LOGGER_CHANNEL::ERROR, Neki::VK_LOGGER_LAYER::APPLICATION, "(" + std::to_string(result) + ")\n");
		throw std::runtime_error("");
	}

	camDescriptorSet = vulkanDescriptorPool->AllocateDescriptorSet(camDescriptorSetLayout);
}



void VKApp::BindCamDescriptorSet()
{
	logger->Log(Neki::VK_LOGGER_CHANNEL::HEADING, Neki::VK_LOGGER_LAYER::APPLICATION, "\n\n\n", Neki::VK_LOGGER_WIDTH::DEFAULT, false);
	logger->Log(Neki::VK_LOGGER_CHANNEL::HEADING, Neki::VK_LOGGER_LAYER::APPLICATION, "Binding Camera Descriptor Set", Neki::VK_LOGGER_WIDTH::DEFAULT, false);

	//Bind descriptor set to make descriptor at binding 0 point to camDataUBO
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = camDataUBO;
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;
	VkWriteDescriptorSet descriptorWriteUBO;
	descriptorWriteUBO.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWriteUBO.pNext = nullptr;
	descriptorWriteUBO.dstSet = camDescriptorSet;
	descriptorWriteUBO.dstBinding = 0;
	descriptorWriteUBO.dstArrayElement = 0;
	descriptorWriteUBO.descriptorCount = 1;
	descriptorWriteUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWriteUBO.pBufferInfo = &bufferInfo;
	descriptorWriteUBO.pTexelBufferView = nullptr;
	descriptorWriteUBO.pImageInfo = nullptr;

	vkUpdateDescriptorSets(vulkanDevice->GetDevice(), 1, &descriptorWriteUBO, 0, nullptr);
}



void VKApp::CreatePipeline()
{
	Neki::VKGraphicsPipelineCleanDesc piplDesc{};
	piplDesc.renderPass = vulkanRenderManager->GetRenderPass();

	VkVertexInputBindingDescription vertInputBindingDesc{};
	vertInputBindingDesc.binding = 0;
	vertInputBindingDesc.stride = sizeof(Neki::ModelVertex);
	vertInputBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	piplDesc.vertexBindingDescriptionCount = 1;
	piplDesc.pVertexBindingDescriptions = &vertInputBindingDesc;

	VkVertexInputAttributeDescription posAttribDesc{};
	posAttribDesc.binding = 0;
	posAttribDesc.location = 0;
	posAttribDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
	posAttribDesc.offset = offsetof(Neki::ModelVertex, position);

	VkVertexInputAttributeDescription normalAttribDesc{};
	normalAttribDesc.binding = 0;
	normalAttribDesc.location = 1;
	normalAttribDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
	normalAttribDesc.offset = offsetof(Neki::ModelVertex, normal);

	VkVertexInputAttributeDescription uvAttribDesc{};
	uvAttribDesc.binding = 0;
	uvAttribDesc.location = 2;
	uvAttribDesc.format = VK_FORMAT_R32G32_SFLOAT;
	uvAttribDesc.offset = offsetof(Neki::ModelVertex, texCoord);

	VkVertexInputAttributeDescription tangentAttribDesc{};
	tangentAttribDesc.binding = 0;
	tangentAttribDesc.location = 3;
	tangentAttribDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
	tangentAttribDesc.offset = offsetof(Neki::ModelVertex, tangent);

	VkVertexInputAttributeDescription bitangentAttribDesc{};
	bitangentAttribDesc.binding = 0;
	bitangentAttribDesc.location = 4;
	bitangentAttribDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
	bitangentAttribDesc.offset = offsetof(Neki::ModelVertex, bitangent);

	VkVertexInputAttributeDescription attribDescs[]{ posAttribDesc, normalAttribDesc, uvAttribDesc, tangentAttribDesc, bitangentAttribDesc };

	piplDesc.vertexAttributeDescriptionCount = 5;
	piplDesc.pVertexAttributeDescriptions = attribDescs;

	//Push constant for storing the model matrix
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.size = sizeof(glm::mat4);
	pushConstantRange.offset = 0;
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayout descSetLayouts[]{ camDescriptorSetLayout, modelFactory->GetMaterialDescriptorSetLayout() };

	vulkanGraphicsPipeline = std::make_unique<Neki::VulkanGraphicsPipeline>(*logger, *deviceDebugAllocator, *vulkanDevice, &piplDesc, "pbr.vert", "pbr.frag", nullptr, nullptr, 2, descSetLayouts, 1, &pushConstantRange);
}



void VKApp::UpdateCamData(PlayerCamera& _playerCamera)
{
	CamData camData{};
	camData.view = _playerCamera.GetViewMatrix();
	camData.proj = _playerCamera.GetProjectionMatrix();

	//Write to buffer
	memcpy(camDataUBOMap, &camData, sizeof(CamData));
}



void VKApp::DrawFrame(PlayerCamera& _playerCamera)
{
	vulkanRenderManager->StartFrame(2, clearValues);

	UpdateCamData(_playerCamera);
	
	vkCmdBindPipeline(vulkanRenderManager->GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanGraphicsPipeline->GetPipeline());
	constexpr VkDeviceSize zeroOffset{ 0 };
	vkCmdBindVertexBuffers(vulkanRenderManager->GetCurrentCommandBuffer(), 0, 1, &model.meshes[0].vertexBuffer, &zeroOffset);
	vkCmdBindIndexBuffer(vulkanRenderManager->GetCurrentCommandBuffer(), model.meshes[0].indexBuffer, zeroOffset, VK_INDEX_TYPE_UINT32);
	VkDescriptorSet descSets[]{ camDescriptorSet, model.materials[0].descriptorSet };
	vkCmdBindDescriptorSets(vulkanRenderManager->GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanGraphicsPipeline->GetPipelineLayout(), 0, 2, descSets, 0, nullptr);

	float speed{ 50.0f };
	modelModelMatrix = glm::rotate(modelModelMatrix, glm::radians(speed * static_cast<float>(TimeManager::dt)), glm::vec3(0, 0, 1));
	vkCmdPushConstants(vulkanRenderManager->GetCurrentCommandBuffer(), vulkanGraphicsPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &modelModelMatrix);

	//Define viewport
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(vulkanSwapchain->GetSwapchainExtent().width);
	viewport.height = static_cast<float>(vulkanSwapchain->GetSwapchainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(vulkanRenderManager->GetCurrentCommandBuffer(), 0, 1, &viewport);

	//Define scissor
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = vulkanSwapchain->GetSwapchainExtent();
	vkCmdSetScissor(vulkanRenderManager->GetCurrentCommandBuffer(), 0, 1, &scissor);

	//Draw the model
	vkCmdDrawIndexed(vulkanRenderManager->GetCurrentCommandBuffer(), model.meshes[0].indexCount, 1, 0, 0, 0);

	vulkanRenderManager->SubmitAndPresent();
}