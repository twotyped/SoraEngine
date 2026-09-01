require('vstudio')

premake.override(premake.vstudio.vc2010, "propertyGroup", function(base, cfg, section)
    base(cfg, section)
    if section == "Globals" then
        premake.w("<VcpkgEnabled>true</VcpkgEnabled>")
        premake.w("<VcpkgEnableManifest>true</VcpkgEnableManifest>")
        premake.w("<VcpkgApplocalDeps>true</VcpkgApplocalDeps>")
    end
end)

workspace "SoraEngine"
    architecture "x86_64"
    startproject "RuntimeSandbox"

    buildoptions { "/utf-8" }

    configurations
    {
        "Debug",
        "Release",
    }

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "SoraEngine"
    location "SoraEngine"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("Binaries/" .. outputdir .. "/%{prj.name}")
    objdir ("Intermediates/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/Source/**.h",
        "%{prj.name}/Source/**.cpp"
    }

    includedirs
    {
        "%{wks.location}/%{prj.name}/Source",
        "%{wks.location}/vcpkg_installed/x64-windows/include"
    }

    defines
    {
        "SORA_DYNAMIC_LINK",
        "SORA_BUILD_DLL",
        "SORA_PLATFORM_WINDOWS"
    }

    filter "configurations:Debug"
        defines "SORA_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "SORA_RELEASE"
        runtime "Release"
        optimize "on"

project "RuntimeSandbox"
    location "RuntimeSandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    debugdir "%{wks.location}"

    targetdir ("Binaries/" .. outputdir .. "/%{prj.name}")
    objdir ("Intermediates/" .. outputdir .. "/%{prj.name}")

    vpaths 
    {
        ["*"] = "Source/**"
    }

    files
    {
        "%{prj.name}/Source/**.h",
        "%{prj.name}/Source/**.cpp"
    }

    includedirs
    {
        "%{wks.location}/%{prj.name}/Source",
        "%{wks.location}/SoraEngine/Source",
        "%{wks.location}/vcpkg_installed/x64-windows/include"
    }

    links
    {
        "SoraEngine"
    }

    defines
    {
        "SORA_DYNAMIC_LINK",
        "SORA_PLATFORM_WINDOWS"
    }

    postbuildcommands {
        '{COPY} "%{wks.location}/Binaries/' .. outputdir .. '/SoraEngine/SoraEngine.dll" "$(TargetDir)"',
        '{COPY} "%{wks.location}/vcpkg_installed/x64-windows/bin/fmt.dll" "$(TargetDir)"'
    }

    filter "configurations:Debug"
        defines "SORA_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "SORA_RELEASE"
        runtime "Release"
        optimize "on"