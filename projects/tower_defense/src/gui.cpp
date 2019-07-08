/*
    File: gui.cpp
    Author: Philip Haynes
*/

#include "gui.hpp"
#include "assets.hpp"
#include "rendering.hpp"

void Gui::EventAssetInit(Assets::Manager *assets) {
    assets->filesToLoad.Append("OpenSans-Regular.ttf");
}

void Gui::EventAssetAcquire(Assets::Manager *assets) {
    fontIndex = assets->FindMapping("OpenSans-Regular.ttf");
    font = &assets->fonts[fontIndex];
}

void Gui::EventUpdate(bool buffer, Objects::Manager *objects) {
    size += 0.002 * (size + 1.0) * objects->timestep * dir;
    if (size >= 0.5) {
        dir = -1.0;
    } else if (size <= 0.2) {
        dir = 1.0;
    }
    pos += vel * objects->timestep * 0.025;
    if (pos.x + size * 3.0 > 1.0) {
        vel.x = -1.0;
    } else if (pos.x < -1.0) {
        vel.x = 1.0;
    }
    if (pos.y > 1.0) {
        vel.y = -0.5;
    } else if (pos.y - size * 0.4 < -1.0) {
        vel.y = 0.5;
    }
}

void Gui::EventDraw(bool buffer, Rendering::Manager *rendering, VkCommandBuffer commandBuffer) {
    rendering->BindPipelineFont(commandBuffer);
    rendering->DrawTextSS(commandBuffer, ToWString("Hi there, joy!"), fontIndex, pos, vec2(size));
}
