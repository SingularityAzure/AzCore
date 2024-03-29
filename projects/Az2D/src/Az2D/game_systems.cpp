/*
	File: game_systems.cpp
	Author: Philip Haynes
*/

#include "game_systems.hpp"
#include "gui_basics.hpp"
#include "settings.hpp"
#include "profiling.hpp"
#include "AzCore/Thread.hpp"

namespace Az2D::GameSystems {

using namespace AzCore;

Manager *sys = nullptr;

void System::EventAssetsQueue() {}
void System::EventAssetsAcquire() {}
void System::EventSync() {}
void System::EventUpdate() {}
void System::EventDraw(Array<Rendering::DrawingContext> &contexts) {}
void System::EventInitialize() {}
void System::EventClose() {}

bool Init(SimpleRange<char> windowTitle, Array<System*> systemsToRegister, bool enableVulkanValidation) {
	AZ2D_PROFILING_SCOPED_TIMER(Az2D::GameSystems::Init)
	sys = new Manager();
	for (System *system : systemsToRegister) {
		sys->systems.Append(system);
	}
	sys->window.name = windowTitle;
	sys->sound.name = windowTitle;
	sys->rendering.data.instance.AppInfo(windowTitle.str, 1, 0, 0);
	sys->enableVulkanValidation = enableVulkanValidation;
	return sys->Init();
}

void UpdateProc() {
	sys->Update();
}

void DrawProc() {
	if (!sys->rendering.Draw() || !sys->rendering.Present()) {
		io::cerr.Lock().PrintLn("Error in Rendering::Manager::Draw or Present: ", Rendering::error).Unlock();
		sys->abort = true;
		return;
	};
}

void UpdateLoop() {
	ClockTime frameStart, frameNext;
	frameNext = Clock::now();
	bool soundProblem = false;

	f32 exitDelay = 0.5f;
	bool exit = false;
	i32 frame = 0;

	while (exitDelay > 0.0f && !sys->abort) {
		sys->window.Fullscreen(Settings::ReadBool(Settings::sFullscreen));
		if ((!sys->window.Update() || sys->exit) && !exit) {
			exit = true;
			sys->sound.FadeoutAll(0.2f);
		}
		if (sys->input.Pressed(KC_KEY_F11)) {
			Settings::SetBool(Settings::sFullscreen, !Settings::ReadBool(Settings::sFullscreen));
		}
		if (exit) {
			exitDelay -= sys->timestep;
		}
		bool vsync = Settings::ReadBool(Settings::sVSync);
		if (frame == 0) {
			sys->frametimes.Update();
			if (vsync) {
				// TODO: switch to polling current monitor refresh rate
				sys->SetFramerate(clamp(1000.0f / sys->frametimes.AverageWithoutOutliers(), 30.0f, 300.0f), true);
			}
		}
		if (abs(Nanoseconds(frameNext - Clock::now()).count()) >= 10000000) {
			// Something must have hung the program. Start fresh.
			frameStart = Clock::now();
		} else {
			frameStart = frameNext;
		}
		frameNext = frameStart + sys->frameDuration;
		{
			i32 dpi = sys->window.GetDPI();
			f32 scale = (f32)dpi / 96.0f;
			Gui::guiBasic->scale = scale;
		}
		sys->rawInput.Update(sys->timestep);
		sys->Sync();
		// TODO: Use persistent threads
		Thread threads[2];
		threads[0] = Thread(UpdateProc);
		if (frame == 0) {
			threads[1] = Thread(DrawProc);
		}
		for (i32 i = 0; i < 2; i++) {
			if (threads[i].Joinable()) threads[i].Join();
		}
		if (sys->abort) break;
		if (!soundProblem) {
			if (!sys->sound.Update(sys->timestep)) {
				io::cerr.PrintLn(Sound::error);
				if (!sys->sound.DeleteSources()) {
					io::cerr.PrintLn("Failed to delete sound sources: ", Sound::error);
				}
				// Sound problems probably shouldn't crash the whole game
				soundProblem = true;
			}
		}
		sys->input.Tick(sys->timestep);
		if (!vsync || frame != 0) {
			Nanoseconds frameSleep = frameNext - Clock::now() - Nanoseconds(1000000);
			if (frameSleep.count() >= 1000000) {
				Thread::Sleep(frameSleep);
			}
		}
		frame = (frame + 1) % sys->updateIterations;
	}
	
	for (System* system : sys->systems) {
		system->EventClose();
	}
}

void Deinit() {
	{
		AZ2D_PROFILING_SCOPED_TIMER(Az2D::GameSystems::Deinit)
		sys->Deinit();
		delete sys;
	}
	Az2D::Profiling::Report();
}

bool Manager::Init() {
	window.input = &input;
	rawInput.window = &window;
	LoadLocale();
	Settings::Load();
	if (!Settings::ReadBool(Settings::sVSync)) {
		SetFramerate(Settings::ReadReal(Settings::sFramerate));
	}
	if (!rawInput.Init(io::RAW_INPUT_ENABLE_GAMEPAD_BIT)) {
		error = Stringify("Failed to initialize RawInput: ", io::error);
		return false;
	}
	if (!sound.Initialize()) {
		error = Stringify("Failed to initialize sound: ", Sound::error);
		return false;
	}
	GetAssets();
	if (!assets.LoadAll()) {
		error = Stringify("Failed to load assets: ", Assets::error);
		return false;
	}
	UseAssets();
	RegisterDrawing();
	CallInitialize();
	
	if (enableVulkanValidation) {
		Array<const char*> layers = {
			"VK_LAYER_KHRONOS_validation",
		};
		rendering.data.instance.AddLayers(layers);
	}
	rendering.data.concurrency = 4;
	
	if (!window.Open()) {
		error = Stringify("Failed to open window: ", io::error);
		return false;
	}
	{
		i32 dpi = window.GetDPI();
		f32 scale = (f32)dpi / 96.0f;
		Gui::guiBasic->scale = scale;
		window.Resize(u32((f32)window.width * scale), u32((u32)window.height * scale));
	}
	
	rendering.msaa = false;
	
	if (!rendering.Init()) {
		error = Stringify("Failed to init Rendering::Manager: ", Rendering::error);
		return false;
	}

	if (!window.Show()) {
		error = Stringify("Failed to show window: ", io::error);
		return false;
	}

	window.Fullscreen(Settings::ReadBool(Settings::sFullscreen));
	
	return true;
}

void Manager::Deinit() {
	if (!rendering.Deinit()) {
		io::cerr.PrintLn("Error deinitializing Rendering: ", Rendering::error);
	}
	window.Close();
	Settings::Save();
	if (!sound.DeleteSources()) {
		io::cerr.PrintLn("Failed to delete sound sources: ", Sound::error);
	}
	assets.sounds.Clear(); // Deletes the OpenAL buffers
	assets.streams.Clear(); // Deletes the OpenAL buffers
	if (!sound.Deinitialize()) {
		io::cerr.PrintLn("Failed to deinitialize sound: ", Sound::error);
	}
	// NOTE: There appears to be a bug on shutdown where the last second or so of audio gets repeated for a split second before being cut off. (Confirmed on Windows, may be an OpenAL bug)
}

void Manager::LoadLocale() {
	AZ2D_PROFILING_SCOPED_TIMER(Az2D::GameSystems::Manager::LoadLocale)
	String localeName;
	localeName.Reserve(21);
	localeName = "data/locale/";

	// if (localeOverride[0] != 0) {
	// 	localeName += localeOverride[0];
	// 	localeName += localeOverride[1];
	// } else {
		std::setlocale(LC_ALL, "");
		char *localeString = std::setlocale(LC_CTYPE, NULL);

		io::cout.PrintLn("localeString = ", localeString);

		localeName += localeString[0];
		localeName += localeString[1];
	// }

	localeName += ".locale";

	Array<char> buffer = FileContents(localeName);
	if (buffer.size == 0) {
		buffer = FileContents("data/locale/en.locale");
	}
	if (buffer.size == 0) return;

	bool skipToNewline = buffer[0] == '#';

	for (i32 i = 0; i < buffer.size;)
	{
		if (buffer[i] == '\n') {
			i++;
			if (i < buffer.size) {
				skipToNewline = buffer[i] == '#';
			}
			continue;
		}
		if (skipToNewline) {
			i += CharLen(buffer[i]);
			continue;
		}
		String name;
		String text;
		for (i32 j = i; j < buffer.size; j += CharLen(buffer[j])) {
			if (buffer[j] == '=')
			{
				name.Resize(j - i);
				memcpy(name.data, &buffer[i], name.size);
				i += name.size + 1;
				break;
			}
		}
		for (; i < buffer.size; i += CharLen(buffer[i])) {
			if (buffer[i] == '"') {
				i++;
				break;
			}
		}
		i32 start = i;
		for (; i < buffer.size; i += CharLen(buffer[i])) {
			if (buffer[i] == '"')
				break;
		}
		text.Resize(i - start);
		if (text.size) {
			memcpy(text.data, &buffer[start], text.size);
		}
		locale[name] = ToWString(text);
		i++;
	}
}

void Manager::SetFramerate(f32 framerate, bool tryCatchup) {
	timestep = 1.0f / framerate;
	updateIterations = ceil(minUpdateFrequency * timestep);
	timestep /= updateIterations;
	if (tryCatchup && updateIterations > 1) {
		framerate *= updateIterations+1;
	} else {
		framerate *= updateIterations;
	}
	// frameDuration affects in-between frametimes even with vsync
	frameDuration = AzCore::Nanoseconds(1000000000/(i32)framerate);
}

void Manager::RenderCallback(void *userdata, Rendering::Manager *rendering, Array<Rendering::DrawingContext> &contexts) {
	((Manager*)userdata)->Draw(contexts);
}

void Manager::RegisterDrawing() {
	rendering.AddRenderCallback(RenderCallback, this);
}

void Manager::GetAssets() {
	AZ2D_PROFILING_SCOPED_TIMER(Az2D::GameSystems::Manager::GetAssets)
	for (System* system : systems) {
		system->EventAssetsQueue();
	}
}

void Manager::UseAssets() {
	AZ2D_PROFILING_SCOPED_TIMER(Az2D::GameSystems::Manager::UseAssets)
	for (System* system : systems) {
		system->EventAssetsAcquire();
	}
}

void Manager::CallInitialize() {
	AZ2D_PROFILING_SCOPED_TIMER(Az2D::GameSystems::Manager::CallInitialize)
	for (System* system : systems) {
		system->EventInitialize();
	}
}

void Manager::Sync() {
	AZ2D_PROFILING_SCOPED_TIMER(Az2D::GameSystems::Manager::Sync)
	buffer = !buffer;
	if (!paused) {
		sys->simulationRate = min(1.0f, sys->simulationRate + sys->timestep * 5.0f);
	} else {
		sys->simulationRate = max(0.0f, sys->simulationRate - sys->timestep * 5.0f);
	}
	if (rawInput.AnyGP.Pressed()) {
		gamepad = &rawInput.gamepads[rawInput.AnyGPIndex];
	}
	for (System* system : systems) {
		system->EventSync();
	}
}

void Manager::Update() {
	AZ2D_PROFILING_SCOPED_TIMER(Az2D::GameSystems::Manager::Update)
	for (System* system : systems) {
		system->EventUpdate();
	}
}

// void DrawThreadProc(System *system, Array<Rendering::DrawingContext> *contexts) {
//	 system->EventDraw(*contexts);
// }

void Manager::Draw(Array<Rendering::DrawingContext>& contexts) {
	AZ2D_PROFILING_SCOPED_TIMER(Az2D::GameSystems::Manager::Draw)
	for (System *system : systems) {
		system->EventDraw(contexts);
	}
}

io::ButtonState* Manager::GetButtonState(u8 keyCode) {
	if (KeyCodeIsGamepad(keyCode)) {
		if (gamepad == nullptr) {
			return nullptr;
		}
		return gamepad->GetButtonState(keyCode);
	} else {
		return &input.GetButtonState(keyCode);
	}
}

bool Manager::Repeated(u8 keyCode) {
	io::ButtonState* state = GetButtonState(keyCode);
	if (!state) return false;
	return state->Repeated();
}

bool Manager::Pressed(u8 keyCode) {
	io::ButtonState* state = GetButtonState(keyCode);
	if (!state) return false;
	return state->Pressed();
}

bool Manager::Down(u8 keyCode) {
	io::ButtonState* state = GetButtonState(keyCode);
	if (!state) return false;
	return state->Down();
}

bool Manager::Released(u8 keyCode) {
	io::ButtonState* state = GetButtonState(keyCode);
	if (!state) return false;
	return state->Released();
}

void Manager::ConsumeInput(u8 keyCode) {
	io::ButtonState* state = GetButtonState(keyCode);
	if (!state) return;
	state->Set(false, false, false);
}

}
