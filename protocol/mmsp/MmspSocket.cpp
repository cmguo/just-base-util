// MmspSocket.cpp

#include "util/Util.h"
#include "util/protocol/mmsp/MmspSocket.h"
#include "util/protocol/mmsp/MmspMessage.hpp"
#include "util/protocol/mmsp/MmspSocket.hpp"

namespace util
{
    namespace protocol
    {

        MmspSocket::MmspSocket(
            boost::asio::io_service & io_svc)
            : MessageSocket(io_svc, read_parser_)
        {
        }

        MmspSocket::~MmspSocket()
        {
        }

        void MmspSocket::tick(
            std::vector<MmspMessage> & resp)
        {
        }

        bool MmspSocket::process_protocol_message(
            MmspMessage const & msg, 
            std::vector<MmspMessage> & resp)
        {
            switch (msg.MID) {
                default:
                    return false;
            }
            return true;
        }

    } // namespace protocol
} // namespace util
