#pragma once

#include <Core/Application/Public/Application.h>

#ifndef SORA_DEBUG
    #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

class SandboxApplication : public Sora::Application {
public:
	SandboxApplication();
	~SandboxApplication() override;
};
