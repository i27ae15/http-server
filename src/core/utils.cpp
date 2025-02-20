#include <iomanip>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <vector>

#include <utils.h>

#include <core/utils.h>


namespace CoreUtils {

    std::vector<std::string> split(const std::string var, char delimiter) {
        std::vector<std::string> toReturn {};
        std::stringstream ss(var);
        std::string token;

        while (std::getline(ss, token, delimiter)) {
            toReturn.push_back(token);
        }

        return toReturn;
    }

    RequestObj::RequestObj() : protocol{}, target{}, httpVersion{}, header{}, splitTarget{} {}

    RequestObj::RequestObj(
        Types::Protocol protocol,
        std::string target,
        std::string httpVersion,
        RequestObjHeader header
    ) : protocol{protocol}, target{target}, httpVersion{httpVersion}, header{header},
        splitTarget{split(target, '/')} {}

    void RequestObj::setSplitTarget() {
        if (splitTarget.size()) return;
        splitTarget = split(target, '/');
    }

    ReturnObject::ReturnObject(std::string rValue, uint8_t behavior, bool sendResponse) {
        this->rValue = rValue;
        this->behavior = behavior;
        this->bytes = rValue.size();
        this->sendResponse = sendResponse;
    }

    void assignValue(RequestObj* requestObj, uint8_t objectiveValue, const std::string& currentData) {

        switch (objectiveValue) {
            case 0:
                requestObj->protocol = Types::getProtocol(currentData);
                break;
            case 1:
                requestObj->target = currentData;
                requestObj->setSplitTarget();
                break;
            case 2:
                requestObj->httpVersion = currentData;
                break;
            case 3:
                requestObj->header.host = currentData;
                break;
            case 4:
                requestObj->header.userAgent = currentData;
                break;
            case 5:
                requestObj->header.mediaType = currentData;
                break;
            default:
                break;
        }
    }

    void printBuffer(const uint8_t* buffer, size_t bufferSize) {
        for (size_t i {}; i < bufferSize; i++) {
            std::cout << buffer[i];
        }

        std::cout << '\x0A';
    }

    void parseFirstPart(RequestObj* requestObj, const uint8_t* buffer, uint8_t& index) {

        std::string currentData {};
        uint8_t objectiveValue {};

        while ( objectiveValue < 3) {

            uint8_t c = buffer[index];
            index++;

            if (c == '\x20' || c == '\x0D') {
                assignValue(requestObj, objectiveValue, currentData);
                currentData = "";
                objectiveValue++;
                if (c == '\x0D') { index++; return; }
                continue;
            }

            currentData += c;

        }

    }

    void parseSecondPart(RequestObj* RequestObj, const uint8_t* buffer, uint8_t& index) {

        std::string toSearch[2] = {"Host", "User-Agent"};
        std::string currentData = {};
        uint8_t idxWord {};
        uint8_t objectiveValue {3};

        bool parse {};

        while (idxWord < toSearch->size() - 1) {

            uint8_t c = buffer[index];
            currentData += c;

            if (currentData == toSearch[idxWord]) {
                parse = true;
                currentData = "";
                index += 2;
            }

            if (parse && c == '\x20' || c == '\x0D') {
                assignValue(RequestObj, objectiveValue, currentData);
                parse = false;
                currentData = "";
                idxWord++;
                index++;
                objectiveValue++;
            }

            index++;
        }

    }

    RequestObj* parseRequest(const uint8_t* buffer, size_t bufferSize) {

        RequestObj* requestObj = new RequestObj();
        uint8_t index {};

        parseFirstPart(requestObj, buffer, index);
        parseSecondPart(requestObj, buffer, index);

        return requestObj;

    }

}
