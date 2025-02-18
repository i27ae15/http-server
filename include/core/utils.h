#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <cstdint>
#include <string>
#include <vector>

#include <core/types.h>

namespace CoreUtils {

    constexpr uint16_t BUFFER_SIZE = 1024;

    std::vector<std::string> split(const std::string var, char delimiter);

    struct ReturnObject {
        std::string rValue; ///< The return value as a string.
        size_t bytes; ///< The size of the return value in bytes. calculated as rValue.size().
        uint8_t behavior; ///< A behavior code associated with the return object.
        bool sendResponse; ///< Indicates whether a response should be sent.

        /**
         * @brief Constructs a ReturnObject with specified values.
         *
         * @param rValue The return value as a string.
         * @param behavior An optional behavior code (default: 0).
         * @param sendResponse An optional boolean indicating whether to send a response (default: true).
        */
        ReturnObject(std::string rValue, uint8_t behavior = 0, bool sendResponse = true);
    };

    struct RequestObjHeader {

        std::string host;
        std::string userAgent;
        std::string mediaType;

    };

    struct RequestObj {
        Types::Protocol protocol;
        std::string target;
        std::string httpVersion;
        std::vector<std::string> splitTarget;

        RequestObjHeader header;

        RequestObj();
        RequestObj(Types::Protocol protocol, std::string target, std::string httpVersion, RequestObjHeader header);

        void setSplitTarget();
    };

    RequestObj* parseRequest(const uint8_t* buffer, size_t bufferSize);

    void printBuffer(const uint8_t* buffer, size_t bufferSize);
}

#endif // SERVER_UTILS_H