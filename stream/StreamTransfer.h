// StreamTransfer.h

#ifndef _UTIL_STREAM_STREAM_TRANSFER_H_
#define _UTIL_STREAM_STREAM_TRANSFER_H_

#include "util/stream/TransferBuffers.h"

#include <framework/network/AsioHandlerHelper.h>

#include <boost/mpl/if.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/non_type.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/detail/throw_error.hpp>
#include <boost/asio/detail/handler_alloc_helpers.hpp>
#include <boost/asio/detail/handler_invoke_helpers.hpp>

namespace util
{
    namespace stream
    {

        typedef std::pair<std::size_t, std::size_t> transfer_size;

        namespace detail
        {
            class transfer_all_t
            {
            public:
                typedef bool result_type;

                template <typename Error>
                bool operator()(
                    bool is_read, 
                    const Error& err, 
                    transfer_size const & bytes_transferred)
                {
                    return !!err;
                }
            };

            class transfer_at_least_t
            {
            public:
                typedef bool result_type;

                explicit transfer_at_least_t(std::size_t minimum)
                    : minimum_(minimum)
                {
                }

                template <typename Error>
                bool operator()(bool is_read, const Error& err, transfer_size const & bytes_transferred)
                {
                    if (is_read)
                        return !!err || bytes_transferred.first >= minimum_;
                    else
                        return !!err || bytes_transferred.second >= minimum_;
                }

            private:
                std::size_t minimum_;
            };
        }

        inline detail::transfer_all_t transfer_all()
        {
            return detail::transfer_all_t();
        }

        inline detail::transfer_at_least_t transfer_at_least(std::size_t minimum)
        {
            return detail::transfer_at_least_t(minimum);
        }

        template <
            typename SyncReadStream, 
            typename SyncWriteStream, 
            typename Buffers, 
            typename Elem, 
            typename Traits, 
            typename CompletionCondition
        >
        transfer_size transfer(
            SyncReadStream & r, 
            SyncWriteStream & w, 
            util::buffers::CycleBuffers<Buffers, Elem, Traits> & buffers, 
            CompletionCondition completion_condition, 
            boost::system::error_code & ec, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            size_t capacity = buffers.capacity();
            if (high_level > capacity)
                high_level = capacity;
            transfer_size total_transferred(0, 0);
            bool read_end = false;
            bool write_end = false;
            size_t level = buffers.in_avail();
            ec.clear();
            while (true) {
                if (level < high_level || (write_end && level < capacity)) {
                    std::size_t bytes_transferred = r.read_some(buffers.prepare(), ec);
                    total_transferred.first += bytes_transferred;
                    level += bytes_transferred;
                    buffers.commit(bytes_transferred);
                    if (completion_condition(true, ec, total_transferred)) {
                        read_end = true;
                        high_level = 0; // to stop later read
                        if (write_end)
                            break;
                    }
                } else if (write_end) {
                    break;
                }
                if (level > low_level || (read_end && level > 0)) {
                    std::size_t bytes_transferred = w.write_some(buffers.data(), ec);
                    total_transferred.second += bytes_transferred;
                    level -= bytes_transferred;
                    buffers.consume(bytes_transferred);
                    if (completion_condition(false, ec, total_transferred)) {
                        write_end = true;
                        low_level = capacity; // to stop later write
                        if (read_end)
                            break;
                    }
                } else if (read_end) {
                    break;
                }
            }
            return total_transferred;
        }

        template <
            typename SyncReadStream, 
            typename SyncWriteStream, 
            typename Buffers, 
            typename Elem, 
            typename Traits, 
            typename CompletionCondition
        >
        transfer_size transfer(
            SyncReadStream & r, 
            SyncWriteStream & w, 
            util::buffers::CycleBuffers<Buffers, Elem, Traits> & buffers, 
            CompletionCondition completion_condition, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            boost::system::error_code ec;
            transfer_size bytes_transferred = 
                transfer(r, w, buffers, completion_condition, ec, low_level, high_level);
            boost::asio::detail::throw_error(ec);
            return bytes_transferred;
        }

