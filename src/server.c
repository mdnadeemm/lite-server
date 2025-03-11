#include <stdio.h>      // Standard input/output (printf, etc.)
#include <stdlib.h>     // Standard library (exit, etc.)
#include <string.h>     // String operations (strlen, memset, etc.)
#include <winsock2.h>   // Winsock API for socket programming on Windows
#include <windows.h>    // Windows API for threading

#pragma comment(lib, "ws2_32.lib")  // Automatically link Winsock library (Visual Studio only)

/**
 * Logs an error message with the corresponding error code.
 * @param msg Descriptive error message
 * @param error_code System error code (e.g., from WSAGetLastError or GetLastError)
 */
void log_error(const char *msg, int error_code) {
    printf("ERROR: %s (Code: %d)\n", msg, error_code);
}

/**
 * Thread function to handle a single client connection.
 * Reads the request, sends an HTTP response, and closes the connection.
 * @param arg Client socket passed as a void pointer (cast to SOCKET)
 * @return Thread exit code (0 for success)
 */
DWORD WINAPI handle_client(LPVOID arg) {
    SOCKET client_fd = (SOCKET)arg;  // Cast the argument to a SOCKET type
    char buffer[1024] = {0};         // Buffer to store client request (zero-initialized)
    int bytes_received;              // Number of bytes received from client

    // Receive data from the client (up to 1024 bytes)
    bytes_received = recv(client_fd, buffer, 1024, 0);
    if (bytes_received == SOCKET_ERROR) {
        // If recv fails, log the error with Winsock error code
        log_error("Receive failed", WSAGetLastError());
    } else if (bytes_received > 0) {
        // If data was received, print it
        printf("Client sent: %s\n", buffer);

        // Define a basic HTTP response
        const char *response =
            "HTTP/1.1 200 OK\r\n"           // Status line: HTTP version and success code
            "Content-Type: text/plain\r\n"  // Header: Response is plain text
            "Content-Length: 20\r\n"        // Header: Length of body (20 bytes for "Hello from C Server!")
            "Connection: close\r\n"         // Header: Close connection after response
            "\r\n"                          // Empty line separating headers and body
            "Hello from C Server!";         // Response body

        // Send the response to the client
        if (send(client_fd, response, strlen(response), 0) == SOCKET_ERROR) {
            // If send fails, log the error
            log_error("Send failed", WSAGetLastError());
        }
    }

    // Close the client socket to end the connection
    closesocket(client_fd);
    return 0;  // Thread exits successfully
}

/**
 * Main function: Sets up a server, listens for connections, and spawns threads for clients.
 */
int main() {
    WSADATA wsa_data;                    // Structure to hold Winsock initialization data
    SOCKET server_fd = INVALID_SOCKET;   // Server socket (initialized to invalid)
    SOCKET client_fd;                    // Client socket (assigned per connection)
    struct sockaddr_in server_addr;      // Server address structure (IP and port)
    struct sockaddr_in client_addr;      // Client address structure (filled by accept)
    int client_len = sizeof(client_addr); // Size of client address structure

    // Initialize Winsock (version 2.2)
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        log_error("WSAStartup failed", WSAGetLastError());
        exit(EXIT_FAILURE);  // Exit if initialization fails
    }

    // Create a TCP socket (AF_INET for IPv4, SOCK_STREAM for TCP)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        log_error("Socket creation failed", WSAGetLastError());
        WSACleanup();  // Cleanup Winsock resources
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;          // Use IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Bind to any available network interface
    server_addr.sin_port = htons(8081);        // Port 8081 (htons converts to network byte order)

    // Bind the socket to the address and port
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        log_error("Bind failed", WSAGetLastError());
        closesocket(server_fd);  // Close socket on failure
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections (backlog of 5 queued connections)
    if (listen(server_fd, 5) == SOCKET_ERROR) {
        log_error("Listen failed", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port 8081\n");

    // Infinite loop to accept client connections
    while (1) {
        // Accept a client connection, filling client_addr with client info
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == INVALID_SOCKET) {
            log_error("Accept failed", WSAGetLastError());
            continue;  // Skip to next iteration on failure
        }
        printf("Client connected\n");

        // Create a thread to handle the client
        HANDLE thread = CreateThread(
            NULL,           // Default security attributes
            0,              // Default stack size
            handle_client,  // Thread function
            (LPVOID)client_fd,  // Pass client socket as argument
            0,              // Default creation flags (run immediately)
            NULL            // No thread ID stored
        );
        if (thread == NULL) {
            // If thread creation fails, log error and clean up client socket
            log_error("Thread creation failed", GetLastError());
            closesocket(client_fd);
            continue;
        }
        CloseHandle(thread);  // Detach thread to avoid resource leak
    }

    // Cleanup (unreachable due to infinite loop, but included for completeness)
    closesocket(server_fd);
    WSACleanup();
    return 0;
}