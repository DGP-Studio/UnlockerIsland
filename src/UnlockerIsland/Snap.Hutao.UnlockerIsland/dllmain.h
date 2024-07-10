#pragma once

#include <Windows.h>
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
            HANDLE hThread = NULL;
            BOOL bDllExit = FALSE;
            struct IslandEnvironment* pIslandEnvironment = NULL;

            enum struct IslandState;

            struct IslandEnvironment;

            using UNIQUE_HANDLE = wil::unique_any<HANDLE, decltype(&CloseHandle), CloseHandle>;
            using UNIQUE_VIEW_OF_FILE = wil::unique_any<LPVOID, decltype(&UnmapViewOfFile), UnmapViewOfFile>;
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

struct Snap::Hutao::UnlockerIsland::IslandEnvironment
{
    INT32* Address;
    INT32 Value;
    enum IslandState State;
    DWORD LastError;
    INT32 Reserved;
};