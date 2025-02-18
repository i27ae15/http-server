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

    class Sender;

    class Server {

        public:

        friend void startServer(Server* server);

        Server(uint8_t connBacklog, uint16_t port = 4221);
        ~Server();

        static Server* createServer(uint8_t connBacklog = 5, uint16_t port = 4221);

        void incrementBacklog();
        void decrementBacklog();
        void handleResponse(uint16_t clientFD);

        private:

        void initServer();

        void handleEcho(CoreUtils::RequestObj* obj, Sender* sender);

        int8_t serverFd;

        uint8_t connBacklog;
        uint16_t port;

        struct sockaddr_in serverAddr;
        struct sockaddr_in clientAddr;

        std::unordered_map<
        std::string,
        std::function<void(CoreUtils::RequestObj*, Sender*)>> methodRouter;
    };

    void startServer(Server* server);

}

#endif // SERVER_H