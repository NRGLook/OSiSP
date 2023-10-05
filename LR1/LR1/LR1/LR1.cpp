//#include <windows.h>
//#include <vector>
//#include <ctime>
//#include <string>
//
//#include "global_defines.h"
//
////Эти переменные хранят информацию о текущем экземпляре приложения(hInst), о главном окне приложения(hWnd) и о кнопке рестарта(hRestartButton).
//HINSTANCE hInst;//Хранит информацию о текущем экземпляре приложения
//HWND hWnd;//Хранит хендл (дескриптор) главного окна приложения
//HWND hRestartButton;//Хранит хендл кнопки "Restart"
////HBITMAP pattern;
//
////Здесь определены константы для размера сетки и размера поля игры в клетках.
//const int gridSize = 20;//Константа, определяющая размер клетки на игровом поле
//int width = 20;
//int height = 15;
//
////Эти переменные хранят информацию о положении змейки (snake) и еде (food) на поле игры.
//std::vector<POINT> snake;//Вектор, хранящий координаты сегментов змейки
//POINT food;//Координаты еды на поле
//
////Эти переменные определяют направление движения змейки по осям X и Y.Например, (1, 0) означает движение вправо, (-1, 0) - влево, (0, 1) - вниз, и(0, -1) - вверх.
//int directionX = 1;
//int directionY = 0;
//
////Переменные gameOver и foodCount используются для отслеживания состояния игры: завершена ли она и сколько еды съела змейка.
//bool gameOver = false;
//int foodCount = 0;
//
////Это прототипы функций, которые будут определены позже в коде. Они включают в себя регистрацию класса окна, инициализацию экземпляра приложения, обработчик оконных сообщений (WndProc), обновление игры, отрисовку игры, создание еды и функцию перезапуска игры.
//ATOM MyRegisterClass(HINSTANCE hInstance);//Прототип функции регистрации класса окна
//BOOL InitInstance(HINSTANCE, int);//Прототип функции инициализации экземпляра приложения и создания главного окна
//LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);//Прототип функции обработки оконных сообщений
//void UpdateGame();
//void DrawGame(HDC hdc);
//void CreateFood();
//void RestartGame();
//
////Эта переменная будет использоваться для хранения информации о глобальном хуке клавиш.
//HHOOK g_hKeyboardHook = NULL;
//
////Это прототип функции, которая будет использоваться как обработчик глобального хука клавиш.
//LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
//
//// Глобальная переменная для хранения хендла окна сообщения, которое будет отображаться в игре.
//HWND g_hMessageBox = NULL;
//
//
///*
//// Вспомогательная функция для сохранения данных в файл
//bool SaveGameToFile(const std::wstring& filename) {
//    std::ofstream file(filename, std::ios::binary);
//    if (!file.is_open()) {
//        std::wcerr << L"Ошибка: Не удалось открыть файл для записи." << std::endl;
//        return false;
//    }
//
//    // Сохраните данные игры, например, состояние змейки и позицию еды, в файл
//    // Здесь вы можете добавить код для сохранения данных вашей игры
//
//    file.close();
//    return true;
//}
//
//// Вспомогательная функция для загрузки данных из файла
//bool LoadGameFromFile(const std::wstring& filename) {
//    std::ifstream file(filename, std::ios::binary);
//    if (!file.is_open()) {
//        std::wcerr << L"Ошибка: Не удалось открыть файл для чтения." << std::endl;
//        return false;
//    }
//
//    // Загрузите данные игры из файла и восстановите состояние змейки и позицию еды
//    // Здесь вы можете добавить код для загрузки данных вашей игры
//
//    file.close();
//    return true;
//}
//
//// Функция для асинхронного чтения данных из файла (пример)
//void AsyncReadFile(const std::wstring& filename) {
//    // Инициируйте асинхронное чтение данных из файла и обработайте результат
//    // Здесь вы можете добавить код для асинхронного чтения данных из файла вашей игры
//
//    std::wcout << L"Асинхронное чтение файла: " << filename << std::endl;
//}
//
//// Функция для отображения файла в память (пример)
//void MapFileToMemory(const std::wstring& filename) {
//    HANDLE hFile = CreateFileW(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
//    if (hFile == INVALID_HANDLE_VALUE) {
//        std::wcerr << L"Ошибка: Не удалось открыть файл для отображения в память." << std::endl;
//        return;
//    }
//
//    HANDLE hMapFile = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
//    if (hMapFile == NULL) {
//        CloseHandle(hFile);
//        std::wcerr << L"Ошибка: Не удалось создать отображение файла в памяти." << std::endl;
//        return;
//    }
//
//    LPVOID pData = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
//    if (pData == NULL) {
//        CloseHandle(hMapFile);
//        CloseHandle(hFile);
//        std::wcerr << L"Ошибка: Не удалось отобразить файл в памяти." << std::endl;
//        return;
//    }
//
//    // Теперь у вас есть доступ к данным файла через pData
//    // Здесь вы можете добавить код для работы с данными из отображенного файла вашей игры
//
//    UnmapViewOfFile(pData);
//    CloseHandle(hMapFile);
//    CloseHandle(hFile);
//}
//*/
//
//void ShowNotification(LPCWSTR message) {
//    // Получить размер экрана
//    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
//    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
//
//    // Размер и положение окна сообщения
//    int notificationWidth = 300;
//    int notificationHeight = 100;
//    int notificationX = (screenWidth - notificationWidth) / 2;
//    int notificationY = (screenHeight - notificationHeight) / 2;
//
//    // Создание окна сообщения
//    g_hMessageBox = CreateWindow(L"STATIC", message, WS_POPUP | WS_VISIBLE | SS_CENTER | WS_BORDER | MB_TOPMOST,
//        notificationX, notificationY, notificationWidth, notificationHeight, hWnd, NULL, hInst, NULL);
//
//    // Установка таймера для закрытия окна через 5 секунды
//    SetTimer(hWnd, 2, 500, NULL);
//
//    // Центрирование текста в окне сообщения
//    SendMessage(g_hMessageBox, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
//    SendMessage(g_hMessageBox, STM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(NULL, IDI_INFORMATION));
//}
//
//
//void CloseNotification() {
//    if (g_hMessageBox != NULL) {
//        DestroyWindow(g_hMessageBox);
//        g_hMessageBox = NULL;
//    }
//}
//
////Это точка входа в приложение, где инициализируются глобальные переменные, регистрируется класс окна и запускается цикл обработки сообщений
////Это основная функция приложения, которая инициализирует приложение, регистрирует класс окна, создает окно, и входит в цикл обработки сообщений.
//int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
//    UNREFERENCED_PARAMETER(hPrevInstance);
//    UNREFERENCED_PARAMETER(lpCmdLine);
//
//    hInst = hInstance;
//    MyRegisterClass(hInstance);
//
//    if (!InitInstance(hInstance, nCmdShow)) {
//        return FALSE;
//    }
//
//    MSG msg;
//    UINT_PTR timerId = SetTimer(hWnd, 1, 100, NULL);
//
//    while (GetMessage(&msg, nullptr, 0, 0)) {
//        TranslateMessage(&msg);
//        DispatchMessage(&msg);
//    }
//
//    KillTimer(hWnd, timerId);
//
//    return (int)msg.wParam;
//}
//
////Когда вы регистрируете класс окна, Windows возвращает значение типа ATOM, которое представляет собой уникальный идентификатор зарегистрированного класса.
////Эта функция регистрирует класс окна.
//ATOM MyRegisterClass(HINSTANCE hInstance) {
//    WNDCLASSEXW wcex;
//    wcex.cbSize = sizeof(WNDCLASSEX);
//    wcex.style = CS_HREDRAW | CS_VREDRAW;
//    wcex.lpfnWndProc = WndProc;
//    wcex.cbClsExtra = 0;
//    wcex.cbWndExtra = 0;
//    wcex.hInstance = hInstance;
//    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
//    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
//    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
//    wcex.lpszMenuName = nullptr;
//    wcex.lpszClassName = L"SnakeGame";
//    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
//    return RegisterClassExW(&wcex);
//}
//
////Эта функция инициализирует экземпляр приложения и создает главное окно.
//BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
//    hInst = hInstance;
//
//    //pattern = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));
//
//    WCHAR szTitle[MAX_LOADSTRING];
//    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
//    hWnd = CreateWindow(L"SnakeGame", szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
//
//    if (!hWnd) {
//        return FALSE;
//    }
//
//    ShowWindow(hWnd, nCmdShow);
//    UpdateWindow(hWnd);
//
//    width = (GetSystemMetrics(SM_CXSCREEN) - 100) / gridSize; // Ширина поля зависит от размеров экрана
//    height = (GetSystemMetrics(SM_CYSCREEN) - 100) / gridSize; // Высота поля зависит от размеров экрана
//
//    snake.push_back({ width / 2, height / 2 });
//    CreateFood();
//
//    // Создание кнопки рестарта
//    hRestartButton = CreateWindow(L"BUTTON", L"Restart", WS_CHILD | WS_VISIBLE, 50, 10, 100, 30, hWnd, (HMENU)1, hInstance, NULL);
//
//    // Создание кнопки Help
//    CreateWindow(L"BUTTON", L"Help", WS_CHILD | WS_VISIBLE, 50, 50, 100, 30, hWnd, (HMENU)2, hInstance, NULL);
//
//    // Установка глобального хука на клавиши
//    g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
//
//    return TRUE;
//}
//
////Это функция-обработчик оконных сообщений, которая обрабатывает события, такие как отрисовка окна, нажатия клавиш и другие.
//LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
//    switch (message) {
//    case WM_PAINT: {
//        PAINTSTRUCT ps;
//        HDC hdc = BeginPaint(hWnd, &ps);
//
//        //rendering mode
//        //create picture
//        //HBRUSH newBrush = CreatePatternBrush(pattern);
//        //BITMAP pattern;
//        //GetObject(pattern, sizeof(BITMAP), &patternObject);
//        //create time context
//        //HDC memDC = CreateCompatibleDC(hds);
//        //SelectObject(memDC, kitty);
//        //BitBlt
//        //  (hdc,30,50,kittyObject.bnWidth, kittyObject.bmHeight,memDC,0,0,SRCCOPY)//copy to 1 to 2 pixecls
//        //StretchBlt...
//        //DeleteDC(memDC)
//
//        DrawGame(hdc);
//        EndPaint(hWnd, &ps);
//    }
//                 break;
//    case WM_KEYDOWN: {
//        switch (wParam) {
//        case VK_LEFT:
//            if (directionX == 0) {
//                directionX = -1;
//                directionY = 0;
//            }
//            break;
//        case VK_UP:
//            if (directionY == 0) {
//                directionX = 0;
//                directionY = -1;
//            }
//            break;
//        case VK_RIGHT:
//            if (directionX == 0) {
//                directionX = 1;
//                directionY = 0;
//            }
//            break;
//        case VK_DOWN:
//            if (directionY == 0) {
//                directionX = 0;
//                directionY = 1;
//            }
//            break;
//        }
//    }
//                   break;
//    case WM_TIMER:
//        if (wParam == 2) {
//            CloseNotification(); // Закрыть окно уведомления
//        }
//        else if (gameOver) {
//            KillTimer(hWnd, 1);
//        }
//        else {
//            UpdateGame();
//        }
//        break;
//    case WM_COMMAND:
//        switch (LOWORD(wParam)) {
//        case 1: // Обработка сообщений от кнопки рестарта
//            RestartGame();
//            break;
//        case 2: // Обработка сообщений от кнопки Help
//            MessageBox(hWnd, L"Game Rules:\n\nSnake control: Arrows:\n← Left\n↑ Up\n→ Right\n↓ Down\n\nCollect red squares (apples) to grow\n\nAvoid collisions with screen boundaries and yourself", L"Help", MB_OK | MB_ICONINFORMATION);
//            break;
//        }
//        break;
//    case WM_CLOSE: // Обработка закрытия окна
//        KillTimer(hWnd, 1);
//        PostQuitMessage(0);
//        break;
//    default:
//        return DefWindowProc(hWnd, message, wParam, lParam);
//    }
//    return 0;
//}
//
//void UpdateGame() {
//    POINT newHead = snake.front();
//    newHead.x += directionX;
//    newHead.y += directionY;
//    snake.insert(snake.begin(), newHead);
//
//    if (newHead.x == food.x && newHead.y == food.y) {
//        foodCount++;
//        CreateFood();
//        WCHAR szTitle[MAX_LOADSTRING];
//        wsprintf(szTitle, L"SnakeGame - Food: %d", foodCount);
//        SetWindowText(hWnd, szTitle);
//
//        if (foodCount % 5 == 0) {
//            ShowNotification(L"Congratulations! You have  5 apples!");
//        }
//    }
//    else {
//        snake.pop_back();
//    }
//
//    if (newHead.x < 0 || newHead.x >= width || newHead.y < 0 || newHead.y >= height) {
//        gameOver = true;
//    }
//
//    for (size_t i = 1; i < snake.size(); i++) {
//        if (snake[i].x == newHead.x && snake[i].y == newHead.y) {
//            gameOver = true;
//        }
//    }
//
//    if (gameOver) {
//        ShowNotification(L"Game Over");
//    }
//
//    InvalidateRect(hWnd, nullptr, TRUE);
//}
//
//void DrawGame(HDC hdc) {
//    HBRUSH greenBrush = CreateSolidBrush(RGB(0, 128, 0));
//    HBRUSH redBrush = CreateSolidBrush(RGB(255, 0, 0));
//    //HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255)); // Создание кисти с белым цветом - для змеи
//    HBRUSH borderBrush = CreateSolidBrush(RGB(0, 0, 0)); // Цвет границы
//    HBRUSH backgroundBrush = CreateSolidBrush(RGB(255, 255, 255)); // Цвет заднего фона за границей
//
//    RECT rect;
//    GetClientRect(hWnd, &rect);
//    FillRect(hdc, &rect, backgroundBrush); // Заливаем задний фон цветом за границей
//
//    // Рассчитываем размеры и координаты игровой области
//    int gameAreaWidth = width * gridSize;
//    int gameAreaHeight = height * gridSize;
//    int borderSize = 10; // Толщина границы
//
//    int gameAreaLeft = (rect.right - gameAreaWidth) / 2;
//    int gameAreaTop = (rect.bottom - gameAreaHeight) / 2;
//    int gameAreaRight = gameAreaLeft + gameAreaWidth;
//    int gameAreaBottom = gameAreaTop + gameAreaHeight;
//
//    // Рисуем границу игровой области
//    RECT borderRect = { gameAreaLeft - borderSize, gameAreaTop - borderSize, gameAreaRight + borderSize, gameAreaBottom + borderSize };
//    FillRect(hdc, &borderRect, borderBrush);
//
//    // Рисуем вертикальные линии границы
//    for (int x = gameAreaLeft - borderSize; x <= gameAreaRight + borderSize; x += gridSize) {
//        MoveToEx(hdc, x, gameAreaTop - borderSize, NULL);
//        LineTo(hdc, x, gameAreaBottom + borderSize);
//    }
//
//    // Рисуем горизонтальные линии границы
//    for (int y = gameAreaTop - borderSize; y <= gameAreaBottom + borderSize; y += gridSize) {
//        MoveToEx(hdc, gameAreaLeft - borderSize, y, NULL);
//        LineTo(hdc, gameAreaRight + borderSize, y);
//    }
//
//    
//    for (const auto& segment : snake) {
//        rect.left = gameAreaLeft + segment.x * gridSize;
//        rect.top = gameAreaTop + segment.y * gridSize;
//        rect.right = rect.left + gridSize;
//        rect.bottom = rect.top + gridSize;
//        FillRect(hdc, &rect, greenBrush);
//    }
//
//    rect.left = gameAreaLeft + food.x * gridSize;
//    rect.top = gameAreaTop + food.y * gridSize;
//    rect.right = rect.left + gridSize;
//    rect.bottom = rect.top + gridSize;
//    FillRect(hdc, &rect, redBrush);
//
//    /*
//    for (const auto& segment : snake) {
//        int x = gameAreaLeft + segment.x * gridSize + gridSize / 2; // Центр круга по X
//        int y = gameAreaTop + segment.y * gridSize + gridSize / 2; // Центр круга по Y
//        int radius = gridSize / 2; // Радиус круга
//        HBRUSH brush = greenBrush; // Зеленый цвет для змеи
//        Ellipse(hdc, x - radius, y - radius, x + radius, y + radius);
//    }
//
//    int x = gameAreaLeft + food.x * gridSize + gridSize / 2; // Центр круга по X
//    int y = gameAreaTop + food.y * gridSize + gridSize / 2; // Центр круга по Y
//    int radius = gridSize / 2; // Радиус круга
//    HBRUSH brush = redBrush; // Красный цвет для яблока
//    Ellipse(hdc, x - radius, y - radius, x + radius, y + radius);
//    */
//
//    DeleteObject(greenBrush);
//    DeleteObject(redBrush);
//    DeleteObject(borderBrush);
//    DeleteObject(backgroundBrush);
////  DeleteObject(whiteBrush); // Освобождение кисти
//}
//
//void CreateFood() {
//    srand(static_cast<unsigned int>(time(nullptr)));
//    food.x = rand() % width;
//    food.y = rand() % height;
//}
//
//
//void RestartGame() {
//    if (gameOver) {
//        snake.clear();
//        snake.push_back({ width / 2, height / 2 });
//        CreateFood();
//        foodCount = 0;
//        gameOver = false;
//        SetWindowText(hWnd, L"SnakeGame");
//        InvalidateRect(hWnd, nullptr, TRUE);
//        SetFocus(hWnd); // Вернуть фокус на окно игры
//
//        // Включить таймер снова
//        SetTimer(hWnd, 1, 100, NULL);
//    }
//}
//
////Это функция-обработчик глобального хука клавиш, которая позволяет реагировать на определенные клавиши, например, для рестарта игры.
//LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
//    if (nCode == HC_ACTION) {
//        if (wParam == WM_KEYDOWN) {
//            // Обработка нажатия клавиши (например, 'R' для рестарта игры)
//            KBDLLHOOKSTRUCT* pKeyStruct = (KBDLLHOOKSTRUCT*)lParam;
//            if (pKeyStruct->vkCode == 'R') {
//                // Вызывайте функцию рестарта игры здесь
//                RestartGame();
//            }
//        }
//    }
//    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
//}
#include <windows.h>
#include <deque>
#include <fstream>
#include <thread>
#include <mutex>
#include <string>
#include "global_defines.h"

