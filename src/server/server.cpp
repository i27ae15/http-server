#include <cstdint>

#include <utils.h>

#include <server/server.h>
#include <server/exceptions.h>
#include <server/utils.h>

namespace Server {

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

    void Server::sendResponse(
        uint16_t clientFD,
        ServerUtils::ReturnObject* rObj
    ) {

        if (!rObj->sendResponse) return;
        send(clientFD, rObj->rValue.c_str(), rObj->bytes, rObj->behavior);
    }

    void startServer(Server* server) {

        // while (true) {
        int clientAddrLen = sizeof(server->clientAddr);

        uint16_t clientFD = accept(
            server->serverFd,
            (struct sockaddr *) &server->clientAddr,
            (socklen_t *) &clientAddrLen
        );

        ServerUtils::ReturnObject* rObj = new ServerUtils::ReturnObject("HTTP/1.1 200 OK\r\n\r\n");
        server->sendResponse(clientFD, rObj);

        // }

        close(server->serverFd);
    }

}


