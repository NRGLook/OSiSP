#include <windows.h>
#include <deque>
#include <fstream>
#include <thread>
#include <mutex>
#include <string>
#include <iostream>
#include <fstream>
#include "global_defines.h"


using namespace std;

const int gridSize = 25;

const int screenWidth = 1100;
const int screenHeight = 800;

const wchar_t* saveFileName = L"D:\\Study\\OSiSP\\LR1\\LR1\\snake_save.bin";

HWND hWnd; // Хранит хендл (дескриптор) главного окна приложения
HWND restartButton; // Хендл кнопки перезапуска игры
HHOOK g_hook = NULL; // Хендл глобального хука

// Объявление функции LoadGameAsync
void LoadGameAsync();
// Объявление функции SaveGameAsync
void SaveGameAsync();
// Объявление функции RestartGame
void RestartGame();
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

struct SnakeSegment {
    int x, y;
    SnakeSegment() : x(0), y(0) {} // Конструктор по умолчанию
    SnakeSegment(int _x, int _y) : x(_x), y(_y) {}
};

deque<SnakeSegment> snake; //двусторонняя очередь
int foodX, foodY;
bool gameOver = false;
int direction = 1; // 0 - влево, 1 - вверх, 2 - вправо, 3 - вниз
int foodEaten = 0; // Счетчик съеденной еды

// Это синхронизационный механизм, используемый для организации взаимного доступа нескольких потоков к общим ресурсам или критическим участкам кода таким образом, чтобы только один поток имел доступ к этим ресурсам или коду в определенный момент времени.
mutex snakeMutex; // Мьютекс для защиты доступа к змейки в многопоточной среде

//функция для отрисовки ячейки на экране с заданным цветом, для отображения змейки и еды на игровом поле
void DrawCell(HDC hdc, int x, int y, COLORREF color) {
    int cellWidth = screenWidth / gridSize;
    int cellHeight = screenHeight / gridSize;

    // Создаем кисть с заданным цветом
    HBRUSH brush = CreateSolidBrush(color);
    // Задаем цвет для контура ячейки
    HPEN pen = CreatePen(PS_SOLID, 1, color);

    // Выбираем кисть и перо в контексте устройства
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    // Рисуем прямоугольник
    Rectangle(hdc, x * cellWidth, y * cellHeight, (x + 1) * cellWidth, (y + 1) * cellHeight);

    // Восстанавливаем оригинальные кисть и перо
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);

    // Удаляем созданные кисть и перо
    DeleteObject(brush);
    DeleteObject(pen);
}

// Функция обработки нажатия клавиш - ввода пользователя
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

// Функция обновления игры
void UpdateGame(HWND hWnd) {
    // Обновляем позицию змейки в зависимости от направления
    int headX = snake.front().x;
    int headY = snake.front().y;

    switch (direction) {
    case 0: // Влево
        headX--;
        break;
    case 1: // Вверх
        headY--;
        break;
    case 2: // Вправо
        headX++;
        break;
    case 3: // Вниз
        headY++;
        break;
    }

    // Проверяем столкновение с границей экрана
    if (headX < 0 || headX >= screenWidth / gridSize || headY < 0 || headY >= screenHeight / gridSize) {
        gameOver = true;
        return;
    }

    // Проверяем столкновение с самой собой
    lock_guard<mutex> lock(snakeMutex); // средство для автоматической блокировки и разблокировки мьютекса (или другого объекта семафора) в рамках области видимости 
    // Когда объект lock_guard создается и инициализируется мьютексом, он захватывает этот мьютекс, блокируя его -> другие потоки не могут получить доступ к этому мьютексу, пока lock_guard существует и находится в области видимости. Когда lock_guard выходит из области видимости (например, при завершении функции или блока кода), он автоматически разблокирует мьютекс, позволяя другим потокам получить доступ к нему.
    for (const SnakeSegment& segment : snake) {
        if (segment.x == headX && segment.y == headY) {
            gameOver = true;
            return;
        }
    }

    // Проверяем, съела ли змейка еду
    if (headX == foodX && headY == foodY) {
        // Генерируем новую позицию для еды
        foodX = rand() % (screenWidth / gridSize);
        foodY = rand() % (screenHeight / gridSize);
        foodEaten++; // Увеличиваем счетчик съеденной еды

        // Проверяем, сколько яблок съедено, и выводим уведомление каждые 5 яблок
        if (foodEaten % 5 == 0) {
            MessageBox(hWnd, L"You are get 5 apples!", L"Notification", MB_OK | MB_ICONINFORMATION);
        }
    }
    else {
        // Удаляем хвост змейки
        snake.pop_back();
    }

    // Добавляем новую голову
    snake.push_front(SnakeSegment(headX, headY));

    // Обновляем экран
    InvalidateRect(hWnd, NULL, TRUE);
}

