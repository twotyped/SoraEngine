#pragma once

#include "Application.h"
#include "Core/Core.h"
#include "Core/Logging/Public/Log.h"

#ifdef SORA_PLATFORM_WINDOWS

extern Sora::Application* Sora::CreateApplication();

int main(int argc, char** argv)
{
	// 1. Engine pre-initialization (logging, memory allocation, profilers, etc.)
	Sora::Log::Initialize();
	SORA_ENGINE_INFO("Logging initialized");

	// 2. Instantiate the client applciation
	auto app = Sora::CreateApplication();
	SORA_ENGINE_INFO("Created client application.");
	SORA_CLIENT_INFO("Application created.");

	// 3. Start main loop
	SORA_ENGINE_INFO("Started client application.");
	SORA_CLIENT_INFO("Application started.");
	app->Run();

	// 4. Clean up client and engine systems
	SORA_ENGINE_INFO("Shutting down.");
	delete app;

	return 0;
}

#endif
