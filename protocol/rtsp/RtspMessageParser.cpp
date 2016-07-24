// RtspMessageParser.cpp

#include "util/Util.h"
#include "util/protocol/rtsp/RtspMessageParser.h"
#include "util/protocol/rtsp/RtspMessage.h"
#include "util/protocol/rtsp/RtspMessageData.h"
#include "util/protocol/Message.hpp"

namespace util
{
    namespace protocol
    {

        RtspMessageParser::RtspMessageParser(
            RtspMessageContext & ctx)
            : data_def_(data_def_.data_message)
            , ctx_(ctx)
        {
        }

        void RtspMessageParser::parse(
            boost::asio::const_buffer const & buf)
        {
            char const * b = 
                boost::asio::buffer_cast<char const *>(buf);
            char const * e = 
                b + boost::asio::buffer_size(buf);
            (void)e;
            switch (step_) {
                case 0:
                    size_ = 8;
                    step_ = 1;
                    break;
                case 1:
                    if (*b == '$') {
                        ok_ = true;
                        step_ = 3;
                        size_ = 4 + (boost::uint16_t)b[2] * 256 + (boost::uint8_t)b[3];
                        msg_def_ = &data_def_;
                        break;
                    } else {
                        if (b[4] == '/') {
                            ctx_.read_type = RtspMessageType::RESPONSE;
                        } else {
                            ctx_.read_type = RtspMessageType::REQUEST;
                        }
                        msg_def_ = RtspMessage::find_msg(ctx_.read_type);
                        step_ = 2;
                    }
                case 2:
                    if (e[-1] == '\n') {
                        if (e[-3] == '\n') {
                            while (b != e) {
                                if (strncasecmp(b, "Content-Length:", 15) == 0) {
                                    b += 15;
                                    while (*b == ' ') ++b;
                                    size_t len = atol(b);
                                    size_ += len;
                                    break;
                                }
                                b = strchr(b, '\n') + 1;
                            }
                            ok_ = true;
                            step_ = 3;
                        } else {
                            size_ += 2;
                        }
                    } else if (e[-1] == '\r') {
                        if (e[-3] == '\r') {
                            size_ += 1;
                        } else {
                            size_ += 3;
                        }
                    } else {
                        size_ += 4;
                    }
                    break;
                default:
                    assert(false);
            }
        }

    } // namespace protocol
} // namespace util
