#pragma once

#include "Application.h"
#include "Core/Core.h"

#ifdef SORA_PLATFORM_WINDOWS

extern Sora::Application* Sora::CreateApplication();

int main(int argc, char** argv)
{
	// 1. Engine pre-initialization (logging, memory allocation, profilers, etc.)

	// 2. Instantiate the client applciation
	auto app = Sora::CreateApplication();

	// 3. Start main loop
	app->Run();

	// 4. Clean up client and engine systems
	delete app;

	return 0;
}

#endif
