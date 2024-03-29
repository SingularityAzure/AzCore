/*
	File: vk.cpp
	Author: Philip Haynes
*/
#include "io.hpp"
#include "vk.hpp"
#include "IO/Log.hpp"
#include <cstring>
#include <cstdlib>
#include <fstream>

#ifdef __unix
	#include <xcb/xcb.h>
	#include <xkbcommon/xkbcommon.h>
	#include <xkbcommon/xkbcommon-x11.h>

	#define explicit extern_explicit // Preventing C++ keyword bug
	#include <xcb/xkb.h>
	#undef explicit
#endif

#ifdef AZCORE_IO_FOR_VULKAN

#ifdef __unix
	#include "IO/Linux/WindowData.hpp"
#elif defined(_WIN32)
	#include "IO/Win32/WindowData.hpp"
#endif

#endif // AZCORE_IO_FOR_VULKAN

namespace AzCore {

namespace vk {

	String error = "No Error";
	bool hadValidationError = false;

#ifndef AZCORE_VK_LOGGING_NO_CONSOLE
	io::Log cout("vk.log", true, true);
#else
	io::Log cout("vk.log", false, true);
#endif

	const char *QueueTypeString[5] = {
		"UNDEFINED",
		"COMPUTE",
		"GRAPHICS",
		"TRANSFER",
		"PRESENT"
	};

	String ErrorString(VkResult errorCode) {
		// Thanks to Sascha Willems for this snippet! :D
		switch (errorCode) {
#define STR(r) case VK_ ##r: return #r
			STR(SUCCESS);
			STR(NOT_READY);
			STR(TIMEOUT);
			STR(EVENT_SET);
			STR(EVENT_RESET);
			STR(INCOMPLETE);
			STR(ERROR_OUT_OF_HOST_MEMORY);
			STR(ERROR_OUT_OF_DEVICE_MEMORY);
			STR(ERROR_INITIALIZATION_FAILED);
			STR(ERROR_DEVICE_LOST);
			STR(ERROR_MEMORY_MAP_FAILED);
			STR(ERROR_LAYER_NOT_PRESENT);
			STR(ERROR_EXTENSION_NOT_PRESENT);
			STR(ERROR_FEATURE_NOT_PRESENT);
			STR(ERROR_INCOMPATIBLE_DRIVER);
			STR(ERROR_TOO_MANY_OBJECTS);
			STR(ERROR_FORMAT_NOT_SUPPORTED);
			STR(ERROR_FRAGMENTED_POOL);
			STR(ERROR_UNKNOWN);
			#if VK_VERSION_1_1
			STR(ERROR_OUT_OF_POOL_MEMORY);
			STR(ERROR_INVALID_EXTERNAL_HANDLE);
			#endif
			#if VK_VERSION_1_2
			STR(ERROR_FRAGMENTATION);
			STR(ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);
			#endif
			#if VK_KHR_surface
			STR(ERROR_SURFACE_LOST_KHR);
			STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
			#endif
			#if VK_KHR_swapchain
			STR(SUBOPTIMAL_KHR);
			STR(ERROR_OUT_OF_DATE_KHR);
			#endif
			#if VK_KHR_display_swapchain
			STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
			#endif
			#if VK_EXT_debug_report
			STR(ERROR_VALIDATION_FAILED_EXT);
			#endif
			#if VK_NV_glsl_shader
			STR(ERROR_INVALID_SHADER_NV);
			#endif
			#if VK_EXT_image_drm_format_modifier
			STR(ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
			#endif
			#if VK_EXT_global_priority
			STR(ERROR_NOT_PERMITTED_EXT);
			#endif
			#if VK_EXT_full_screen_exclusive
			STR(ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT);
			#endif
			#if VK_KHR_deferred_host_operations
			STR(THREAD_IDLE_KHR);
			#endif
			#if VK_KHR_deferred_host_operations
			STR(THREAD_DONE_KHR);
			#endif
			#if VK_KHR_deferred_host_operations
			STR(OPERATION_DEFERRED_KHR);
			#endif
			#if VK_KHR_deferred_host_operations
			STR(OPERATION_NOT_DEFERRED_KHR);
			#endif
			#if VK_EXT_pipeline_creation_cache_control
			STR(PIPELINE_COMPILE_REQUIRED_EXT);
			#endif
#undef STR
		default:
			return "UNKNOWN_ERROR " + ToString((i64)errorCode);
		}
	}

	String ObjectTypeString(VkObjectType type) {
		switch (type) {
#define STR(r) case VK_OBJECT_TYPE_ ##r: return #r
			STR(UNKNOWN);
			STR(INSTANCE);
			STR(PHYSICAL_DEVICE);
			STR(DEVICE);
			STR(QUEUE);
			STR(SEMAPHORE);
			STR(COMMAND_BUFFER);
			STR(FENCE);
			STR(DEVICE_MEMORY);
			STR(BUFFER);
			STR(IMAGE);
			STR(EVENT);
			STR(QUERY_POOL);
			STR(BUFFER_VIEW);
			STR(IMAGE_VIEW);
			STR(SHADER_MODULE);
			STR(PIPELINE_CACHE);
			STR(PIPELINE_LAYOUT);
			STR(RENDER_PASS);
			STR(PIPELINE);
			STR(DESCRIPTOR_SET_LAYOUT);
			STR(SAMPLER);
			STR(DESCRIPTOR_POOL);
			STR(DESCRIPTOR_SET);
			STR(FRAMEBUFFER);
			STR(COMMAND_POOL);
			STR(SAMPLER_YCBCR_CONVERSION);
			STR(DESCRIPTOR_UPDATE_TEMPLATE);
			STR(SURFACE_KHR);
			STR(SWAPCHAIN_KHR);
			STR(DISPLAY_KHR);
			STR(DISPLAY_MODE_KHR);
			STR(DEBUG_REPORT_CALLBACK_EXT);
			// Now not defined in general???
			// STR(OBJECT_TABLE_NVX);
			// STR(INDIRECT_COMMANDS_LAYOUT_NVX);
			STR(DEBUG_UTILS_MESSENGER_EXT);
			STR(VALIDATION_CACHE_EXT);
			// Not defined on some older versions of the API
			// STR(ACCELERATION_STRUCTURE_NV);
#undef STR
		default:
			return "UNKNOWN_DEFAULT";
		}
	}

	void PrintDashed(String str) {
		i32 width = 80-(i32)str.size;
		if (width > 0) {
			for (u32 i = (width+1)/2; i > 0; i--) {
				cout.Print("-");
			}
			cout.Print(str);
			for (u32 i = width/2; i > 0; i--) {
				cout.Print("-");
			}
		} else {
			cout.Print(str);
		}
		cout.Newline();
	}

	String FormatSize(u64 size) {
		String str;
		bool space = false;
		if (size > 1024*1024*1024) {
			str = ToString(size/(1024*1024*1024)) + " GiB";
			size %= (1024*1024*1024);
			space = true;
		}
		if (size > 1024*1024) {
			if (space) {
				str += ", ";
			}
			str += ToString(size/(1024*1024)) + " MiB";
			size %= (1024*1024);
			space = true;
		}
		if (size > 1024) {
			if (space) {
				str += ", ";
			}
			str += ToString(size/1024) + " KiB";
			size %= 1024;
			space = true;
		}
		if (size > 0) {
			if (space) {
				str += ", ";
			}
			str += ToString(size) + " B";
		}
		return str;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void* userData) {

		const VkDebugUtilsMessengerCallbackDataEXT& data = *pCallbackData;

		hadValidationError = true;
		cout.Lock();

		PrintDashed("Validation Message Begin");

		cout.PrintLn("Message ID Name: \"", data.pMessageIdName, "\"\nMessage: \"", data.pMessage);

		cout.PrintLn(data.queueLabelCount, " Queue Labels:");
		for (u32 i = 0; i < data.queueLabelCount; i++) {
			const VkDebugUtilsLabelEXT& label = data.pQueueLabels[i];
			cout.PrintLn("\t", label.pLabelName, " with color {", label.color[0], ", ", label.color[1], ", ", label.color[2], ", ", label.color[3], "}");
		}
		cout.PrintLn(data.cmdBufLabelCount, " Command Buffer Labels:");
		for (u32 i = 0; i < data.cmdBufLabelCount; i++) {
			const VkDebugUtilsLabelEXT& label = data.pCmdBufLabels[i];
			cout.PrintLn("\t", label.pLabelName, " with color {", label.color[0], ", ", label.color[1], ", ", label.color[2], ", ", label.color[3], "}");
		}
		cout.PrintLn(data.objectCount, " Objects:");
		for (u32 i = 0; i < data.objectCount; i++) {
			const VkDebugUtilsObjectNameInfoEXT& name = data.pObjects[i];
			cout.Print("\tType: ", ObjectTypeString(name.objectType), " with name: ");
			if (name.pObjectName != nullptr) {
				cout.PrintLn(name.pObjectName, "");
			} else {
				cout.PrintLn("nullptr");
			}
		}

		PrintDashed("Validation Message End");

		cout.Unlock();

		return VK_FALSE;
	}

#ifndef VK_NO_ALLOCATION_CALLBACKS

	VKAPI_ATTR void* VKAPI_CALL Allocate(void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
		Instance *instance = (Instance*)pUserData;
		size_t aligned = align(size, alignment);
		// cout.PrintLn("Allocate size ", size, " with alignment ", alignment, " should be ", aligned);
		instance->data.allocationMutex.lock();
#ifdef __unix
		void *ptr = aligned_alloc(alignment, aligned);
#elif defined(_WIN32)
		void *ptr = malloc(aligned);
#endif
		instance->data.allocations.Append({ptr, aligned});
		instance->data.totalHeapMemory += aligned;
		instance->data.allocationMutex.unlock();
		return ptr;
	}

	VKAPI_ATTR void* VKAPI_CALL Reallocate(void *pUserData, void *pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
		// cout.PrintLn("Reallocate");
		if (pOriginal == nullptr) {
			return Allocate(pUserData, size, alignment, allocationScope);
		}
		Instance *instance = (Instance*)pUserData;
		size_t aligned = align(size, alignment);
		size_t originalSize = 0;
		Instance::data_t::Allocation *allocation = nullptr;
		instance->data.allocationMutex.lock();
		for (auto& i : instance->data.allocations) {
			if (i.ptr == pOriginal) {
				originalSize = i.size;
				allocation = &i;
				break;
			}
		}
		if (aligned <= originalSize) {
			// cout.PrintLn("Reallocate doesn't need to do anything, returning original ptr");
			return pOriginal;
		}
#ifdef __unix
		void *ptr = aligned_alloc(alignment, aligned);
#elif defined(_WIN32)
		void *ptr = malloc(aligned);
#endif
		allocation->ptr = ptr;
		allocation->size = aligned;
		instance->data.totalHeapMemory += aligned;
		instance->data.totalHeapMemory -= originalSize;
		instance->data.allocationMutex.unlock();
		memcpy(ptr, pOriginal, min(originalSize, size));
		free(pOriginal);
		return ptr;
	}

	VKAPI_ATTR void VKAPI_CALL Free(void *pUserData, void *pMemory) {
		// cout.PrintLn("Free");
		if (pMemory == nullptr) {
			return;
		}
		Instance *instance = (Instance*)pUserData;
		i32 index = 0;
		size_t originalSize = 0;
		instance->data.allocationMutex.lock();
		for (auto& i : instance->data.allocations) {
			if (i.ptr == pMemory) {
				originalSize = i.size;
				break;
			}
			index++;
		}
		if (index == instance->data.allocations.size) {
			cout.PrintLn("Attempted to free memory that we don't have in our list of allocations: ", (u64)pMemory);
			return;
		}
		instance->data.allocations.Erase(index);
		instance->data.totalHeapMemory -= originalSize;
		instance->data.allocationMutex.unlock();
		free(pMemory);
	}

#endif // VK_NO_ALLOCATION_CALLBACKS

	void CmdBindVertexBuffers(VkCommandBuffer commandBuffer, u32 firstBinding,
							  Array<Ptr<Buffer>> buffers, Array<VkDeviceSize> offsets) {
		if (offsets.size == 0) {
			// We can automatically put them all after each other
			offsets.Resize(buffers.size);
			for (i32 i = 1; i < buffers.size; i++) {
				offsets[i] = offsets[i-1] + buffers[i]->size;
			}
		} else if (offsets.size != buffers.size) {
			error = "CmdBindVertexBuffers needs offsets to either be empty or the same size as buffers";
			return;
		}
		Array<VkBuffer> vkBuffers(buffers.size);
		for (i32 i = 0; i < buffers.size; i++) {
			vkBuffers[i] = buffers[i]->data.buffer;
		}
		vkCmdBindVertexBuffers(commandBuffer, firstBinding, vkBuffers.size, vkBuffers.data, offsets.data);
	}

#ifdef AZCORE_IO_FOR_VULKAN

#ifdef __unix
	bool Window::CreateVkSurface(Instance *instance) {
		if (nullptr == surfaceWindow || !surfaceWindow->open) {
			error = "CreateVkSurface was called before the window was created!";
			return false;
		}
		if (surfaceWindow->data->useWayland) {
			VkWaylandSurfaceCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR};
			createInfo.display = surfaceWindow->data->wayland.display;
			createInfo.surface = surfaceWindow->data->wayland.surface;
			VkResult result = vkCreateWaylandSurfaceKHR(instance->data.instance, &createInfo, nullptr, &surface);
			if (result != VK_SUCCESS) {
				error = "Failed to create Vulkan Wayland surface: " + ErrorString(result);
				return false;
			}
		} else {
			VkXcbSurfaceCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR};
			createInfo.connection = surfaceWindow->data->x11.connection;
			createInfo.window = surfaceWindow->data->x11.window;
			VkResult result = vkCreateXcbSurfaceKHR(instance->data.instance, &createInfo, nullptr, &surface);
			if (result != VK_SUCCESS) {
				error = "Failed to create Vulkan XCB surface!";
				return false;
			}
		}
		return true;
	}
#elif defined(_WIN32)
	bool Window::CreateVkSurface(Instance *instance) {
		if (nullptr == surfaceWindow || !surfaceWindow->open) {
			error = "CreateVkSurface was called before the window was created!";
			return false;
		}
		VkWin32SurfaceCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
		createInfo.hinstance = surfaceWindow->data->instance;
		createInfo.hwnd = surfaceWindow->data->window;
		VkResult result = vkCreateWin32SurfaceKHR(instance->data.instance, &createInfo, nullptr, &surface);
		if (result != VK_SUCCESS) {
			error = "Failed to create Win32 Surface!";
			return false;
		}
		return true;
	}
#endif

#endif // AZCORE_IO_FOR_VULKAN

	static void SetDebugMarker(Device *device, const String &debugMarker, VkObjectType objectType, u64 objectHandle) {
		if (debugMarker.size != 0) {
			VkDebugUtilsObjectNameInfoEXT nameInfo = {};
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			nameInfo.objectType = objectType;
			nameInfo.objectHandle = objectHandle;
			nameInfo.pObjectName = debugMarker.data;
			device->data.instance->data.fpSetDebugUtilsObjectNameEXT(device->data.device, &nameInfo);
		}
	}

	bool PhysicalDevice::Init(VkInstance instance) {
		properties = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
		vkGetPhysicalDeviceProperties2(physicalDevice, &properties);
		features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
		features.pNext = &scalarBlockLayoutFeatures;
		scalarBlockLayoutFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES};
		vkGetPhysicalDeviceFeatures2(physicalDevice, &features);

		u32 extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
		extensionsAvailable.Resize(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensionsAvailable.data);

		u32 queueFamiliesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamiliesCount, nullptr);
		queueFamiliesAvailable.Resize(queueFamiliesCount, {VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2});
		vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamiliesCount, queueFamiliesAvailable.data);

		memoryProperties = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
		vkGetPhysicalDeviceMemoryProperties2(physicalDevice, &memoryProperties);

		score = 0;

		if (properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}

		score += properties.properties.limits.maxImageDimension2D;
		return true;
	}

	void PhysicalDevice::PrintInfo(Array<Window> windows, bool checkSurface) {
		// Basic info
		cout.PrintLn("Name: ", properties.properties.deviceName, "\nVulkan: ", VK_VERSION_MAJOR(properties.properties.apiVersion), ".", VK_VERSION_MINOR(properties.properties.apiVersion), ".", VK_VERSION_PATCH(properties.properties.apiVersion));
		// Memory
		u64 deviceLocalMemory = 0;
		for (u32 i = 0; i < memoryProperties.memoryProperties.memoryHeapCount; i++) {
			if (memoryProperties.memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
				deviceLocalMemory += memoryProperties.memoryProperties.memoryHeaps[i].size;
		}
		cout.PrintLn("Memory: ", FormatSize(deviceLocalMemory));
		// Queue families
		cout.Print("Queue Families:");
		for (i32 i = 0; i < queueFamiliesAvailable.size; i++) {
			const VkQueueFamilyProperties2 &props = queueFamiliesAvailable[i];
			VkQueueFlags queueFlags = props.queueFamilyProperties.queueFlags;
			cout.Print("\n\tFamily[", i, "] Queue count: ", props.queueFamilyProperties.queueCount, "\tSupports: ", ((queueFlags & VK_QUEUE_COMPUTE_BIT) ? "COMPUTE " : ""), ((queueFlags & VK_QUEUE_GRAPHICS_BIT) ? "GRAPHICS " : ""), ((queueFlags & VK_QUEUE_TRANSFER_BIT) ? "TRANSFER " : ""));
			if (checkSurface) {
				String presentString = "PRESENT on windows {";
				VkBool32 presentSupport = false;
				bool first = true;
				for (i32 j = 0; j < windows.size; j++) {
					vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, windows[j].surface, &presentSupport);
					if (presentSupport) {
						if (!first)
							presentString += ", ";
						presentString += ToString(j);
						first = false;
						break;
					}
				}
				presentString += "}";
				if (!first)
					cout.Print(presentString);
			}
		}
		cout.Newline();
	}

	void Image::Init(Device *device, String debugMarker) {
		data.device = device;
		if (debugMarker.size != 0) {
			data.debugMarker[0] = debugMarker;
			data.debugMarker[1] = std::move(debugMarker);
		}
	}

	bool Image::CreateImage(bool hostVisible) {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.imageExists) {
			error = "Attempting to create image that already exists!";
			return false;
		}
		if (samples != VK_SAMPLE_COUNT_1_BIT) {
			if (mipLevels != 1) {
				error = "Images with a sample count higher than 1 must have exactly 1 mipLevel";
				return false;
			}
			VkImageFormatProperties imageFormatProperties;
			VkResult result = vkGetPhysicalDeviceImageFormatProperties(
				data.device->data.physicalDevice.physicalDevice, format, VK_IMAGE_TYPE_2D,
				hostVisible ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL,
				usage, 0, &imageFormatProperties);
			if (result != VK_SUCCESS) {
				error = "Failed to vkGetPhysicalDeviceImageFormatProperties!";
				return false;
			}
			if (!(imageFormatProperties.sampleCounts & samples)) {
				error = "Image format 0x" + ToString(format, 16) + " doesn't support sample count " + ToString(samples);
				return false;
			}
		}
