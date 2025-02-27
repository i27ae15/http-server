#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstdint>
#include <unordered_map>
#include <functional>

#include <core/utils.h>

namespace Core {

    const constexpr char* ECHO = "echo";
    const constexpr char* USER_AGENT = "user-agent";
    const constexpr char* FILES = "files";

    class Sender;

    class Server {

        public:

        friend void startServer(Server* server);
        friend void handleClient(uint16_t clientFD, Server* server);

        Server(uint8_t connBacklog = 10, uint16_t port = 4221);
        ~Server();

        static Server* createServer(uint32_t argc, char** argv);
        static Server* createServer(uint8_t connBacklog = 5, uint16_t port = 4221);

        void handleResponse(uint16_t clientFD);
        void handleArguments(uint32_t argc, char** argv);

        private:

        void initServer();

        void handleEcho(CoreUtils::RequestObj* obj, Sender* sender);
        void handleUserAgent(CoreUtils::RequestObj* obj, Sender* sender);
        void handleFiles(CoreUtils::RequestObj* obj, Sender* sender);

        int8_t serverFd;

        uint8_t connBacklog;
        uint16_t port;

        struct sockaddr_in serverAddr;

        std::unordered_map<
        std::string,
        std::function<void(CoreUtils::RequestObj*, Sender*)>> methodRouter;
        std::string dirName;
    };

    void startServer(Server* server);
    void handleClient(uint16_t clientFD, Server* server);

}

#endif // SERVER_H