        template <
            typename SyncReadStream, 
            typename SyncWriteStream, 
            typename Buffers, 
            typename Elem, 
            typename Traits
        >
        transfer_size transfer(
            SyncReadStream & r, 
            SyncWriteStream & w, 
            util::buffers::CycleBuffers<Buffers, Elem, Traits> & buffers, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            boost::system::error_code ec;
            transfer_size bytes_transferred = 
                transfer(r, w, buffers, transfer_all(), ec, low_level, high_level);
            boost::asio::detail::throw_error(ec);
            return bytes_transferred;
        }

        template <
            typename SyncReadStream, 
            typename SyncWriteStream, 
            typename MutableBufferSequence, 
            typename CompletionCondition
        >
        transfer_size transfer(
            SyncReadStream & r, 
            SyncWriteStream & w, 
            MutableBufferSequence const & buffers, 
            CompletionCondition completion_condition, 
            boost::system::error_code & ec, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            typedef util::buffers::CycleBuffers<MutableBufferSequence> cycle_buffers_type;
            cycle_buffers_type tmp(buffers);
            return transfer(r, w, tmp, completion_condition, ec, low_level, high_level);
        }

        template <
            typename SyncReadStream, 
            typename SyncWriteStream, 
            typename MutableBufferSequence, 
            typename CompletionCondition
        >
        transfer_size transfer(
            SyncReadStream & r, 
            SyncWriteStream & w, 
            MutableBufferSequence const & buffers, 
            CompletionCondition completion_condition, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            boost::system::error_code ec;
            transfer_size bytes_transferred = 
                transfer(r, w, buffers, completion_condition, ec, low_level, high_level);
            boost::asio::detail::throw_error(ec);
            return bytes_transferred;
        }

        template <
            typename SyncReadStream, 
            typename SyncWriteStream, 
            typename MutableBufferSequence
        >
        transfer_size transfer(
            SyncReadStream & r, 
            SyncWriteStream & w, 
            MutableBufferSequence const & buffers, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            boost::system::error_code ec;
            transfer_size bytes_transferred = 
                transfer(r, w, buffers, transfer_all(), ec, low_level, high_level);
            boost::asio::detail::throw_error(ec);
            return bytes_transferred;
        }

        template <
            typename SyncReadStream, 
            typename SyncWriteStream, 
            typename Elem, 
            typename Allocator, 
            typename Traits, 
            typename CompletionCondition
        >
        transfer_size transfer(
            SyncReadStream & r, 
            SyncWriteStream & w, 
            BasicTransferBuffers<Elem, Allocator, Traits> & buffers, 
            CompletionCondition completion_condition, 
            boost::system::error_code & ec, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            typedef BasicTransferBuffers<Elem, Allocator, Traits>::cycle_buffers_type cycle_buffers_type;
            return transfer(r, w, (cycle_buffers_type &)buffers, completion_condition, ec, low_level, high_level);
        }

        template <
            typename SyncReadStream, 
            typename SyncWriteStream, 
            typename Elem, 
            typename Allocator, 
            typename Traits, 
            typename CompletionCondition
        >
        transfer_size transfer(
            SyncReadStream & r, 
            SyncWriteStream & w, 
            BasicTransferBuffers<Elem, Allocator, Traits> & buffers, 
            CompletionCondition completion_condition, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            boost::system::error_code ec;
            transfer_size bytes_transferred = 
                transfer(r, w, buffers, completion_condition, ec, low_level, high_level);
            boost::asio::detail::throw_error(ec);
            return bytes_transferred;
        }

        template <
            typename SyncReadStream, 
            typename SyncWriteStream, 
            typename Elem, 
            typename Allocator, 
            typename Traits
        >
        transfer_size transfer(
            SyncReadStream & r, 
            SyncWriteStream & w, 
            BasicTransferBuffers<Elem, Allocator, Traits> & buffers, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            boost::system::error_code ec;
            transfer_size bytes_transferred = 
                transfer(r, w, buffers, transfer_all(), ec, low_level, high_level);
            boost::asio::detail::throw_error(ec);
            return bytes_transferred;
        }