#endif
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1; // TODO: Animations?
		imageInfo.format = format;
		imageInfo.tiling = hostVisible ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = samples;
		imageInfo.flags = 0;

		VkResult result = vkCreateImage(data.device->data.device, &imageInfo, nullptr, &data.image);
		if (result != VK_SUCCESS) {
			error = "Failed to create image: " + ErrorString(result);
			return false;
		}

		SetDebugMarker(data.device, data.debugMarker[0], VK_OBJECT_TYPE_IMAGE, (u64)data.image);

		data.imageExists = true;
		return true;
	}

	bool Image::CreateImageView() {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.imageViewExists) {
			error = "Attempting to create an image view that already exists!";
			return false;
		}
#endif
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = data.image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = aspectFlags;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = mipLevels;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(data.device->data.device, &createInfo, nullptr, &data.imageView);
		if (result != VK_SUCCESS) {
			error = "Failed to create image view: " + ErrorString(result);
			return false;
		}

		SetDebugMarker(data.device, data.debugMarker[1], VK_OBJECT_TYPE_IMAGE_VIEW, (u64)data.imageView);

		data.imageViewExists = true;
		return true;
	}

	void Image::CopyData(void *src, u32 bytesPerPixel) {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (src == nullptr) {
			cout.PrintLn("Warning: Image::CopyData src is nullptr! Skipping copy.");
			return;
		}
		if (data.memory == nullptr) {
			cout.PrintLn("Warning: Image::CopyData: Image is not associated with a Memory object! Skipping copy.");
			return;
		}
#endif
		data.memory->CopyData2D(src, Ptr<Image>(this), data.offsetIndex, bytesPerPixel);
	}

	void Image::TransitionLayout(VkCommandBuffer commandBuffer, VkImageLayout from, VkImageLayout to, u32 baseMipLevel, u32 mipLevelCount) {
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = aspectFlags;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;
		subresourceRange.baseMipLevel = baseMipLevel;
		subresourceRange.levelCount = mipLevelCount;

		TransitionLayout(commandBuffer, from, to, subresourceRange);
	}

	void Image::TransitionLayout(VkCommandBuffer commandBuffer, VkImageLayout from, VkImageLayout to, VkImageSubresourceRange subresourceRange) {
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = from;
		barrier.newLayout = to;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = data.image;
		barrier.subresourceRange = subresourceRange;

		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;

		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

		switch (from) {
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_HOST_BIT;
			break;
		case VK_IMAGE_LAYOUT_UNDEFINED:
			barrier.srcAccessMask = 0;
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
			// NOTE: Not sure exactly how to handle the last two cases???
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		default:
			cout.PrintLn("Warning: Image::TransitionLayout from layout is not explicitly supported! Keeping defaults... This may not work as intended.");
			break;
		}

		switch (to) {
		case VK_IMAGE_LAYOUT_GENERAL:
			barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			dstStage = VK_PIPELINE_STAGE_HOST_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
			// NOTE: Not sure exactly how to handle the last two cases???
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		default:
			cout.PrintLn("Warning: Image::TransitionLayout to layout is not explicitly supported! Keeping defaults... This may not work as intended.");
			break;
		}

		vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	void Image::Copy(VkCommandBuffer commandBuffer, Ptr<Buffer> src) {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (!src.Valid()) {
			cout.PrintLn("Warning: Image::Copy src is not a valid Ptr! Skipping copy.");
			return;
		}
#endif

		VkBufferImageCopy copyRegion = {};
		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;

		copyRegion.imageOffset = {0, 0, 0};
		copyRegion.imageExtent = {width, height, 1};
		vkCmdCopyBufferToImage(commandBuffer, src->data.buffer, data.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
	}

	void Image::GenerateMipMaps(VkCommandBuffer commandBuffer, VkImageLayout startingLayout, VkImageLayout finalLayout) {
		if (mipLevels <= 1) {
			cout.Print("Warning: Image::GenerateMipMaps only has 1 mipLevel.");
			if (startingLayout != finalLayout) {
				cout.PrintLn(" Doing the transition only.");
				TransitionLayout(commandBuffer, startingLayout, finalLayout);
			} else {
				cout.PrintLn(" We have nothing to do.");
			}
			return;
		}
		TransitionLayout(commandBuffer, startingLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		for (u32 mip = 1; mip < mipLevels; mip++) {
			VkImageBlit imageBlit{};
			imageBlit.srcSubresource.aspectMask = aspectFlags;
			imageBlit.srcSubresource.layerCount = 1;
			imageBlit.srcSubresource.mipLevel = mip-1;
			imageBlit.srcOffsets[1].x = (i32)max(width >> (mip-1), 1u);
			imageBlit.srcOffsets[1].y = (i32)max(height >> (mip-1), 1u);
			imageBlit.srcOffsets[1].z = 1;

			imageBlit.dstSubresource.aspectMask = aspectFlags;
			imageBlit.dstSubresource.layerCount = 1;
			imageBlit.dstSubresource.mipLevel = mip;
			imageBlit.dstOffsets[1].x = (i32)max(width >> mip, 1u);
			imageBlit.dstOffsets[1].y = (i32)max(height >> mip, 1u);
			imageBlit.dstOffsets[1].z = 1;

			TransitionLayout(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip, 1);

			vkCmdBlitImage(commandBuffer, data.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
							data.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

			TransitionLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mip, 1);
		}

		TransitionLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, finalLayout, 0, mipLevels);
	}

	void Image::Clean() {
		if (data.imageViewExists) {
			vkDestroyImageView(data.device->data.device, data.imageView, nullptr);
			data.imageViewExists = false;
		}
		if (data.imageExists) {
			vkDestroyImage(data.device->data.device, data.image, nullptr);
			data.imageExists = false;
		}
	}

	void Buffer::Init(Device *device, String debugMarker) {
		data.device = device;
		data.debugMarker = std::move(debugMarker);
	}

	bool Buffer::Create() {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.exists) {
			error = "Buffer already exists!";
			return false;
		}
#endif
		VkBufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = size;
		createInfo.usage = usage;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(data.device->data.device, &createInfo, nullptr, &data.buffer);

		if (result != VK_SUCCESS) {
			error = "Failed to create buffer: " + ErrorString(result);
			return false;
		}

		SetDebugMarker(data.device, data.debugMarker, VK_OBJECT_TYPE_BUFFER, (u64)data.buffer);

		data.exists = true;
		return true;
	}

	void Buffer::CopyData(void *src, VkDeviceSize copySize, VkDeviceSize dstOffset) {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (src == nullptr) {
			cout.PrintLn("Warning: Buffer::CopyData src is nullptr! Skipping copy.");
			return;
		}
		if (data.memory == nullptr) {
			cout.PrintLn("Warning: Buffer::CopyData: Buffer is not associated with a Memory object! Skipping copy.");
			return;
		}
		if (copySize+dstOffset > size) {
			cout.PrintLn("Warning: Buffer::CopyData copySize+dstOffset goes beyond buffer size! Skipping copy.");
			return;
		}
#endif
		if (copySize == 0) {
			copySize = size-dstOffset;
		}
		data.memory->CopyData(src, copySize, data.offsetIndex);
	}

	void Buffer::Copy(VkCommandBuffer commandBuffer, Ptr<Buffer> src,
			VkDeviceSize copySize, VkDeviceSize dstOffset, VkDeviceSize srcOffset)
	{
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (!src.Valid()) {
			cout.PrintLn("Warning: Buffer::Copy src is not a valid Ptr! Skipping copy.");
			return;
		}
		if (dstOffset >= size) {
			cout.PrintLn("Warning: Buffer::Copy dstOffset is greater than dst size! Skipping copy.");
			return;
		}
		if (srcOffset >= src->size) {
			cout.PrintLn("Warning: Buffer::Copy srcOffset is greater than src size! Skipping copy.");
			return;
		}
#endif
		if (copySize == 0) {
			if (src->size-srcOffset > size-dstOffset) {
				cout.PrintLn("Warning: Buffer::Copy with unspecified copySize has a larger src size than dst at given offsets!");
				copySize = size - dstOffset;
			} else {
				copySize = src->size - srcOffset;
			}
		}

		VkBufferCopy copyRegion;
		copyRegion.size = copySize;
		copyRegion.dstOffset = dstOffset;
		copyRegion.srcOffset = srcOffset;
		vkCmdCopyBuffer(commandBuffer, src->data.buffer, data.buffer, 1, &copyRegion);
	}

	void Buffer::QueueOwnershipRelease(VkCommandBuffer commandBuffer, Ptr<Queue> srcQueue, Ptr<Queue> dstQueue,
									   VkAccessFlags srcAccessMask, VkPipelineStageFlags srcStageMask) {
		if (srcQueue->queueFamilyIndex == dstQueue->queueFamilyIndex) {
			return; // No transfer necessary
		}
		VkBufferMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.buffer = data.buffer;
		barrier.size = size;
		barrier.srcQueueFamilyIndex = static_cast<u32>(srcQueue->queueFamilyIndex);
		barrier.dstQueueFamilyIndex = static_cast<u32>(dstQueue->queueFamilyIndex);
		barrier.srcAccessMask = srcAccessMask;
		barrier.dstAccessMask = 0;

		vkCmdPipelineBarrier(commandBuffer, srcStageMask, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);
	}

	void Buffer::QueueOwnershipAcquire(VkCommandBuffer commandBuffer, Ptr<Queue> srcQueue, Ptr<Queue> dstQueue,
									   VkAccessFlags dstAccessMask, VkPipelineStageFlags dstStageMask) {
		if (srcQueue->queueFamilyIndex == dstQueue->queueFamilyIndex) {
			return; // No transfer necessary
		}
		VkBufferMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.buffer = data.buffer;
		barrier.size = size;
		barrier.srcQueueFamilyIndex = static_cast<u32>(srcQueue->queueFamilyIndex);
		barrier.dstQueueFamilyIndex = static_cast<u32>(dstQueue->queueFamilyIndex);
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = dstAccessMask;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, dstStageMask, 0, 0, nullptr, 1, &barrier, 0, nullptr);
	}

	void Buffer::Clean() {
		if (data.exists) {
			vkDestroyBuffer(data.device->data.device, data.buffer, nullptr);
			data.exists = false;
		}
	}

	Ptr<Image> Memory::AddImage(Image image) {
		data.images.Append(image);
		return data.images.GetPtr(data.images.size-1);
	}

	Ptr<Buffer> Memory::AddBuffer(Buffer buffer) {
		data.buffers.Append(buffer);
		return data.buffers.GetPtr(data.buffers.size-1);
	}

	Range<Image> Memory::AddImages(u32 count, Image image) {
		data.images.Resize(data.images.size+count, image);
		return data.images.GetRange(data.images.size-count, count);
	}

	Range<Buffer> Memory::AddBuffers(u32 count, Buffer buffer) {
		data.buffers.Resize(data.buffers.size+count, buffer);
		return data.buffers.GetRange(data.buffers.size-count, count);
	}

	bool Memory::Init(Device *device, String debugMarker) {
		PrintDashed("Initializing Memory");
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.initted) {
			error = "Memory has already been initialized!";
			return false;
		}
		if (data.allocated) {
			error = "Memory has already been allocated!";
			return false;
		}
		if (device == nullptr) {
			error = "device is nullptr!";
			return false;
		}
#endif
		data.device = device;
		data.physicalDevice = &device->data.physicalDevice;
		data.debugMarker = std::move(debugMarker);
		// First we figure out how big we are by going through the images and buffers
		data.offsets.Resize(1);
		data.offsets[0] = 0;
		data.memoryTypeBits = 0;
		u32 index = 0;
		VkResult result;
		if (deviceLocal) {
			data.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		} else {
			data.memoryProperties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		}

		cout.PrintLn("Memory will create ", data.images.size, " images and ", data.buffers.size, " buffers.");

		for (Image& image : data.images) {
			image.data.memory = this;
			if (data.debugMarker.size == 0) {
				image.Init(data.device);
			} else {
				image.Init(data.device, data.debugMarker + ".images[" + ToString(index) + "]");
				index++;
			}
			if (!image.CreateImage(!deviceLocal)) {
				goto failure;
			}
			if ((image.data.offsetIndex = GetImageChunk(image, data.memoryTypeBits!=0)) == -1) {
				goto failure;
			}
		}
		index = 0;
		for (Buffer& buffer : data.buffers) {
			buffer.data.memory = this;
			if (data.debugMarker.size == 0) {
				buffer.Init(data.device);
			} else {
				buffer.Init(data.device, data.debugMarker + ".buffers[" + ToString(index) + "]");
				index++;
			}
			if (!buffer.Create()) {
				goto failure;
			}
			if ((buffer.data.offsetIndex = GetBufferChunk(buffer, data.memoryTypeBits!=0)) == -1) {
				// There's a solid chance that memory types are incompatible between
				// images and buffers, so this will probably fail if you have images too.
				goto failure;
			}
		}
		// Then allocate as much space as we need
		if (!Allocate()) {
			goto failure;
		}
		index = 0;
		// Now bind our images and buffers to the memory
		for (Image& image : data.images) {
			result = vkBindImageMemory(data.device->data.device, image.data.image, data.memory, data.offsets[index++]);
			if (result != VK_SUCCESS) {
				error = "Failed to bind image to memory: " + ErrorString(result);
				goto failure;
			}
			if (!image.CreateImageView()) {
				goto failure;
			}
		}
		for (Buffer& buffer : data.buffers) {
			result = vkBindBufferMemory(data.device->data.device, buffer.data.buffer, data.memory, data.offsets[index++]);
			if (result != VK_SUCCESS) {
				error = "Failed to bind buffer to memory: " + ErrorString(result);
				goto failure;
			}
		}
		data.initted = true;
		return true;
failure:
		for (Image& image : data.images) {
			image.Clean();
		}
		for (Buffer& buffer : data.buffers) {
			buffer.Clean();
		}
		if (data.allocated) {
			vkFreeMemory(data.device->data.device, data.memory, nullptr);
			data.allocated = false;
		}
		return false;
	}

	bool Memory::Deinit() {
		PrintDashed("Destroying Memory");
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (!data.initted) {
			// error = "Memory isn't initialized!";
			return false;
		}
#endif
		for (Image& image : data.images) {
			image.Clean();
		}
		for (Buffer& buffer : data.buffers) {
			buffer.Clean();
		}
		data.initted = false;
		if (data.allocated)
			vkFreeMemory(data.device->data.device, data.memory, nullptr);
		data.allocated = false;
		return true;
	}

	i32 Memory::GetImageChunk(Image image, bool noChange) {
		i32 index = data.offsets.size-1;
		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(data.device->data.device, image.data.image, &memReqs);
		if (noChange && data.memoryTypeBits != memReqs.memoryTypeBits) {
			error = "An image is incompatible with the memory previously alotted!";
			return -1;
		}
		data.memoryTypeBits = memReqs.memoryTypeBits;

		// NOTE: Images can have different alignment values, and must begin at an aligned offset
		data.offsets.Back() = align(data.offsets.Back(), memReqs.alignment);

		// u32 alignedOffset;
		// if (memReqs.size % memReqs.alignment == 0) {
		//	 alignedOffset = memReqs.size;
		// } else {
		//	 alignedOffset = (memReqs.size/memReqs.alignment+1)*memReqs.alignment;
		// }

		data.offsets.Append(data.offsets.Back() + memReqs.size);
		return index;
	}

	i32 Memory::GetBufferChunk(Buffer buffer, bool noChange) {
		i32 index = data.offsets.size-1;
		VkMemoryRequirements memReqs;
		vkGetBufferMemoryRequirements(data.device->data.device, buffer.data.buffer, &memReqs);
		if (noChange && data.memoryTypeBits != memReqs.memoryTypeBits) {
			error = "A buffer is incompatible with the memory previously alotted!";
			return -1;
		}
		data.memoryTypeBits = memReqs.memoryTypeBits;

		u32 alignedOffset;
		if (memReqs.size % memReqs.alignment == 0) {
			alignedOffset = memReqs.size;
		} else {
			alignedOffset = (memReqs.size/memReqs.alignment+1)*memReqs.alignment;
		}

		data.offsets.Append(data.offsets.Back() + alignedOffset);
		return index;
	}

	VkDeviceSize Memory::ChunkSize(i32 index) {
		return data.offsets[index+1] - data.offsets[index];
	}

	i32 Memory::FindMemoryType() {
		VkPhysicalDeviceMemoryProperties2 memProps = data.physicalDevice->memoryProperties;
		for (u32 i = 0; i < memProps.memoryProperties.memoryTypeCount; i++) {
			if ((data.memoryTypeBits & (1 << i)) && (memProps.memoryProperties.memoryTypes[i].propertyFlags & data.memoryProperties) == data.memoryProperties) {
				return i;
			}
		}
		for (u32 i = 0; i < memProps.memoryProperties.memoryTypeCount; i++) {
			if ((data.memoryTypeBits & (1 << i)) && (memProps.memoryProperties.memoryTypes[i].propertyFlags & data.memoryPropertiesDeferred) == data.memoryPropertiesDeferred) {
				return i;
			}
		}

		error = "Failed to find a suitable memory type!";
		return -1;
	}

	bool Memory::Allocate() {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.allocated) {
			error = "Memory already allocated!";
			return false;
		}
