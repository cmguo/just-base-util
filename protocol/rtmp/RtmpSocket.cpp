// RtmpSocket.cpp

#include "util/Util.h"
#include "util/protocol/rtmp/RtmpSocket.h"
#include "util/protocol/rtmp/RtmpMessage.hpp"
#include "util/protocol/rtmp/RtmpSocket.hpp"
#include "util/protocol/rtmp/RtmpMessageDataProtocolControl.h"
#include "util/protocol/rtmp/RtmpMessageDataUserControl.h"

namespace util
{
    namespace protocol
    {

        RtmpSocket::RtmpSocket(
            boost::asio::io_service & io_svc)
            : MessageSocket(io_svc, read_parser_, &context_)
            , status_(closed)
            , read_parser_(&context_)
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
            make_c01(snd_buf_);
            boost::asio::write(*this, snd_buf_.data(), boost::asio::transfer_all(), ec);
            snd_buf_.consume(snd_buf_.size());
            boost::asio::read(*this, rcv_buf_.prepare(1 + HANDSHAKE_SIZE * 2), boost::asio::transfer_all(), ec);
            rcv_buf_.commit(1 + HANDSHAKE_SIZE * 2);
            check_s012(rcv_buf_);
            make_c2(snd_buf_);
            rcv_buf_.consume(1 + HANDSHAKE_SIZE * 2);
            boost::asio::write(*this, snd_buf_.data(), boost::asio::transfer_all(), ec);
            snd_buf_.consume(snd_buf_.size());
            return ec;
        }

        size_t RtmpSocket::read_msg(
            RtmpMessage & msg, 
            boost::system::error_code & ec)
        {
            while (MessageSocket::read_msg(help_chunk_, ec)) {
                if (help_chunk_.finish) {
                    help_raw_chunk_.finish = false;
                    RtmpChunkMessage & cm(context_.read.chunk(help_chunk_.cs_id));
                    boost::uint32_t size = cm.data.size();
                    msg.from_data(cm.data, &context_);
                    return size;
                }
            }
            return 0;
        }

        size_t RtmpSocket::write_msgs(
            std::vector<RtmpMessage> const & msgs, 
            boost::system::error_code & ec)
        {
            return write_msg(RtmpMessageVector(msgs), ec);
        }

        void RtmpSocket::tick(
            std::vector<RtmpMessage> & resp)
        {
            boost::uint32_t n = context_.read.sequence() - context_.read.acknowledgement();
            if (n >= 128 * 1024) {
                n = context_.read.sequence();
                context_.read.acknowledgement(n);
                resp.push_back(RtmpMessage());
                resp.back().reset(RtmpMessageDataAcknowledgement(n));
            }
        }

        bool RtmpSocket::process_protocol_message(
            RtmpMessage const & msg, 
            std::vector<RtmpMessage> & resp)
        {
            switch (msg.type) {
                case RCMT_SetChunkSize:
                    context_.read.chunk_size(msg.as<RtmpMessageDataSetChunkSize>().chunk_size);
                    break;
                case RCMT_AbortMessage:
                    //context_.write.acknowledgement(msg.as<RtmpProtocolControlMessageAbortMessage>().sequence_number);
                    break;
                case RCMT_Acknowledgement:
                    context_.write.acknowledgement(msg.as<RtmpMessageDataAcknowledgement>().sequence_number);
                    break;
                case RCMT_UserControl:
                    //context_.write.acknowledgement(msg.as<RtmpMessageUserControl>()._union[0]);
                    break;
                case RCMT_WindowAcknowledgementSize:
                    //context_.write.acknowledgement(msg.as<RtmpMessageDataWindowAcknowledgementSize>().acknowledgement_window_size);
                    break;
                case RCMT_SetPeerBandwidth:
                    //context_.write.acknowledgement(msg.as<RtmpProtocolControlMessageSetPeerBandwidth>().sequence_number);
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

        void RtmpMessageVector::to_data(
            StreamBuffer & buf, 
            void * vctx) const
        {
            RtmpMessageContext * ctx = 
                reinterpret_cast<RtmpMessageContext *>(vctx);
            for (size_t i = 0; i < msgs_.size(); ++i) {
                boost::uint32_t stream = msgs_[i].stream;
                if (!ctx->write.stream_status(stream)) {
                    RtmpMessage msg;
                    RtmpMessageUserControl ctrl;
                    ctrl.event_type = RUCE_StreamBegin;
                    ctrl._union[0] = stream;
                    msg.reset(ctrl);
                    ctx->write.stream_begin(stream);
                    msg.to_data(buf, ctx);
                }
                msgs_[i].to_data(buf, ctx);
            }
        }

        void RtmpChunk::from_data(
            StreamBuffer & buf, 
            void * vctx)
        {
            RtmpMessageContext * ctx = (RtmpMessageContext *)vctx;
            boost::uint8_t const * p = 
                boost::asio::buffer_cast<boost::uint8_t const *>(buf.data());
            RtmpChunkBasicHeader h;
            h.one_byte = p[0];
            if (h.cs_id0 < 2) {
                if (h.cs_id0 == 0) {
                    h.cs_id1 = p[1];
                } else {
                    h.cs_id2 = (boost::uint16_t)p[1] << 8 | p[2];
                }
            }
            cs_id = h.cs_id();
            finish = ctx->read.chunk(cs_id).put_data(buf, ctx->read.chunk_size());
        }

    } // namespace protocol
} // namespace util
