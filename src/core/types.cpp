#include <string>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>

#include <core/types.h>

namespace Types {

    Protocol getProtocol(const std::string& method) {
        static const std::unordered_map<std::string, Protocol> stringToProtocol {
            {"get", GET},
            {"post", POST},
            {"put", PUT},
            {"delete", DELETE},
            {"patch", PATCH}
        };

        std::string lowerMethod = method;
        std::transform(lowerMethod.begin(), lowerMethod.end(), lowerMethod.begin(), ::tolower);

        auto it = stringToProtocol.find(lowerMethod);
        if (it != stringToProtocol.end()) {
            return it->second;
        }

        throw std::runtime_error("Invalid method");
    }

}