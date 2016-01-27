// StreamHandler.h

#ifndef _UTIL_STREAM_STREAM_HANDLER_H_
#define _UTIL_STREAM_STREAM_HANDLER_H_

#include <framework/network/AsioHandlerHelper.h>

#include <boost/asio/buffer.hpp>
#include <boost/asio/detail/bind_handler.hpp>
#include <boost/intrusive_ptr.hpp>

#include <utility>

namespace util
{
    namespace stream
    {

        class StreamHandler
        {
        public:
            typedef void result_type;

        public:
            StreamHandler()
            {
            }

            template <typename Handler>
            StreamHandler(
                Handler const & handler)
                : handler_(create(handler))
            {
            }

        public:
            void swap(
                StreamHandler & r)
            {
                std::swap(handler_, r.handler_);
            }

            size_t shared_count() const
            {
                return handler_->shared_count();
            }

        public:
            void operator()(
                boost::system::error_code const & ec, 
                size_t bytes_transferred) const
            {
                return handler_->invoke(ec, bytes_transferred);
            }

        private:
            class HandlerBase
            {
            public:
                void invoke(
                    boost::system::error_code const & ec, 
                    size_t bytes_transferred)
                {
                    return invoker_(*this, ec, bytes_transferred);
                }

                size_t shared_count() const
                {
                    return nref_;
                }

                friend void intrusive_ptr_add_ref(
                    HandlerBase * p)
                {
                    ++p->nref_;
                }

                friend void intrusive_ptr_release(
                    HandlerBase * p)
                {
                    if (--p->nref_ == 0) {
                        p->deleter_(*p);
                    }
                }

            private:
                typedef void (*invoker_type)(
                    HandlerBase & handler, 
                    boost::system::error_code const & ec, 
                    size_t bytes_transferred);

                typedef void (*deleter_type)(
                    HandlerBase & handler);

            protected:
                HandlerBase(
                    invoker_type invoker, 
                    deleter_type deleter)
                    : nref_(0)
                    , invoker_(invoker)
                    , deleter_(deleter)
                {
                }

                size_t nref_;
                invoker_type invoker_;
                deleter_type deleter_;
            };

            template <typename Handler>
            class HandlerT
                : public HandlerBase
            {
            public:
                HandlerT(
                    Handler const & h)
                    : HandlerBase(&HandlerT::invoker, &HandlerT::deleter)
                    , handler_(h)
                {
                }

            private:
                static void invoker(
                    HandlerBase & handler, 
                    boost::system::error_code const & ec, 
                    size_t bytes_transferred)
                {
                    HandlerT & h = static_cast<HandlerT &>(handler);
                    boost::asio::asio_handler_invoke(boost::asio::detail::bind_handler(h.handler_, ec, bytes_transferred), &h.handler_);
                }

                static void deleter(
                    HandlerBase & handler)
                {
                    HandlerT & h = static_cast<HandlerT &>(handler);
                    h.~HandlerT();
                    boost::asio::asio_handler_deallocate(&h, sizeof(h), &h.handler_);
                }
                
            private:
                Handler handler_;
            };

        private:
            template <typename Handler>
            static HandlerT<Handler> * create(
                Handler const & handler)
            {
                void * ptr = 
                    boost::asio::asio_handler_allocate(sizeof(HandlerT<Handler>), &handler);
                if (ptr)
                    new (ptr) HandlerT<Handler>(handler);
                return (HandlerT<Handler> *)ptr;
            }

        private:
            boost::intrusive_ptr<HandlerBase> handler_;
        };

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_STREAM_HANDLER_H_
