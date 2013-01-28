// MmspSocket.cpp

#include "util/Util.h"
#include "util/protocol/mmsp/MmspSocket.h"
#include "util/protocol/mmsp/MmspSocket.hpp"

#include "util/serialization/Array.h"

namespace util
{
    namespace protocol
    {

        MmspSocket::MmspSocket(
            boost::asio::io_service & io_svc)
            : super(io_svc)
            , write_tcp_size_(0)
            , read_tcp_size_(0)
        {
        }

        void MmspSocket::close()
        {
            snd_buf_.reset();
            rcv_buf_.reset();
            super::close();
        }

        boost::system::error_code MmspSocket::close(
            boost::system::error_code & ec)
        {
            snd_buf_.reset();
            rcv_buf_.reset();
            return super::close(ec);
        }

        size_t MmspSocket::write(
            MmspMessage const & msg)
        {
            boost::system::error_code ec;
            size_t bytes_transferred = write(msg, ec);
            boost::asio::detail::throw_error(ec);
            return bytes_transferred;
        }

        size_t MmspSocket::write(
            MmspMessage const & msg, 
            boost::system::error_code & ec)
        {
            size_t bytes_transferred = 0;
            if (write_tcp_size_ == 0) {
                bytes_transferred = write_msg(msg, ec);
                if (ec) {
                    return bytes_transferred;
                }
            }
            write_tcp_msg(ec);
            return bytes_transferred;
        }

        size_t MmspSocket::read(
            MmspMessage & msg)
        {
            boost::system::error_code ec;
            size_t bytes_transferred = read(msg, ec);
            boost::asio::detail::throw_error(ec);
            return bytes_transferred;
        }

        size_t MmspSocket::read(
            MmspMessage & msg, 
            boost::system::error_code & ec)
        {
            if (read_tcp_size_ == 0) {
                read_tcp_msg(ec);
                if (ec) {
                    return 0;
                }
            }
            return read_msg(msg, ec);
        }

        size_t MmspSocket::write_msg(
            MmspMessage const & msg, 
            boost::system::error_code & ec)
        {
            if (write_tcp_size_ == 0) {
                snd_buf_.prepare(MmspTcpMessageHeader::HEAD_SIZE);
                snd_buf_.commit(MmspTcpMessageHeader::HEAD_SIZE);
            }

            MmspMessageHeader h(msg.id());

            snd_buf_.prepare(MmspMessageHeader::HEAD_SIZE);
            snd_buf_.commit(MmspMessageHeader::HEAD_SIZE);
            size_t bytes_transferred = MmspMessageHeader::HEAD_SIZE;

            std::map<boost::uint32_t, write_msg_t>::const_iterator iter = 
                write_msg_funcs_.find(h.MID);
            assert(iter != write_msg_funcs_.end());
            bytes_transferred -= snd_buf_.size();
            iter->second(snd_buf_, msg, ec);
            bytes_transferred += snd_buf_.size();

            char * buf = (char *)
                boost::asio::buffer_cast<char const *>(snd_buf_.data());
            buf += MmspTcpMessageHeader::HEAD_SIZE;
            buf += write_tcp_size_;

            boost::uint32_t len = bytes_transferred;
            h.chunkLen = h.chunk_size(len);
            util::archive::ArchiveBuffer<> abuf(buf, MmspMessageHeader::HEAD_SIZE);
            util::archive::LittleEndianBinaryOArchive<> oa(abuf);
            oa << h;

            // padding
            if (len) {
                boost::uint64_t z = 0;
                util::archive::LittleEndianBinaryOArchive<> oa(snd_buf_);
                oa << framework::container::make_array((boost::uint8_t *)&z, len);
                bytes_transferred += len;
            }

            write_tcp_size_ += bytes_transferred;
            return bytes_transferred;
        }

        size_t MmspSocket::read_msg(
            MmspMessage & msg, 
            boost::system::error_code & ec)
        {
            if (read_tcp_size_ < MmspMessageHeader::HEAD_SIZE) {
                size_t tmp = read_tcp_size_;
                rcv_buf_.consume(tmp);
                read_tcp_size_ = 0;
                return tmp;
            }

            util::archive::LittleEndianBinaryIArchive<> ia(rcv_buf_);
            MmspMessageHeader h;
            ia >> h;
            size_t bytes_transferred = MmspMessageHeader::HEAD_SIZE;

            std::map<boost::uint32_t, read_msg_t>::const_iterator iter = 
                read_msg_funcs_.find(h.MID);
            assert(iter != read_msg_funcs_.end());
            bytes_transferred += rcv_buf_.size();
            iter->second(rcv_buf_, msg, ec);
            bytes_transferred -= rcv_buf_.size();

            boost::uint32_t len = bytes_transferred;
            assert(h.chunkLen == h.chunk_size(len));

            // padding
            if (len) {
                boost::uint64_t z = 0;
                ia >> framework::container::make_array((boost::uint8_t *)&z, len);
                bytes_transferred += len;
            }

            read_tcp_size_ -= bytes_transferred;
            return bytes_transferred;
        }

        size_t MmspSocket::write_tcp_msg(
            boost::system::error_code & ec)
        {
            return boost::asio::write(
                (super &)*this, 
                snd_buf_, 
                detail::mmsp_write_condition(snd_buf_, write_tcp_msg_, write_tcp_size_), 
                ec);
        }

        size_t MmspSocket::read_tcp_msg(
            boost::system::error_code & ec)
        {
            return boost::asio::read(
                (super &)*this, 
                rcv_buf_, 
                detail::mmsp_read_condition(rcv_buf_, read_tcp_msg_, read_tcp_size_), 
                ec);
        }

    } // namespace protocol
} // namespace util
