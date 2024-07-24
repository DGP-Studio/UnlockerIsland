#pragma once

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>
#include <wil/resource.h>

#define ISLAND_API EXTERN_C __declspec(dllexport)

// Feature switch
constexpr auto ISLAND_FEATURE_HANDLE_DLL_PROCESS_DETACH = true;

constexpr PCWSTR ISLAND_ENVIRONMENT_NAME = L"4F3E8543-40F7-4808-82DC-21E48A6037A7";

// This function is only meant to get an arbitrary function pointer from the DLL
// So that we can use SetWindowHookEx to inject the DLL into the game
ISLAND_API HRESULT WINAPI IslandGetWindowHook(_Out_ HOOKPROC* pHookProc);

namespace Snap
{
    namespace Hutao
    {
        namespace UnlockerIsland
        {
            using UNIQUE_HANDLE = wil::unique_any<HANDLE, decltype(&CloseHandle), CloseHandle>;
            using UNIQUE_VIEW_OF_FILE = wil::unique_any<LPVOID, decltype(&UnmapViewOfFile), UnmapViewOfFile>;

            enum struct IslandState;

            struct IslandEnvironment;
            struct IslandStaging
            {
                LPVOID FunctionFieldOfView;
                LPVOID FunctionTargetFrameRate;
                LPVOID FunctionFog;
            };
        }
    }
}

enum struct Snap::Hutao::UnlockerIsland::IslandState : int
{
    None = 0,
    Error = 1,
    Started = 2,
    Stopped = 3,
};

// Layout:
// 0            1                  4                 8
// ┌-------------------------------------------------┐
// │ Reserved1                                       │
// ├-------------------------------┬-----------------┤ 8
// │ Reserved2                     │ State           │
// ├-------------------------------┼-----------------┤ 16
// │ LastError                     │ Reserved3       │
// ├-------------------------------┼-----------------┤ 24
// │ FieldOfView                   │ TargetFrameRate │
// ├------------┬------------------┴-----------------┤ 32
// │ DisableFog │                                    │
// ├------------┴------------------------------------┤ 40
// │ FunctionOffsetFieldOfView                       │
// ├-------------------------------------------------┤ 48
// │ FunctionOffsetTargetFrameRate                   │
// ├-------------------------------------------------┤ 56
// │ FunctionOffsetFog                               │
// └-------------------------------------------------┘ 64
struct Snap::Hutao::UnlockerIsland::IslandEnvironment
{
    LPVOID Reserved1;
    DWORD Reserved2;
    enum IslandState State;
    DWORD LastError;
    DWORD Reserved3;

    FLOAT FieldOfView;
    INT32 TargetFrameRate;
    bool DisableFog;

    UINT64 FunctionOffsetFieldOfView;
    UINT64 FunctionOffsetTargetFrameRate;
    UINT64 FunctionOffsetFog;
};