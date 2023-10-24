#include <windows.h>
#include <winevt.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include <windows.ui.notifications.h>
#include <windows.data.xml.dom.h>

using namespace ABI::Windows::UI::Notifications;
using namespace ABI::Windows::Data::Xml::Dom;


// Идентификаторы элементов управления.
#define ID_FILTER_COMBOBOX 101
#define ID_EVENT_LISTBOX 102
#define ID_SAVE_BUTTON 103

EVT_HANDLE hQuery = NULL; // Обработчик запроса событий.

// Прототип функции окна.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Функция для сохранения настроек в файл.
void SaveSettings(int selectedFilter) {
    std::ofstream configFile("config.txt");

    if (configFile.is_open()) {
        configFile << selectedFilter;
        configFile.close();
    }
}

// Функция для загрузки настроек из файла.
int LoadSettings() {
    int selectedFilter = 0;
    std::ifstream configFile("config.txt");

    if (configFile.is_open()) {
        configFile >> selectedFilter;
        configFile.close();
    }

    return selectedFilter;
}

// Обработчик событий
void ProcessEvent(EVT_HANDLE hEvent, HWND hListBox, IToastNotifier* toastNotifier) {
    // Здесь можно добавить код для анализа и обработки события
    // В данном примере, события просто выводятся в список и отправляются уведомления Toast.

    std::wstring eventText = L"Событие не определено";

    if (hEvent != NULL) {
        DWORD bufferSize = 0;
        EvtRender(NULL, hEvent, EvtRenderEventXml, 0, NULL, &bufferSize, NULL);

        if (bufferSize > 0) {
            std::vector<wchar_t> buffer(bufferSize);
            EvtRender(NULL, hEvent, EvtRenderEventXml, bufferSize, &buffer[0], &bufferSize, NULL);
            eventText = &buffer[0];
        }
    }

    SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)eventText.c_str());

    // Отправка уведомления Toast.
    // (код для отправки уведомления оставлен без изменений)
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Регистрация класса окна.
    const wchar_t* className = L"MyWin32AppClass";

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, L"Не удалось зарегистрировать класс окна.", L"Ошибка", MB_ICONERROR);
        return 1;
    }

    // Создание окна.
    HWND hwnd = CreateWindow(className, L"Мое Win32 приложение", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, L"Не удалось создать окно.", L"Ошибка", MB_ICONERROR);
        return 1;
    }

    // Создание выпадающего списка для фильтров.
    CreateWindow(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 10, 10, 150, 200, hwnd, (HMENU)ID_FILTER_COMBOBOX, hInstance, NULL);
    SendMessage(GetDlgItem(hwnd, ID_FILTER_COMBOBOX), CB_ADDSTRING, 0, (LPARAM)L"Все события");
    SendMessage(GetDlgItem(hwnd, ID_FILTER_COMBOBOX), CB_ADDSTRING, 0, (LPARAM)L"Информация");
    SendMessage(GetDlgItem(hwnd, ID_FILTER_COMBOBOX), CB_ADDSTRING, 0, (LPARAM)L"Предупреждение");
    SendMessage(GetDlgItem(hwnd, ID_FILTER_COMBOBOX), CB_ADDSTRING, 0, (LPARAM)L"Ошибка");

    // Создание кнопки для сохранения настроек.
    CreateWindow(L"BUTTON", L"Сохранить настройки", WS_CHILD | WS_VISIBLE, 170, 10, 150, 30, hwnd, (HMENU)ID_SAVE_BUTTON, hInstance, NULL);

    // Создание списка для вывода событий.
    CreateWindow(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | LBS_STANDARD | LBS_NOTIFY, 10, 50, 760, 500, hwnd, (HMENU)ID_EVENT_LISTBOX, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    // Загрузка настроек пользователя.
    int selectedFilter = LoadSettings();

    // Установка выбранного фильтра в выпадающем списке.
    SendMessage(GetDlgItem(hwnd, ID_FILTER_COMBOBOX), CB_SETCURSEL, selectedFilter, 0);

    // Создание нового запроса на основе выбранного фильтра.
    const wchar_t* query = L"*"; // По умолчанию, выбраны все события.
    switch (selectedFilter) {
    case 1:
        query = L"Event[System[(Level=4)]]"; // Фильтр для информационных событий.
        break;
    case 2:
        query = L"Event[System[(Level=3)]]"; // Фильтр для предупреждений.
        break;
    case 3:
        query = L"Event[System[(Level=2)]]"; // Фильтр для ошибок.
        break;
    }

    // Создание нового запроса.
    hQuery = EvtQuery(NULL, NULL, query, EvtQueryChannelPath);

    if (hQuery == NULL) {
        MessageBox(NULL, L"Не удалось создать запрос.", L"Ошибка", MB_ICONERROR);
    }

    MSG msg = { 0 };

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Закрытие запроса и сохранение настроек пользователя перед завершением программы.
    if (hQuery != NULL) {
        EvtClose(hQuery);
    }

    // Сохранение настроек пользователя перед завершением.
    selectedFilter = SendMessage(GetDlgItem(hwnd, ID_FILTER_COMBOBOX), CB_GETCURSEL, 0, 0);
    SaveSettings(selectedFilter);

    return static_cast<int>(msg.wParam);
}

// Обработчик сообщений окна.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hListBox = NULL; // Обработчик списка событий.

    switch (uMsg) {
    case WM_CREATE:
        // Сохранение обработчика списка событий.
        hListBox = CreateWindow(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | LBS_STANDARD | LBS_NOTIFY, 10, 50, 760, 500, hwnd, (HMENU)ID_EVENT_LISTBOX, NULL, NULL);
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_FILTER_COMBOBOX) {
            // Обработка выбора фильтра в выпадающем списке.

            // Очистка списка событий перед загрузкой новых.
            SendMessage(hListBox, LB_RESETCONTENT, 0, 0);

            // Закрытие предыдущего запроса, если он был.
            if (hQuery != NULL) {
                EvtClose(hQuery);
            }

            // Создание нового запроса в зависимости от выбранного фильтра.
            const wchar_t* query = L"*"; // По умолчанию, выбраны все события.
            switch (SendMessage(GetDlgItem(hwnd, ID_FILTER_COMBOBOX), CB_GETCURSEL, 0, 0)) {
            case 1:
                query = L"Event[System[(Level=4)]]"; // Фильтр для информационных событий.
                break;
            case 2:
                query = L"Event[System[(Level=3)]]"; // Фильтр для предупреждений.
                break;
            case 3:
                query = L"Event[System[(Level=2)]]"; // Фильтр для ошибок.
                break;
            }

            // Создание нового запроса.
            hQuery = EvtQuery(NULL, NULL, query, EvtQueryChannelPath);

            if (hQuery == NULL) {
                MessageBox(NULL, L"Не удалось создать запрос.", L"Ошибка", MB_ICONERROR);
            }
        }
        else if (LOWORD(wParam) == ID_SAVE_BUTTON) {
            // Обработка нажатия кнопки "Сохранить настройки".

            int selectedFilter = SendMessage(GetDlgItem(hwnd, ID_FILTER_COMBOBOX), CB_GETCURSEL, 0, 0);
            SaveSettings(selectedFilter);
            MessageBox(NULL, L"Настройки сохранены.", L"Успех", MB_ICONINFORMATION);
        }
        break;

    case WM_DESTROY:
        // Закрытие запроса и завершение программы.
        if (hQuery != NULL) {
            EvtClose(hQuery);
        }
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}
