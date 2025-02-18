#include <cstdint>
#include <vector>

#include <utils.h>

#include <core/server.h>
#include <core/sender.h>
#include <core/exceptions.h>
#include <core/utils.h>

namespace Core {

    Server* Server::createServer(uint8_t connBacklog, uint16_t port) {
        return new Server(connBacklog, port);
    }

    Server::Server(uint8_t connBacklog, uint16_t port) :
        serverFd {},
        connBacklog {connBacklog},
        port{port},
        methodRouter{
            {
                ECHO, [this](CoreUtils::RequestObj* obj, Core::Sender* sender) {
                    return this->handleEcho(obj, sender);
                }
            }
        }
        {
            initServer();
        }

    Server::~Server() {}

    void Server::initServer() {

        serverFd = socket(AF_INET, SOCK_STREAM, 0);
        if (serverFd < 0) throw ServerException::SocketConnFailed();

        // Since the tester restarts your program quite often, setting SO_REUSEADDR
        // ensures that we don't run into 'Address already in use' errors
        int reuse = 1;
        if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
            throw ServerException::SetSockOptConnFailed();
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (bind(serverFd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) != 0) {
            throw ServerException::BinToPortFailed(port);
        }

        if (listen(serverFd, connBacklog) != 0) ServerException::ListenFailed();

        PRINT_SUCCESS("SERVER INITIATED");
    }

    void Server::handleEcho(CoreUtils::RequestObj* obj, Core::Sender* sender) {

        const std::string toEcho = obj->splitTarget[1];
        std::string msg = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length:" + std::to_string(toEcho.size()) + "\r\n\r\n" + toEcho;

        CoreUtils::ReturnObject* rObj = new CoreUtils::ReturnObject(msg);
        sender->sendResponse(rObj);
    }

    void Server::handleResponse(uint16_t clientFD) {
        uint8_t buffer[CoreUtils::BUFFER_SIZE];
        uint16_t bytesReceived = recv(clientFD, buffer, CoreUtils::BUFFER_SIZE, 0);

        if (bytesReceived < 0) return;

        CoreUtils::printBuffer(buffer, bytesReceived);
        CoreUtils::RequestObj* requestObj = CoreUtils::parseRequest(buffer, bytesReceived);

        Core::Sender* sender = new Core::Sender(clientFD);
        // Sendin OK here, because the absence of a route means that we want the
        // route of the server, which is just "/".
        if (!requestObj->splitTarget.size()) { (void)sender->sendOk(); return; }

        const std::string route = requestObj->splitTarget[0];

        // If we cannot route the method, return not found.
        if (methodRouter.find(route) == methodRouter.end()) return (void)sender->sendNotFound();
        (void)methodRouter[route](requestObj, sender);
    }

    void startServer(Server* server) {

        int clientAddrLen = sizeof(server->clientAddr);

        uint16_t clientFD = accept(
            server->serverFd,
            (struct sockaddr *) &server->clientAddr,
            (socklen_t *) &clientAddrLen
        );

        server->handleResponse(clientFD);

        close(server->serverFd);
    }

}