        namespace detail
        {

            template <
                typename AsyncReadStream, 
                typename AsyncWriteStream, 
                typename Buffers, 
                typename Elem, 
                typename Traits, 
                typename CompletionCondition, 
                typename TransferHandler
            >
            class transfer_cycle_buffer_handler
            {
            public:
                typedef util::buffers::CycleBuffers<
                    Buffers, Elem, Traits> cycle_buffers_type;

                typedef boost::intrusive_ptr<transfer_cycle_buffer_handler> transfer_handler_ptr;

                transfer_cycle_buffer_handler(
                    AsyncReadStream & read_stream, 
                    AsyncWriteStream & write_stream, 
                    cycle_buffers_type & buffers, 
                    CompletionCondition completion_condition, 
                    TransferHandler handler, 
                    size_t low_level, 
                    size_t high_level)
                    : read_stream_(read_stream)
                    , write_stream_(write_stream)
                    , buffers_(buffers)
                    , completion_condition_(completion_condition)
                    , handler_(handler)
                    , strand_(read_stream.get_io_service())
                    , capacity_(buffers.capacity())
                    , level_(buffers.in_avail())
                    , low_level_(low_level)
                    , high_level_(high_level)
                    , total_transferred_(0, 0)
                    , read_end_(false)
                    , reading_(false)
                    , write_end_(false)
                    , writing_(false)
                    , ref_count_(0)
                {
                    if (high_level_ > capacity_)
                        high_level_ = capacity_;
                }

                friend void intrusive_ptr_add_ref(transfer_cycle_buffer_handler * p)
                {
                    ++p->ref_count_;
                }

                friend void intrusive_ptr_release(transfer_cycle_buffer_handler * p)
                {
                    if (--p->ref_count_ == 0) {
                        delete p;
                    }
                }

                struct read_handler_t
                {
                    read_handler_t(
                        transfer_cycle_buffer_handler & handler)
                        : handler_(&handler)
                    {
                    }

                    void operator()(
                        boost::system::error_code const & ec, 
                        std::size_t bytes_transferred)
                    {
                        handler_->handler_read(ec, bytes_transferred);
                    }

                    PASS_DOWN_ASIO_HANDLER_FUNCTION(read_handler_t, handler_->handler_)

                private:
                    transfer_handler_ptr handler_;
                };

                struct write_handler_t
                {
                    write_handler_t(
                        transfer_cycle_buffer_handler & handler)
                        : handler_(&handler)
                    {
                    }

                    void operator()(
                        boost::system::error_code const & ec, 
                        std::size_t bytes_transferred)
                    {
                        handler_->handler_write(ec, bytes_transferred);
                    }

                    PASS_DOWN_ASIO_HANDLER_FUNCTION(write_handler_t, handler_->handler_)

                private:
                    transfer_handler_ptr handler_;
                };

                void start()
                {
                    boost::system::error_code ec;
                    handler_read(ec, 0);
                    handler_write(ec, 0);
                }

                void handler_read(
                    const boost::system::error_code & ec,
                    std::size_t bytes_transferred)
                {
                    total_transferred_.first += bytes_transferred;
                    level_ += bytes_transferred;
                    buffers_.commit(bytes_transferred);
                    reading_ = false;
                    if (completion_condition_(true, ec, total_transferred_)) {
                        read_end_ = true;
                        if (write_end_) {
                            handler_(ec, total_transferred_);
                        }
                    } else if (level_ < high_level_ || (write_end_ && level_ < capacity_)) {
                        reading_ = true;
                        read_stream_.async_read_some(buffers_.prepare(), get_read_handler());
                    } else if (write_end_) {
                        handler_(ec, total_transferred_);
                    }
                    if (!writing_ && !write_end_) {
                        handler_write(boost::system::error_code(), 0);
                    }
                }

