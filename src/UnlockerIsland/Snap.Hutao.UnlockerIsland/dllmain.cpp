#include <Windows.h>

#define ISLAND_API extern "C" __declspec(dllexport)
#define ISLAND_FEATURE_HANDLE_DLL_PROCESS_DETACH true

constexpr PCWSTR ISLAND_ENVIRONMENT_NAME = L"4F3E8543-40F7-4808-82DC-21E48A6037A7";

HANDLE hThread = NULL;
BOOL bDllExit = FALSE;
struct IslandEnvironment* pIslandEnvironment = NULL;

enum IslandState : int
{
    None = 0,
    Error = 1,
    Started = 2,
    Stopped = 3,
};

struct IslandEnvironment {
    LPVOID Address;
    INT32 Value;
    IslandState State;
    DWORD LastError;
    INT32 Reserved;
    HHOOK HHook;
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

static DWORD WINAPI IslandThread(LPVOID lpParam)
{
    const SafeFileHandle hFile = SafeFileHandle(OpenFileMappingW(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, ISLAND_ENVIRONMENT_NAME), CloseHandle);
    if (!hFile)
    {
        return GetLastError();
    }
    
    const SafeMappedView lpView = SafeMappedView(MapViewOfFile(hFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0), UnmapViewOfFile);
    if(!lpView)
    {
        return GetLastError();
    }

    pIslandEnvironment = static_cast<IslandEnvironment*>(lpView.Get());
    INT32* const address = reinterpret_cast<INT32*>(pIslandEnvironment->Address);

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
            if (pIslandEnvironment)
            {
                UnhookWindowsHookEx(pIslandEnvironment->HHook);
            }

            Sleep(500);

            break;
    }

    return TRUE;
}

static LRESULT WINAPI IslandGetWindowHookImpl(int code, WPARAM wParam, LPARAM lParam)
{
    return CallNextHookEx(NULL, code, wParam, lParam);
}

// This function is only meant to get an arbitrary function pointer from the DLL
// So that we can use SetWindowHookEx to inject the DLL into the game
ISLAND_API HRESULT WINAPI IslandGetWindowHook(OUT HOOKPROC* pHookProc)
{
    *pHookProc = IslandGetWindowHookImpl;
    return S_OK;
}