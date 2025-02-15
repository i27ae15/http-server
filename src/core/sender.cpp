#include <cstdint>
#include <sys/socket.h>

#include <utils.h>

#include <core/utils.h>
#include <core/sender.h>


namespace Core {

    Sender::Sender(uint16_t clientFD) : clientFD {clientFD} {}
    Sender::~Sender() {}

    void Sender::sendResponse(
        CoreUtils::ReturnObject* rObj
    ) {
        if (!rObj->sendResponse) return;
        send(clientFD, rObj->rValue.c_str(), rObj->bytes, rObj->behavior);
    }

    void Sender::sendOk() {

        CoreUtils::ReturnObject* rObj = new CoreUtils::ReturnObject("HTTP/1.1 200 OK\r\n\r\n");
        sendResponse(rObj);

    }

    void Sender::sendNotFound() {

        CoreUtils::ReturnObject* rObj = new CoreUtils::ReturnObject("HTTP/1.1 404 Not Found\r\n\r\n");
        sendResponse(rObj);

    }

}