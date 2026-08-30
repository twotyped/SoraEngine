#pragma once

#include "Application.h"
#include "Core/Core.h"
#include "Core/Logging/Public/Log.h"

#ifdef SORA_PLATFORM_WINDOWS

extern Sora::Application* Sora::CreateApplication();

int main(int argc, char** argv) {
    // 1. Engine pre-initialization
    Sora::Log::Initialize();
    SORA_ENGINE_INFO("Logging initialized");

    // 2. Instantiate the client application
    auto app = Sora::CreateApplication();
    SORA_ENGINE_INFO("Created client application.");
    SORA_CLIENT_INFO("Application created.");

    // 3. Initialize subsystems & window
    if (!app || !app->Initialize())
    {
        SORA_ENGINE_ERROR("Application failed to initialize!");
        delete app;
        return -1;
    }

    // 4. Start main loop
    SORA_ENGINE_INFO("Started client application.");
    SORA_CLIENT_INFO("Application started.");
    app->Run();

    // 5. Clean up
    SORA_ENGINE_INFO("Shutting down.");
    delete app;

    return 0;
}

#endif
