/*
	File: entities.hpp
	Author: Philip Haynes
	All the different types of objects that can interact with each other with collision.
*/

#ifndef ENTITIES_HPP
#define ENTITIES_HPP

#include "Az2D/game_systems.hpp"
#include "AzCore/math.hpp"

#include "Az2D/entity_basics.hpp"

namespace Az2D::Entities {

using namespace AzCore;

struct Player : public Entity {
	f32 screamTimer;
	f32 facing;
	f32 hue;
	void EventCreate();
	void Update(f32 timestep);
	void Draw(Rendering::DrawingContext &context);
};
extern template struct DoubleBufferArray<Player>;

struct Tail : public Entity {
	IdGeneric target;
	void EventCreate();
	void Update(f32 timestep);
	void UpdateSync(f32 timestep);
	void Draw(Rendering::DrawingContext &context);
};
extern template struct DoubleBufferArray<Tail>;

struct Manager : public ManagerBasic {
	DoubleBufferArray<Player> players{};
	DoubleBufferArray<Tail> tails{};

	// sprites
	Assets::TexIndex texPlayer;
	Assets::TexIndex texPlayerScream;

	// sounds
	Sound::Source sndScream;
	Sound::Stream sndMusic;
	f32 pitch;

	Manager();

	void EventAssetInit();
	void EventAssetAcquire();
	void EventSync();
	void EventClose();

	void Reset();

	void HandleUI();
};

extern Manager *entities;

} // namespace Entities

#endif // ENTITIES_HPP