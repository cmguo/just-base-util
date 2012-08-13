// Sink.h

#ifndef _UTIL_STREAM_SINK_H_
#define _UTIL_STREAM_SINK_H_

#include "util/stream/Base.h"
#include "util/stream/StreamBuffers.h"
#include "util/stream/StreamHandler.h"

namespace util
{
    namespace stream
    {

        class Sink
            : virtual public Base
        {
        public:
            Sink(
                boost::asio::io_service & io_svc)
            {
                set_io_service(io_svc);
            }

            /// Read some data from the stream. Returns the number of bytes write. Throws
            /// an exception on failure.
            template <typename ConstBufferSequence>
            std::size_t write_some(
                const ConstBufferSequence& buffers)
            {
                boost::system::error_code ec;
                std::size_t bytes_transferred = 
                    private_write_some(StreamConstBuffers(buffers), ec);
                boost::asio::detail::throw_error(ec);
                return bytes_transferred;
            }

            /// Read some data from the stream. Returns the number of bytes write or 0 if
            /// an error occurred.
            template <typename ConstBufferSequence>
            std::size_t write_some(
                const ConstBufferSequence& buffers,
                boost::system::error_code& ec)
            {
                std::size_t bytes_transferred = 
                    private_write_some(StreamConstBuffers(buffers), ec);
                return bytes_transferred;
            }

            /// Start an asynchronous write. The buffer into which the data will be write
            /// must be valid for the lifetime of the asynchronous operation.
            template <typename ConstBufferSequence, typename WriteHandler>
            void async_write_some(
                const ConstBufferSequence& buffers,
                WriteHandler handler)
            {
                private_async_write_some(
                    StreamConstBuffers(buffers), 
                    StreamHandler(handler));
            }

        protected:
            virtual std::size_t private_write_some(
                boost::asio::const_buffer const & buffer,
                boost::system::error_code & ec);

            virtual std::size_t private_write_some(
                StreamConstBuffers const & buffers,
                boost::system::error_code & ec);

            virtual void private_async_write_some(
                boost::asio::const_buffer const & buffer, 
                StreamHandler const & handler);

            virtual void private_async_write_some(
                StreamConstBuffers const & buffers, 
                StreamHandler const & handler);
        };

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_SINK_H_