#endif
		cout.PrintLn("Allocating ", (deviceLocal ? "Device" : "Host"), " Memory with size: ", FormatSize(data.offsets.Back()));
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = data.offsets.Back();
		i32 mti = FindMemoryType();
		if (mti == -1) {
			return false;
		}
		allocInfo.memoryTypeIndex = (u32)mti;

		VkResult result = vkAllocateMemory(data.device->data.device, &allocInfo, nullptr, &data.memory);
		if (result != VK_SUCCESS) {
			error = "Failed to allocate memory: " + ErrorString(result);
			return false;
		}
		data.allocated = true;
		return true;
	}

	void Memory::CopyData(void *src, VkDeviceSize size, i32 index) {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (src == nullptr) {
			cout.PrintLn("Warning: Memory::CopyData has nullptr src! Skipping copy.");
			return;
		}
		if (size == 0) {
			cout.PrintLn("Warning: Memory::CopyData has size of 0! Skipping copy.");
			return;
		}
		if ((data.memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0
			&& (data.memoryPropertiesDeferred & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
			cout.PrintLn("Warning: Memory::CopyData is trying to copy memory that isn't host coherent!");
		}
		if ((data.memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0
			&& (data.memoryPropertiesDeferred & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0) {
			cout.PrintLn("Warning: Memory::CopyData is trying to copy memory that isn't host visible!");
		}
		if (index >= data.offsets.size-1) {
			cout.PrintLn("Warning: Memory::CopyData offset index is out of bounds! Skipping copy.");
		}
#endif
		void* dst;
		vkMapMemory(data.device->data.device, data.memory, data.offsets[index], size, 0, &dst);
			memcpy(dst, src, (size_t)size);
		vkUnmapMemory(data.device->data.device, data.memory);
	}

	void Memory::CopyData2D(void *src, Ptr<Image> image, i32 index, u32 bytesPerPixel) {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (src == nullptr) {
			cout.PrintLn("Warning: Memory::CopyData2D has nullptr src! Skipping copy.");
			return;
		}
		if (!image.Valid()) {
			cout.PrintLn("Warning: Memory::CopyData2D image is not a valid Ptr! Skipping copy.");
			return;
		}
		if (image->width == 0) {
			cout.PrintLn("Warning: Memory::CopyData2D image has width of 0! Skipping copy.");
			return;
		}
		if (image->height == 0) {
			cout.PrintLn("Warning: Memory::CopyData2D image has height of 0! Skipping copy.");
			return;
		}
		if ((data.memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0
			&& (data.memoryPropertiesDeferred & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
			cout.PrintLn("Warning: Memory::CopyData2D is trying to copy memory that isn't host coherent!");
		}
		if ((data.memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0
			&& (data.memoryPropertiesDeferred & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0) {
			cout.PrintLn("Warning: Memory::CopyData2D is trying to copy memory that isn't host visible!");
		}
		if (index >= data.offsets.size-1) {
			cout.PrintLn("Warning: Memory::CopyData2D offset index is out of bounds! Skipping copy.");
		}
#endif
		VkImageSubresource subresource = {};
		subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource.mipLevel = 0;
		subresource.arrayLayer = 0;

		VkSubresourceLayout stagingImageLayout;
		vkGetImageSubresourceLayout(data.device->data.device, image->data.image, &subresource, &stagingImageLayout);

		size_t size = data.offsets[index+1] - data.offsets[index];

		void* dst;
		vkMapMemory(data.device->data.device, data.memory, data.offsets[index], size, 0, &dst);

		if (stagingImageLayout.rowPitch == image->width * bytesPerPixel) {
			memcpy(dst, src, image->width * image->height * bytesPerPixel);
		} else {
			u8* dataBytes = reinterpret_cast<u8*>(dst);

			for (u32 y = 0; y < image->height; y++) {
				memcpy( &dataBytes[y * stagingImageLayout.rowPitch],
						&reinterpret_cast<u8*>(src)[y * image->width * bytesPerPixel],
						image->width * bytesPerPixel );
			}
		}
		vkUnmapMemory(data.device->data.device, data.memory);
	}

	void* Memory::MapMemory(VkDeviceSize size, i32 index) {
		void *ptr;
		vkMapMemory(data.device->data.device, data.memory, data.offsets[index], size, 0, &ptr);
		data.mapped = true;
		return ptr;
	}

	void Memory::UnmapMemory() {
		if (data.mapped)
			vkUnmapMemory(data.device->data.device, data.memory);
		data.mapped = false;
	}

	Sampler::~Sampler() {
		Clean();
	}

	void Sampler::Init(Device *device, String debugMarker) {
		data.device = device;
		data.debugMarker = std::move(debugMarker);
	}

	bool Sampler::Create() {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.exists) {
			error = "Sampler already exists!";
			return false;
		}
#endif
		VkSamplerCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		createInfo.magFilter = magFilter;
		createInfo.minFilter = minFilter;
		createInfo.addressModeU = addressModeU;
		createInfo.addressModeV = addressModeV;
		createInfo.addressModeW = addressModeW;
		createInfo.maxAnisotropy = anisotropy;
		if (anisotropy != 1) {
			createInfo.anisotropyEnable = VK_TRUE;
		} else {
			createInfo.anisotropyEnable = VK_FALSE;
		}
		createInfo.borderColor = borderColor;
		createInfo.unnormalizedCoordinates = (VkBool32)unnormalizedCoordinates;
		createInfo.compareOp = compareOp;
		if (compareOp == VK_COMPARE_OP_NEVER) {
			createInfo.compareEnable = VK_FALSE;
		} else {
			createInfo.compareEnable = VK_TRUE;
		}
		createInfo.mipmapMode = mipmapMode;
		createInfo.mipLodBias = mipLodBias;
		createInfo.minLod = minLod;
		createInfo.maxLod = maxLod;

		VkResult result = vkCreateSampler(data.device->data.device, &createInfo, nullptr, &data.sampler);

		if (result != VK_SUCCESS) {
			error = "Failed to create sampler: " + ErrorString(result);
			return false;
		}

		SetDebugMarker(data.device, data.debugMarker, VK_OBJECT_TYPE_SAMPLER, (u64)data.sampler);

		data.exists = true;
		return true;
	}

	void Sampler::Clean() {
		if (data.exists) {
			vkDestroySampler(data.device->data.device, data.sampler, nullptr);
			data.exists = false;
		}
	}

	DescriptorLayout::~DescriptorLayout() {
		Clean();
	}

	void DescriptorLayout::Init(Device *device, String debugMarker) {
		data.device = device;
		data.debugMarker = std::move(debugMarker);
	}

	bool DescriptorLayout::Create() {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.exists) {
			error = "DescriptorLayout already created!";
			return false;
		}
#endif
		Array<VkDescriptorSetLayoutBinding> bindingInfo(bindings.size);
		for (i32 i = 0; i < bindingInfo.size; i++) {
			bindingInfo[i].binding = bindings[i].binding;
			bindingInfo[i].descriptorCount = bindings[i].count;
			bindingInfo[i].descriptorType = bindings[i].type;
			bindingInfo[i].stageFlags = bindings[i].stage;
			bindingInfo[i].pImmutableSamplers = nullptr; // TODO: Use these
		}
		VkDescriptorSetLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = bindingInfo.size;
		createInfo.pBindings = bindingInfo.data;

		VkResult result = vkCreateDescriptorSetLayout(data.device->data.device, &createInfo, nullptr, &data.layout);

		if (result != VK_SUCCESS) {
			error = "Failed to create Descriptor Set Layout: " + ErrorString(result);
			return false;
		}

		SetDebugMarker(data.device, data.debugMarker, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (u64)data.layout);

		data.exists = true;
		return true;
	}

	void DescriptorLayout::Clean() {
		if (data.exists) {
			vkDestroyDescriptorSetLayout(data.device->data.device, data.layout, nullptr);
			data.exists = false;
		}
	}

	bool DescriptorSet::AddDescriptor(Range<Buffer> buffers, i32 binding) {
		// TODO: Support other types of descriptors
		for (i32 i = 0; i < data.layout->bindings.size; i++) {
			if (data.layout->bindings[i].binding == binding) {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
				if (data.layout->bindings[i].type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
					error = "AddDescriptor failed because binding type is not for uniform buffers!";
					return false;
				}
#endif
				if (data.layout->bindings[i].count != buffers.size) {
					error = "AddDescriptor failed because input size is wrong("
						  + ToString(buffers.size) + ") for binding "
						  + ToString(binding) + " which expects "
						  + ToString(data.layout->bindings[i].count) + " buffers.";
					return false;
				}
				data.bindings.Append(data.layout->bindings[i]);
				break;
			}
		}
		data.bufferDescriptors.Append({buffers});
		return true;
	}

	bool DescriptorSet::AddDescriptor(Range<Image> images, Ptr<Sampler> sampler, i32 binding) {
		// TODO: Support other types of descriptors
		for (i32 i = 0; i < data.layout->bindings.size; i++) {
			if (data.layout->bindings[i].binding == binding) {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
				if (data.layout->bindings[i].type != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
					error = "AddDescriptor failed because binding type is not for combined image samplers!";
					return false;
				}
#endif
				if (data.layout->bindings[i].count != images.size) {
					error = "AddDescriptor failed because input size is wrong("
						  + ToString(images.size) + ") for binding "
						  + ToString(binding) + " which expects "
						  + ToString(data.layout->bindings[i].count) + " images.";
					return false;
				}
				data.bindings.Append(data.layout->bindings[i]);
				break;
			}
		}
		data.imageDescriptors.Append({images, sampler});
		return true;
	}

	bool DescriptorSet::AddDescriptor(Ptr<Buffer> buffer, i32 binding) {
		return AddDescriptor(Range<Buffer>((Array<Buffer>*)buffer.ptr, buffer.index, 1), binding);
	}

	bool DescriptorSet::AddDescriptor(Ptr<Image> image, Ptr<Sampler> sampler, i32 binding) {
		return AddDescriptor(Range<Image>((Array<Image>*)image.ptr, image.index, 1), sampler, binding);
	}

	Descriptors::~Descriptors() {
		Clean();
	}

	void Descriptors::Init(Device *device, String debugMarker) {
		data.device = device;
		data.debugMarker = std::move(debugMarker);
	}

	Ptr<DescriptorLayout> Descriptors::AddLayout() {
		data.layouts.Append(DescriptorLayout());
		return data.layouts.GetPtr(data.layouts.size-1);
	}

	Ptr<DescriptorSet> Descriptors::AddSet(Ptr<DescriptorLayout> layout) {
		DescriptorSet set{};
		set.data.layout = layout;
		data.sets.Append(set);
		return data.sets.GetPtr(data.sets.size-1);
	}

	bool Descriptors::Create() {
		PrintDashed("Creating Descriptors");
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.exists) {
			error = "Descriptors already exist!";
			return false;
		}
#endif
		Array<VkDescriptorPoolSize> poolSizes;
		poolSizes.Reserve(data.layouts.size);
		for (i32 i = 0; i < data.layouts.size; i++) {
			data.layouts[i].Init(data.device);
			if (!data.layouts[i].Create()) {
				error = "Failed to created descriptor set layout[" + ToString(i) + "]: " + error;
				Clean();
				return false;
			}
			for (i32 j = 0; j < data.layouts[i].bindings.size; j++) {
				VkDescriptorPoolSize poolSize;
				poolSize.type = data.layouts[i].bindings[j].type;
				poolSize.descriptorCount = data.layouts[i].bindings[j].count;
				poolSizes.Append(poolSize);
			}
		}

		VkDescriptorPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.poolSizeCount = poolSizes.size;
		createInfo.pPoolSizes = poolSizes.data;
		createInfo.maxSets = data.sets.size;

		VkResult result = vkCreateDescriptorPool(data.device->data.device, &createInfo, nullptr, &data.pool);

		if (result != VK_SUCCESS) {
			error = "Failed to create Descriptor Pool: " + ErrorString(result);
			return false;
		}

		SetDebugMarker(data.device, data.debugMarker, VK_OBJECT_TYPE_DESCRIPTOR_POOL, (u64)data.pool);

		data.exists = true;
		cout.PrintLn("Allocating ", data.sets.size, " Descriptor Sets.");
		Array<VkDescriptorSetLayout> setLayouts(data.sets.size);
		for (i32 i = 0; i < data.sets.size; i++) {
			setLayouts[i] = data.sets[i].data.layout->data.layout;
		}
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = data.pool;
		allocInfo.descriptorSetCount = setLayouts.size;
		allocInfo.pSetLayouts = setLayouts.data;

		Array<VkDescriptorSet> setsTemp(data.sets.size);

		result = vkAllocateDescriptorSets(data.device->data.device, &allocInfo, setsTemp.data);
		if (result != VK_SUCCESS) {
			error = "Failed to allocate Descriptor Sets: " + ErrorString(result);
			Clean();
			return false;
		}
		for (i32 i = 0; i < data.sets.size; i++) {
			data.sets[i].data.set = setsTemp[i];
			data.sets[i].data.exists = true;
			SetDebugMarker(data.device, data.sets[i].data.debugMarker.data, VK_OBJECT_TYPE_DESCRIPTOR_SET, (u64)data.sets[i].data.set);
		}

		return true;
	}

	bool Descriptors::Update() {
		// Find out how many of each we have ahead of time so we can size our arrays appropriately
		u32 totalBufferInfos = 0;
		u32 totalImageInfos = 0;

		for (i32 i = 0; i < data.sets.size; i++) {
			for (i32 j = 0; j < data.sets[i].data.bufferDescriptors.size; j++) {
				totalBufferInfos += data.sets[i].data.bufferDescriptors[j].buffers.size;
			}
			for (i32 j = 0; j < data.sets[i].data.imageDescriptors.size; j++) {
				totalImageInfos += data.sets[i].data.imageDescriptors[j].images.size;
			}
		}

		Array<VkWriteDescriptorSet> writes{};

		Array<VkDescriptorBufferInfo> bufferInfos(totalBufferInfos);
		Array<VkDescriptorImageInfo> imageInfos(totalImageInfos);

		u32 bInfoOffset = 0;
		u32 iInfoOffset = 0;

		for (i32 i = 0; i < data.sets.size; i++) {
			u32 setBufferDescriptor = 0;
			u32 setImageDescriptor = 0;
			VkWriteDescriptorSet write = {};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = data.sets[i].data.set;
			write.dstArrayElement = 0;
			for (i32 j = 0; j < data.sets[i].data.bindings.size; j++) {
				write.descriptorType = data.sets[i].data.bindings[j].type;
				write.dstBinding = data.sets[i].data.bindings[j].binding;
				write.descriptorCount = data.sets[i].data.bindings[j].count;

				switch(write.descriptorType) {
				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
					for (i32 x = 0; x < data.sets[i].data.bindings[j].count; x++) {
						VkDescriptorBufferInfo bufferInfo = {};
						Buffer &buffer = data.sets[i].data.bufferDescriptors[setBufferDescriptor].buffers[x];
						bufferInfo.buffer = buffer.data.buffer;
						bufferInfo.offset = 0;
						bufferInfo.range = buffer.size;
						bufferInfos[bInfoOffset++] = bufferInfo;
					}
					setBufferDescriptor++;
					write.pBufferInfo = &bufferInfos[bInfoOffset - data.sets[i].data.bindings[j].count];
					break;
				case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
					for (i32 x = 0; x < data.sets[i].data.bindings[j].count; x++) {
						VkDescriptorImageInfo imageInfo = {};
						ImageDescriptor &imageDescriptor = data.sets[i].data.imageDescriptors[setImageDescriptor];
						imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						imageInfo.imageView = imageDescriptor.images[x].data.imageView;
						imageInfo.sampler = imageDescriptor.sampler->data.sampler;
						imageInfos[iInfoOffset++] = imageInfo;
					}
					setImageDescriptor++;
					write.pImageInfo = &imageInfos[iInfoOffset - data.sets[i].data.bindings[j].count];
					break;
				default:
					error = "Unsupported descriptor type for updating descriptors!";
					return false;
				}
				writes.Append(write);
			}
		}
		// Well wasn't that just a lovely mess of code?
		vkUpdateDescriptorSets(data.device->data.device, writes.size, writes.data, 0, nullptr);
		return true;
	}

	void Descriptors::Clean() {
		if (data.exists) {
			PrintDashed("Destroying Descriptors");
			vkDestroyDescriptorPool(data.device->data.device, data.pool, nullptr);
			for (i32 i = 0; i < data.sets.size; i++) {
				data.sets[i].data.exists = false;
			}
		}
		for (i32 i = 0; i < data.layouts.size; i++) {
			data.layouts[i].Clean();
		}
		data.exists = false;
	}

	Attachment::Attachment() {}

	Attachment::Attachment(Ptr<Swapchain> swch) {
		swapchain = swch;
		if (swapchain.Valid()) {
			bufferColor = true;
			keepColor = true;
		}
	}

	bool Attachment::Config() {
		if (swapchain.Valid()) {
			formatColor = swapchain->data.surfaceFormat.format;
		}
		data.descriptions.Resize(0);
		if (bufferColor) {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
			if (initialLayoutColor == VK_IMAGE_LAYOUT_UNDEFINED && loadColor) {
				error = "For the contents of this attachment to be loaded, you must specify an initialLayout for Color.";
				return false;
			}
			if (swapchain != nullptr && sampleCount != VK_SAMPLE_COUNT_1_BIT && !resolveColor) {
				error = "You must resolve multisampling when connected to a swapchain.";
				return false;
			}
#endif
			if (sampleCount != VK_SAMPLE_COUNT_1_BIT && resolveColor) {
				// SSAA enabled, first attachment should be multisampled color buffer
				VkAttachmentDescription description{};
				description.format = formatColor;
				description.samples = sampleCount;

				if (clearColor) {
					description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				} else {
					description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				}
				if (loadColor) {
					description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				}

				description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

				description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

				description.initialLayout = initialLayoutColor;
				description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				data.descriptions.Append(description);

				// Next attachment should be the color buffer we resolve to

				// Keep same format.

				description.samples = VK_SAMPLE_COUNT_1_BIT;

				description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				if (keepColor) {
					description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				} else {
					description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				}

				// We don't care about the stencil since we're a color buffer.

				description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				if (swapchain.Valid()) {
					description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				} else {
					description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}
				data.descriptions.Append(description);
			} else {
				// Resolving disabled or unnecessary
				VkAttachmentDescription description{};
				description.format = formatColor;
				description.samples = sampleCount;

				if (clearColor) {
					description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				} else {
					description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				}
				if (loadColor) {
					description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				}
				if (keepColor) {
					description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				} else {
					description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				}

				description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

				description.initialLayout = initialLayoutColor;
				if (swapchain.Valid()) {
					description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				} else {
					description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}
				data.descriptions.Append(description);
			}
		}
		if (bufferDepthStencil) {
			// Even with multisampling, we only have one attachment for depth/stencil
			VkAttachmentDescription description{};
			description.format = formatDepthStencil;
			description.samples = sampleCount;

			if (clearDepth) {
				description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			} else {
				description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			}
			if (loadDepth) {
				description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			}

			if (keepDepth) {
				description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			} else {
				description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			}

			if (clearStencil) {
				description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			} else {
				description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			}
			if (loadStencil) {
				description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			}

			if (keepStencil) {
				description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			} else {
				description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			}
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
			if (initialLayoutDepthStencil == VK_IMAGE_LAYOUT_UNDEFINED
			&& (loadDepth || loadStencil)) {
				error = "For the contents of this attachment to be loaded, you must specify an initialLayout for DepthStencil.";
				return false;
			}
#endif

			description.initialLayout = initialLayoutDepthStencil;
			description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			data.descriptions.Append(description);
		}
		return true;
	}

	void Subpass::UseAttachment(Ptr<Attachment> attachment, AttachmentType type, VkAccessFlags accessFlags) {
		AttachmentUsage usage = {attachment.index, type, accessFlags};
		data.attachments.Append(usage);
	}

	Ptr<Subpass> RenderPass::AddSubpass() {
		data.subpasses.Append(Subpass());
		return Ptr<Subpass>(&data.subpasses, data.subpasses.size-1);
	}

	Ptr<Attachment> RenderPass::AddAttachment(Ptr<Swapchain> swapchain) {
		data.attachments.Append(Attachment(swapchain));
		return Ptr<Attachment>(&data.attachments, data.attachments.size-1);
	}

	void RenderPass::Begin(VkCommandBuffer commandBuffer, Ptr<Framebuffer> framebuffer, bool subpassContentsInline) {
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = data.renderPass;
		renderPassInfo.framebuffer = framebuffer->data.framebuffers[framebuffer->data.currentFramebuffer];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent.width = framebuffer->width;
		renderPassInfo.renderArea.extent.height = framebuffer->height;

		Array<VkClearValue> clearValues{};
		u32 i = 0; // Index of actual attachment for clearValues
		for (Attachment& attachment : data.attachments) {
			if (attachment.bufferColor) {
				if (attachment.clearColor) {
					clearValues.Resize(i+1);
					clearValues[i].color = attachment.clearColorValue;
				}
				i++;
			}
			if (attachment.resolveColor) {
				i++;
			}
			if (attachment.bufferDepthStencil) {
				if (attachment.clearDepth || attachment.clearStencil) {
					clearValues.Resize(i+1);
					clearValues[i].depthStencil = attachment.clearDepthStencilValue;
				}
				i++;
			}
		}
		renderPassInfo.clearValueCount = clearValues.size;
		if (clearValues.size != 0) {
			renderPassInfo.pClearValues = clearValues.data;
		}

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
			subpassContentsInline ? VK_SUBPASS_CONTENTS_INLINE : VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
	}

	RenderPass::~RenderPass() {
		if (data.initted) {
			if (!Deinit()) {
				cout.PrintLn("Failed to clean up vk::RenderPass: ", error);
			}
		}
	}

	bool RenderPass::Init(Device *device, String debugMarker) {
		PrintDashed("Initializing RenderPass");
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.initted) {
			error = "Renderpass is already initialized!";
			return false;
		}
		if (device == nullptr) {
			error = "Device is nullptr!";
			return false;
		}
		if (data.subpasses.size == 0) {
			error = "You must have at least 1 subpass in your renderpass!";
			return false;
		}
#endif
		data.device = device;
		data.debugMarker = std::move(debugMarker);
		// First we need to configure our subpass attachments
		for (i32 i = 0; i < data.attachments.size; i++) {
			if (!data.attachments[i].Config()) {
				error = "With attachment[" + ToString(i) + "]: " + error;
				return false;
			}
		}
		// Then we concatenate our attachmentDescriptions from the Attachments
		u32 nextAttachmentIndex = 0; // Since each Attachment can map to multiple attachments
		for (i32 i = 0; i < data.attachments.size; i++) {
			data.attachments[i].data.firstIndex = nextAttachmentIndex;
			data.attachmentDescriptions.Resize(nextAttachmentIndex + data.attachments[i].data.descriptions.size);
			for (i32 x = 0; x < data.attachments[i].data.descriptions.size; x++) {
				data.attachmentDescriptions[nextAttachmentIndex++] = data.attachments[i].data.descriptions[x];
			}
		}
		for (i32 i = 0; i < data.subpasses.size; i++) {
			bool depthStencilTaken = false; // Verify that we only have one depth/stencil attachment
			Subpass& subpass = data.subpasses[i];
			subpass.data.referencesColor.Resize(0);
			subpass.data.referencesResolve.Resize(0);
			subpass.data.referencesInput.Resize(0);
			subpass.data.referencesPreserve.Resize(0);

			for (i32 j = 0; j < subpass.data.attachments.size; j++) {
				String errorPrefix = "Subpass[" + ToString(i) + "] AttachmentUsage[" + ToString(j) + "] ";
				AttachmentUsage& usage = subpass.data.attachments[j];
				if (usage.index >= data.attachments.size) {
					error = errorPrefix + "index is out of bounds: " + ToString(usage.index);
					return false;
				}
				Attachment& attachment = data.attachments[usage.index];
				nextAttachmentIndex = attachment.data.firstIndex;
				i32 colorIndex = -1;
				i32 resolveIndex = -1;
				i32 depthStencilIndex = -1;
				if (attachment.bufferColor) {
					colorIndex = nextAttachmentIndex++;
					if (attachment.resolveColor && attachment.sampleCount != VK_SAMPLE_COUNT_1_BIT) {
						resolveIndex = nextAttachmentIndex++;
					}
				}
				if (attachment.bufferDepthStencil) {
					if (depthStencilTaken) {
						error = errorPrefix + "defines a second depth/stencil attachment. "
								"You can't have more than one depth/stencil attachment in a single subpass!";
						return false;
					}
					if (usage.type == ATTACHMENT_ALL || usage.type == ATTACHMENT_DEPTH_STENCIL) {
						depthStencilTaken = true;
					}
					depthStencilIndex = nextAttachmentIndex++;
				}
				u32 index = 0;
				if (usage.type == ATTACHMENT_COLOR) {
					if (colorIndex != -1) {
						index = colorIndex;
					} else {
						error = errorPrefix + "expects there to be a color buffer when there is not!";
						return false;
					}
				} else if (usage.type == ATTACHMENT_DEPTH_STENCIL) {
					if (depthStencilIndex != -1) {
						index = depthStencilIndex;
					} else {
						error = errorPrefix + "expects there to be a depth/stencil buffer when there is not!";
						return false;
					}
				} else if (usage.type == ATTACHMENT_RESOLVE) {
					if (resolveIndex != -1) {
						index = resolveIndex;
					} else {
						error = errorPrefix + "expects there to be a resolved color buffer when there is not!";
						return false;
					}
				} else if (usage.type != ATTACHMENT_ALL) {
					error = errorPrefix + "usage.type is an invalid value: " + ToString(usage.type);
					return false;
				}
				if (usage.accessFlags & VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT) {
					if (colorIndex == -1) {
						error = errorPrefix + "requests a color buffer for writing, but none is available.";
						return false;
					}
					VkAttachmentReference ref{};
					ref.attachment = colorIndex;
					ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					subpass.data.referencesColor.Append(ref);
					if (attachment.resolveColor && attachment.sampleCount != VK_SAMPLE_COUNT_1_BIT) {
						ref.attachment = resolveIndex;
						subpass.data.referencesResolve.Append(ref);
					}
				}
				if (usage.accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT) && attachment.bufferDepthStencil) {
					if (depthStencilIndex == -1) {
						error = errorPrefix + "requests a depth/stencil buffer for writing, but none is available.";
						return false;
					}
					VkAttachmentReference ref{};
					ref.attachment = depthStencilIndex;
					ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					subpass.data.referenceDepthStencil = ref;
				}
				if (usage.accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) {
					VkAttachmentReference ref{};
					ref.attachment = index;
					if (usage.type == ATTACHMENT_COLOR || usage.type == ATTACHMENT_RESOLVE) {
						ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					} else if (usage.type == ATTACHMENT_DEPTH_STENCIL) {
						ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
					}
					subpass.data.referencesInput.Append(ref);
				}
			}
			cout.PrintLn("Subpass[", i, "] is using the following attachments:");
			if (subpass.data.referencesColor.size != 0) {
				cout.Print("\tColor: ");
				for (auto& ref : subpass.data.referencesColor) {
					cout.Print(ref.attachment, " ");
				}
			}
			if (subpass.data.referencesResolve.size != 0) {
				cout.Print("\n\tResolve: ");
				for (auto& ref : subpass.data.referencesResolve) {
					cout.Print(ref.attachment, " ");
				}
			}
			if (subpass.data.referencesInput.size != 0) {
				cout.Print("\n\tInput: ");
				for (auto& ref : subpass.data.referencesInput) {
					cout.Print(ref.attachment, " ");
				}
			}
			if (subpass.data.referencesPreserve.size != 0) {
				cout.Print("\n\tPreserve: ");
				for (auto& ref : subpass.data.referencesPreserve) {
					cout.Print(ref, " ");
				}
			}
			if (depthStencilTaken) {
				cout.Print("\n\tDepth: ", subpass.data.referenceDepthStencil.attachment);
			}
			cout.Newline();
			VkSubpassDescription description{};
			description.pipelineBindPoint = subpass.pipelineBindPoint;
			description.colorAttachmentCount = subpass.data.referencesColor.size;
			description.pColorAttachments = subpass.data.referencesColor.data;
			description.inputAttachmentCount = subpass.data.referencesInput.size;
			description.pInputAttachments = subpass.data.referencesInput.data;
			description.preserveAttachmentCount = subpass.data.referencesPreserve.size;
			description.pPreserveAttachments = subpass.data.referencesPreserve.data;
			if (subpass.data.referencesResolve.size != 0) {
				description.pResolveAttachments = subpass.data.referencesResolve.data;
			}
			if (depthStencilTaken) {
				description.pDepthStencilAttachment = &subpass.data.referenceDepthStencil;
			}
			data.subpassDescriptions.Append(description);
		}
		// Now we configure our dependencies
		if (initialTransition) {
			VkSubpassDependency dep;
			dep.srcSubpass = VK_SUBPASS_EXTERNAL; // Transition from before this RenderPass
			dep.dstSubpass = 0; // Our first subpass
			dep.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // Make sure the image is done being used
			dep.dstStageMask = initialAccessStage; // The stage at which we need the image in a certain layout
			dep.srcAccessMask = initialAccess;
			bool depth = false;
			bool color = false;
			bool resolve = false;
			for (AttachmentUsage& usage : data.subpasses[0].data.attachments) {
				switch (usage.type) {
					case ATTACHMENT_COLOR: {
						color = true;
						break;
					}
					case ATTACHMENT_DEPTH_STENCIL: {
						depth = true;
						break;
					}
					case ATTACHMENT_RESOLVE: {
						resolve = true;
						break;
					}
					case ATTACHMENT_ALL: {
						if (data.attachments[usage.index].resolveColor && data.attachments[usage.index].sampleCount != VK_SAMPLE_COUNT_1_BIT) {
							resolve = true;
						} else {
							color = true;
						}
						break;
					}
				}
			}
			if (depth && !color && !resolve) {
				dep.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			} else if (color || resolve) {
				dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			}
			dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			data.subpassDependencies.Append(dep);
		}
		if (data.subpasses.size > 2) {
			cout.PrintLn("Warning: Automatic subpass dependencies are currently not implemented.");
		}
		for (i32 i = 1; i < data.subpasses.size-1; i++) {
			// VkSubpassDependency dep;
			// dep.srcSubpass = i-1;
			// dep.dstSubpass = i;
			// TODO: Finish inter-subpass dependencies
			AzAssert(false, "Unimplemented for multiple subpasses");
		}
		if (finalTransition) {
			VkSubpassDependency dep;
			dep.srcSubpass = data.subpasses.size-1; // Our last subpass
			dep.dstSubpass = VK_SUBPASS_EXTERNAL; // Transition for use outside our RenderPass
			dep.srcStageMask = finalAccessStage; // Make sure the image is done being used
			dep.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			bool depth = false;
			bool color = false;
			bool resolve = false;
			for (AttachmentUsage& usage : data.subpasses[0].data.attachments) {
				switch (usage.type) {
					case ATTACHMENT_COLOR: {
						color = true;
						break;
					}
					case ATTACHMENT_DEPTH_STENCIL: {
						depth = true;
						break;
					}
					case ATTACHMENT_RESOLVE: {
						resolve = true;
						break;
					}
					case ATTACHMENT_ALL: {
						if (data.attachments[usage.index].resolveColor && data.attachments[usage.index].sampleCount != VK_SAMPLE_COUNT_1_BIT) {
							resolve = true;
						} else {
							color = true;
						}
						break;
					}
				}
			}
			if (depth && !color && !resolve) {
				dep.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			} else if (color || resolve) {
				dep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			}
			dep.dstAccessMask = finalAccess;
			dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			data.subpassDependencies.Append(dep);
		}
		// Finally put it all together
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = data.attachmentDescriptions.size;
		renderPassInfo.pAttachments = data.attachmentDescriptions.data;
		renderPassInfo.subpassCount = data.subpassDescriptions.size;
		renderPassInfo.pSubpasses = data.subpassDescriptions.data;
		renderPassInfo.subpassCount = data.subpassDescriptions.size;
		renderPassInfo.dependencyCount = data.subpassDependencies.size;
		renderPassInfo.pDependencies = data.subpassDependencies.data;

		VkResult result = vkCreateRenderPass(data.device->data.device, &renderPassInfo, nullptr, &data.renderPass);

		if (result != VK_SUCCESS) {
			error = "Failed to create RenderPass: " + ErrorString(result);
			return false;
		}

		SetDebugMarker(data.device, data.debugMarker, VK_OBJECT_TYPE_RENDER_PASS, (u64)data.renderPass);

		data.initted = true;
		return true;
	}

	bool RenderPass::Deinit() {
		PrintDashed("Destroying RenderPass");
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (!data.initted) {
			// error = "RenderPass hasn't been initialized yet!";
			return false;
		}
#endif

		vkDestroyRenderPass(data.device->data.device, data.renderPass, nullptr);
		data.initted = false;
		return true;
	}

	Framebuffer::~Framebuffer() {
		if (data.initted) {
			if (!Deinit()) {
				cout.PrintLn("Failed to clean up vk::Framebuffer: ", error);
			}
		}
		if (depthMemory != nullptr) {
			delete depthMemory;
		}
		if (colorMemory != nullptr) {
			delete colorMemory;
		}
	}

	bool Framebuffer::Init(Device *device, String debugMarker) {
		PrintDashed("Initializing Framebuffer");
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.initted) {
			error = "Framebuffer is already initialized!";
			return false;
		}
		if (device == nullptr) {
			error = "Device is nullptr!";
			return false;
		}
		if (renderPass == nullptr) {
			error = "RenderPass is nullptr!";
			return false;
		}
		if (!renderPass->data.initted) {
			error = "RenderPass is not initialized!";
			return false;
		}
#endif
		data.device = device;
		data.debugMarker = std::move(debugMarker);
		bool depth = false, color = false;
		for (auto& attachment : renderPass->data.attachments) {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
			if (attachment.swapchain.Valid() && swapchain.Valid()) {
				if (attachment.swapchain != swapchain) {
					if (attachment.swapchain->data.surfaceFormat.format != swapchain->data.surfaceFormat.format) {
						error = "Framebuffer swapchain differing from RenderPass swapchain: Surface formats do not match!";
						return false;
					}
				}
			}
			if (attachment.swapchain != nullptr && swapchain == nullptr) {
				error = "RenderPass is connected to a swapchain, but this Framebuffer is not!";
				return false;
			}
#endif
			if (attachment.bufferDepthStencil) {
				depth = true;
			}
			if (attachment.bufferColor) {
				if (attachment.swapchain == nullptr
				|| (attachment.resolveColor && attachment.sampleCount != VK_SAMPLE_COUNT_1_BIT)) {
					color = true;
				}
			}
		}
		if (swapchain != nullptr) {
			numFramebuffers = swapchain->data.imageCount;
			width = swapchain->data.extent.width;
			height = swapchain->data.extent.height;
			bool found = false;
			for (auto& ptr : swapchain->data.framebuffers) {
				if (ptr == this) {
					found = true;
					break;
				}
			}
			if (!found) {
				swapchain->data.framebuffers.Append(Ptr<Framebuffer>(this));
			}
		}
		cout.PrintLn("Width: ", width, "  Height: ", height);
		if (ownMemory) {
			// Create Memory objects according to the RenderPass attachments
			if (depth && depthMemory == nullptr) {
				depthMemory = new Memory();
			}
			if (color && colorMemory == nullptr) {
				colorMemory = new Memory();
			}
		} else {
			// Verify that we have the memory we need
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
			bool failed = false;
			if (depth && depthMemory == nullptr) {
				error = "Framebuffer set to use outside memory, but none is given for depth ";
				failed = true;
			}
			if (color && colorMemory == nullptr) {
				if (!failed) {
					error = "Framebuffer set to use outside memory, but none is given f";
				}
				error = error + "or color ";
				failed = true;
			}
			if (failed) {
				error.Back() = '!';
				return false;
			}
#endif
		}
		if (ownImages) {
			if (width == 0 || height == 0) {
				error = "Framebuffer can't create images with size (" + ToString(width) + ", " + ToString(height) + ")!";
				return false;
			}
			Array<Ptr<Image>> ourImages{};
			// Create Images according to the RenderPass attachments
			u32 i = 0;
			for (auto& attachment : renderPass->data.attachmentDescriptions) {
				Image image;
				image.width = width;
				image.height = height;
				image.format = attachment.format;
				image.samples = attachment.samples;
				if (attachment.finalLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
					cout.PrintLn("Adding color image ", i, " to colorMemory.");
					image.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
					image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
					ourImages.Append(colorMemory->AddImage(image));
				} else if (attachment.finalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
					cout.PrintLn("Adding depth image ", i, " to depthMemory.");
					image.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
					image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
					ourImages.Append(depthMemory->AddImage(image));
				} else if (attachment.finalLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
					cout.PrintLn("Adding null image ", i, " for replacement with swapchain images.");
					ourImages.Append(Ptr<Image>());
				}
				i++;
			}
			// Now we need to make sure we know which images are also being used as input attachments
			for (auto& subpass : renderPass->data.subpasses) {
				for (auto& input : subpass.data.referencesInput) {
					ourImages[input.attachment]->usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
				}
			}
			// Now add ourImages to each index of attachmentImages
			attachmentImages.Resize(numFramebuffers, ourImages);
			// Now replace any null Ptr's with the actual swapchain images
			for (i32 i = 0; i < numFramebuffers; i++) {
				bool once = false;
				for (auto& ptr : attachmentImages[i]) {
					if (ptr.ptr == nullptr) {
						if (once) {
							error = "You can't have multiple swapchain images in a single framebuffer!";
							return false;
						}
						once = true;
						ptr.Set(&swapchain->data.images, i);
					}
				}
			}
		} else {
			// Verify the images are set up correctly
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
			if (attachmentImages.size != numFramebuffers) {
				error = "attachmentImages must have the size numFramebuffers.";
				return false;
			}
			for (i32 i = 0; i < numFramebuffers; i++) {
				if (attachmentImages[i].size == 0) {
					error = "Framebuffer has no attachment images!";
					return false;
				}
				if (attachmentImages[i].size != renderPass->data.attachmentDescriptions.size) {
					error = "RenderPass expects " + ToString(renderPass->data.attachmentDescriptions.size)
					+ " attachment images but this framebuffer only has " + ToString(attachmentImages.size) + ".";
					return false;
				}
				if (i >= 1) {
					if (attachmentImages[i-1].size != attachmentImages[i].size) {
						error = "attachmentImages[" + ToString(i-1) + "].size is "
							  + ToString(attachmentImages[i-1].size)
							  + " while attachmentImages[" + ToString(i) + "].size is "
							  + ToString(attachmentImages[i].size)
							  + ". These must be equal across every framebuffer!";
						return false;
					}
				}
				for (i32 j = 0; j < attachmentImages[i].size; j++) {
					if (attachmentImages[i][j]->width != width || attachmentImages[i][j]->height != height) {
						error = "All attached images must be the same width and height!";
						return false;
					}
				}
			}
#endif
		}
		data.initted = true;
		return true;
	}

	bool Framebuffer::Create() {
		PrintDashed("Creating Framebuffer");
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.created) {
			error = "Framebuffer already exists!";
			return false;
		}
#endif
		if (ownMemory) {
			if (depthMemory != nullptr) {
				if (data.debugMarker.size == 0) {
					 if (!depthMemory->Init(data.device)) {
						 return false;
					 }
				 } else {
					 if (!depthMemory->Init(data.device, data.debugMarker + ".depthMemory")) {
						 return false;
					 }
				 }
			}
			if (colorMemory != nullptr) {
				if (data.debugMarker.size == 0) {
					 if (!colorMemory->Init(data.device)) {
						 return false;
					 }
				 } else {
					 if (!colorMemory->Init(data.device, data.debugMarker + ".colorMemory")) {
						 return false;
					 }
				 }
			}
		}
		cout.PrintLn("Making ", numFramebuffers, " total framebuffers.");
		data.framebuffers.Resize(numFramebuffers);
		if (data.debugMarker.size != 0) {
			data.debugMarkers.Resize(numFramebuffers);
		}
		for (i32 fb = 0; fb < numFramebuffers; fb++) {
			Array<VkImageView> attachments(attachmentImages[0].size);
			for (i32 i = 0; i < attachmentImages[fb].size; i++) {
				if (!attachmentImages[fb][i]->data.imageViewExists) {
					error = "Framebuffer attachment " + ToString(i) + "'s image view has not been created!";
					return false;
				}
				attachments[i] = attachmentImages[fb][i]->data.imageView;
			}
			VkFramebufferCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			createInfo.renderPass = renderPass->data.renderPass;
			createInfo.width = width;
			createInfo.height = height;
			createInfo.layers = 1;
			createInfo.attachmentCount = attachments.size;
			createInfo.pAttachments = attachments.data;

			VkResult result = vkCreateFramebuffer(data.device->data.device, &createInfo, nullptr, &data.framebuffers[fb]);

			if (result != VK_SUCCESS) {
				for (fb--; fb >= 0; fb--) {
					vkDestroyFramebuffer(data.device->data.device, data.framebuffers[fb], nullptr);
				}
				error = "Failed to create framebuffers[" + ToString(fb) + "]: " + ErrorString(result);
				return false;
			}
			if (data.debugMarker.size != 0) {
				data.debugMarkers[fb] = data.debugMarker + ".framebuffers[" + ToString(fb) + "]";
				SetDebugMarker(data.device, data.debugMarkers[fb], VK_OBJECT_TYPE_FRAMEBUFFER, (u64)data.framebuffers[fb]);
			}
		}
		data.created = true;
		return true;
	}

	void Framebuffer::Destroy() {
		PrintDashed("Destroying Framebuffer");
		if (data.created) {
			for (i32 i = 0; i < data.framebuffers.size; i++) {
				vkDestroyFramebuffer(data.device->data.device, data.framebuffers[i], nullptr);
			}
			data.created = false;
		}
	}

	bool Framebuffer::Deinit() {
		PrintDashed("De-Initializing Framebuffer");
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (!data.initted) {
			// error = "Framebuffer has not been initialized!";
			return false;
		}
#endif
		if (ownMemory) {
			if (depthMemory != nullptr) {
				if (!depthMemory->Deinit()) {
					return false;
				}
				depthMemory->data.images.Resize(0);
			}
			if (colorMemory != nullptr) {
				if (!colorMemory->Deinit()) {
					return false;
				}
				colorMemory->data.images.Resize(0);
			}
		}
		Destroy();
		data.initted = false;
		return true;
	}
	
	bool Framebuffer::Recreate() {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (!data.created) {
			error = "Framebuffer::Recreate() called when framebuffer hasn't been created yet.";
			return false;
		}
#endif
		if (!Deinit()) return false;
		if (!Init(data.device, data.debugMarker)) return false;
		if (!Create()) return false;
		return true;
	}

	bool Fence::Create(Device *dev, String debugMarker) {
		PrintDashed("Creating Fence");
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.created) {
			error = "Fence already exists!";
			return false;
		}
#endif
		data.device = dev;
		data.debugMarker = debugMarker;
		
		VkFenceCreateInfo info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
		if (startSignaled) {
			info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		}
		VkResult result = vkCreateFence(data.device->data.device, &info, nullptr, &data.fence);
		if (result != VK_SUCCESS) {
			error = "Failed to create Fence: " + ErrorString(result);
			return false;
		}
		SetDebugMarker(data.device, data.debugMarker, VK_OBJECT_TYPE_FENCE, (u64)data.fence);
		data.created = true;
		return true;
	}

	VkResult Fence::GetStatus() {
		return vkGetFenceStatus(data.device->data.device, data.fence);
	}

	bool Fence::ResetSignaled() {
		if (GetStatus() == VK_NOT_READY) {
			return true;
		}
		VkResult result = vkResetFences(data.device->data.device, 1, &data.fence);
		if (result != VK_SUCCESS) {
			error = "vkResetFences failed with " + ErrorString(result);
			return false;
		}
		return true;
	}

	bool Fence::WaitForSignal(u64 timeout, bool *dstWasTimeout) {
		VkResult result = vkWaitForFences(data.device->data.device, 1, &data.fence, VK_TRUE, timeout);
		bool wasTimeout;
		if (result == VK_SUCCESS) {
			wasTimeout = false;
		} else if (result == VK_TIMEOUT) {
			wasTimeout = true;
		} else {
			error = "vkWaitForFences failed with " + ErrorString(result);
			return false;
		}
		if (dstWasTimeout) *dstWasTimeout = wasTimeout;
		return true;
	}

	void Fence::Destroy() {
		PrintDashed("Destroying Fence");
		vkDestroyFence(data.device->data.device, data.fence, nullptr);
		data.created = false;
	}

	bool Shader::Init(Device *device, String debugMarker) {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.initted) {
			error = "Shader is already initialized!";
			return false;
		}
		if (device == nullptr) {
			error = "Device is nullptr!";
			return false;
		}