using namespace std;



const int gridSize = 25;

const int screenWidth = 1100;
const int screenHeight = 800;

const wchar_t* saveFileName = L"D:\\Study\\OSiSP\\LR1\\LR1\\snake_save.txt";

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

deque<SnakeSegment> snake;
int foodX, foodY;
bool gameOver = false;
int direction = 1; // 0 - влево, 1 - вверх, 2 - вправо, 3 - вниз
int foodEaten = 0; // Счетчик съеденной еды

mutex snakeMutex; // Мьютекс для защиты доступа к змейке

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

// Функция обработки нажатия клавиш
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
    lock_guard<mutex> lock(snakeMutex);
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
            MessageBox(hWnd, L"Вы съели 5 яблок!", L"Уведомление", MB_OK | MB_ICONINFORMATION);
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

//in binary format

//void SaveGameAsync() {
//    lock_guard<mutex> lock(snakeMutex);
//    ofstream file(saveFileName, ios::binary); // Открываем файл в бинарном режиме
//    if (file.is_open()) {
//        int size = snake.size();
//        file.write(reinterpret_cast<char*>(&size), sizeof(size)); // Записываем размер змейки
//
//        for (const SnakeSegment& segment : snake) {
//            file.write(reinterpret_cast<const char*>(&segment), sizeof(SnakeSegment)); // Записываем каждый сегмент змейки
//        }
//        file.close();
//    }
//}
//
//void LoadGameAsync() {
//    ifstream file(saveFileName, ios::binary); // Открываем файл в бинарном режиме
//    if (file.is_open()) {
//        int size;
//        file.read(reinterpret_cast<char*>(&size), sizeof(size)); // Читаем размер змейки
//
//        deque<SnakeSegment> newSnake;
//        for (int i = 0; i < size; i++) {
//            SnakeSegment segment;
//            file.read(reinterpret_cast<char*>(&segment), sizeof(SnakeSegment)); // Читаем каждый сегмент змейки
//            newSnake.push_back(segment);
//        }
//        file.close();
//
//        lock_guard<mutex> lock(snakeMutex);
//        snake = newSnake;
//    }
//}


