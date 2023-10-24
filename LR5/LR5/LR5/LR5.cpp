#include <Windows.h>
#include <tchar.h>
#include <evntrace.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int mainScreenWidth = -1;
int mainScreenHeight = -1;
HKL mainInputLocale = NULL;

// Функция для создания записи в журнале Windows Event Log
void LogEventToEventLog(LPCTSTR message) {
    HANDLE hEventLog = RegisterEventSource(NULL, _T("ParameterMonitorApp"));

    if (hEventLog) {
        const WORD eventCategory = 1;
        const DWORD eventId = 1001;

        ReportEvent(hEventLog, EVENTLOG_INFORMATION_TYPE, eventCategory, eventId, NULL, 1, 0, &message, NULL);

        DeregisterEventSource(hEventLog);
    }

    MessageBox(NULL, message, _T("Информация"), MB_ICONINFORMATION);
}


void HandleResolutionChange() {
    int newScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int newScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    if (newScreenWidth != mainScreenWidth || newScreenHeight != mainScreenHeight) {
        mainScreenWidth = newScreenWidth;
        mainScreenHeight = newScreenHeight;
        LogEventToEventLog(_T("Изменилось разрешение экрана!"));
    }
}

void HandleInputLocaleChange() {
    HKL newInputLocale = GetKeyboardLayout(0);
    if (newInputLocale != mainInputLocale) {
        mainInputLocale = newInputLocale;
        LogEventToEventLog(_T("Изменился язык ввода!"));
    }
}

void HandleSystemParamChange(UINT uiAction) {
    if (uiAction == SPI_SETDESKWALLPAPER) {
        LogEventToEventLog(_T("Изменились обои рабочего стола!"));
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

        HandleResolutionChange();
        HandleInputLocaleChange();
    }

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        SetTimer(hwnd, 1, 1000, NULL);

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

    case WM_DESTROY:
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}
