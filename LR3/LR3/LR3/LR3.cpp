#include <Windows.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <tlhelp32.h>

// Глобальные переменные
HINSTANCE hInst;
HWND hWndList;
std::vector<std::wstring> processNames;

// Идентификатор для списка процессов
#define IDC_PROCESS_LIST 1001

// Прототипы функций
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL GetProcessNames();

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    hInst = hInstance; // Сохраняем инстанс приложения

    // Создаем и настраиваем окно
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = _T("MemoryMonitorApp");
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex)) {
        OutputDebugString(L"Ошибка при регистрации класса окна.");
        return FALSE;
    }

    // Создаем главное окно приложения
    HWND hWnd = CreateWindow(_T("MemoryMonitorApp"), _T("Memory Monitor"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) {
        OutputDebugString(L"Ошибка при создании окна.");
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Получаем список активных процессов
    if (!GetProcessNames()) {
        OutputDebugString(L"Ошибка при получении списка процессов.");
        MessageBox(hWnd, _T("Ошибка при получении списка процессов."), _T("Ошибка"), MB_ICONERROR);
    }

    // Главный цикл сообщений
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        // Создаем список для отображения информации о процессах
        hWndList = CreateWindowEx(WS_EX_CLIENTEDGE, _T("LISTBOX"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL,
            10, 10, 760, 540, hWnd, (HMENU)IDC_PROCESS_LIST, hInst, NULL);

        if (hWndList == NULL) {
            OutputDebugString(L"Не удалось создать список.");
            MessageBox(hWnd, _T("Не удалось создать список."), _T("Ошибка"), MB_ICONERROR);
        }

        // Заполняем список процессов
        for (const std::wstring& processName : processNames) {
            SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)processName.c_str());
        }
        break;

    case WM_SIZE:
        // Обновляем размер списка при изменении размера окна
        MoveWindow(hWndList, 10, 10, LOWORD(lParam) - 20, HIWORD(lParam) - 20, TRUE);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

BOOL GetProcessNames() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        OutputDebugString(L"Ошибка при создании снимка процессов.");
        return FALSE;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32)) {
        do {
            processNames.push_back(pe32.szExeFile);
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return TRUE;
}
