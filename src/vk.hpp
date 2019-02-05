/*
    File: vk.hpp
    Author: Philip Haynes
    Utilities to manage a Vulkan renderer in an RAII style.
    Using the classes in here, one should be able to set up an entire
    system that manages its own resources in a user-defined tree structure.

    NOTE: This library is not intended to replace an understanding of the Vulkan API,
          but rather to reduce the total amount of code necessary to write in order
          to make good use of it. It does this by making several assumptions based on
          the context of the configurations it's given.
*/
#ifndef VK_HPP
#define VK_HPP

#include "common.hpp"
#include <vulkan/vulkan.h>

namespace io {
    class Window;
}

namespace vk {

    extern String error;

    /*  enum: Type
        Author: Philip Haynes
        How Nodes know what other Nodes are.    */
    enum Type {
        UNKNOWN=0,
        INSTANCE,
        LOGICAL_DEVICE,
    };

    /*  struct: Node
        Author: Philip Haynes
        Any object that has a two-way relationship with another.
        Simply used as a basis to keep references of multiple types of nodes.  */
    struct Node {
        Type type = UNKNOWN;
    };

    /*  struct: PhysicalDevice
        Author: Philip Haynes
        Some kind of GPU which we use to create our logical device  */
    struct PhysicalDevice {
        i32 score; // How the device rates for desirability (to choose a logical default)
        VkPhysicalDevice physicalDevice;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        Array<VkExtensionProperties> extensionsAvailable{};
        Array<VkQueueFamilyProperties> queueFamiliesAvailable{};
        VkPhysicalDeviceMemoryProperties memoryProperties;
        bool Init(VkInstance instance);
        bool PrintInfo(VkSurfaceKHR surface=0, bool checkSurface=false);
    };

    /*  class: Device
        Author: Philip Haynes
        Our interface to actually use our physical GPUs to do work  */
    class Device : public Node {
        bool initted = false;
        bool reconfigured = false;
        VkInstance instance;
        PhysicalDevice physicalDevice;
        VkDevice device;
        // TODO: What am I doing here again?
    public:
        bool Init(VkInstance inst);
        bool Deinit();
    };

    /*  class: Instance
        Author: Philip Haynes
        More or less the context for the whole renderer.
        Manages the state of everything else in this toolkit.
        Used as a top-level control of all of the tasks created for it to execute.  */
    class Instance : public Node {
        friend io::Window;
        PFN_vkCreateDebugReportCallbackEXT
            fpCreateDebugReportCallbackEXT;
        PFN_vkDestroyDebugReportCallbackEXT
            fpDestroyDebugReportCallbackEXT;
        bool initted = false;
        bool reconfigured = false;
        bool enableLayers = false;
        VkInstance instance;
        io::Window *surfaceWindow = nullptr;
        VkSurfaceKHR surface;
        bool useSurface = false;
        VkDebugReportCallbackEXT debugReportCallback;
        VkApplicationInfo appInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "AzCore Test",
            .applicationVersion = 1,
            .pEngineName = "AzCore",
            .engineVersion = VK_MAKE_VERSION(0, 1, 0),
            .apiVersion = VK_API_VERSION_1_1
        };
        Array<VkExtensionProperties> extensionsAvailable;
        Array<const char*> extensionsRequired{};
        Array<VkLayerProperties> layersAvailable;
        Array<const char*> layersRequired{};

        Array<PhysicalDevice> physicalDevices{};
    public:

        Instance();
        ~Instance();

        // Configuring functions
        bool AppInfo(const char *name, u32 versionMajor, u32 versionMinor, u32 versionPatch);
        bool SetWindowForSurface(io::Window *window);
        bool AddExtensions(Array<const char*> extensions);
        bool AddLayers(Array<const char*> layers);

        // If the instance is active, you must call this for the changes to be effective.
        bool Reconfigure();

        bool Init(); // Constructs the entire tree
        bool Deinit(); // Cleans everything up
    };

}

#endif