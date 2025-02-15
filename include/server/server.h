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

#include <server/utils.h>

namespace Server {

    class Server {

        public:

        friend void startServer(Server* server);

        Server(uint8_t connBacklog, uint16_t port = 4221);
        ~Server();

        static Server* createServer(uint8_t connBacklog = 5, uint16_t port = 4221);

        void incrementBacklog();
        void decrementBacklog();

        private:

        void initServer();
        void sendResponse(uint16_t clientFD, ServerUtils::ReturnObject* rObj);

        int8_t serverFd;

        uint8_t connBacklog;
        uint16_t port;

        struct sockaddr_in serverAddr;
        struct sockaddr_in clientAddr;

    };

    void startServer(Server* server);

}

#endif // SERVER_H