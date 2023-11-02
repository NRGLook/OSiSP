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

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;

    std::string serverIP;
    int serverPort;
    std::cout << "Enter the server IP address: ";
    std::cin >> serverIP;
    std::cout << "Enter the server port: ";
    std::cin >> serverPort;

    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr) != 1) {
        std::cerr << "Invalid IP address." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    serverAddress.sin_port = htons(serverPort);

    if (connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to the server." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::string message;
    while (true) {
        std::cout << "Enter a message (or 'exit' to exit): ";
        std::getline(std::cin, message);

        send(clientSocket, message.c_str(), static_cast<int>(message.size()), 0);

        if (message == "exit") {
            break;
        }
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
