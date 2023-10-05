/*
	File: main.cpp
	Author: Philip Haynes
*/

#include "Az3D/game_systems.hpp"
#include "Az3D/settings.hpp"
#include "Az3D/profiling.hpp"
#include "Az3D/rendering.hpp"

using namespace AzCore;
using namespace Az3D;
using GameSystems::sys;

Settings::Name sLookSmoothing;
Settings::Name sFlickTilting;

struct Test : public GameSystems::System {
	vec3 pos = vec3(1.0f, 1.0f, 1.0f);
	quat facingDir = quat(1.0f);
	quat objectOrientation = quat(1.0f);
	vec2 facingDiff = 0.0f;
	Degrees32 targetFOV = 90.0f;
	Angle32 sunAngle = Degrees32(45.0f);
	bool mouseLook = false;
	bool sunTurning = false;
	Assets::MeshIndex meshes[4];
	Assets::MeshIndex meshGround;
	i32 currentMesh = 0;
	virtual void EventAssetsQueue() override {
		sys->assets.QueueFile("suzanne.az3d");
		sys->assets.QueueFile("F-232 Eagle.az3d");
		sys->assets.QueueFile("Tree.az3d");
		sys->assets.QueueFile("Grass_Blade.az3d");
		sys->assets.QueueFile("ground.az3d");
	}
	virtual void EventAssetsAcquire() override {
		meshes[0] = sys->assets.FindMesh("suzanne.az3d");
		meshes[1] = sys->assets.FindMesh("F-232 Eagle.az3d");
		meshes[2] = sys->assets.FindMesh("Tree.az3d");
		meshes[3] = sys->assets.FindMesh("Grass_Blade.az3d");
		meshGround = sys->assets.FindMesh("ground.az3d");
	}
	virtual void EventUpdate() override {
		f32 speed = sys->Down(KC_KEY_LEFTSHIFT) ? 8.0f : 2.0f;
		if (sys->Pressed(KC_KEY_ESC)) sys->exit = true;
		if (sys->Pressed(KC_KEY_T)) sunTurning = !sunTurning;
		if (sunTurning) {
			sunAngle += Radians32(halfpi * 0.5f * sys->timestep);
		}
		Rendering::Camera &camera = sys->rendering.camera;
		vec3 camRight = normalize(cross(camera.forward, camera.up));
		vec3 camUp = normalize(cross(camera.forward, camRight));
		{
			vec2i center = vec2i(sys->window.width / 2, sys->window.height / 2);
			if (mouseLook) {
				facingDiff.x += f32(sys->input.cursor.x - center.x) * camera.fov.value() / 60.0f / sys->rendering.screenSize.x;
				facingDiff.y -= f32(sys->input.cursor.y - center.y) * camera.fov.value() / 60.0f / sys->rendering.screenSize.x;
				sys->window.MoveCursor(center.x, center.y);
			}
			if (sys->Pressed(KC_KEY_TAB)) {
				mouseLook = !mouseLook;
				sys->window.HideCursor(mouseLook);
				sys->input.cursor = center;
			}
		}
		{
			f32 factor = 1.0f;
			if (Settings::ReadBool(sLookSmoothing)) {
				factor = decayFactor(0.01f, sys->timestep);
			}
			vec2 diff = facingDiff * factor;
			facingDiff -= diff;
			quat zRot = quat::Rotation(
				diff.x,
				facingDir.Conjugate().RotatePoint(camUp)
			);
			quat xRot = quat::Rotation(
				diff.y,
				facingDir.Conjugate().RotatePoint(camRight)
			);
			facingDir *= zRot * xRot;
			facingDir = normalize(facingDir);
		}
		if (sys->Down(KC_MOUSE_RIGHT)) {
			// vec3 right = objectOrientation.RotatePoint(vec3(1.0f, 0.0f, 0.0f));
			// vec3 up = objectOrientation.RotatePoint(vec3(0.0f, 0.0f, 1.0f));
			quat zRot = quat::Rotation(
				-f32(sys->input.cursor.x - sys->input.cursorPrevious.x) * 4.0f / sys->rendering.screenSize.x,
				camUp
			);
			quat xRot = quat::Rotation(
				f32(sys->input.cursor.y - sys->input.cursorPrevious.y) * 4.0f / sys->rendering.screenSize.x,
				camRight
			);
			objectOrientation *= zRot * xRot;
			objectOrientation = normalize(objectOrientation);
		}
		camera.forward = facingDir.RotatePoint(vec3(0.0f, 1.0f, 0.0f));
		camera.up = facingDir.RotatePoint(vec3(0.0f, 0.0f, 1.0f));
		{
			vec3 camUp = facingDir.Conjugate().RotatePoint(camera.up);
			vec3 camForward = facingDir.Conjugate().RotatePoint(camera.forward);
			vec3 trueUp = facingDir.Conjugate().RotatePoint(vec3(0.0f, 0.0f, 1.0f));
			vec3 targetUp = orthogonalize(trueUp, camForward);
			// You have to clamp because if there's the slightest error that bring the magnitudes above 1 acos gives you NaN
			f32 theta = acos(clamp(dot(camUp, targetUp), -1.0f, 1.0f));
			if (theta > 1.0e-14f) {
				vec3 axis = normalize(cross(camUp, targetUp));
				f32 factor = 1.0f;
				if (Settings::ReadBool(sFlickTilting)) {
					factor = decayFactor(0.025f, sys->timestep);
				}
				quat adjust = quat::Rotation(theta * factor, axis);
				facingDir *= adjust;
			}
		}
		camera.forward = facingDir.RotatePoint(vec3(0.0f, 1.0f, 0.0f));
		camera.up = facingDir.RotatePoint(vec3(0.0f, 0.0f, 1.0f));
		if (sys->Down(KC_KEY_UP) || sys->Down(KC_KEY_W)) {
			camera.pos += speed * sys->timestep * camera.forward;
		}
		if (sys->Down(KC_KEY_DOWN) || sys->Down(KC_KEY_S)) {
			camera.pos -= speed * sys->timestep * camera.forward;
		}
		if (sys->Down(KC_KEY_RIGHT) || sys->Down(KC_KEY_D)) {
			camera.pos += speed * sys->timestep * camRight;
		}
		if (sys->Down(KC_KEY_LEFT) || sys->Down(KC_KEY_A)) {
			camera.pos -= speed * sys->timestep * camRight;
		}
		if (sys->Pressed(KC_KEY_1)) currentMesh = 0;
		if (sys->Pressed(KC_KEY_2)) currentMesh = 1;
		if (sys->Pressed(KC_KEY_3)) currentMesh = 2;
		if (sys->Pressed(KC_KEY_4)) currentMesh = 3;
		targetFOV = clamp(targetFOV.value() - sys->input.scroll.y*5.0f, 5.0f, 103.0f);
		camera.fov = decay(camera.fov.value(), targetFOV.value(), 0.5f, sys->timestep);
	}
	virtual void EventDraw(Array<Rendering::DrawingContext> &contexts) override {
		Rendering::DrawMesh(contexts[0], sys->assets.meshes[meshGround], {mat4::Scaler(vec4(vec3(2.0f), 1.0f))}, true, true);
		Rendering::DrawMesh(contexts[0], sys->assets.meshes[2], {Rendering::GetTransform(vec3(-2.0f, 0.0f, 0.0f), quat(1.0f), vec3(1.0f))}, true, true);
		mat4 transform = Rendering::GetTransform(pos, objectOrientation, vec3(1.0f));
		Rendering::DrawMesh(contexts[0], sys->assets.meshes[meshes[currentMesh]], {transform}, true, true);
		for (i32 i = -10; i <= 10; i++) {
			f32 p = i;
			f32 f = (p+10.0f)/20.0f;
			Rendering::DrawDebugLine(contexts[0],
				{vec3(p, -10.0f, 0.0f), vec4(f, 0.0f, 0.5f, 0.5f)},
				{vec3(p,  10.0f, 0.0f), vec4(f, 1.0f, 0.5f, 0.5f)}
			);
			Rendering::DrawDebugLine(contexts[0],
				{vec3(-10.0f, p, 0.0f), vec4(0.0f, f, 0.5f, 0.5f)},
				{vec3( 10.0f, p, 0.0f), vec4(1.0f, f, 0.5f, 0.5f)}
			);
		}
		RandomNumberGenerator rng(69420);
		ArrayWithBucket<mat4, 1> transforms(2000);
		const f32 grassDimensions = 1.5f;
		for (f32 y = -grassDimensions; y <= grassDimensions; y += 1.0f) {
			for (f32 x = -grassDimensions; x <= grassDimensions; x += 1.0f) {
				for (i32 i = 0; i < transforms.size; i++) {
					mat4 &transform = transforms[i];
					transform = mat4::RotationBasic(random(0.0f, tau, &rng), Axis::Z) * mat4::Scaler(vec4(1.0f, 1.0f, random(0.5f, 1.0f, &rng), 1.0f));
					transform.h.w1 = random(x - 0.5f, x + 0.5f, &rng);
					transform.h.w2 = random(y - 0.5f, y + 0.5f, &rng);
				}
				Rendering::DrawMesh(contexts[0], sys->assets.meshes[meshes[3]], transforms, true, true);
			}
		}
		// Rendering::DrawDebugSphere(contexts[0], vec3(0.0f), 1.0f, vec4(1.0f));
		sys->rendering.uniforms.sunDir = vec3(0.0f, vec2::UnitVecFromAngle(sunAngle.value()));
	}
};

i32 main(i32 argumentCount, char** argumentValues) {

	bool enableLayers = false;
	Test test;
	
	sLookSmoothing = "lookSmoothing";
	sFlickTilting = "flickTilting";

	for (i32 i = 0; i < argumentCount; i++) {
		io::cout.PrintLn(i, ": ", argumentValues[i]);
		if (equals(argumentValues[i], "--validation")) {
			enableLayers = true;
		} else if (equals(argumentValues[i], "--profiling")) {
			io::cout.PrintLn("Enabling profiling");
			Profiling::Enable();
		}
	}
	
	Settings::Add(sLookSmoothing, Settings::Setting(true));
	Settings::Add(sFlickTilting, Settings::Setting(true));

	if (!GameSystems::Init("Az3D Example", {&test}, enableLayers)) {
		io::cerr.PrintLn("Failed to Init: ", GameSystems::sys->error);
		return 1;
	}
	
	sys->rendering.camera.pos = vec3(0.0f, -3.0f, 3.0f);
	sys->rendering.uniforms.ambientLight = sRGBToLinear(sys->rendering.backgroundRGB) * 0.5f;

	GameSystems::UpdateLoop();

	GameSystems::Deinit();
	return 0;
}
