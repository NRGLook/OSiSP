#include <windows.h>
#include <deque>
#include <fstream>
#include <thread>
#include <mutex>
#include <string>
#include <iostream>
#include <chrono>
#include <fstream>
#include "global_defines.h"

using namespace std;

const int gridSize = 25;
const int screenWidth = 1100;
const int screenHeight = 800;

const wchar_t* saveFileName = L"D:\\Study\\OSiSP\\LR1\\LR1\\snake_save.bin";
const wchar_t* sharedMemoryName = L"MySharedMemory";

std::thread gameUpdateThread;

HWND hWnd;
HWND restartButton;
HHOOK g_hook = NULL;

struct SnakeSegment {
    int x, y;
    SnakeSegment() : x(0), y(0) {}
    SnakeSegment(int _x, int _y) : x(_x), y(_y) {}
};

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

deque<SnakeSegment> snake;
int foodX, foodY;
bool gameOver = false;
int direction = 1;
int foodEaten = 0;

mutex snakeMutex;
HANDLE hMapFile;
LPCTSTR pBuf;

void CreateMemoryMapping() {
    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(SnakeSegment),
        sharedMemoryName);

    if (hMapFile == NULL) {
        cout << "Failed to create file display object" << endl;
        return;
    }

    pBuf = (LPCTSTR)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SnakeSegment));

    if (pBuf == NULL) {
        cout << "Failed to display file view" << endl;
        CloseHandle(hMapFile);
        return;
    }
}

void CloseMemoryMapping() {
    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
}

void SaveGame() {
    lock_guard<mutex> lock(snakeMutex);

    ofstream file(saveFileName, ios::binary);
    if (file.is_open()) {
        int size = static_cast<int>(snake.size());
        file.write(reinterpret_cast<char*>(&size), sizeof(int));
        for (const SnakeSegment& segment : snake) {
            file.write(reinterpret_cast<const char*>(&segment), sizeof(SnakeSegment));
        }
        file.close();
    }

    SnakeSegment segment;
    if (!snake.empty()) {
        segment = snake.front();
    }
    memcpy((LPVOID)pBuf, &segment, sizeof(SnakeSegment));
}

void LoadGame() {
    ifstream file(saveFileName, ios::binary);
    if (file.is_open()) {
        int size;
        file.read(reinterpret_cast<char*>(&size), sizeof(int));

        deque<SnakeSegment> newSnake;
        for (int i = 0; i < size; i++) {
            SnakeSegment segment;
            file.read(reinterpret_cast<char*>(&segment), sizeof(SnakeSegment));
            newSnake.push_back(segment);
        }
        file.close();

        SnakeSegment segment;
        memcpy(&segment, pBuf, sizeof(SnakeSegment));

        {
            lock_guard<mutex> lock(snakeMutex);
            snake = newSnake;
            snake.push_front(segment);
        }
    }
}