void PaintGame(HDC hdc) {
    // Очищаем экран и рисуем фон зеленым цветом
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    HBRUSH greenBrush = CreateSolidBrush(RGB(0, 255, 0)); // Зеленый цвет для фона
    FillRect(hdc, &clientRect, greenBrush);
    DeleteObject(greenBrush);

    // Отображаем счетчик съеденной еды
    wstring foodEatenText = L"Eat: " + to_wstring(foodEaten);
    TextOut(hdc, 10, 10, foodEatenText.c_str(), foodEatenText.length());

    // Рисуем еду красным цветом
    SetBkColor(hdc, RGB(0, 255, 0)); // Задаем зеленый цвет текста (фона текста)
    SetTextColor(hdc, RGB(0, 0, 0)); // Задаем черный цвет текста
    DrawCell(hdc, foodX, foodY, RGB(255, 0, 0)); // Красный цвет для еды

    // Рисуем змейку черным цветом
    SetBkColor(hdc, RGB(0, 255, 0)); // Задаем зеленый цвет текста (фона текста)
    SetTextColor(hdc, RGB(0, 0, 0)); // Задаем черный цвет текста
    lock_guard<mutex> lock(snakeMutex);
    for (const SnakeSegment& segment : snake) {
        DrawCell(hdc, segment.x, segment.y, RGB(0, 0, 0)); // Черный цвет для змейки
    }

    if (gameOver) {
        // Выводим сообщение об окончании игры
        TextOut(hdc, 10, 30, L"Game Over", 9);
    }
}

// функция для асинхронного сохранения состояния игры в файл
// мьютекс для защиты доступа к данным змейки и записывает состояние в бинарном формате в файл
void SaveGameAsync() {
    lock_guard<mutex> lock(snakeMutex);

    // Открываем файл для записи в бинарном режиме
    ofstream file(saveFileName, ios::binary);
    if (file.is_open()) {
        int size = static_cast<int>(snake.size());

        // Записываем размер змейки в файл
        file.write(reinterpret_cast<char*>(&size), sizeof(int));

        // Записываем данные о сегментах змейки в файл
        for (const SnakeSegment& segment : snake) {
            file.write(reinterpret_cast<const char*>(&segment), sizeof(SnakeSegment));
        }

        file.close();
    }
}

// функция для асинхронной загрузки состояния игры из файла
// использует мьютекс для защиты доступа к данным змейки и считывает состояние из файла
void LoadGameAsync() {
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

        lock_guard<mutex> lock(snakeMutex);
        snake = newSnake;
    }
}

// очищает данные о змейке, генерирует новую позицию для еды и сбрасывает счетчики
void RestartGame() {
    lock_guard<mutex> lock(snakeMutex);
    snake.clear();
    snake.push_back(SnakeSegment(5, 5));
    foodX = rand() % (screenWidth / gridSize);
    foodY = rand() % (screenHeight / gridSize);
    direction = 1;
    gameOver = false;
    foodEaten = 0;
    InvalidateRect(hWnd, NULL, TRUE); // Добавляем это для перерисовки экрана
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

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// обработчик сообщений для кнопки перезапуска игры
// обрабатывает события, связанные с этой кнопкой
LRESULT CALLBACK RestartButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_RESTART_BUTTON) {
            // Нажата кнопка "Restart"
            RestartGame();
        }
        break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Регистрируем класс окна
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

    // Создаем главное окно приложения
    hWnd = CreateWindow(L"SnakeGame", L"Snake Game", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 400, 400, NULL, NULL, hInstance, NULL);

    if (!hWnd) {
        return 0;
    }

    restartButton = CreateWindow(L"BUTTON", L"Restart", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | BS_PUSHBUTTON, 10, 50, 100, 30, hWnd, (HMENU)IDC_RESTART_BUTTON, hInstance, NULL);

    if (!restartButton) {
        MessageBox(hWnd, L"Failed to create the restart button!", L"Error", MB_ICONERROR | MB_OK);
        return 0;
    }

    WNDPROC oldButtonProc = (WNDPROC)SetWindowLongPtr(restartButton, GWLP_WNDPROC, (LONG_PTR)RestartButtonProc);
    if (!oldButtonProc) {
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Инициализация змейки
    snake.push_back(SnakeSegment(5, 5));
    foodX = rand() % (screenWidth / gridSize);
    foodY = rand() % (screenHeight / gridSize);

    // Устанавливаем глобальный хук для клавиши "R"
    g_hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (g_hook == NULL) {
        MessageBox(hWnd, L"Failed to install the keyboard hook!", L"Error", MB_ICONERROR | MB_OK);
    }

    // Главный цикл программы
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
                UpdateGame(hWnd); // Вызываем функцию обновления игры
                InvalidateRect(hWnd, NULL, TRUE);
            }
            Sleep(100); // Задержка для контроля скорости змейки
        }
    }

    // Удаляем глобальный хук
    if (g_hook != NULL) {
        UnhookWindowsHookEx(g_hook);
    }

    return (int)msg.wParam;
}

// обработчик глобального хука клавиш, который реагирует на нажатие клавиши "R" и вызывает функцию перезапуска игры
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT* pKB = (KBDLLHOOKSTRUCT*)lParam;
        if (pKB->vkCode == 'R') {
            RestartGame();
        }
    }
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}
