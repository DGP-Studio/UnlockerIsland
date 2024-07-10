#include "dllmain.h"
#include "ntprivate.h"

using namespace Snap::Hutao::UnlockerIsland;

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

    pIslandEnvironment = static_cast<IslandEnvironment*>(lpView.get());
    INT32* const address = pIslandEnvironment->Address;

    MEMORY_BASIC_INFORMATION mbi = { 0 };
    if (!VirtualQuery(address, &mbi, sizeof(mbi)))
    {
        DWORD error = GetLastError();
        pIslandEnvironment->State = IslandState::Error;
        pIslandEnvironment->LastError = error;
        return error;
    }

    if (mbi.Protect != PAGE_READWRITE)
    {
        DWORD error = ERROR_INVALID_ADDRESS;
        pIslandEnvironment->State = IslandState::Error;
        pIslandEnvironment->LastError = error;
        return error;
    }

    pIslandEnvironment->State = IslandState::Started;

    while (!bDllExit)
    {
        *address = pIslandEnvironment->Value;
        Sleep(62);
    }

    pIslandEnvironment->State = IslandState::Stopped;

    FreeLibraryAndExitThread(static_cast<HMODULE>(lpParam), 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
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