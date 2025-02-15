#include <cstdint>

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
        serverFd {}, connBacklog {connBacklog}, port{port} {
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

    void Server::handleResponse(uint16_t clientFD) {
        uint8_t buffer[CoreUtils::BUFFER_SIZE];
        uint16_t bytesReceived = recv(clientFD, buffer, CoreUtils::BUFFER_SIZE, 0);

        if (bytesReceived < 0) return;

        CoreUtils::printBuffer(buffer, bytesReceived);
        CoreUtils::RequestObj* requestObj = CoreUtils::parseRequest(buffer, bytesReceived);

        Core::Sender sender = Core::Sender(clientFD);

        if (requestObj->target != "/") {
            sender.sendNotFound();
        } else {
            sender.sendOk();
        }
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