#endif
		data.device = device;
		data.debugMarker = std::move(debugMarker);
		// Load in the code
		std::ifstream file(filename.data, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			error = "Failed to load shader file: \"" + filename + "\"";
			return false;
		}

		size_t fileSize = (size_t) file.tellg();
		data.code.Resize(intDivCeil(fileSize, (size_t)4));

		file.seekg(0);
		file.read((char*)data.code.data, fileSize);
		file.close();

		// Now create the shader module
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = fileSize;
		createInfo.pCode = data.code.data;

		VkResult result = vkCreateShaderModule(data.device->data.device, &createInfo, nullptr, &data.module);

		SetDebugMarker(data.device, data.debugMarker, VK_OBJECT_TYPE_SHADER_MODULE, (u64)data.module);

		if (result != VK_SUCCESS) {
			error = "Failed to create shader module: " + ErrorString(result);
			return false;
		}
		data.initted = true;
		return true;
	}

	void Shader::Clean() {
		if (data.initted) {
			data.code.Resize(0);
			vkDestroyShaderModule(data.device->data.device, data.module, nullptr);
			data.initted = false;
		}
	}

	ShaderRef::ShaderRef(String fn) : shader() , stage() , functionName(fn) {}

	ShaderRef::ShaderRef(Ptr<Shader> ptr, VkShaderStageFlagBits s, String fn) : shader(ptr) , stage(s) , functionName(fn) {}

	void Pipeline::Bind(VkCommandBuffer commandBuffer) const {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data.pipeline);
	}

	Pipeline::Pipeline() {
		// Time for a WHOLE LOTTA ASSUMPTIONS
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		data.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0;
		rasterizer.depthBiasClamp = 0.0;
		rasterizer.depthBiasSlopeFactor = 0.0;

		data.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		data.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		data.multisampling.minSampleShading = 1.0;
		data.multisampling.pSampleMask = nullptr;
		data.multisampling.alphaToCoverageEnable = VK_FALSE;
		data.multisampling.alphaToOneEnable = VK_FALSE;

		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_FALSE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0;
		depthStencil.maxDepthBounds = 1.0;
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {};
		depthStencil.back = {};

		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.blendConstants[0] = 0.0;
		colorBlending.blendConstants[1] = 0.0;
		colorBlending.blendConstants[2] = 0.0;
		colorBlending.blendConstants[3] = 0.0;
	}

	Pipeline::~Pipeline() {
		if (data.initted) {
			if (!Deinit()) {
				cout.PrintLn("Failed to clean up pipeline: ", error);
			}
		}
	}

	bool Pipeline::Init(Device *device, String debugMarker) {
		PrintDashed("Initializing Pipeline");
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.initted) {
			error = "Pipeline is already initialized!";
			return false;
		}
		if (device == nullptr) {
			error = "Device is nullptr!";
			return false;
		}
		if (renderPass == nullptr) {
			error = "Pipeline needs a renderPass!";
			return false;
		}
		if (!renderPass->data.initted) {
			error = "RenderPass isn't initialized!";
			return false;
		}
		if (subpass >= renderPass->data.subpasses.size) {
			error = "subpass[" + ToString(subpass) + "] is out of bounds for the RenderPass which only has " + ToString(renderPass->data.subpasses.size) + " subpasses!";
			return false;
		}
		if (renderPass->data.subpasses[subpass].data.referencesColor.size != colorBlendAttachments.size) {
			error = "You must have one colorBlendAttachment per color attachment in the associated Subpass!\nThe subpass has " + ToString(renderPass->data.subpasses[subpass].data.referencesColor.size) + " color attachments while the pipeline has " + ToString(colorBlendAttachments.size) + " colorBlendAttachments.";
			return false;
		}