                void handler_write(
                    const boost::system::error_code & ec,
                    std::size_t bytes_transferred)
                {
                    total_transferred_.second += bytes_transferred;
                    level_ -= bytes_transferred;
                    buffers_.consume(bytes_transferred);
                    writing_ = false;
                    if (completion_condition_(false, ec, total_transferred_)) {
                        write_end_ = true;
                        if (read_end_) {
                            handler_(ec, total_transferred_);
                        }
                    } else if (level_ > low_level_ || (read_end_ && level_ > 0)) {
                        writing_ = true;
                        write_stream_.async_write_some(buffers_.data(), get_write_handler());
                    } else if (read_end_) {
                        handler_(ec, total_transferred_);
                    }
                    if (!reading_ && !read_end_) {
                        handler_read(boost::system::error_code(), 0);
                    }
                }

                boost::asio::detail::wrapped_handler<boost::asio::strand, read_handler_t> get_read_handler()
                {
                    return strand_.wrap(read_handler_t(*this));
                }

                boost::asio::detail::wrapped_handler<boost::asio::strand, write_handler_t> get_write_handler()
                {
                    return strand_.wrap(write_handler_t(*this));
                }

            //private:
                friend struct read_handler_t;
                friend struct write_handler_t;

                AsyncReadStream & read_stream_;
                AsyncWriteStream & write_stream_;
                cycle_buffers_type & buffers_;
                CompletionCondition completion_condition_;
                TransferHandler handler_;
                boost::asio::strand strand_;
                size_t capacity_;
                size_t level_;
                size_t low_level_;
                size_t high_level_;
                transfer_size total_transferred_;
                bool read_end_;
                bool reading_;
                bool write_end_;
                bool writing_;
                size_t ref_count_;
            };

            template <
                typename AsyncReadStream, 
                typename AsyncWriteStream, 
                typename MutableBufferSequence,
                typename CompletionCondition, 
                typename TransferHandler
            >
            class transfer_handler
                : public transfer_cycle_buffer_handler<
                    AsyncReadStream, AsyncWriteStream, MutableBufferSequence, char, std::char_traits<char>, CompletionCondition, TransferHandler>
            {
            public:
                typedef transfer_cycle_buffer_handler<
                    AsyncReadStream, AsyncWriteStream, MutableBufferSequence, char, std::char_traits<char>, CompletionCondition, TransferHandler> super;

                transfer_handler(
                    AsyncReadStream & read_stream, 
                    AsyncWriteStream & write_stream, 
                    MutableBufferSequence const & buffers,
                    CompletionCondition completion_condition, 
                    TransferHandler handler, 
                    size_t low_level, 
                    size_t high_level)
                    : super(read_stream, write_stream, cycle_buffers_, completion_condition, handler, low_level, high_level)
                    , cycle_buffers_(buffers)
                {
                }

                cycle_buffers_type cycle_buffers_;
            };

            template <
                typename AsyncReadStream, 
                typename AsyncWriteStream, 
                typename Elem, 
                typename Allocator, 
                typename Traits, 
                typename CompletionCondition, 
                typename TransferHandler
            >
            class transfer_buffer_handler
                : public BasicTransferBuffers<Elem, Allocator, Traits>::cycle_buffers_type
            {
            public:
                typedef typename BasicTransferBuffers<Elem, Allocator, Traits>::cycle_buffers_type super;

                transfer_buffer_handler(
                    AsyncReadStream & read_stream, 
                    AsyncWriteStream & write_stream, 
                    BasicTransferBuffers<Elem, Allocator, Traits> & buffers, 
                    CompletionCondition completion_condition, 
                    TransferHandler handler, 
                    size_t low_level, 
                    size_t high_level)
                    : super(read_stream, write_stream, buffers, completion_condition, handler, low_level, high_level)
                {
                }
            };

        } // namespace detail

