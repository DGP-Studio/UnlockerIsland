#include "dllmain.hpp"
#include "ntprivate.h"
#include "hook.hpp"

using namespace Snap::Hutao::UnlockerIsland;

static VOID SetFieldOfViewEndpoint(LPVOID pThis, FLOAT value)
{
    if (pEnvironment->FieldOfView == 30.0f)
    {
        reinterpret_cast<void (*)(BYTE)>(staging.FunctionFog)(false);
    }

    if (pEnvironment->FieldOfView >= 45.0f && pEnvironment->FieldOfView <= 55.0f)
    {
        reinterpret_cast<void (*)(INT32)>(staging.FunctionTargetFrameRate)(pEnvironment->TargetFrameRate);
        reinterpret_cast<void (*)(BYTE)>(staging.FunctionFog)(pEnvironment->DisableFog);
    }

    reinterpret_cast<void (*)(LPVOID, FLOAT)>(staging.FunctionFieldOfView)(pThis, value);
}

static DWORD WINAPI IslandThread(LPVOID lpParam)
{
    const UNIQUE_HANDLE hFile = UNIQUE_HANDLE(OpenFileMappingW(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, ISLAND_ENVIRONMENT_NAME));
    if (!hFile)
    {
        return GetLastError();
    }

    const UNIQUE_VIEW_OF_FILE lpView = UNIQUE_VIEW_OF_FILE(MapViewOfFile(hFile.get(), FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0));
    if (!lpView)
    {
        return GetLastError();
    }

    pEnvironment = static_cast<IslandEnvironment*>(lpView.get());

    pEnvironment->State = IslandState::Started;

    UINT64 base = (UINT64)GetModuleHandleW(NULL);

    staging.FunctionFieldOfView = reinterpret_cast<LPVOID>(pEnvironment->FunctionOffsetFieldOfView + base);
    staging.FunctionTargetFrameRate = reinterpret_cast<LPVOID>(pEnvironment->FunctionOffsetTargetFrameRate + base);
    staging.FunctionFog = reinterpret_cast<LPVOID>(pEnvironment->FunctionOffsetFog + base);

    Detours::Hook(&staging.FunctionFieldOfView, SetFieldOfViewEndpoint);

    while (!bDllExit)
    {
        // Do nothing
    }

    pEnvironment->State = IslandState::Stopped;

    FreeLibraryAndExitThread(static_cast<HMODULE>(lpParam), 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (hModule)
    {
        DisableThreadLibraryCalls(hModule);
    }

    // Check if the module is loaded by the game
    if (!GetModuleHandleA("mhypbase.dll"))
    {
        return TRUE;
    }

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            if (hModule)
            {
                LdrAddRefDll(LDR_ADDREF_DLL_PIN, hModule);
            }

            hThread = CreateThread(NULL, 0, IslandThread, hModule, 0, NULL);
            if (!hThread)
            {
                return FALSE;
            }

            CloseHandle(hThread);
            break;

        case DLL_PROCESS_DETACH:
            if (!ISLAND_FEATURE_HANDLE_DLL_PROCESS_DETACH)
            {
                break;
            }

            bDllExit = TRUE;

            Sleep(500);
            break;
    }

    return TRUE;
}

#pragma region Exports

static LRESULT WINAPI IslandGetWindowHookImpl(int code, WPARAM wParam, LPARAM lParam)
{
    return CallNextHookEx(NULL, code, wParam, lParam);
}

ISLAND_API HRESULT WINAPI IslandGetWindowHook(_Out_ HOOKPROC* pHookProc)
{
    *pHookProc = IslandGetWindowHookImpl;
    return S_OK;
}
#pragma endregion