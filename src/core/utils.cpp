#include <iomanip>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <array>
#include <unordered_map>

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

    RequestObj::RequestObj() : protocol{}, target{}, httpVersion{}, header{}, splitTarget{}, content{} {}

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

    uint32_t RequestObjHeader::getContentLength() {
        return std::stoi(contentLength);
    }

    void assignValue(RequestObj* requestObj, uint8_t objectiveValue, const std::string& currentData) {

        std::cout << "TO ADD: " + currentData << '\x0A';

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
            case 6:
                requestObj->header.contentLength = currentData;
                break;
            case 7:
                requestObj->header.accept = currentData;
                break;
            case 8:

                PRINT_SUCCESS(std::to_string(currentData.size()));

                if (currentData == "gzip\x0D") {
                    requestObj->header.acceptEncoding = currentData;
                    PRINT_HIGHLIGHT(requestObj->header.acceptEncoding);
                }
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

    void parseHeader(RequestObj* requestObj, const uint8_t* buffer, uint8_t& index, size_t& bufferSize) {

        const std::unordered_map<std::string, uint8_t> headers = {
            {"Host:", 3},
            {"User-Agent:", 4},
            {"Content-Type:", 5},
            {"Content-Length:", 6},
            {"Accept:", 7},
            {"Accept-Encoding:", 8}
        };

        std::string currentData = {};
        uint8_t idxWord {};
        uint8_t objectiveValue {};
        bool isContentType {};

        bool parse {};

        while (bufferSize > index) {

            uint8_t c = buffer[index];
            currentData += c;
            if (headers.count(currentData) > 0) {
                objectiveValue = headers.at(currentData);
                parse = true;

                if (objectiveValue == 5) isContentType = true;

                currentData = "";
                index += 2;
                continue;



            }

            if (parse && (c == '\x20' || c == '\x0D')) {
                assignValue(requestObj, objectiveValue, currentData);
                parse = false;
                currentData = "";
                idxWord++;
                index++;
                objectiveValue++;
                if (isContentType) { index++; break;}
            }

            index++;
        }

        index += 2;

    }

    void parseRequestContent(RequestObj* requestObj, const uint8_t* buffer, uint8_t& index) {

        uint32_t contentLength = requestObj->header.getContentLength();

        while (contentLength--) {
            const uint8_t& c = buffer[index++];
            std::cout << c;
            requestObj->content += c;
        }
    }

    RequestObj* parseRequest(const uint8_t* buffer, size_t bufferSize) {

        RequestObj* requestObj = new RequestObj();
        uint8_t index {};

        (void)parseFirstPart(requestObj, buffer, index);
        (void)parseHeader(requestObj, buffer, index, bufferSize);

        if (requestObj->protocol == Types::POST) (void)parseRequestContent(requestObj, buffer, index);

        return requestObj;

    }

    std::string readFileContent(const std::filesystem::directory_entry& entry) {

        std::ifstream inputFile(entry.path());
        std::string fContent {};

        char ch {};

        while (inputFile.get(ch)) {
            fContent += ch;
        }

        return fContent;
    }

    void writeFileContent(const std::string& fileName, const std::string& fContent) {

        std::ofstream outputFile(fileName);

        outputFile << fContent;
        outputFile.close();

    }

}
