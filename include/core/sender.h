#ifndef SENDER_H
#define SENDER_H

#include <cstdint>

#include <core/utils.h>


namespace Core {

    class Sender {
        public:

            Sender(uint16_t clientFD);
            ~Sender();

            void sendResponse(CoreUtils::ReturnObject* rObj);
            void sendNotFound();
            void sendOk();

        private:
            uint16_t clientFD;
    };

}

#endif // SENDER_H