#endif
		data.device = device;
		data.debugMarker = std::move(debugMarker);
		// Inherit multisampling information from renderPass
		for (auto& attachment : renderPass->data.subpasses[subpass].data.attachments) {
			if (attachment.accessFlags &
				(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) {
				data.multisampling.rasterizationSamples = renderPass->data.attachments[attachment.index].sampleCount;
				break;
			}
		}
		// First we grab our shaders
		Array<VkPipelineShaderStageCreateInfo> shaderStages(shaders.size);
		for (i32 i = 0; i < shaders.size; i++) {
			switch (shaders[i].stage) {
			case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
				cout.Print("Tesselation Control ");
				break;
			case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
				cout.Print("Tesselation Evaluation ");
				break;
			case VK_SHADER_STAGE_GEOMETRY_BIT:
				cout.Print("Geometry ");
				break;
			case VK_SHADER_STAGE_VERTEX_BIT:
				cout.Print("Vertex ");
				break;
			case VK_SHADER_STAGE_FRAGMENT_BIT:
				cout.Print("Fragment ");
				break;
			default:
				break;
			}
			cout.PrintLn("Shader: \"", shaders[i].shader->filename
				, "\" accessing function \"", shaders[i].functionName, "\"");
			shaderStages[i] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
			shaderStages[i].stage = shaders[i].stage;
			shaderStages[i].module = shaders[i].shader->data.module;
			shaderStages[i].pName = shaders[i].functionName.data;
		}
		// Then we set up our vertex buffers
		data.vertexInputInfo.vertexBindingDescriptionCount = inputBindingDescriptions.size;
		data.vertexInputInfo.pVertexBindingDescriptions = inputBindingDescriptions.data;
		data.vertexInputInfo.vertexAttributeDescriptionCount = inputAttributeDescriptions.size;
		data.vertexInputInfo.pVertexAttributeDescriptions = inputAttributeDescriptions.data;
		// Viewport
		VkViewport viewport = {};
		viewport.x = 0.0;
		viewport.y = 0.0;
		viewport.width = (float) 640;
		viewport.height = (float) 480;
		viewport.minDepth = 0.0;
		viewport.maxDepth = 1.0;

		VkRect2D scissor = {};
		scissor.offset = {0, 0};
		scissor.extent = {640, 480};

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
		// Color blending
		colorBlending.attachmentCount = colorBlendAttachments.size;
		colorBlending.pAttachments = colorBlendAttachments.data;
		// Dynamic states
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = dynamicStates.size;
		dynamicState.pDynamicStates = dynamicStates.data;
		// Pipeline layout
		Array<VkDescriptorSetLayout> descriptorSetLayouts(descriptorLayouts.size);
		for (i32 i = 0; i < descriptorLayouts.size; i++) {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
			if (!descriptorLayouts[i]->data.exists) {
				error = "descriptorLayouts[" + ToString(i) + "] doesn't exist!";
				return false;
			}
#endif
			descriptorSetLayouts[i] = descriptorLayouts[i]->data.layout;
		}
		// TODO: Separate out pipeline layouts
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size;
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data;
		pipelineLayoutInfo.pushConstantRangeCount = pushConstantRanges.size;
		pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data;

		VkResult result;
		result = vkCreatePipelineLayout(data.device->data.device, &pipelineLayoutInfo, nullptr, &data.layout);
		if (result != VK_SUCCESS) {
			error = "Failed to create pipeline layout: " + ErrorString(result);
			return false;
		}

		if (data.debugMarker.size != 0) {
			VkDebugUtilsObjectNameInfoEXT nameInfo = {};
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			nameInfo.objectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT;
			nameInfo.objectHandle = (u64)data.layout;
			nameInfo.pObjectName = data.debugMarker.data;
			data.device->data.instance->data.fpSetDebugUtilsObjectNameEXT(data.device->data.device, &nameInfo);
		}

		// Pipeline time!
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = shaderStages.size;
		pipelineInfo.pStages = shaderStages.data;
		pipelineInfo.pVertexInputState = &data.vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &data.multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		if (dynamicStates.size != 0) {
			pipelineInfo.pDynamicState = &dynamicState;
		}
		pipelineInfo.layout = data.layout;
		pipelineInfo.renderPass = renderPass->data.renderPass;
		pipelineInfo.subpass = subpass;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		result = vkCreateGraphicsPipelines(data.device->data.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &data.pipeline);
		if (result != VK_SUCCESS) {
			error = "Failed to create pipeline: " + ErrorString(result);
			vkDestroyPipelineLayout(data.device->data.device, data.layout, nullptr);
			return false;
		}

		SetDebugMarker(data.device.RawPtr(), data.debugMarker, VK_OBJECT_TYPE_PIPELINE, (u64)data.pipeline);

		data.initted = true;
		return true;
	}

	bool Pipeline::Deinit() {
		PrintDashed("Destroying Pipeline");
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (!data.initted) {
			// error = "Pipeline is not initted!";
			return false;
		}
#endif
		vkDestroyPipeline(data.device->data.device, data.pipeline, nullptr);
		vkDestroyPipelineLayout(data.device->data.device, data.layout, nullptr);
		data.initted = false;
		return true;
	}

	VkCommandBuffer CommandBuffer::Begin() {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.commandBuffer == VK_NULL_HANDLE) {
			error = "CommandBuffer doesn't exist!";
			return VK_NULL_HANDLE;
		}
		if (data.recording) {
			error = "Cannot begin a CommandBuffer that's already recording!";
			return VK_NULL_HANDLE;
		}
