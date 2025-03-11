# Lite Server

A lightweight, multi-threaded HTTP server implemented in C using Windows Sockets (Winsock2). This server demonstrates basic network programming concepts including socket programming, multi-threading, and HTTP protocol handling.

## Features

- Multi-threaded request handling
- Basic HTTP/1.1 protocol support
- Windows platform support using Winsock2
- Simple error logging system
- Configurable port (default: 8081)

## Prerequisites

- Windows operating system
- C compiler (e.g., MinGW, Visual Studio)
- Windows SDK (for Winsock2 and threading support)

## Building the Project

### Using Visual Studio

1. Open the project in Visual Studio
2. Ensure `ws2_32.lib` is linked (already configured in the source)
3. Build the project

### Using MinGW

```bash
gcc -o bin/server src/server.c -lws2_32 -lwinmm
```

## Running the Server

1. Execute the compiled binary:
   ```bash
   ./bin/server
   ```
2. The server will start and listen on port 8081
3. You should see the message: "Server listening on port 8081"

## Testing the Server

1. Open a web browser and navigate to:
   ```
   http://localhost:8081
   ```
2. You should receive a "Hello from C Server!" message

## Implementation Details

- Uses `winsock2.h` for network operations
- Creates a new thread for each client connection
- Implements basic HTTP response handling
- Includes error logging for debugging

## Error Handling

The server includes basic error handling for common scenarios:
- Socket creation failures
- Binding errors
- Connection acceptance issues
- Client communication errors

## Limitations

- Basic HTTP implementation (text/plain responses only)
- Windows-specific implementation
- Fixed response message
- No configuration options (hardcoded port)

## Future Improvements

- Add support for different content types
- Implement configuration file support
- Add request routing capabilities
- Improve error handling and logging
- Add support for other platforms

## License

This project is open source and available for educational purposes.