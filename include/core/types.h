#ifndef SERVER_TYPES_H
#define SERVER_TYPES_H

#include <string>

namespace Types {

    enum Protocol {
        GET,
        POST,
        PUT,
        DELETE,
        PATCH
    };

    Protocol getProtocol(const std::string& method);
}


#endif // SERVER_TYPES_H