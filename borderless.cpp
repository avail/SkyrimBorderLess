#pragma comment(lib, "user32.lib")

#include <windows.h>
#include "Hooking.h"

DWORD g_newStyle = WS_VISIBLE | WS_POPUP;

HWND WINAPI fpCreateWindowEx = NULL;

FILE* logFile;

bool hasCreated = false;
HWND outerWindow;

BOOL WINAPI SetWindowPosHooked(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
    fprintf(logFile, "hWnd %p X %i Y %i cx %i cy %i\n", hWnd, X, Y, cx, cy);
    fflush(logFile);

    if (outerWindow == hWnd)
    {
        //X -= cx + 5; move to monitor to the left
        X = 0;
        Y = 0;
    }

    return SetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

BOOL WINAPI AdjustWindowRectHooked(LPRECT lpRect, DWORD dwStyle, BOOL bMenu)
{
    fprintf(logFile, "lpRect %i dwStyle %p bMenu %i\n", lpRect, dwStyle, bMenu);
    fflush(logFile);

    dwStyle = g_newStyle;

    return AdjustWindowRect(lpRect, dwStyle, bMenu);
}

HWND WINAPI CreateWindowExHooked(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    fprintf(logFile, "lpClassName %s lpWindowName %s x %i y %i nWidth %i nHeight %i dwStyle %p\n", lpClassName, lpWindowName, x, y, nWidth, nHeight, dwStyle);
    fflush(logFile);

    if (lpWindowName)
    {
        // x -= nWidth; moves it to the monitor to the left
        dwStyle = g_newStyle;

        hasCreated = true;
    }

    HWND thisWnd = CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

    if (lpWindowName)
    {
        outerWindow = thisWnd;
    }

    return thisWnd;
}

void Init()
{
    DWORD oldProtect;
    VirtualProtect((void*)0x106B2C4, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
    VirtualProtect((void*)0x106B2C8, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
    VirtualProtect((void*)0x106B2F8, 4, PAGE_EXECUTE_READWRITE, &oldProtect);

    *(HWND*)0x106B2C4 = (HWND)CreateWindowExHooked;
    *(BOOL*)0x106B2C8 = (BOOL)AdjustWindowRectHooked;
    *(BOOL*)0x106B2F8 = (BOOL)SetWindowPosHooked;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            logFile = fopen("RIL.BorderLess.log", "w");
            Init();
            break;
        }

        case DLL_PROCESS_DETACH:
        {
            fclose(logFile);
            break;
        }
    }
    return TRUE;
}

#include "Hooking.cpp"