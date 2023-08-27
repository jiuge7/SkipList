#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

const int BUFFER_SIZE = 4096; // 缓冲区大小

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        return -1;
    }

    // 设置服务器的IP地址和端口号
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8989);
    if (inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr)) <= 0) {
        std::cerr << "Invalid address." << std::endl;
        return -1;
    }

    // 连接服务器
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Failed to connect to server." << std::endl;
        return -1;
    }

    while (true) {
        // 发送消息
        std::cout << "Enter Command: ";
          
        char message[BUFFER_SIZE];
        std::cin.getline(message, BUFFER_SIZE);
        ssize_t result = send(clientSocket, message, strlen(message), 0);
        if (result == -1) {
            std::cerr << "Failed to send Command." << std::endl;
            break;
        }

        // 接收回应
        char buffer[BUFFER_SIZE];
        result = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (result == -1) {
            std::cerr << "Failed to receive response." << std::endl;
            break;
        }
        buffer[result] = '\0';
        std::cout << buffer << std::endl;
    }

    close(clientSocket);
    return 0;
}
