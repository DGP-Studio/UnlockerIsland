#pragma once

#include "Detours/detours.h"
#include <map>

class Detours
{
public:
    static bool Hook(LPVOID* ppOriginal, LPVOID pDetour)
    {
        if (ppOriginal == nullptr || pDetour == nullptr)
        {
            return false;
        }

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(ppOriginal, pDetour);
        LONG error = DetourTransactionCommit();

        if (error == NO_ERROR)
        {
            hookedFunctions[pDetour] = ppOriginal;
            return true;
        }

        return false;
    }

    static bool Unhook(LPVOID pDetour)
    {
        if (pDetour == nullptr || hookedFunctions.find(pDetour) == hookedFunctions.end())
        {
            return false;
        }

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(hookedFunctions[pDetour], pDetour);
        LONG error = DetourTransactionCommit();

        if (error == NO_ERROR)
        {
            hookedFunctions.erase(pDetour);
            return true;
        }

        return false;
    }

    static void UnhookAll()
    {
        for (auto& pair : hookedFunctions)
        {
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourDetach(pair.second, pair.first);
            DetourTransactionCommit();
        }

        hookedFunctions.clear();
    }

private:
    inline static std::map<LPVOID, LPVOID*> hookedFunctions = {};
};