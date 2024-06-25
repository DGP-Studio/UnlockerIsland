#pragma once

#include <Windows.h>

#define ISLAND_API extern "C" __declspec(dllexport)

// Feature switch
constexpr auto ISLAND_FEATURE_HANDLE_DLL_PROCESS_DETACH = true;

constexpr PCWSTR ISLAND_ENVIRONMENT_NAME = L"4F3E8543-40F7-4808-82DC-21E48A6037A7";
constexpr PCWSTR ISLAND_FILE_NAME = L"Snap.Hutao.UnlockerIsland.dll";

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

            enum struct IslandState : int
            {
                None = 0,
                Error = 1,
                Started = 2,
                Stopped = 3,
            };

            struct IslandEnvironment {
                LPVOID Address;
                INT32 Value;
                enum IslandState State;
                DWORD LastError;
                INT32 Reserved;
            };

            template <typename THandle, typename TFree>
            class SafeHandle
            {
            private:
                THandle m_handle;
                TFree m_free;
            public:
                SafeHandle(THandle handle, TFree free) : m_handle(handle), m_free(free)
                {
                }
                ~SafeHandle()
                {
                    if (m_handle)
                    {
                        m_free(m_handle);
                    }
                }
                THandle Get() const
                {
                    return m_handle;
                }
                operator THandle() const
                {
                    return m_handle;
                }
                operator bool() const
                {
                    return m_handle != NULL && m_handle != INVALID_HANDLE_VALUE;
                }
            };

            using SafeFileHandle = SafeHandle<HANDLE, decltype(&CloseHandle)>;
            using SafeMappedView = SafeHandle<LPVOID, decltype(&UnmapViewOfFile)>;
        }
    }
}