        template <
            typename AsyncReadStream, 
            typename AsyncWriteStream, 
            typename MutableBufferSequence, 
            typename CompletionCondition, 
            typename TransferHandler
        >
        void async_transfer(
            AsyncReadStream & r, 
            AsyncWriteStream & w, 
            MutableBufferSequence const & buffers, 
            CompletionCondition completion_condition, 
            TransferHandler handler, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            typedef detail::transfer_handler<AsyncReadStream, AsyncWriteStream, 
                MutableBufferSequence, CompletionCondition, TransferHandler> transfer_handler_t;
            typename transfer_handler_t::transfer_handler_ptr ptr(new transfer_handler_t(
                r, w, buffers, completion_condition, handler, low_level, high_level));
            ptr->start();
        }

        template <
            typename AsyncReadStream, 
            typename AsyncWriteStream, 
            typename MutableBufferSequence, 
            typename TransferHandler
        >
        void async_transfer(
            AsyncReadStream & r, 
            AsyncWriteStream & w, 
            MutableBufferSequence const & buffers, 
            TransferHandler handler, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            return async_transfer(r, w, buffers, transfer_all(), handler, low_level, high_level);
        }

        template <
            typename AsyncReadStream, 
            typename AsyncWriteStream, 
            typename Elem, 
            typename Allocator, 
            typename Traits, 
            typename CompletionCondition, 
            typename TransferHandler
        >
        void async_transfer(
            AsyncReadStream & r, 
            AsyncWriteStream & w, 
            BasicTransferBuffers<Elem, Allocator, Traits> & buffers, 
            CompletionCondition completion_condition, 
            TransferHandler handler, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            typedef detail::transfer_buffer_handler<AsyncReadStream, AsyncWriteStream, 
                Elem, Allocator, Traits, CompletionCondition, TransferHandler> transfer_handler_t;
            typename transfer_handler_t::transfer_handler_ptr ptr(new transfer_handler_t(
                r, w, buffers, completion_condition, handler, low_level, high_level));
            ptr->start();
        }

        template <
            typename AsyncReadStream, 
            typename AsyncWriteStream, 
            typename Elem, 
            typename Allocator, 
            typename Traits, 
            typename TransferHandler
        >
        void async_transfer(
            AsyncReadStream & r, 
            AsyncWriteStream & w, 
            BasicTransferBuffers<Elem, Allocator, Traits> & buffers, 
            TransferHandler handler, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            return async_transfer(r, w, buffers, transfer_all(), handler, low_level, high_level);
        }

        template <
            typename AsyncReadStream, 
            typename AsyncWriteStream, 
            typename Buffers, 
            typename Elem, 
            typename Traits, 
            typename CompletionCondition, 
            typename TransferHandler
        >
        void async_transfer(
            AsyncReadStream & r, 
            AsyncWriteStream & w, 
            util::buffers::CycleBuffers<Buffers, Elem, Traits> & buffers, 
            CompletionCondition completion_condition, 
            TransferHandler handler, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            typedef detail::transfer_cycle_buffer_handler<AsyncReadStream, AsyncWriteStream, 
                Buffers, Elem, Traits, CompletionCondition, TransferHandler> transfer_handler_t;
            typename transfer_handler_t::transfer_handler_ptr ptr(new transfer_handler_t(
                r, w, buffers, completion_condition, handler, low_level, high_level));
            ptr->start();
        }

        template <
            typename AsyncReadStream, 
            typename AsyncWriteStream, 
            typename Buffers, 
            typename Elem, 
            typename Traits, 
            typename TransferHandler
        >
        void async_transfer(
            AsyncReadStream & r, 
            AsyncWriteStream & w, 
            util::buffers::CycleBuffers<Buffers, Elem, Traits> & buffers, 
            TransferHandler handler, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            return async_transfer(r, w, buffers, transfer_all(), handler, low_level, high_level);
        }

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_STREAM_TRANSFER_H_
