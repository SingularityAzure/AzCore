/*
    File: objects.hpp
    Author: Philip Haynes
    Defines an abstract outline for enumerable objects.
*/

#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#include "AzCore/memory.hpp"
#include "AzCore/io.hpp"
#include "AzCore/vk.hpp"

namespace Assets {
struct Manager;
}
namespace Rendering {
struct Manager;
}

namespace Objects {

struct Manager;

// The basis for any object registered with Manager
struct Object {
    virtual ~Object() = default;

    virtual void EventAssetInit() = 0;
    virtual void EventAssetAcquire() = 0;
    virtual void EventInitialize();
    virtual void EventUpdate();
    virtual void EventDraw(VkCommandBuffer commandBuffer);
};

struct Manager {
    // buffer swaps every frame. Used for lockless multithreading.
    Array<Object*> objects;
    bool buffer = false;
    f32 timestep = 1.0/60.0;

    static void RenderCallback(void *userdata, Rendering::Manager *rendering, Array<VkCommandBuffer>& commandBuffers);

    // The first thing you do with the manager
    inline void Register(Object *object) {
        objects.Append(object);
    }
    // Registers the rendering callbacks
    void RegisterDrawing(Rendering::Manager *rendering);
    // Calls EventAssetInit for every type of object.
    void GetAssets();
    // Calls EventAssetAcquire for every type of object.
    void UseAssets();
    // Calls EventInitialize
    void CallInitialize();
    // Calls different Update events.
    void Update();
    // Calls different Draw events.
    void Draw(Array<VkCommandBuffer>& commandBuffers);

    bool Pressed(u8 keyCode) const;
    bool Down(u8 keyCode) const;
    bool Released(u8 keyCode) const;
};

}


#endif // OBJECTS_HPP