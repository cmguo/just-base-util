// MmspMessageParser.cpp

#include "util/Util.h"
#include "util/protocol/mmsp/MmspMessageParser.h"
#include "util/protocol/mmsp/MmspMessage.h"
#include "util/protocol/mmsp/MmspData.h"

namespace util
{
    namespace protocol
    {

        MmspMessageParser::MmspMessageParser()
            : data_def_(data_def_.data_message)
        {
        }

        void MmspMessageParser::parse(
            boost::asio::const_buffer const & buf)
        {
            boost::uint8_t const * b = 
                boost::asio::buffer_cast<boost::uint8_t const *>(buf);
            boost::uint8_t const * e = 
                b + boost::asio::buffer_size(buf);
            switch (step_) {
                case 0:
                    size_ = 8;
                    step_ = 1;
                    break;
                case 1:
                    {
                        boost::uint32_t id;
                        memcpy(&id, b + 4, 4);
#ifdef BOOST_BIG_ENDIAN
                        if (id == 0xCEFA0BB0) { // sessionId
#else
                        if (id == 0xB00BFACE) { // sessionId
#endif
                            size_ = MmspTcpMessageHeader::HEAD_SIZE + MmspMessageHeader::HEAD_SIZE;
                            step_ = 2;
                        } else {
                            boost::uint16_t len;
                            memcpy(&len, b + 6, 2);
                            len = framework::system::BytesOrder::little_endian_to_host(len);
                            size_ = len;
                            msg_def_ = &data_def_;
                            step_ = 3;
                            ok_ = true;
                        }
                    }
                    break;
                case 2:
                    {
                        boost::uint32_t len;
                        memcpy(&len, b + 8, 4);
                        len = framework::system::BytesOrder::little_endian_to_host(len);
                        size_ = len + 16;

                        boost::uint32_t mid;
                        memcpy(&mid, b + 36, 4);
                        mid = framework::system::BytesOrder::little_endian_to_host(mid);
                        msg_def_ = MmspMessage::find_msg(mid);
                        assert(msg_def_);
                    }
                    ok_ = true;
                    step_ = 3;
                    break;
                default:
                    assert(false);
            }
        }

    } // namespace protocol
} // namespace util
