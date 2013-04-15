// MmspSocket.cpp

#include "util/Util.h"
#include "util/protocol/mmsp/MmspSocket.h"
#include "util/protocol/mmsp/MmspMessage.hpp"
#include "util/protocol/mmsp/MmspSocket.hpp"
#include "util/protocol/mmsp/MmspViewerToMacMessage.h"
#include "util/protocol/mmsp/MmspMacToViewerMessage.h"

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
            MmspMessage & resp)
        {
            switch (msg.MID) {
                case MmspMacToViewerMessage::PING:
                    msg.as<MmspDataPing>();
                    resp.get<MmspDataPong>();
                    return true;
                default:
                    return false;
            }
        }

    } // namespace protocol
} // namespace util