#endif
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		VkCommandBufferInheritanceInfo inheritanceInfo = {};

		if (secondary) {
			inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			if (renderPass.Valid()) {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
				if (!renderPass->data.initted) {
					error = "Associated RenderPass is not initialized!";
					return VK_NULL_HANDLE;
				}
#endif
				inheritanceInfo.renderPass = renderPass->data.renderPass;
				inheritanceInfo.subpass = subpass;
			}
			if (framebuffer.Valid()) {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
				if (!framebuffer->data.initted) {
					error = "Associated Framebuffer is not initialized!";
					return VK_NULL_HANDLE;
				}
#endif
				inheritanceInfo.framebuffer = framebuffer->data.framebuffers[framebuffer->data.currentFramebuffer];
			}
			if (occlusionQueryEnable) {
				inheritanceInfo.occlusionQueryEnable = VK_TRUE;
				inheritanceInfo.queryFlags = queryControlFlags;
				inheritanceInfo.pipelineStatistics = queryPipelineStatisticFlags;
			}

			beginInfo.pInheritanceInfo = &inheritanceInfo;

			if (renderPassContinue) {
				beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
			}
		}

		if (oneTimeSubmit) {
			beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		}
		if (simultaneousUse) {
			beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		}

		VkResult result = vkBeginCommandBuffer(data.commandBuffer, &beginInfo);

		if (result != VK_SUCCESS) {
			error = "Failed to begin CommandBuffer: " + ErrorString(result);
			return VK_NULL_HANDLE;
		}

		data.recording = true;
		return data.commandBuffer;
	}

	bool CommandBuffer::End() {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (!data.recording) {
			error = "Can't end a CommandBuffer that's not recording!";
			return false;
		}
#endif

		VkResult result = vkEndCommandBuffer(data.commandBuffer);

		if (result != VK_SUCCESS) {
			error = "Failed to end CommandBuffer: " + ErrorString(result);
			return false;
		}
		data.recording = false;
		return true;
	}

	bool CommandBuffer::Reset() {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (!data.pool->resettable) {
			error = "This CommandBuffer was not allocated from a CommandPool that's resettable!";
			return false;
		}
		if (data.recording) {
			error = "Can't reset a CommandBuffer while recording!";
			return false;
		}
#endif
		VkCommandBufferResetFlags resetFlags = {};
		if (releaseResourcesOnReset) {
			resetFlags = VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT;
		}
		VkResult result = vkResetCommandBuffer(data.commandBuffer, resetFlags);

		if (result != VK_SUCCESS) {
			error = "Failed to reset CommandBuffer: " + ErrorString(result);
			return false;
		}
		return true;
	}

	CommandPool::CommandPool(Ptr<Queue> q) : queue(q) {}

	CommandPool::~CommandPool() {
		Clean();
	}

	Ptr<CommandBuffer> CommandPool::AddCommandBuffer() {
		data.commandBuffers.Append(CommandBuffer());
		return Ptr<CommandBuffer>(&data.commandBuffers, data.commandBuffers.size-1);
	}

	Ptr<CommandBuffer> CommandPool::CreateDynamicBuffer(bool secondary) {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (!data.initted) {
			error = "Command Pool is not initialized!";
			return nullptr;
		}
#endif
		data.dynamicBuffers.Append(CommandBuffer());
		CommandBuffer *buffer = &data.dynamicBuffers.Back();
		buffer->secondary = secondary;
		buffer->data.pool = this;
		buffer->data.device = data.device;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = data.commandPool;
		allocInfo.commandBufferCount = 1;
		allocInfo.level = secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VkResult result = vkAllocateCommandBuffers(data.device->data.device, &allocInfo, &buffer->data.commandBuffer);
		if (result != VK_SUCCESS) {
			error = "Failed to allocate dynamic command buffer: " + ErrorString(result);
			return nullptr;
		}
		return buffer;
	}

	void CommandPool::DestroyDynamicBuffer(CommandBuffer *buffer) {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (!data.initted) {
			return;
		}
#endif
		vkFreeCommandBuffers(data.device->data.device, data.commandPool, 1, &buffer->data.commandBuffer);
		for (ListIndex<CommandBuffer>& b : data.dynamicBuffers) {
			if (b.next && &b.next->value == buffer) {
				b.EraseNext();
				break;
			}
		}
	}

	bool CommandPool::Init(Device *device, String debugMarker) {
		PrintDashed("Initializing Command Pool");
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.initted) {
			error = "CommandPool is already initialized!";
			return false;
		}
		if (device == nullptr) {
			error = "Device is nullptr!";
			return false;
		}
		if (queue == nullptr) {
			error = "You must specify a queue to create a CommandPool!";
			return false;
		}
#endif
		data.device = device;
		data.debugMarker = std::move(debugMarker);
		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = queue->queueFamilyIndex;
		if (transient) {
			createInfo.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		}
		if (resettable) {
			createInfo.flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		}
		if (protectedMemory) {
			createInfo.flags |= VK_COMMAND_POOL_CREATE_PROTECTED_BIT;
		}

		VkResult result = vkCreateCommandPool(data.device->data.device, &createInfo, nullptr, &data.commandPool);

		if (result != VK_SUCCESS) {
			error = "Failed to create CommandPool: " + ErrorString(result);
			return false;
		}

		SetDebugMarker(data.device, data.debugMarker, VK_OBJECT_TYPE_COMMAND_POOL, (u64)data.commandPool);

		// Now the command buffers
		u32 p = 0, s = 0;
		Array<VkCommandBuffer> primaryBuffers;
		Array<VkCommandBuffer> secondaryBuffers;
		for (CommandBuffer& buffer : data.commandBuffers) {
			if (buffer.secondary) {
				s++;
			} else {
				p++;
			}
		}
		cout.PrintLn("Allocating ", p, " primary command buffers and ", s, " secondary command buffers.");
		primaryBuffers.Resize(p);
		secondaryBuffers.Resize(s);
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = data.commandPool;
		if (p != 0) {
			allocInfo.commandBufferCount = p;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

			VkResult result = vkAllocateCommandBuffers(data.device->data.device, &allocInfo, primaryBuffers.data);
			if (result != VK_SUCCESS) {
				error = "Failed to allocate primary command buffers: " + ErrorString(result);
				vkDestroyCommandPool(data.device->data.device, data.commandPool, nullptr);
				return false;
			}
		}
		if (s != 0) {
			allocInfo.commandBufferCount = s;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

			VkResult result = vkAllocateCommandBuffers(data.device->data.device, &allocInfo, secondaryBuffers.data);
			if (result != VK_SUCCESS) {
				error = "Failed to allocate secondary command buffers: " + ErrorString(result);
				vkDestroyCommandPool(data.device->data.device, data.commandPool, nullptr);
				return false;
			}
		}
		p = 0; s = 0;
		i32 index = 0;
		for (CommandBuffer& buffer : data.commandBuffers) {
			if (buffer.secondary) {
				buffer.data.commandBuffer = secondaryBuffers[s++];
			} else {
				buffer.data.commandBuffer = primaryBuffers[p++];
			}
			buffer.data.device = data.device;
			buffer.data.pool = this;
			if (data.debugMarker.size != 0) {
				buffer.data.debugMarker = data.debugMarker + ".commandBuffer[" + ToString(index) + "]";
			}
			SetDebugMarker(data.device, buffer.data.debugMarker, VK_OBJECT_TYPE_COMMAND_BUFFER, (u64)buffer.data.commandBuffer);
		}
		data.initted = true;
		return true;
	}

	void CommandPool::Clean() {
		if (data.initted) {
			PrintDashed("Destroying Command Pool");
			vkDestroyCommandPool(data.device->data.device, data.commandPool, nullptr);
			data.initted = false;
		}
	}

	VkResult Swapchain::AcquireNextImage() {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (!data.initted) {
			error = "Swapchain is not initialized!";
			return VK_ERROR_INITIALIZATION_FAILED;
		}
		if (!data.created) {
			error = "Swapchain does not exist!";
			return VK_ERROR_OUT_OF_DATE_KHR;
		}
#endif
		data.buffer = !data.buffer;
		VkFence fence;
		if (data.fences[data.buffer].Valid()) {
			if (!data.fences[data.buffer]->ResetSignaled()) {
				// return the only error code that can be sent by vkResetFences
				return VK_ERROR_OUT_OF_DEVICE_MEMORY;
			}
			fence = data.fences[data.buffer]->data.fence;
		} else {
			fence = VK_NULL_HANDLE;
		}
		VkResult result = vkAcquireNextImageKHR(data.device->data.device, data.swapchain, timeout, data.semaphores[data.buffer]->semaphore, fence, &data.currentImage);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_TIMEOUT || result == VK_NOT_READY) {
			// Don't update framebuffers
			data.buffer = !data.buffer;
			return result;
		} else if (result != VK_SUCCESS) {
			error = "Failed to acquire swapchain image: " + ErrorString(result);
			return result;
		}
		for (Ptr<Framebuffer>& framebuffer : data.framebuffers) {
			framebuffer->data.currentFramebuffer = data.currentImage;
		}
		return result;
	}

	Ptr<Semaphore> Swapchain::SemaphoreImageAvailable() {
		return data.semaphores[data.buffer];
	}

	Ptr<Fence> Swapchain::FenceImageAvailable() {
		return data.fences[data.buffer];
	}

	bool Swapchain::Present(Ptr<Queue> queue, Array<VkSemaphore> waitSemaphores) {
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = waitSemaphores.size;
		presentInfo.pWaitSemaphores = waitSemaphores.data;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &data.swapchain;
		presentInfo.pImageIndices = &data.currentImage;

		VkResult result = vkQueuePresentKHR(queue->queue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			cout.PrintLn("Swapchain::Present Warning: vkQueuePresentKHR returned ", ErrorString(result));
		} else if (result != VK_SUCCESS) {
			error = "Failed to present swapchain image: " + ErrorString(result);
			return false;
		}
		return true;
	}
	
	void Swapchain:: UpdateSurfaceCapabilities() {
		vkDeviceWaitIdle(data.device->data.device);
		VkPhysicalDevice physicalDevice = data.device->data.physicalDevice.physicalDevice;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, data.surface, &data.surfaceCapabilities);
	}

	bool Swapchain::Resize() {
		cout.Newline(2);
		PrintDashed("Resizing Swapchain");
		cout.Newline();
		UpdateSurfaceCapabilities();
		if (data.created) {
			if (!Create()) {
				return false;
			}
		}
		for (Ptr<Framebuffer> framebuffer : data.framebuffers) {
			if (framebuffer->data.created) {
				if (!framebuffer->Recreate()) return false;
			}
		}
		return true;
	}

	Swapchain::~Swapchain() {
		if (data.initted) {
			if (!Deinit()) {
				cout.PrintLn("Failed to clean up vk::Swapchain: ", error);
			}
		}
	}

	bool Swapchain::Init(Device *device, String debugMarker) {
		PrintDashed("Initializing Swapchain");
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.initted) {
			error = "Swapchain is already initialized!";
			return false;
		}
		if (device == nullptr) {
			error = "Device is nullptr!";
			return false;
		}
		if (!window.Valid()) {
			error = "Cannot create a swapchain without a window surface!";
			return false;
		}
#endif
		data.device = device;
		for (i32 i = 0; i < 2; i++) {
			if (!data.semaphores[i].Valid()) {
				data.semaphores[i] = data.device->AddSemaphore();
			}
		}
		if (useFences) {
			for (i32 i = 0; i < 2; i++) {
				if (!data.fences[i].Valid()) {
					data.fences[i] = data.device->AddFence();
				}
			}
		}
		data.surface = window->surface;
		data.debugMarker = std::move(debugMarker);
		// Get information about what we can or can't do
		VkPhysicalDevice physicalDevice = data.device->data.physicalDevice.physicalDevice;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, data.surface, &data.surfaceCapabilities);
		u32 count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, data.surface, &count, nullptr);
		if (count != 0) {
			data.surfaceFormats.Resize(count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, data.surface, &count, data.surfaceFormats.data);
		}
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, data.surface, &count, nullptr);
		if (count != 0) {
			data.presentModes.Resize(count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, data.surface, &count, data.presentModes.data);
		}
		// We'll probably re-create the swapchain a bunch of times without a full Deinit() Init() cycle
		if (!Create()) {
			return false;
		}
		data.initted = true;
		return true;
	}

	bool Swapchain::Create() {
		PrintDashed("Creating Swapchain");
		// Choose our surface format
		{
			bool found = false;
			if (data.surfaceFormats.size == 1 && data.surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
				data.surfaceFormat = formatPreferred;
				found = true;
			} else {
				for (const auto& format : data.surfaceFormats) {
					if (format.format == formatPreferred.format
					&& format.colorSpace == formatPreferred.colorSpace) {
						data.surfaceFormat = formatPreferred;
						found = true;
						break;
					}
				}
			}
			if (!found && data.surfaceFormats.size > 0) {
				cout.PrintLn("We couldn't use our preferred window surface format!");
				data.surfaceFormat = data.surfaceFormats[0];
				found = true;
			}
			if (!found) {
				error = "We don't have any surface formats to choose from!!! >:(";
				return false;
			}
		}
		// Choose our present mode
		{
			bool found = false;
			if (vsync) {
				for (const auto& mode : data.presentModes) {
					if (mode == VK_PRESENT_MODE_FIFO_KHR) {
						data.presentMode = mode;
						found = true;
						break;
					}
				}
			} else {
				for (const auto& mode : data.presentModes) {
					if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
						data.presentMode = mode;
						found = true;
						break; // Ideal choice, don't keep looking
					} else if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
						data.presentMode = mode;
						found = true;
						// Acceptable choice, but keep looking
					}
				}
			}
			if (!found && data.presentModes.size > 0) {
				cout.PrintLn("Our preferred present modes aren't available, but we can still do something");
				data.presentMode = data.presentModes[0];
				found = true;
			}
			if (!found) {
				error = "No adequate present modes available! ¯\\_(ツ)_/¯";
				return false;
			}
			cout.Print("Present Mode: ");
			switch(data.presentMode) {
				case VK_PRESENT_MODE_FIFO_KHR:
					cout.PrintLn("VK_PRESENT_MODE_FIFO_KHR");
					break;
				case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
					cout.PrintLn("VK_PRESENT_MODE_FIFO_RELAXED_KHR");
					break;
				case VK_PRESENT_MODE_MAILBOX_KHR:
					cout.PrintLn("VK_PRESENT_MODE_MAILBOX_KHR");
					break;
				case VK_PRESENT_MODE_IMMEDIATE_KHR:
					cout.PrintLn("VK_PRESENT_MODE_IMMEDIATE_KHR");
					break;
				default:
					cout.PrintLn("wtf the fuck");
					break;
			}
		}
		// Now we gotta find our extent
		if (data.surfaceCapabilities.currentExtent.width != UINT32_MAX) {
			data.extent = data.surfaceCapabilities.currentExtent;
		} else {
			data.extent = {(u32)window->surfaceWindow->width, (u32)window->surfaceWindow->height};

			data.extent.width = max(data.surfaceCapabilities.minImageExtent.width,
						   min(data.surfaceCapabilities.maxImageExtent.width, data.extent.width));
			data.extent.height = max(data.surfaceCapabilities.minImageExtent.height,
							min(data.surfaceCapabilities.maxImageExtent.height, data.extent.height));
		}

		cout.PrintLn("extent is {", data.extent.width, ", ", data.extent.height, "}");
		data.imageCount = max(data.surfaceCapabilities.minImageCount, min(data.surfaceCapabilities.maxImageCount, imageCountPreferred));
		cout.PrintLn("Swapchain will use ", data.imageCount, " images");
		// Put it all together
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = data.surface;
		createInfo.minImageCount = data.imageCount;
		createInfo.imageFormat = data.surfaceFormat.format;
		createInfo.imageColorSpace = data.surfaceFormat.colorSpace;
		createInfo.imageExtent = data.extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = usage;
		// TODO: Inherit imageUsage from ???

		// Queue family sharing...ugh
		Array<u32> queueFamilies{};
		for (auto &queue : data.device->data.queues) {
			bool found = false;
			for (i32 j = 0; j < queueFamilies.size; j++) {
				if (queue.value.queueFamilyIndex == (i32)queueFamilies[j]) {
					found = true;
					break;
				}
			}
			if (!found) {
				queueFamilies.Append(queue.value.queueFamilyIndex);
			}
		}
		if (queueFamilies.size > 1) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = queueFamilies.size;
			createInfo.pQueueFamilyIndices = queueFamilies.data;
			cout.PrintLn("Swapchain image sharing mode is concurrent");
		} else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
			cout.PrintLn("Swapchain image sharing mode is exclusive");
		}
		createInfo.preTransform = data.surfaceCapabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = data.presentMode;
		createInfo.clipped = VK_TRUE;

		if (data.created) {
			VkSwapchainKHR oldSwapchain = data.swapchain;
			createInfo.oldSwapchain = oldSwapchain;
		}

		VkSwapchainKHR newSwapchain;
		VkResult result = vkCreateSwapchainKHR(data.device->data.device, &createInfo, nullptr, &newSwapchain);
		if (result != VK_SUCCESS) {
			error = "Failed to create swap chain: " + ErrorString(result);
			return false;
		}
		if (data.created) {
			vkDestroySwapchainKHR(data.device->data.device, data.swapchain, nullptr);
		}
		data.swapchain = newSwapchain;

		SetDebugMarker(data.device, data.debugMarker, VK_OBJECT_TYPE_SWAPCHAIN_KHR, (u64)data.swapchain);

		cout.PrintLn("Acquiring images and creating image views...");

		// Get our images
		Array<VkImage> imagesTemp;
		vkGetSwapchainImagesKHR(data.device->data.device, data.swapchain, &data.imageCount, nullptr);
		imagesTemp.Resize(data.imageCount);
		vkGetSwapchainImagesKHR(data.device->data.device, data.swapchain, &data.imageCount, imagesTemp.data);

		data.images.Resize(data.imageCount);
		for (u32 i = 0; i < data.imageCount; i++) {
			data.images[i].Clean();
			if (data.debugMarker.size == 0) {
				data.images[i].Init(data.device);
			} else {
				data.images[i].Init(data.device, data.debugMarker + ".images[" + ToString(i) + "]");
			}
			data.images[i].data.image = imagesTemp[i];
			data.images[i].format = data.surfaceFormat.format;
			data.images[i].width = data.extent.width;
			data.images[i].height = data.extent.height;
			data.images[i].aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
			data.images[i].usage = usage;

			SetDebugMarker(data.device, data.images[i].data.debugMarker[0], VK_OBJECT_TYPE_IMAGE, (u64)data.images[i].data.image);

			if (!data.images[i].CreateImageView()) {
				return false;
			}
		}

		data.created = true;
		return true;
	}

	bool Swapchain::Reconfigure() {
		if (data.initted) {
			if (!Create())
				return false;
		}
		for (Ptr<Framebuffer> framebuffer : data.framebuffers) {
			if (framebuffer->data.created) {
				if (!framebuffer->Recreate()) return false;
			}
		}
		return true;
	}

	bool Swapchain::Deinit() {
		PrintDashed("Destroying Swapchain");
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (!data.initted) {
			// error = "Swapchain isn't initialized!";
			return false;
		}
#endif
		for (u32 i = 0; i < data.imageCount; i++) {
			data.images[i].Clean();
		}
		vkDestroySwapchainKHR(data.device->data.device, data.swapchain, nullptr);
		data.initted = false;
		data.created = false;
		return true;
	}

	SemaphoreWait::SemaphoreWait(Ptr<Semaphore> inSemaphore, VkPipelineStageFlags inDstStageMask) :
		semaphore(inSemaphore) , swapchain() , dstStageMask(inDstStageMask) {}

	SemaphoreWait::SemaphoreWait(Ptr<Swapchain> inSwapchain, VkPipelineStageFlags inDstStageMask) :
		semaphore() , swapchain(inSwapchain) , dstStageMask(inDstStageMask) {}

	bool QueueSubmission::Config() {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (commandBuffers.size == 0) {
			error = "You can't submit 0 command buffers!";
			return false;
		}
#endif
		data.commandBuffers.Resize(commandBuffers.size);
		for (i32 i = 0; i < commandBuffers.size; i++) {
			data.commandBuffers[i] = commandBuffers[i]->data.commandBuffer;
		}
		data.submitInfo.commandBufferCount = data.commandBuffers.size;
		data.submitInfo.pCommandBuffers = data.commandBuffers.data;

		data.waitSemaphores.Resize(waitSemaphores.size);
		data.waitDstStageMasks.Resize(waitSemaphores.size);
		for (i32 i = 0; i < waitSemaphores.size; i++) {
			if (waitSemaphores[i].semaphore.Valid()) {
				data.waitSemaphores[i] = waitSemaphores[i].semaphore->semaphore;
			} else {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
				if (!waitSemaphores[i].swapchain.Valid()) {
					error = "Neither the semaphore Ptr nor the swapchain Ptr are valid!";
					return false;
				}
#endif
				data.waitSemaphores[i] = waitSemaphores[i].swapchain->SemaphoreImageAvailable()->semaphore;
			}
			data.waitDstStageMasks[i] = waitSemaphores[i].dstStageMask;
		}
		data.submitInfo.waitSemaphoreCount = waitSemaphores.size;
		if (waitSemaphores.size != 0) {
			data.submitInfo.pWaitSemaphores = data.waitSemaphores.data;
			data.submitInfo.pWaitDstStageMask = data.waitDstStageMasks.data;
		}
		data.signalSemaphores.Resize(signalSemaphores.size);
		for (i32 i = 0; i < signalSemaphores.size; i++) {
			data.signalSemaphores[i] = signalSemaphores[i]->semaphore;
		}
		data.submitInfo.signalSemaphoreCount = data.signalSemaphores.size;
		data.submitInfo.pSignalSemaphores = data.signalSemaphores.data;

		return true;
	}

	Device::Device() {
		// type = LOGICAL_DEVICE;
	}

	Device::~Device() {
		if (data.initted) {
			if (!Deinit()) {
				cout.PrintLn("Failed to clean up vk::Device: ", error);
			}
		}
	}

	Ptr<Queue> Device::AddQueue() {
		data.queues.Append(Queue());
		return &data.queues.Back();
	}

	Ptr<Swapchain> Device::AddSwapchain() {
		data.swapchains.Append(Swapchain());
		return &data.swapchains.Back();
	}

	Ptr<RenderPass> Device::AddRenderPass() {
		data.renderPasses.Append(RenderPass());
		return &data.renderPasses.Back();
	}

	Ptr<Sampler> Device::AddSampler() {
		data.samplers.Append(Sampler());
		return Ptr<Sampler>(&data.samplers, data.samplers.size-1);
	}

	Ptr<Memory> Device::AddMemory() {
		data.memories.Append(Memory());
		return &data.memories.Back();
	}

	Ptr<Descriptors> Device::AddDescriptors() {
		data.descriptors.Append(Descriptors());
		return &data.descriptors.Back();
	}

	Ptr<Shader> Device::AddShader() {
		data.shaders.Append(Shader());
		return Ptr<Shader>(&data.shaders, data.shaders.size-1);
	}

	Range<Shader> Device::AddShaders(u32 count) {
		data.shaders.Resize(data.shaders.size+count);
		return Range<Shader>(&data.shaders, data.shaders.size-count, count);
	}

	Ptr<Pipeline> Device::AddPipeline() {
		data.pipelines.Append(Pipeline());
		return &data.pipelines.Back();
	}

	Ptr<CommandPool> Device::AddCommandPool(Ptr<Queue> queue) {
		data.commandPools.Append(CommandPool(queue));
		return &data.commandPools.Back();
	}

	Ptr<Framebuffer> Device::AddFramebuffer() {
		data.framebuffers.Append(Framebuffer());
		return &data.framebuffers.Back();
	}

	Ptr<Semaphore> Device::AddSemaphore() {
		data.semaphores.Append(Semaphore());
		return Ptr<Semaphore>(&data.semaphores, data.semaphores.size-1);
	}
	
	Ptr<Fence> Device::AddFence() {
		data.fences.Append(Fence());
		return Ptr<Fence>(&data.fences, data.fences.size-1);
	}

	Ptr<QueueSubmission> Device::AddQueueSubmission() {
		data.queueSubmissions.Append(QueueSubmission());
		return &data.queueSubmissions.Back();
	}

	bool Device::SubmitCommandBuffers(Ptr<Queue> queue, Array<Ptr<QueueSubmission>> submissions) {
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (!data.initted) {
			error = "Device not initialized!";
			return false;
		}
		if (queue->queue == VK_NULL_HANDLE) {
			error = "Queue is not valid!";
			return false;
		}
		if (submissions.size == 0) {
			error = "submissions is an empty array!";
			return false;
		}
#endif
		Array<VkSubmitInfo> submitInfos(submissions.size);
		for (i32 i = 0; i < submitInfos.size; i++) {
			submitInfos[i] = submissions[i]->data.submitInfo;
		}
		VkResult result = vkQueueSubmit(queue->queue, submitInfos.size, submitInfos.data, VK_NULL_HANDLE);

		if (result != VK_SUCCESS) {
			error = "Failed to submit " + ToString(submitInfos.size) + " submissions: " + ErrorString(result);
			return false;
		}
		return true;
	}

	bool Device::Init(Instance *instance, String debugMarker) {
		PrintDashed("Initializing Logical Device");
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.initted) {
			error = "Device is already initialized!";
			return false;
		}
		if (instance == nullptr) {
			error = "Instance is nullptr!";
			return false;
		}
