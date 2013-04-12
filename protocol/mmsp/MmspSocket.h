// MmspSocket.h

#ifndef _UTIL_PROTOCOL_MMSP_MMSP_SOCKET_H_
#define _UTIL_PROTOCOL_MMSP_MMSP_SOCKET_H_

#include "util/protocol/mmsp/MmspMessage.h"
#include "util/protocol/mmsp/MmspMessageParser.h"
#include "util/protocol/MessageSocket.h"

namespace util
{
    namespace protocol
    {

        class MmspSocket
            : public MessageSocket
        {
        public:
            MmspSocket(
                boost::asio::io_service & io_svc);

            ~MmspSocket();

        protected:
            void tick(
                std::vector<MmspMessage> & resp);

        protected:
            bool process_protocol_message(
                MmspMessage const & msg, 
                std::vector<MmspMessage> & resp);

        private:
            MmspMessageParser read_parser_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMSP_MMSP_SOCKET_H_
