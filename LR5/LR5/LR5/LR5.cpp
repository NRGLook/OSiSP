#include <Windows.h>
#include <tchar.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool resolutionChanged = false;
bool inputLocaleChanged = false;
bool wallpaperChanged = false;
bool systemColorChanged = false;
bool fontChanged = false;

int currentScreenWidth = GetSystemMetrics(SM_CXSCREEN);
int currentScreenHeight = GetSystemMetrics(SM_CYSCREEN);
HKL currentInputLocale = GetKeyboardLayout(0);

void HandleResolutionChange() {
    int newScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int newScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    if (newScreenWidth != currentScreenWidth || newScreenHeight != currentScreenHeight) {
        if (!resolutionChanged) {
            MessageBox(NULL, _T("Изменилось разрешение экрана!"), _T("Изменение параметров"), MB_ICONINFORMATION);
            resolutionChanged = true;
        }
        currentScreenWidth = newScreenWidth;
        currentScreenHeight = newScreenHeight;
    }
}

void HandleInputLocaleChange() {
    HKL newInputLocale = GetKeyboardLayout(0);
    if (newInputLocale != currentInputLocale) {
        if (!inputLocaleChanged) {
            MessageBox(NULL, _T("Изменился язык ввода!"), _T("Изменение параметров"), MB_ICONINFORMATION);
            inputLocaleChanged = true;
        }
        currentInputLocale = newInputLocale;
    }
}

void HandleSystemParamChange(UINT uiAction) {
    if (uiAction == SPI_SETDESKWALLPAPER) {
        if (!wallpaperChanged) {
            MessageBox(NULL, _T("Изменились обои рабочего стола!"), _T("Изменение параметров"), MB_ICONINFORMATION);
            wallpaperChanged = true;
        }
    }
}

void HandleSystemColorChange() {
    if (!systemColorChanged) {
        MessageBox(NULL, _T("Изменились системные цвета!"), _T("Изменение параметров"), MB_ICONINFORMATION);
        systemColorChanged = true;
    }
}

void HandleFontChange() {
    if (!fontChanged) {
        MessageBox(NULL, _T("Изменился шрифт системы!"), _T("Изменение параметров"), MB_ICONINFORMATION);
        fontChanged = true;
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ParameterMonitorApp"), NULL };
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, _T("Parameter Monitor Application"), WS_OVERLAPPEDWINDOW, 100, 100, 400, 300, NULL, NULL, wc.hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        SetTimer(hwnd, 1, 1000, NULL);
        break;

    case WM_TIMER:
        if (wParam == 1) {
            HandleResolutionChange();
            HandleInputLocaleChange();
        }
        break;

    case WM_DISPLAYCHANGE:
        HandleResolutionChange();
        break;

    case WM_INPUTLANGCHANGE:
        HandleInputLocaleChange();
        break;

    case WM_SETTINGCHANGE:
        HandleSystemParamChange(static_cast<UINT>(wParam));
        break;

    case WM_SYSCOLORCHANGE:
        HandleSystemColorChange();
        break;

    case WM_FONTCHANGE:
        HandleFontChange();
        break;

    case WM_DESTROY:
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}
