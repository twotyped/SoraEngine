#include "Runtime/Application/Public/SandboxApplication.h"
#include <Core/Application/Public/EntryPoint.h>

SandboxApplication::SandboxApplication() {

}

SandboxApplication::~SandboxApplication() {

}

Sora::Application* Sora::CreateApplication() {
    return new SandboxApplication();
}