void DrawCell(HDC hdc, int x, int y, COLORREF color) {
    int cellWidth = screenWidth / gridSize;
    int cellHeight = screenHeight / gridSize;
    HBRUSH brush = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    Rectangle(hdc, x * cellWidth, y * cellHeight, (x + 1) * cellWidth, (y + 1) * cellHeight);
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

void HandleInput(WPARAM wParam) {
    switch (wParam) {
    case VK_LEFT:
        if (direction != 2) direction = 0;
        break;
    case VK_UP:
        if (direction != 3) direction = 1;
        break;
    case VK_RIGHT:
        if (direction != 0) direction = 2;
        break;
    case VK_DOWN:
        if (direction != 1) direction = 3;
        break;
    case 'S':
        SaveGame();
        break;
    case 'L':
        LoadGame();
        break;
    }
}

void UpdateGame(HWND hWnd) {
    auto start = std::chrono::high_resolution_clock::now();

    if (snake.empty()) {
        return;
    }

    int headX = snake.front().x;
    int headY = snake.front().y;

    switch (direction) {
    case 0:
        headX--;
        break;
    case 1:
        headY--;
        break;
    case 2:
        headX++;
        break;
    case 3:
        headY++;
        break;
    }

    if (headX < 0 || headX >= screenWidth / gridSize || headY < 0 || headY >= screenHeight / gridSize) {
        gameOver = true;
        return;
    }

    lock_guard<mutex> lock(snakeMutex);
    for (const SnakeSegment& segment : snake) {
        if (segment.x == headX && segment.y == headY) {
            gameOver = true;
            return;
        }
    }

    if (headX == foodX && headY == foodY) {
        foodX = rand() % (screenWidth / gridSize);
        foodY = rand() % (screenHeight / gridSize);
        foodEaten++;
        if (foodEaten % 5 == 0) {
            MessageBox(hWnd, L"You are getting 5 apples!", L"Notification", MB_OK | MB_ICONINFORMATION);
        }
    }
    else {
        snake.pop_back();
    }

    snake.push_front(SnakeSegment(headX, headY));
    InvalidateRect(hWnd, NULL, TRUE);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Отображение частоты обновления в верхней части окна
    wstring updateRateText = L"Update Rate (μs): " + to_wstring(duration.count());
    TextOut(GetDC(hWnd), 10, 10, updateRateText.c_str(), static_cast<int>(updateRateText.length()));
}

void PaintGame(HDC hdc) {
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    HBRUSH greenBrush = CreateSolidBrush(RGB(0, 255, 0));
    FillRect(hdc, &clientRect, greenBrush);
    DeleteObject(greenBrush);

    auto start = std::chrono::high_resolution_clock::now();

    wstring foodEatenText = L"Get: " + to_wstring(foodEaten);

    COLORREF backgroundColor = RGB(0, 0, 0);
    COLORREF textColor = RGB(255, 255, 255);

    SetBkColor(hdc, backgroundColor);
    SetTextColor(hdc, textColor);

    TextOut(hdc, 10, 30, foodEatenText.c_str(), static_cast<int>(foodEatenText.length()));

    DrawCell(hdc, foodX, foodY, RGB(255, 0, 0)); // for apple

    lock_guard<mutex> lock(snakeMutex);
    for (const SnakeSegment& segment : snake) {
        DrawCell(hdc, segment.x, segment.y, RGB(0, 0, 0));
    }

    if (gameOver) {
        TextOut(hdc, 10, 30, L"Game Over", 9);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    wstring updateRateText = L"Update Rate (μs): " + to_wstring(duration.count());
    TextOut(hdc, 10, 10, updateRateText.c_str(), static_cast<int>(updateRateText.length()));
}


void RestartGame() {
    lock_guard<mutex> lock(snakeMutex);
    snake.clear();
    snake.push_back(SnakeSegment(5, 5));
    foodX = rand() % (screenWidth / gridSize);
    foodY = rand() % (screenHeight / gridSize);
    direction = 1;
    gameOver = false;
    foodEaten = 0;
    InvalidateRect(hWnd, NULL, TRUE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_KEYDOWN:
        HandleInput(wParam);
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        PaintGame(hdc);
        EndPaint(hWnd, &ps);
    } break;
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK RestartButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_RESTART_BUTTON) {
            RestartGame();
        }
        break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

void CreateGameUpdateThread();

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"SnakeGame";
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex)) {
        return 0;
    }

    hWnd = CreateWindow(L"SnakeGame", L"Snake Game", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 400, 400, NULL, NULL, hInstance, NULL);

    if (!hWnd) {
        return 0;
    }

    restartButton = CreateWindow(L"BUTTON", L"Restart", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | BS_PUSHBUTTON, 10, 50, 100, 30, hWnd, (HMENU)IDC_RESTART_BUTTON, hInstance, NULL);

    if (!restartButton) {
        MessageBox(hWnd, L"Failed to create restart button!", L"Error", MB_ICONERROR | MB_OK);
        return 0;
    }

    WNDPROC oldButtonProc = (WNDPROC)SetWindowLongPtr(restartButton, GWLP_WNDPROC, (LONG_PTR)RestartButtonProc);
    if (!oldButtonProc) {
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    snake.push_back(SnakeSegment(5, 5));
    foodX = rand() % (screenWidth / gridSize);
    foodY = rand() % (screenHeight / gridSize);

    g_hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (g_hook == NULL) {
        MessageBox(hWnd, L"Failed to set keyboard hook!", L"Error", MB_ICONERROR | MB_OK);
    }

    CreateMemoryMapping();

    MSG msg;
    while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            if (!gameOver) {
                UpdateGame(hWnd);
                InvalidateRect(hWnd, NULL, TRUE);
            }
            Sleep(100);
        }
    }

    CloseMemoryMapping();
    return (int)msg.wParam;
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT* pKB = (KBDLLHOOKSTRUCT*)lParam;
        if (pKB->vkCode == 'R') {
            RestartGame();
        }
    }
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

void CreateGameUpdateThread() {
    gameUpdateThread = std::thread([] {
        while (true) {
            if (!gameOver) {
                UpdateGame(hWnd);
                InvalidateRect(hWnd, NULL, TRUE);
            }
            Sleep(100);
        }
        });
}
