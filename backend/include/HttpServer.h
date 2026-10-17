#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <string>
#include <winsock2.h>
#include "ExchangeManager.h"

/**
 * @brief Lightweight Native Windows Sockets HTTP Server
 * Serves the responsive web frontend and exposes JSON REST endpoints
 * directly connecting to the in-memory C++ Data Structures.
 */
class HttpServer {
private:
    int port;
    ExchangeManager& manager;
    SOCKET listenSocket;
    bool isRunning;

    void handleClient(SOCKET clientSocket);
    std::string handleApiRequest(const std::string& method,
                                 const std::string& path,
                                 const std::string& query,
                                 const std::string& body);
    void sendResponse(SOCKET clientSocket,
                      int statusCode,
                      const std::string& contentType,
                      const std::string& content);
    void sendFile(SOCKET clientSocket,
                  const std::string& filepath,
                  const std::string& contentType);

public:
    HttpServer(int port, ExchangeManager& mgr);
    ~HttpServer();

    void start();
    void stop();
};

#endif // HTTP_SERVER_H