void SaveGameAsync() {
    lock_guard<mutex> lock(snakeMutex);
    ofstream file(saveFileName); // Открываем файл в текстовом режиме
    if (file.is_open()) {
        file << snake.size() << endl; // Записываем размер змейки
        for (const SnakeSegment& segment : snake) {
            file << segment.x << "," << segment.y << endl; // Записываем каждый сегмент змейки
        }
        file.close();
    }
}

void LoadGameAsync() {
    ifstream file(saveFileName); // Открываем файл в текстовом режиме
    if (file.is_open()) {
        int size;
        file >> size; // Читаем размер змейки

        deque<SnakeSegment> newSnake;
        for (int i = 0; i < size; i++) {
            SnakeSegment segment;
            char comma;
            file >> segment.x >> comma >> segment.y; // Читаем каждый сегмент змейки
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
    hWnd = CreateWindow(L"SnakeGame", L"Snake Game", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

    if (!hWnd) {
        return 0;
    }

    // Создаем кнопку "Restart"
    restartButton = CreateWindow(L"BUTTON", L"Restart", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 50, 100, 30, hWnd, (HMENU)IDC_RESTART_BUTTON, hInstance, NULL);
    if (!restartButton) {
        return 0;
    }

    // Устанавливаем процедуру окна для кнопки "Restart"
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

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT* pKB = (KBDLLHOOKSTRUCT*)lParam;
        if (pKB->vkCode == 'R') {
            RestartGame();
        }
    }
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

