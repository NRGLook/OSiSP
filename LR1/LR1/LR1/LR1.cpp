#include <windows.h>
#include <deque>
#include <fstream>
#include <thread>
#include <mutex>
#include <string>
#include <chrono>
#include <fstream>
#include "resource.h"

using namespace std;

const int gridSize = 25;
const int screenWidth = 1100;
const int screenHeight = 800;

const wchar_t* saveFileName = L"D:\\Study\\OSiSP\\LR1\\LR1\\snake_save.bin";
const wchar_t* sharedMemoryName = L"MySharedMemory";

std::thread inputThread; // Новый поток для обработки пользовательского ввода
std::thread updateGameThread; // Новый поток для функции обновления игры

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

CONDITION_VARIABLE conditionVariable;
CRITICAL_SECTION cs;

bool condition = false;

// Функция для установки приоритета процесса
void SetProcessPriority() {
    HANDLE hProcess = GetCurrentProcess();
    BOOL success = SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS);
    if (success) {
        MessageBox(hWnd, L"Process priority set to HIGH_PRIORITY_CLASS.", L"Priority Set", MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBox(hWnd, L"Failed to set process priority.", L"Priority Set Failed", MB_OK | MB_ICONERROR);
    }
}

// Создаём разделяемую память, которая будет использоваться для обмена данными между процессами (для хранения координат головы змейки)
void CreateMemoryMapping() {
    // создаем область разделяемой памяти
    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE, // дескриптор файла
        NULL, // атрибут для области разделяемой памяти
        PAGE_READWRITE, // определение защиты - чтение + запись
        0, // - старшее слово максимального размера файла-карты, у нас без привязки -> 0
        sizeof(SnakeSegment), // младшее слово максимального размера файла-карты
        sharedMemoryName); // создание идентификатора области разделяемой памяти - для доступа к данной области разделяемой памяти

    if (hMapFile == NULL) {
        MessageBox(hWnd, L"Failed to create file-mapping object", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    // создаем представление разделяемой памяти (через которое будем получать доступ к данным в области разделяемой памяти)
    pBuf = (LPCTSTR)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SnakeSegment));
    // 0 + 0 - без ограничений - sizeof.. - размер части области разделяемой памяти

    if (pBuf == NULL) {
        MessageBox(hWnd, L"Failed to create file view", L"Error", MB_ICONERROR | MB_OK);
        CloseHandle(hMapFile);
        return;
    }
}

// Закрываем и освобождаем связанные с разделяемой памятью ресурсы
void CloseMemoryMapping() {
    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
}

// Сохраняем текущее состояние игры - положение змейки в бинарный файл
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

// Загружаем сохраненное состояние игры из файла
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

// Рисуем одну ячейку игрового поля - 1 - контекст устройства, координаты x + y - цвет
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

// Принимаем код клавиши - обрабатываем пользовательский ввод (управление и действия: сохранить и загрузить)
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

// Обновление состояния игры (движение змейки + проверка столкновений) - принимаем дескриптор окна
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
        if (segment.x == headX and segment.y == headY) {
            gameOver = true;
            return;
        }
    }

    if (headX == foodX and headY == foodY) {
        foodX = rand() % (screenWidth / gridSize);
        foodY = rand() % (screenHeight / gridSize);
        foodEaten++;
        if (foodEaten % 5 == 0) {
            MessageBox(hWnd, L"You have eaten 5 apples!", L"Notification", MB_OK | MB_ICONINFORMATION);
        }
    }
    else {
        snake.pop_back();
    }

    snake.push_front(SnakeSegment(headX, headY));
    InvalidateRect(hWnd, NULL, TRUE);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    wstring updateRateText = L"Update Rate (μs): " + to_wstring(duration.count());
    TextOut(GetDC(hWnd), 10, 10, updateRateText.c_str(), static_cast<int>(updateRateText.length()));
}

// Функция для обновления игры в отдельном потоке - периодически обновляет состояние игры
DWORD WINAPI GameUpdateThreadWrapper(LPVOID lpParam) {
    HWND hWnd = static_cast<HWND>(lpParam);
    while (true) {
        if (!gameOver) {
            UpdateGame(hWnd);
            InvalidateRect(hWnd, NULL, TRUE);
        }
        Sleep(100);
    }
    return 0;
}

// Используется для перезапуска игры + начальной инициализации игровых параметров
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

// Новая функция для обработки пользовательского ввода в отдельном потоке - сейчас ожидает клавишу "R", но вы можете добавить другие клавиши управления
DWORD WINAPI InputThreadWrapper(LPVOID lpParam) {
    while (true) {
        if (GetAsyncKeyState('R') & 0x8000) {
            RestartGame();
        }

        Sleep(100);
    }
    return 0;
}

// Отрисовка всего игрового поля и игровых элементов (змейка + еда)
void PaintGame(HDC hdc) {
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    HBRUSH greenBrush = CreateSolidBrush(RGB(0, 255, 0));
    FillRect(hdc, &clientRect, greenBrush);
    DeleteObject(greenBrush);

    auto start = std::chrono::high_resolution_clock::now();

    wstring foodEatenText = L"Eaten: " + to_wstring(foodEaten);

    COLORREF backgroundColor = RGB(0, 0, 0);
    COLORREF textColor = RGB(255, 255, 255);

    SetBkColor(hdc, backgroundColor);
    SetTextColor(hdc, textColor);

    TextOut(hdc, 10, 30, foodEatenText.c_str(), static_cast<int>(foodEatenText.length()));

    DrawCell(hdc, foodX, foodY, RGB(255, 0, 0)); // для яблока

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

// Дескриптор окна (какое окно должно быть обработано) + код сообщения (определяет, какая часть кода будет выполнена в ответ на сообщение)
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_PAINT:
        PAINTSTRUCT ps;
        HDC hdc;
        hdc = BeginPaint(hWnd, &ps);
        PaintGame(hdc);
        EndPaint(hWnd, &ps);
        break;
    case WM_COMMAND:
        if ((HWND)lParam == restartButton) {
            RestartGame();
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_KEYDOWN:
        HandleInput(wParam);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }
    return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName = L"SnakeGame";

    RegisterClassEx(&wcex);

    hWnd = CreateWindow(L"SnakeGame", L"Snake Game", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, screenWidth, screenHeight, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) {
        MessageBox(nullptr, L"CreateWindow failed", L"Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    restartButton = CreateWindow(L"BUTTON", L"Restart", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 600, 100, 40, hWnd, (HMENU)1, hInstance, 0);
    if (!restartButton) {
        MessageBox(nullptr, L"Create restart button failed", L"Error", MB_ICONERROR | MB_OK);
        return 2;
    }

    SetProcessPriority();

    // Создаем секцию синхронизации
    InitializeCriticalSection(&cs);

    // Создаем условную переменную
    InitializeConditionVariable(&conditionVariable);

    CreateMemoryMapping();

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    RestartGame();

    inputThread = std::thread(InputThreadWrapper, hWnd);
    inputThread.detach();

    updateGameThread = std::thread(GameUpdateThreadWrapper, hWnd);
    updateGameThread.detach();

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteCriticalSection(&cs);
    CloseMemoryMapping();

    return (int)msg.wParam;
}

