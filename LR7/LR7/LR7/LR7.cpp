#include <windows.h>
#include <bluetoothapis.h>
#include <winsock2.h>
#include <ws2bth.h>
#include <iostream>
#include <string>
#pragma comment(lib, "ws2_32.lib")

SOCKET clientSocket = INVALID_SOCKET;

// Функция для установки Bluetooth-соединения с устройством
bool ConnectToBluetoothDevice(BLUETOOTH_DEVICE_INFO deviceInfo) {
    SOCKADDR_BTH sa = { 0 };
    sa.addressFamily = AF_BTH;
    sa.btAddr = deviceInfo.Address.ullLong;

    clientSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Ошибка при создании сокета." << std::endl;
        return false;
    }

    if (connect(clientSocket, (SOCKADDR*)&sa, sizeof(sa)) == SOCKET_ERROR) {
        std::cerr << "Ошибка при установлении соединения." << std::endl;
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
        return false;
    }

    return true;
}

// Функция для отправки данных через Bluetooth-сокет
void SendDataOverBluetooth(const std::string& data) {
    if (clientSocket != INVALID_SOCKET) {
        int bytesSent = send(clientSocket, data.c_str(), data.length(), 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Ошибка при отправке данных." << std::endl;
        }
    }
}

// Функция для приема данных из Bluetooth-сокета
std::string ReceiveDataOverBluetooth() {
    std::string receivedData;
    char buffer[1024];
    int bytesRead;
    do {
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead > 0) {
            receivedData.append(buffer, bytesRead);
        }
    } while (bytesRead > 0);

    return receivedData;
}

// Функция для закрытия Bluetooth-соединения
void CloseBluetoothConnection() {
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Ошибка при инициализации Winsock." << std::endl;
        return 1;
    }

    BLUETOOTH_DEVICE_INFO deviceInfo; // Полученная информация об устройстве

    // Информация о Bluetooth-устройстве
    BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams = { sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS) };
    searchParams.hRadio = NULL; // Поиск на всех доступных радио-устройствах
    searchParams.fIssueInquiry = TRUE; // Выполнить запрос к устройству

    BLUETOOTH_DEVICE_INFO deviceInfo = { sizeof(BLUETOOTH_DEVICE_INFO) };
    deviceInfo.Address.ullLong = 0;

    // Поиск ближайшего Bluetooth-устройства
    HBLUETOOTH_DEVICE_FIND hFind;
    hFind = BluetoothFindFirstDevice(&searchParams, &deviceInfo);
    if (hFind != NULL) {
        // Подключение к найденному устройству
        if (ConnectToBluetoothDevice(deviceInfo)) {
            // Отправка данных
            std::string dataToSend = "Привет, это данные для отправки через Bluetooth!";
            SendDataOverBluetooth(dataToSend);

            // Прием данных
            std::string receivedData = ReceiveDataOverBluetooth();
            std::cout << "Полученные данные: " << receivedData << std::endl;

            CloseBluetoothConnection();
        }
        BluetoothFindDeviceClose(hFind);
    }
    else {
        std::cerr << "Не найдено Bluetooth-устройство." << std::endl;
    }

    WSACleanup();
    return 0;
}