#endif
		data.instance = instance;
		data.debugMarker = std::move(debugMarker);

		// Select physical device first based on needs.
		// TODO: Right now we just choose the first in the pre-sorted list. We should instead select
		//	   them based on whether they have our desired features.
		data.physicalDevice = data.instance->data.physicalDevices[0];
		
		// for (i32 i = 0; i < data.physicalDevice.extensionsAvailable.size; i++) {
		// 	cout.PrintLn(data.physicalDevice.extensionsAvailable[i].extensionName);
		// }

		// Put together all our needed extensions
		Array<const char*> extensionsAll(data.extensionsRequired);

		// TODO: Find out what extensions we need based on context
		if (!data.swapchains.Empty()) {
			extensionsAll.Append(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		}

		// Verify that our device extensions are available
		Array<const char*> extensionsUnavailable(extensionsAll);
		for (i32 i = 0; i < (i32)extensionsUnavailable.size; i++) {
			for (i32 j = 0; j < (i32)data.physicalDevice.extensionsAvailable.size; j++) {
				if (strcmp(extensionsUnavailable[i], data.physicalDevice.extensionsAvailable[j].extensionName) == 0) {
					extensionsUnavailable.Erase(i);
					i--;
					break;
				}
			}
		}
		if (extensionsUnavailable.size > 0) {
			error = "Device extensions unavailable:";
			for (const char *extension : extensionsUnavailable) {
				error += "\n\t";
				error += extension;
			}
			return false;
		}

		VkPhysicalDeviceFeatures2 deviceFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
		{
			// What features to we want?
			bool anisotropy = false;
			for (auto& sampler : data.samplers) {
				if (sampler.anisotropy != 1) {
					anisotropy = true;
					break;
				}
			}
			if (anisotropy) {
				cout.PrintLn("Enabling samplerAnisotropy optional device feature");
				data.deviceFeaturesOptional.features.samplerAnisotropy = VK_TRUE;
			}
			bool independentBlending = false;
			bool wideLines = false;
			for (auto& pipeline : data.pipelines) {
				for (i32 i = 1; i < pipeline.value.colorBlendAttachments.size; i++) {
					VkPipelineColorBlendAttachmentState& s1 = pipeline.value.colorBlendAttachments[i-1];
					VkPipelineColorBlendAttachmentState& s2 = pipeline.value.colorBlendAttachments[i];
					if (s1.blendEnable != s2.blendEnable
					||  s1.alphaBlendOp != s2.alphaBlendOp
					||  s1.colorBlendOp != s2.colorBlendOp
					||  s1.colorWriteMask != s2.colorWriteMask
					||  s1.dstAlphaBlendFactor != s2.dstAlphaBlendFactor
					||  s1.dstColorBlendFactor != s2.dstColorBlendFactor
					||  s1.srcAlphaBlendFactor != s2.srcAlphaBlendFactor
					||  s1.srcColorBlendFactor != s2.srcColorBlendFactor) {
						independentBlending = true;
						break;
					}
				}
				if (pipeline.value.rasterizer.lineWidth != 1.0f) {
					wideLines = true;
				}
			}
			if (independentBlending) {
				cout.PrintLn("Enabling independentBlend device feature");
				data.deviceFeaturesRequired.features.independentBlend = VK_TRUE;
			}
			if (wideLines) {
				cout.PrintLn("Enabling wideLines device feature");
				data.deviceFeaturesRequired.features.wideLines = VK_TRUE;
			}
			// Which ones are available?
			for (u32 i = 0; i < sizeof(VkPhysicalDeviceFeatures2::features)/4; i++) {
				// I'm not sure why these aren't bit-masked values,
				// but I'm treating it like that anyway.
				*(((u32*)&deviceFeatures.features + i)) = *(((u32*)&data.deviceFeaturesRequired.features + i))
				|| (*(((u32*)&data.physicalDevice.features.features + i)) && *(((u32*)&data.deviceFeaturesOptional.features + i)));
			}
			// Which ones don't we have that we wanted?
			if (anisotropy && deviceFeatures.features.samplerAnisotropy == VK_FALSE) {
				cout.PrintLn("Sampler Anisotropy desired, but unavailable...disabling.");
				for (auto& sampler : data.samplers) {
					sampler.anisotropy = 1;
				}
			}
		}


		// Set up queues
		// First figure out which queue families each queue should use
		const bool preferSameQueueFamilies = false;
		const bool preferMonolithicQueues = true;

		// Make sure we have enough queues in every family
		i32 queueFamilies = data.physicalDevice.queueFamiliesAvailable.size;
		Array<i32> queuesPerFamily(queueFamilies);
		for (i32 i = 0; i < queueFamilies; i++) {
			queuesPerFamily[i] = data.physicalDevice.queueFamiliesAvailable[i].queueFamilyProperties.queueCount;
		}

		for (auto &queueIndex : data.queues) {
			Queue &queue = queueIndex.value;
			for (i32 j = 0; j < queueFamilies; j++) {
				if (queuesPerFamily[j] == 0)
					continue; // This family has been exhausted of queues, try the next.
				VkQueueFamilyProperties2& props = data.physicalDevice.queueFamiliesAvailable[j];
				if (props.queueFamilyProperties.queueCount == 0)
					continue;
				VkBool32 presentSupport = VK_FALSE;
				for (const Window& w : data.instance->data.windows) {
					// TODO: Does this work in all cases or do we need to differentiate between windows?
					vkGetPhysicalDeviceSurfaceSupportKHR(data.physicalDevice.physicalDevice, j, w.surface, &presentSupport);
					if (presentSupport)
						break;
				}
				switch(queue.queueType) {
					case COMPUTE: {
						if (props.queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) {
							queue.queueFamilyIndex = j;
						}
						break;
					}
					case GRAPHICS: {
						if (props.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
							queue.queueFamilyIndex = j;
						}
						break;
					}
					case TRANSFER: {
						if (props.queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT) {
							queue.queueFamilyIndex = j;
						}
						break;
					}
					case PRESENT: {
						if (presentSupport) {
							queue.queueFamilyIndex = j;
						}
						break;
					}
					default: {
						error = "queues[" + ToString(data.queues.IndexOf(&queueIndex)) + "] has a QueueType of UNDEFINED!";
						return false;
					}
				}
				if (preferSameQueueFamilies && queue.queueFamilyIndex != -1)
					break;
			}
			if (queue.queueFamilyIndex == -1) {
				error = "queues[" + ToString(data.queues.IndexOf(&queueIndex)) + "] couldn't find a queue family :(";
				return false;
			}
			if (!preferMonolithicQueues) {
				queuesPerFamily[queue.queueFamilyIndex]--;
			}
		}

		Array<VkDeviceQueueCreateInfo> queueCreateInfos{};
		Array<BinarySet<f32>> queuePriorities(queueFamilies);
		for (i32 i = 0; i < queueFamilies; i++) {
			for (auto& queueIndex : data.queues) {
				Queue &queue = queueIndex.value;
				if (queue.queueFamilyIndex == (i32)i) {
					queuePriorities[i].Emplace(queue.queuePriority);
				}
			}
		}
		Array<Array<f32>> queuePrioritiesArray(queueFamilies);
		for (i32 i = 0; i < queueFamilies; i++) {
			for (const f32& p : queuePriorities[i]) {
				queuePrioritiesArray[i].Append(p);
			}
		}
		for (i32 i = 0; i < queueFamilies; i++) {
			cout.PrintLn("Allocating ", queuePrioritiesArray[i].size, " queues from family ", i);
			if (queuePrioritiesArray[i].size != 0) {
				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = i;
				queueCreateInfo.queueCount = queuePrioritiesArray[i].size;
				queueCreateInfo.pQueuePriorities = queuePrioritiesArray[i].data;
				queueCreateInfos.Append(queueCreateInfo);
			}
		}

		data.vk11FeaturesRequired.pNext = &data.vk12FeaturesRequired;
		deviceFeatures.pNext = &data.vk11FeaturesRequired;

		VkDeviceCreateInfo createInfo = {};
		createInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
		createInfo.pQueueCreateInfos = queueCreateInfos.data;
		createInfo.queueCreateInfoCount = queueCreateInfos.size;

		// To use VkPhysicalDeviceFeatures2 we pass it to pNext and null to pEnabledFeatures
		createInfo.pNext = &deviceFeatures;
		// createInfo.pEnabledFeatures = &deviceFeatures.features;
		createInfo.enabledExtensionCount = extensionsAll.size;
		createInfo.ppEnabledExtensionNames = extensionsAll.data;

		if (data.instance->data.enableLayers) {
			createInfo.enabledLayerCount = data.instance->data.layersRequired.size;
			createInfo.ppEnabledLayerNames = data.instance->data.layersRequired.data;
		} else {
			createInfo.enabledLayerCount = 0;
		}

		VkResult result = vkCreateDevice(data.physicalDevice.physicalDevice, &createInfo, nullptr, &data.device);
		if (result != VK_SUCCESS) {
			error = "Failed to create logical device: ";
			error += ErrorString(result);
			return false;
		}

		SetDebugMarker(this, data.debugMarker, VK_OBJECT_TYPE_DEVICE, (u64)data.device);

		// Get our queues
		i32 index = 0;
		for (i32 i = 0; i < queueFamilies; i++) {
			for (auto& queueIndex : data.queues) {
				Queue &queue = queueIndex.value;
				if (queue.queueFamilyIndex == (i32)i) {
					i32 queueIndex = 0;
					for (i32 p = 0; p < queuePrioritiesArray[i].size; p++) {
						if (queue.queuePriority == queuePrioritiesArray[i][p]) {
							queueIndex = p;
							break;
						}
					}
					vkGetDeviceQueue(data.device, i, queueIndex, &queue.queue);
					if (data.debugMarker.size != 0) {
						queue.debugMarker = data.debugMarker + ".queues[" + ToString(index) + "]";
						SetDebugMarker(this, queue.debugMarker, VK_OBJECT_TYPE_QUEUE, (u64)queue.queue);
						index++;
					}
				}
			}
		}
		// Swapchains
		index = 0;
		for (auto& swapchainIndex : data.swapchains) {
			Swapchain &swapchain = swapchainIndex.value;
			if (data.debugMarker.size == 0) {
				if (!swapchain.Init(this)) {
					goto failed;
				}
			} else {
				if (!swapchain.Init(this, data.debugMarker + ".swapchains[" + ToString(index) + "]")) {
					goto failed;
				}
				index++;
			}
		}
		// RenderPasses
		index = 0;
		for (auto& renderPassIndex : data.renderPasses) {
			RenderPass &renderPass = renderPassIndex.value;
			if (data.debugMarker.size == 0) {
				if (!renderPass.Init(this)) {
					goto failed;
				}
			} else {
				if (!renderPass.Init(this, data.debugMarker + ".renderPasses[" + ToString(index) + "]")) {
					goto failed;
				}
				index++;
			}
		}
		// Framebuffer init phase, may allocate Memory objects with Images
		index = 0;
		for (auto& framebuffer : data.framebuffers) {
			String debugMarker;
			if (data.debugMarker.size != 0) {
				debugMarker = Stringify(data.debugMarker, ".framebuffer[", index, ']');
				index++;
			}
			if (!framebuffer.value.Init(this, debugMarker)) {
				goto failed;
			}
		}
		// Memory
		index = 0;
		for (auto& memoryIndex : data.memories) {
			Memory &memory = memoryIndex.value;
			if (data.debugMarker.size == 0) {
				if (!memory.Init(this)) {
					goto failed;
				}
			} else {
				if (!memory.Init(this, data.debugMarker + ".memories[" + ToString(index) + "]")) {
					goto failed;
				}
				index++;
			}
		}
		// Samplers
		index = 0;
		for (auto& sampler : data.samplers) {
			if (data.debugMarker.size == 0) {
				sampler.Init(this);
			} else {
				sampler.Init(this, data.debugMarker + ".samplers[" + ToString(index) + "]");
				index++;
			}
			if (!sampler.Create()) {
				goto failed;
			}
		}
		// Descriptors
		index = 0;
		for (auto& descriptorIndex : data.descriptors) {
			Descriptors &descriptor = descriptorIndex.value;
			if (data.debugMarker.size == 0) {
				descriptor.Init(this);
			} else {
				descriptor.Init(this, data.debugMarker + ".descriptors[" + ToString(index) + "]");
				index++;
			}
			if (!descriptor.Create()) {
				goto failed;
			}
			if (!descriptor.Update()) {
				goto failed;
			}
		}
		// Shaders
		index = 0;
		for (auto& shader : data.shaders) {
			if (data.debugMarker.size == 0) {
				if (!shader.Init(this)) {
					goto failed;
				}
			} else {
				if (!shader.Init(this, data.debugMarker + ".shaders[" + ToString(index) + "]")) {
					goto failed;
				}
				index++;
			}
		}
		// Pipelines
		index = 0;
		for (auto& pipelineIndex : data.pipelines) {
			Pipeline &pipeline = pipelineIndex.value;
			if (data.debugMarker.size == 0) {
				if (!pipeline.Init(this)) {
					goto failed;
				}
			} else {
				if (!pipeline.Init(this, data.debugMarker + ".pipelines[" + ToString(index) + "]")) {
					goto failed;
				}
				index++;
			}
		}
		// CommandPools
		index = 0;
		for (auto& commandPoolIndex : data.commandPools) {
			CommandPool &commandPool = commandPoolIndex.value;
			if (data.debugMarker.size == 0) {
				if (!commandPool.Init(this)) {
					goto failed;
				}
			} else {
				if (!commandPool.Init(this, data.debugMarker + ".commandPools[" + ToString(index) + "]")) {
					goto failed;
				}
				index++;
			}
		}
		// Framebuffer create phase
		for (auto& framebufferIndex : data.framebuffers) {
			Framebuffer &framebuffer = framebufferIndex.value;
			if (!framebuffer.Create()) {
				goto failed;
			}
		}
		// Semaphores
		cout.PrintLn("Creating ", data.semaphores.size, " semaphores...");
		for (i32 i = 0; i < data.semaphores.size; i++) {
			const VkSemaphoreCreateInfo createInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
			VkResult result = vkCreateSemaphore(data.device, &createInfo, nullptr, &data.semaphores[i].semaphore);
			if (result != VK_SUCCESS) {
				error = "Failed to create semaphore[" + ToString(i) + "]: " + ErrorString(result);
				goto failed;
			}
			if (data.debugMarker.size != 0) {
				data.semaphores[i].debugMarker = data.debugMarker + ".semaphores[" + ToString(i) + "]";
				SetDebugMarker(this, data.semaphores[i].debugMarker, VK_OBJECT_TYPE_SEMAPHORE, (u64)data.semaphores[i].semaphore);
			}
		}
		// Fences
		cout.PrintLn("Creating ", data.fences.size, " fences...");
		for (i32 i = 0; i < data.fences.size; i++) {
			Fence &fence = data.fences[i];
			String debugMarker;
			if (data.debugMarker.size != 0) debugMarker = Stringify(data.debugMarker, ".fences[", i, ']');
			if (!fence.Create(this, debugMarker)) {
				goto failed;
			}
		}
		// Queue Submissions
		cout.PrintLn("Configuring ", data.queueSubmissions.size(), " QueueSubmissions...");
		for (auto& queueSubmissionIndex : data.queueSubmissions) {
			QueueSubmission &queueSubmission = queueSubmissionIndex.value;
			if (!queueSubmission.noAutoConfig) {
				queueSubmission.Config();
			}
		}

		// Once the pipelines are set with all the shaders,
		// we can clean them since they're no longer needed
		// NOTE: Unless we want to recreate the pipelines. Might reconsider this.
		for (auto& shader : data.shaders) {
			shader.Clean();
		}

		// Init everything else here
		data.initted = true;
		return true;
failed:
		for (auto& swapchain : data.swapchains) {
			if (swapchain.value.data.initted) {
				swapchain.value.Deinit();
			}
		}
		for (auto& renderPass : data.renderPasses) {
			if (renderPass.value.data.initted) {
				renderPass.value.Deinit();
			}
		}
		for (auto& memory : data.memories) {
			if (memory.value.data.initted) {
				memory.value.Deinit();
			}
		}
		for (auto& sampler : data.samplers) {
			sampler.Clean();
		}
		for (auto& descriptor : data.descriptors) {
			descriptor.value.Clean();
		}
		for (auto& shader : data.shaders) {
			shader.Clean();
		}
		for (auto& pipeline : data.pipelines) {
			if (pipeline.value.data.initted) {
				pipeline.value.Deinit();
			}
		}
		for (auto& commandPool : data.commandPools) {
			commandPool.value.Clean();
		}
		for (auto& framebuffer : data.framebuffers) {
			if (framebuffer.value.data.initted) {
				framebuffer.value.Deinit();
			}
		}
		for (i32 i = 0; i < data.semaphores.size; i++) {
			if (data.semaphores[i].semaphore != VK_NULL_HANDLE) {
				vkDestroySemaphore(data.device, data.semaphores[i].semaphore, nullptr);
				data.semaphores[i].semaphore = VK_NULL_HANDLE;
			}
		}
		vkDestroyDevice(data.device, nullptr);
		return false;
	}

	bool Device::Deinit() {
		PrintDashed("Destroying Logical Device");
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (!data.initted) {
			// error = "Device isn't initialized!";
			return false;
		}
#endif
		vkDeviceWaitIdle(data.device);
		for (auto& swapchain : data.swapchains) {
			if (swapchain.value.data.initted) {
				swapchain.value.Deinit();
			}
		}
		for (auto& renderPass : data.renderPasses) {
			if (renderPass.value.data.initted) {
				renderPass.value.Deinit();
			}
		}
		for (auto& memory : data.memories) {
			if (memory.value.data.initted) {
				memory.value.Deinit();
			}
		}
		for (auto& sampler : data.samplers) {
			sampler.Clean();
		}
		for (auto& descriptor : data.descriptors) {
			descriptor.value.Clean();
		}
		// We don't need to clean the shaders since they should have already been cleaned
		for (auto& pipeline : data.pipelines) {
			if (pipeline.value.data.initted) {
				pipeline.value.Deinit();
			}
		}
		for (auto& commandPool : data.commandPools) {
			commandPool.value.Clean();
		}
		for (auto& framebuffer : data.framebuffers) {
			if (framebuffer.value.data.initted) {
				framebuffer.value.Deinit();
			}
		}
		for (i32 i = 0; i < data.semaphores.size; i++) {
			if (data.semaphores[i].semaphore != VK_NULL_HANDLE) {
				vkDestroySemaphore(data.device, data.semaphores[i].semaphore, nullptr);
				data.semaphores[i].semaphore = VK_NULL_HANDLE;
			}
		}
		for (i32 i = 0; i < data.fences.size; i++) {
			Fence &fence = data.fences[i];
			fence.Destroy();
		}
		// Destroy everything allocated from the device here
		vkDestroyDevice(data.device, nullptr);
		data.initted = false;
		return true;
	}

	Instance::Instance() {
		// type = INSTANCE;
		u32 extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		data.extensionsAvailable.Resize(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, data.extensionsAvailable.data);
		u32 layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		data.layersAvailable.Resize(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, data.layersAvailable.data);
	}

	Instance::~Instance() {
		if (data.initted) {
			if (!Deinit()) {
				cout.PrintLn("Failed to clean up vk::Instance: ", error);
			}
		}
	}

	void Instance::AppInfo(const char *name, u32 versionMajor, u32 versionMinor, u32 versionPatch) {
		data.appInfo.pApplicationName = name;
		data.appInfo.applicationVersion = VK_MAKE_VERSION(versionMajor, versionMinor, versionPatch);
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.initted) {
			// Should we bother? It only really makes sense to call this at the beginning
			// and it won't change anything about the renderer itself...
			// Oh well, let's fire a warning.
			cout.PrintLn("Warning: vk::Instance::AppInfo should be used before initializing.");
		}
