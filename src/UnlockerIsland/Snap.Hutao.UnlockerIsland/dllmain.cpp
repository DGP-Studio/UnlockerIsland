﻿// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

using namespace Snap::Hutao::UnlockerIsland;

HANDLE hThread = NULL;
BOOL bDllExit = FALSE;
struct IslandEnvironment* pEnvironment = NULL;
struct IslandStaging staging {};

static VOID SetFieldOfViewEndpoint(LPVOID pThis, FLOAT value)
{
    value = std::floor(value);
    if (pEnvironment->FieldOfView == 30.0f)
    {
        staging.SetFog(false);
    }

    if (pEnvironment->FieldOfView >= 45.0f && pEnvironment->FieldOfView <= 55.0f)
    {
        value = pEnvironment->FieldOfView;
        staging.SetTargetFrameRate(pEnvironment->TargetFrameRate);
        staging.SetFog(!pEnvironment->DisableFog);
    }

    staging.SetFieldOfView(pThis, value);
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

    staging.SetFieldOfView = reinterpret_cast<SetFieldOfViewFunc>(base + pEnvironment->FunctionOffsetFieldOfView);
    staging.SetTargetFrameRate = reinterpret_cast<SetTargetFrameRateFunc>(base + pEnvironment->FunctionOffsetTargetFrameRate);
    staging.SetFog = reinterpret_cast<SetFogFunc>(base + pEnvironment->FunctionOffsetFog);

    if (pEnvironment->LoopAdjustFpsOnly)
    {
        while (true)
        {
            staging.SetTargetFrameRate(pEnvironment->TargetFrameRate);
            Sleep(62);
        }
    }
    else
    {
        Detours::Hook(&(LPVOID&)staging.SetFieldOfView, SetFieldOfViewEndpoint);
        WaitForSingleObject(GetCurrentThread(), INFINITE);
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

        if (lpReserved)
        {
            bDllExit = TRUE;
        }

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
