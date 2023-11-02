﻿#include <iostream>
#include <Winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }

    // Создание сокета для сервера
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // Устанавливаем адрес сервера
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345); // Порт сервера
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Принимаем подключения на всех доступных интерфейсах

    // Привязываем сокет к адресу и порту
    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Ожидание подключений
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Failed to listen for connections." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "The server is running and waiting for connections..." << std::endl;

    while (true) {
        // Принимаем подключение от клиента
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Failed to accept connection." << std::endl;
            continue;
        }

        std::cout << "The client is connected." << std::endl;

        char buffer[1024];
        int bytesReceived;

        while (true) {
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived <= 0) {
                std::cerr << "The connection to the client has been lost." << std::endl;
                break;
            }

            buffer[bytesReceived] = '\0';
            std::cout << "Client: " << buffer << std::endl;

            // Отправка ответа клиенту
            send(clientSocket, buffer, bytesReceived, 0);
        }

        closesocket(clientSocket);
        std::cout << "Waiting for a new connection..." << std::endl;
    }

    // Закрытие сокета и очистка Winsock
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
