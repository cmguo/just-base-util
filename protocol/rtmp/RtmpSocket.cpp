// RtmpSocket.cpp

#include "util/Util.h"
#include "util/protocol/rtmp/RtmpSocket.h"
#include "util/protocol/rtmp/RtmpMessage.hpp"
#include "util/protocol/rtmp/RtmpSocket.hpp"
#include "util/protocol/rtmp/RtmpMessageParser.h "

#include "util/serialization/Array.h"

namespace util
{
    namespace protocol
    {

        RtmpSocket::RtmpSocket(
            boost::asio::io_service & io_svc)
            : MessageSocket(io_svc, read_parser_)
        {
        }

        RtmpSocket::~RtmpSocket()
        {
        }

        void RtmpSocket::connect(
            framework::network::NetName const & addr)
        {
            boost::system::error_code ec;
            connect(addr, ec);
            boost::asio::detail::throw_error(ec);
        }

        boost::system::error_code RtmpSocket::connect(
            framework::network::NetName const & addr, 
            boost::system::error_code & ec)
        {
            TcpSocket::connect(addr ,ec);
            make_c01();
            boost::asio::write(*this, snd_buf_, boost::asio::transfer_all(), ec);
            boost::asio::read(*this, rcv_buf_, boost::asio::transfer_at_least(1 + HANDSHAKE_SIZE * 2), ec);
            make_c2();
            rcv_buf_.consume(1 + HANDSHAKE_SIZE * 2);
            boost::asio::write(*this, snd_buf_, boost::asio::transfer_all(), ec);
            return ec;
        }

        size_t RtmpSocket::write_msgs(
            std::vector<RtmpMessage> const & msgs, 
            boost::system::error_code & ec)
        {
            return write_msg(RtmpMessageVector(msgs), ec);
        }

        bool RtmpSocket::process_protocol_message(
            RtmpMessage const & msg, 
            std::vector<RtmpMessage> & resp)
        {
            switch (msg.type) {
                case RCMT_SetChunkSize:
                    read_parser_.set_chunk_size(msg.as<RtmpProtocolControlMessageSetChunkSize>().chunk_size);
                    break;
                case RCMT_AbortMessage:
                    //write_parser_.acknowledgement(msg.as<RtmpProtocolControlMessageAbortMessage>().sequence_number);
                    break;
                case RCMT_Acknowledgement:
                    write_parser_.acknowledgement(msg.as<RtmpProtocolControlMessageAcknowledgement>().sequence_number);
                    break;
                case RCMT_UserControl:
                    //write_parser_.acknowledgement(msg.as<RtmpProtocolControlMessageWindowAcknowledgementSize>().sequence_number);
                    break;
                case RCMT_WindowAcknowledgementSize:
                    //write_parser_.acknowledgement(msg.as<RtmpProtocolControlMessageWindowAcknowledgementSize>().sequence_number);
                    break;
                case RCMT_SetPeerBandwidth:
                    //write_parser_.acknowledgement(msg.as<RtmpProtocolControlMessageSetPeerBandwidth>().sequence_number);
                    break;
                default:
                    return false;
            }
            return true;
        }

         static void assgin_rand(
             boost::uint8_t & v)
         {
             v = (boost::uint8_t)rand();
         }

        void RtmpSocket::make_c01()
        {
            boost::uint8_t * c01 = boost::asio::buffer_cast<boost::uint8_t *>(snd_buf_.prepare(1 + HANDSHAKE_SIZE));
            *c01++ = 3; // version
            std::for_each(c01, c01 + HANDSHAKE_SIZE, assgin_rand);
            snd_buf_.commit(1 + HANDSHAKE_SIZE);
        }

        void RtmpSocket::make_c2()
        {
            boost::uint8_t * c2 = boost::asio::buffer_cast<boost::uint8_t *>(snd_buf_.prepare(HANDSHAKE_SIZE));
            std::for_each(c2, c2 + HANDSHAKE_SIZE, assgin_rand);
        }

        void RtmpSocket::make_s012()
        {
            boost::uint8_t * s012 = boost::asio::buffer_cast<boost::uint8_t *>(snd_buf_.prepare(1 + HANDSHAKE_SIZE * 2));
            *s012++ = 3; // version
            std::for_each(s012, s012 + HANDSHAKE_SIZE * 2, assgin_rand);
            snd_buf_.commit(1 + HANDSHAKE_SIZE * 2);
        }

    } // namespace protocol
} // namespace util
