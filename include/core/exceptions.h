#ifndef SERVER_EXCEPTION_H
#define SERVER_EXCEPTION_H

#include <exception>
#include <string>

#include <cstdint>

namespace ServerException {

    class BaseException : public std::exception {
        protected:
            std::string msg;

        public:
            explicit BaseException(const std::string& msg);
            const char* what() const noexcept override;

    };

    class SocketConnFailed : public BaseException {
        public:
            SocketConnFailed();
    };

    class SetSockOptConnFailed : public BaseException {
        public:
            SetSockOptConnFailed();
    };

    class BinToPortFailed : public BaseException {
        public:
            BinToPortFailed(std::string port);
            BinToPortFailed(uint16_t port);
    };

    class ListenFailed : public BaseException {
        public:
            ListenFailed();
    };

}

#endif // SERVER_EXCEPTION_H