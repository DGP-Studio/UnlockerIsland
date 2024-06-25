#pragma once

#include <Windows.h>

#define ISLAND_API extern "C" __declspec(dllexport)

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

            template <typename THandle, typename TFree>
            class SafeHandle
            {
            private:
                THandle m_handle;
                TFree m_free;
            public:
                SafeHandle(THandle, TFree);
                ~SafeHandle();
                THandle Get() const;
                operator THandle() const;
                operator bool() const;
            };

            using SafeFileHandle = SafeHandle<HANDLE, decltype(&CloseHandle)>;
            using SafeMappedView = SafeHandle<LPVOID, decltype(&UnmapViewOfFile)>;
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
    LPVOID Address;
    INT32 Value;
    enum IslandState State;
    DWORD LastError;
    INT32 Reserved;
};

template <typename THandle, typename TFree>
Snap::Hutao::UnlockerIsland::SafeHandle<THandle, TFree>::SafeHandle(THandle handle, TFree free) : m_handle(handle), m_free(free)
{
}

template <typename THandle, typename TFree>
Snap::Hutao::UnlockerIsland::SafeHandle<THandle, TFree>::~SafeHandle()
{
    if (m_handle)
    {
        m_free(m_handle);
    }
}

template <typename THandle, typename TFree>
THandle Snap::Hutao::UnlockerIsland::SafeHandle<THandle, TFree>::Get() const
{
    return m_handle;
}

template <typename THandle, typename TFree>
Snap::Hutao::UnlockerIsland::SafeHandle<THandle, TFree>::operator THandle() const
{
    return m_handle;
}

template <typename THandle, typename TFree>
Snap::Hutao::UnlockerIsland::SafeHandle<THandle, TFree>::operator bool() const
{
    return m_handle != NULL && m_handle != INVALID_HANDLE_VALUE;
}