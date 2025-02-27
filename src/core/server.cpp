#include <cstdint>
#include <vector>

#include <utils.h>

#include <core/server.h>
#include <core/sender.h>
#include <core/exceptions.h>
#include <core/utils.h>
#include <thread>
#include <filesystem>

#include <core/types.h>

namespace Core {

    Server* Server::createServer(uint32_t argc, char** argv) {
        Server* server = new Server();

        server->handleArguments(argc, argv);

        return server;
    }

    Server* Server::createServer(uint8_t connBacklog, uint16_t port) {
        return new Server(connBacklog, port);
    }

    Server::Server(uint8_t connBacklog, uint16_t port) :
        serverFd {},
        connBacklog {connBacklog},
        port {port},
        dirName {},
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
            },
            {
                FILES, [this](CoreUtils::RequestObj* obj, Core::Sender* sender) {
                    return this->handleFiles(obj, sender);
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

    void Server::handleFiles(CoreUtils::RequestObj* obj, Core::Sender* sender) {

        std::string fileObj = obj->splitTarget[obj->splitTarget.size() - 1];
        std::string fileName = dirName + "/" + fileObj;

        // Loop through the files:
        switch (obj->protocol) {
            case Types::Protocol::GET:
                for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(dirName)) {

                    if (!entry.is_regular_file() || entry.path().filename() != fileObj) continue;

                    std::string fContent = CoreUtils::readFileContent(entry);
                    std::string msg = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length:" + std::to_string(fContent.size()) + "\r\n\r\n" + fContent;

                    std::cout << msg << '\x0A';
                    CoreUtils::ReturnObject* rObj = new CoreUtils::ReturnObject(msg);
                    (void)sender->sendResponse(rObj);

                    return;
                }
                break;

            case Types::Protocol::POST:
                (void)CoreUtils::writeFileContent(fileName, obj->content);
                sender->sendCreated();
                return;

            default:
                break;
        }


        sender->sendNotFound();
    }

    void Server::handleResponse(uint16_t clientFD) {

        uint8_t buffer[CoreUtils::BUFFER_SIZE];
        uint16_t bytesReceived = recv(clientFD, buffer, CoreUtils::BUFFER_SIZE, 0);

        if (bytesReceived < 0) return;

        CoreUtils::RequestObj* requestObj = CoreUtils::parseRequest(buffer, bytesReceived);

        Core::Sender* sender = new Core::Sender(clientFD);
        // Sendin OK here, because the absence of a route means that we want the
        // route of the server, which is just "/".
        if (requestObj->splitTarget.size() == 1) { (void)sender->sendOk(); return; }

        const std::string route = requestObj->splitTarget[1];

        // If we cannot route the method, return not found.
        if (methodRouter.find(route) == methodRouter.end()) return (void)sender->sendNotFound();
        (void)methodRouter[route](requestObj, sender);
    }

    void Server::handleArguments(uint32_t argc, char** argv) {

        for (uint32_t i = 1; i < argc; i++) {
            std::string flag = reinterpret_cast<char*>(argv[i]);
            std::string value = reinterpret_cast<char*>(argv[++i]);

            if (flag == "--directory") dirName = value;
        }

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


