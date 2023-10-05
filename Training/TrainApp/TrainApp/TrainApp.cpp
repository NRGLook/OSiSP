#include <windows.h>
#include <commctrl.h>
#include <deque>
#include <fstream>
#include <thread>
#include <mutex>
#include <string>

#pragma comment(lib, "comctl32.lib")

#define IDC_RESTART_BUTTON 101
#define IDC_HELP_BUTTON 102

using namespace std;

const int gridSize = 25;
const int screenWidth = 1100;
const int screenHeight = 800;
const wchar_t* saveFileName = L"D:\\Study\\OSiSP\\LR1\\LR1\\snake_save.txt";

HWND hWnd;
HWND restartButton;
HWND helpButton;
HHOOK g_hook = NULL;

void LoadGameAsync();
void SaveGameAsync();
void RestartGame();

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

struct SnakeSegment {
    int x, y;
    SnakeSegment() : x(0), y(0) {}
    SnakeSegment(int _x, int _y) : x(_x), y(_y) {}
};

deque<SnakeSegment> snake;
int foodX, foodY;
bool gameOver = false;
int direction = 1;
int foodEaten = 0;
mutex snakeMutex;

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
        SaveGameAsync();
        break;
    case 'L':
        LoadGameAsync();
        break;
    }
}

void UpdateGame(HWND hWnd) {
    lock_guard<mutex> lock(snakeMutex);

    // Проверяем, что змейка не пуста
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
            MessageBox(hWnd, L"Вы съели 5 яблок!", L"Уведомление", MB_OK | MB_ICONINFORMATION);
        }
    }
    else {
        snake.pop_back();
    }

    snake.push_front(SnakeSegment(headX, headY));

    InvalidateRect(hWnd, NULL, TRUE);
}


void PaintGame(HDC hdc) {
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    HBRUSH greenBrush = CreateSolidBrush(RGB(0, 255, 0));
    FillRect(hdc, &clientRect, greenBrush);
    DeleteObject(greenBrush);

    wstring foodEatenText = L"Eat: " + to_wstring(foodEaten);
    TextOut(hdc, 10, 10, foodEatenText.c_str(), foodEatenText.length());

    SetBkColor(hdc, RGB(0, 255, 0));
    SetTextColor(hdc, RGB(0, 0, 0));
    DrawCell(hdc, foodX, foodY, RGB(255, 0, 0));

    SetBkColor(hdc, RGB(0, 255, 0));
    SetTextColor(hdc, RGB(0, 0, 0));
    lock_guard<mutex> lock(snakeMutex);
    for (const SnakeSegment& segment : snake) {
        DrawCell(hdc, segment.x, segment.y, RGB(0, 0, 0));
    }

    if (gameOver) {
        TextOut(hdc, 10, 30, L"Game Over", 9);
    }
}

void SaveGameAsync() {
    lock_guard<mutex> lock(snakeMutex);
    ofstream file(saveFileName);
    if (file.is_open()) {
        file << snake.size() << endl;
        for (const SnakeSegment& segment : snake) {
            file << segment.x << "," << segment.y << endl;
        }
        file.close();
    }
}

void LoadGameAsync() {
    ifstream file(saveFileName);
    if (file.is_open()) {
        int size;
        file >> size;

        deque<SnakeSegment> newSnake;
        for (int i = 0; i < size; i++) {
            SnakeSegment segment;
            char comma;
            file >> segment.x >> comma >> segment.y;
            newSnake.push_back(segment);
        }
        file.close();

        lock_guard<mutex> lock(snakeMutex);
        snake = newSnake;
    }
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
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        PaintGame(hdc);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_RESTART_BUTTON:
            RestartGame();
            break;
        case IDC_HELP_BUTTON:
            MessageBox(hWnd, L"Помощь:\nДля управления змейкой используйте стрелки. Нажмите 'R' для перезапуска игры.\nНажмите 'S' для сохранения игры, 'L' для загрузки.", L"Помощь", MB_OK | MB_ICONINFORMATION);
            break;
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
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

    restartButton = CreateWindow(L"BUTTON", L"Restart", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 50, 100, 30, hWnd, (HMENU)IDC_RESTART_BUTTON, hInstance, NULL);
    helpButton = CreateWindow(L"BUTTON", L"Help", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 90, 100, 30, hWnd, (HMENU)IDC_HELP_BUTTON, hInstance, NULL);

    if (!restartButton || !helpButton) {
        MessageBox(hWnd, L"Failed to create one or more buttons!", L"Error", MB_ICONERROR | MB_OK);
        return 0;
    }

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

    if (g_hook != NULL) {
        UnhookWindowsHookEx(g_hook);
    }

    return (int)msg.wParam;
}
