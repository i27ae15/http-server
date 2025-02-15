#include <cstdint>

#include <server/utils.h>

namespace ServerUtils {
    ReturnObject::ReturnObject(std::string rValue, uint8_t behavior, bool sendResponse) {
        this->rValue = rValue;
        this->behavior = behavior;
        this->bytes = rValue.size();
        this->sendResponse = sendResponse;
    }
}
