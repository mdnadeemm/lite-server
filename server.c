#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

void log_error(const char *msg, int error_code) {
    printf("ERROR: %s (Code: %d)\n", msg, error_code);
}

DWORD WINAPI handle_client(LPVOID arg) {
    SOCKET client_fd = (SOCKET)arg;
    char buffer[1024] = {0};
    int bytes_received;

    bytes_received = recv(client_fd, buffer, 1024, 0);
    if (bytes_received == SOCKET_ERROR) {
        log_error("Receive failed", WSAGetLastError());
    } else if (bytes_received > 0) {
        printf("Client sent: %s\n", buffer);

        // Simple HTTP response
        const char *response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 20\r\n"
            "Connection: close\r\n"
            "\r\n"
            "Hello from C Server!";
        if (send(client_fd, response, strlen(response), 0) == SOCKET_ERROR) {
            log_error("Send failed", WSAGetLastError());
        }
    }

    closesocket(client_fd);
    return 0;
}

int main() {
    WSADATA wsa_data;
    SOCKET server_fd = INVALID_SOCKET, client_fd;
    struct sockaddr_in server_addr, client_addr;
    int client_len = sizeof(client_addr);

    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        log_error("WSAStartup failed", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        log_error("Socket creation failed", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8081);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        log_error("Bind failed", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) == SOCKET_ERROR) {
        log_error("Listen failed", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port 8081\n");

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == INVALID_SOCKET) {
            log_error("Accept failed", WSAGetLastError());
            continue;
        }
        printf("Client connected\n");

        HANDLE thread = CreateThread(NULL, 0, handle_client, (LPVOID)client_fd, 0, NULL);
        if (thread == NULL) {
            log_error("Thread creation failed", GetLastError());
            closesocket(client_fd);
            continue;
        }
        CloseHandle(thread);
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}