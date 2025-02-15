#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <cstdint>
#include <string>
#include <vector>

namespace ServerUtils {
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
}

#endif // SERVER_UTILS_H