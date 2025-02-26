#include <cstdint>
#include <vector>

#include <utils.h>

#include <core/server.h>
#include <core/sender.h>
#include <core/exceptions.h>
#include <core/utils.h>
#include <thread>

namespace Core {

    Server* Server::createServer(uint8_t connBacklog, uint16_t port) {
        return new Server(connBacklog, port);
    }

    Server::Server(uint8_t connBacklog, uint16_t port) :
        serverFd {},
        connBacklog {connBacklog},
        port {port},
        methodRouter{
            {
                ECHO, [this](CoreUtils::RequestObj* obj, Core::Sender* sender) {
                    return this->handleEcho(obj, sender);
                }
            },
            {
                USER_AGENT, [this](CoreUtils::RequestObj* obj, Core::Sender* sender) {
                    return this->handleUserAgent(obj, sender);
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

        const std::string toEcho = obj->splitTarget[2];
        std::string msg = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length:" + std::to_string(toEcho.size()) + "\r\n\r\n" + toEcho;

        CoreUtils::ReturnObject* rObj = new CoreUtils::ReturnObject(msg);
        sender->sendResponse(rObj);
    }

    void Server::handleUserAgent(CoreUtils::RequestObj* obj, Core::Sender* sender) {

        std::string msg = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length:" + std::to_string(obj->header.userAgent.size() - 1) + "\r\n\r\n" + obj->header.userAgent;
        CoreUtils::ReturnObject* rObj = new CoreUtils::ReturnObject(msg);
        sender->sendResponse(rObj);
    }

    void Server::handleResponse(uint16_t clientFD) {

        uint8_t buffer[CoreUtils::BUFFER_SIZE];
        uint16_t bytesReceived = recv(clientFD, buffer, CoreUtils::BUFFER_SIZE, 0);

        if (bytesReceived < 0) return;

        PRINT_HIGHLIGHT("HANDLING RESPONSE 1");


        // CoreUtils::printBuffer(buffer, bytesReceived);
        CoreUtils::RequestObj* requestObj = CoreUtils::parseRequest(buffer, bytesReceived);

        Core::Sender* sender = new Core::Sender(clientFD);
        // Sendin OK here, because the absence of a route means that we want the
        // route of the server, which is just "/".
        if (requestObj->splitTarget.size() == 1) { (void)sender->sendOk(); return; }

        PRINT_HIGHLIGHT("HANDLING RESPONSE 2");

        const std::string route = requestObj->splitTarget[1];

        // If we cannot route the method, return not found.
        if (methodRouter.find(route) == methodRouter.end()) return (void)sender->sendNotFound();
        (void)methodRouter[route](requestObj, sender);
    }


    void handleClient(uint16_t clientFD, Server* server) {
        server->handleResponse(clientFD);
        close(clientFD);
    }

    void startServer(Server* server) {

        while (true) {
            struct sockaddr_in clientAddr {};
            int clientAddrLen = sizeof(clientAddr);

            uint16_t clientFD = accept(
                server->serverFd,
                (struct sockaddr *) &clientAddr,
                (socklen_t *) &clientAddrLen
            );

            std::thread clientThread(handleClient, clientFD, server);
            clientThread.detach();
        }


        close(server->serverFd);
    }

}


