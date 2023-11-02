#include <iostream>
#include <Winsock2.h>
#include <string>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }

    // Создание сокета для клиента
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // Устанавливаем адрес сервера, к которому хотим подключиться
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345); // Порт сервера

    // Используем функцию inet_pton для преобразования IP-адреса
    if (inet_pton(AF_INET, "192.168.0.100", &serverAddress.sin_addr) != 1) {
        std::cerr << "Invalid IP address." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Подключение к серверу
    if (connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::string message;
    while (true) {
        std::cout << "Введите сообщение (или 'exit' для выхода): ";
        std::getline(std::cin, message);

        // Отправка сообщения на сервер
        send(clientSocket, message.c_str(), static_cast<int>(message.size()), 0);

        if (message == "exit") {
            break;
        }

        char buffer[1024];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Error in receiving data from server." << std::endl;
            break;
        }

        buffer[bytesReceived] = '\0';
        std::cout << "Сервер: " << buffer << std::endl;
    }

    // Закрытие сокета и очистка Winsock
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