#endif
	}

	Ptr<Window> Instance::AddWindowForSurface(io::Window *window) {
		Window w;
		w.surfaceWindow = window;
		data.windows.Append(w);
		return Ptr<Window>(&data.windows, data.windows.size-1);
	}

	void Instance::AddExtensions(Array<const char*> extensions) {
		for (i32 i = 0; i < extensions.size; i++) {
			data.extensionsRequired.Append(extensions[i]);
		}
	}

	void Instance::AddLayers(Array<const char*> layers) {
		if (layers.size > 0)
			data.enableLayers = true;
		for (i32 i = 0; i < layers.size; i++) {
			data.layersRequired.Append(layers[i]);
		}
	}

	Ptr<Device> Instance::AddDevice() {
		data.devices.Append(Device());
		return &data.devices.Back();
	}

	bool Instance::Reconfigure() {
		if (data.initted) {
			if (!Deinit())
				return false;
			if (!Init())
				return false;
		}
		return true;
	}

	bool Instance::Initted() const {
		return data.initted;
	}

	bool Instance::Init() {
		PrintDashed("Initializing Vulkan Tree");
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (data.initted) {
			error = "Tree is already initialized!";
			return false;
		}
#endif
		// Put together all needed extensions.
		Array<const char*> extensionsAll(data.extensionsRequired);
		if (data.enableLayers) {
			extensionsAll.Append(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		if (data.windows.size > 0) {
			extensionsAll.Append("VK_KHR_surface");
#ifdef __unix
			if (data.windows[0].surfaceWindow->data->useWayland) {
				extensionsAll.Append("VK_KHR_wayland_surface");
			} else {
				extensionsAll.Append("VK_KHR_xcb_surface");
			}
#elif defined(_WIN32)
			extensionsAll.Append("VK_KHR_win32_surface");
#endif
		}
		// Check required extensions
		Array<const char*> extensionsUnavailable(extensionsAll);
		for (i32 i = 0; i < (i32)extensionsUnavailable.size; i++) {
			for (i32 j = 0; j < (i32)data.extensionsAvailable.size; j++) {
				if (equals(extensionsUnavailable[i], data.extensionsAvailable[j].extensionName)) {
					extensionsUnavailable.Erase(i);
					i--;
					break;
				}
			}
		}
		if (extensionsUnavailable.size > 0) {
			error = "Instance extensions unavailable:";
			for (const char *extension : extensionsUnavailable) {
				error += "\n\t";
				error += extension;
			}
			return false;
		}
		// Check required layers
		Array<const char*> layersUnavailable(data.layersRequired);
		for (i32 i = 0; i < layersUnavailable.size; i++) {
			for (i32 j = 0; j < data.layersAvailable.size; j++) {
				if (equals(layersUnavailable[i], data.layersAvailable[j].layerName)) {
					layersUnavailable.Erase(i);
					i--;
					break;
				}
			}
		}
		if (layersUnavailable.size > 0) {
			error = "Instance layers unavailable:";
			for (const char *layer : layersUnavailable) {
				error += "\n\t";
				error += layer;
			}
			return false;
		}
		// Create the instance
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &data.appInfo;
		createInfo.enabledExtensionCount = extensionsAll.size;
		createInfo.ppEnabledExtensionNames = extensionsAll.data;

		if (data.enableLayers) {
			createInfo.enabledLayerCount = data.layersRequired.size;
			createInfo.ppEnabledLayerNames = data.layersRequired.data;
		} else {
			createInfo.enabledLayerCount = 0;
		}

#ifndef VK_NO_ALLOCATION_CALLBACKS

		data.allocationCallbacks.pUserData = this;
		data.allocationCallbacks.pfnAllocation = Allocate;
		data.allocationCallbacks.pfnReallocation = Reallocate;
		data.allocationCallbacks.pfnFree = Free;
		data.allocationCallbacks.pfnInternalAllocation = nullptr;
		data.allocationCallbacks.pfnInternalFree = nullptr;

		VkResult result = vkCreateInstance(&createInfo, &data.allocationCallbacks, &data.instance);
#else
		VkResult result = vkCreateInstance(&createInfo, nullptr, &data.instance);
#endif
		if (result != VK_SUCCESS) {
			error = "vkCreateInstance failed with error: " + ErrorString(result);
			return false;
		}
		if (data.enableLayers) {
			data.fpCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)
					vkGetInstanceProcAddr(data.instance, "vkCreateDebugUtilsMessengerEXT");
			if (data.fpCreateDebugUtilsMessengerEXT == nullptr) {
				error = "vkGetInstanceProcAddr failed to get vkCreateDebugUtilsMessengerEXT";
#ifndef VK_NO_ALLOCATION_CALLBACKS
				vkDestroyInstance(data.instance, &data.allocationCallbacks);
#else
				vkDestroyInstance(data.instance, nullptr);
#endif
				return false;
			}
			data.fpDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)
					vkGetInstanceProcAddr(data.instance, "vkDestroyDebugUtilsMessengerEXT");
			if (data.fpDestroyDebugUtilsMessengerEXT == nullptr) {
				error = "vkGetInstanceProcAddr failed to get vkDestroyDebugUtilsMessengerEXT";
#ifndef VK_NO_ALLOCATION_CALLBACKS
				vkDestroyInstance(data.instance, &data.allocationCallbacks);
#else
				vkDestroyInstance(data.instance, nullptr);
#endif
				return false;
			}
			data.fpSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)
					vkGetInstanceProcAddr(data.instance, "vkSetDebugUtilsObjectNameEXT");
			if (data.fpSetDebugUtilsObjectNameEXT == nullptr) {
				error = "vkGetInstanceProcAddr failed to get vkSetDebugUtilsObjectNameEXT";
#ifndef VK_NO_ALLOCATION_CALLBACKS
				vkDestroyInstance(data.instance, &data.allocationCallbacks);
#else
				vkDestroyInstance(data.instance, nullptr);
#endif
				return false;
			}
			VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
			createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			createInfo.pfnUserCallback = debugCallback;
			result = data.fpCreateDebugUtilsMessengerEXT(data.instance, &createInfo, nullptr, &data.debugUtilsMessenger);

		}
		// Create a surface if we want one
#ifdef AZCORE_IO_FOR_VULKAN
		for (Window& w : data.windows) {
			if (!w.CreateVkSurface(this)) {
				error = "Failed to CreateVkSurface: " + error;
				goto failed;
			}
			// if (!w.surfaceWindow->CreateVkSurface(this, &w.surface)) {
			//	 error = "Failed to CreateVkSurface: " + error;
			//	 goto failed;
			// }
		}
#endif
		{
			// Get our list of physical devices
			u32 physicalDeviceCount = 0;
			vkEnumeratePhysicalDevices(data.instance, &physicalDeviceCount, nullptr);
			if (physicalDeviceCount == 0) {
				error = "Failed to find GPUs with Vulkan support";
				goto failed;
			}
			Array<VkPhysicalDevice> devices(physicalDeviceCount);
			vkEnumeratePhysicalDevices(data.instance, &physicalDeviceCount, devices.data);
			// Sort them by score while adding them to our permanent list
			for (i32 i = 0; i < (i32)physicalDeviceCount; i++) {
				PhysicalDevice temp;
				temp.physicalDevice = devices[i];
				if (!temp.Init(data.instance)) {
					goto failed;
				}
				i32 spot;
				for (spot = 0; spot < data.physicalDevices.size; spot++) {
					if (temp.score > data.physicalDevices[spot].score) {
						break;
					}
				}
				data.physicalDevices.Insert(spot, temp);
			}
			cout.Print("Physical Devices:");
			for (u32 i = 0; i < physicalDeviceCount; i++) {
				cout.PrintLn("\n\tDevice #", i, "");
				data.physicalDevices[i].PrintInfo(data.windows, data.windows.size > 0);
			}
		}
		// Initialize our logical devices according to their rules
		{
			i32 i = 0;
			for (auto &device : data.devices) {
				if (data.enableLayers) {
					if (!device.value.Init(this, debugMarker + ".devices[" + ToString(i) + "]")) {
						goto failed;
					}
					i++;
				} else {
					if (!device.value.Init(this)) {
						goto failed;
					}
				}
			}
		}

		// Tell everything else to initialize here
		// If it fails, clean up the instance.
		data.initted = true;
		cout.Newline(2);
		PrintDashed("Vulkan Tree Initialized");
#ifndef VK_NO_ALLOCATION_CALLBACKS
		cout.PrintLn("Total Heap Memory Used: ", FormatSize(data.totalHeapMemory), "\nAcross ", data.allocations.size, " allocations.");
#endif
		cout.Newline(2);
		return true;
failed:
		for (auto &device : data.devices) {
			if (device.value.data.initted)
				device.value.Deinit();
		}
		if (data.enableLayers) {
			// data.fpDestroyDebugReportCallbackEXT(data.instance, data.debugReportCallback, &data.allocationCallbacks);
			data.fpDestroyDebugUtilsMessengerEXT(data.instance, data.debugUtilsMessenger, nullptr);
		}
#ifndef VK_NO_ALLOCATION_CALLBACKS
		vkDestroyInstance(data.instance, &data.allocationCallbacks);
#else
		vkDestroyInstance(data.instance, nullptr);
#endif
		return false;
	}

	bool Instance::Deinit() {
		PrintDashed("Destroying Vulkan Tree");
#ifndef AZCORE_VK_SANITY_CHECKS_MINIMAL
		if (!data.initted) {
			error = "Tree isn't initialized!";
			return false;
		}
#endif
		for (auto &device : data.devices) {
			device.value.Deinit();
		}
		// Clean up everything else here
#ifdef AZCORE_IO_FOR_VULKAN
		for (const Window& w : data.windows) {
			vkDestroySurfaceKHR(data.instance, w.surface, nullptr);
		}
#endif
		if (data.enableLayers) {
			data.fpDestroyDebugUtilsMessengerEXT(data.instance, data.debugUtilsMessenger, nullptr);
		}
#ifndef VK_NO_ALLOCATION_CALLBACKS
		vkDestroyInstance(data.instance, &data.allocationCallbacks);
		if (data.totalHeapMemory != 0) {
			cout.PrintLn("Some memory (", FormatSize(data.totalHeapMemory), ") was not freed by the Vulkan driver!\nallocations.size = ", data.allocations.size);
			for (auto& i : data.allocations) {
				cout.PrintLn("\tAllocation at address: ", i.ptr, " with size ", i.size, " freed by Instance.");
				free(i.ptr);
			}
			data.allocations.Resize(0);
		}
#else
		vkDestroyInstance(data.instance, nullptr);
#endif
		data.initted = false;
		return true;
	}

	void CmdExecuteCommands(VkCommandBuffer commandBuffer, Array<Ptr<CommandBuffer>> commandBuffers) {
		Array<VkCommandBuffer> cmdBufs;
		cmdBufs.Reserve(commandBuffers.size);
		for (auto& cmdBuf : commandBuffers) {
			cmdBufs.Append(cmdBuf->data.commandBuffer);
		}
		vkCmdExecuteCommands(commandBuffer, cmdBufs.size, cmdBufs.data);
	}

} // namespace vk

} // namespace AzCore
