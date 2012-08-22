// StreamHandler.h

#ifndef _UTIL_STREAM_STREAM_HANDLER_H_
#define _UTIL_STREAM_STREAM_HANDLER_H_

#include <framework/network/AsioHandlerHelper.h>

#include <boost/asio/buffer.hpp>
#include <boost/asio/detail/bind_handler.hpp>

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
            template <typename Handler>
            StreamHandler(
                Handler const & handler)
                : handler_(colon(handler))
                , invoker_(&StreamHandler::invoker<Handler>)
                , deleter_(&StreamHandler::deleter<Handler>)
            {
            }

            StreamHandler(
                StreamHandler const & r)
                : handler_(r.handler_)
                , invoker_(r.invoker_)
                , deleter_(r.deleter_)
            {
                r.handler_ = NULL;
            }

            ~StreamHandler()
            {
                if (handler_) {
                    deleter_(handler_);
                    handler_ = NULL;
                }
            }

        public:
            void operator()(
                boost::system::error_code const & ec, 
                size_t bytes_transferred) const
            {
                return invoker_(handler_, ec, bytes_transferred);
            }

            void invoke(
                boost::system::error_code const & ec, 
                size_t bytes_transferred) const
            {
                return invoker_(handler_, ec, bytes_transferred);
            }

        private:
            typedef void (*invoker_type)(
                void * handler, 
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            typedef void (*deleter_type)(
                void * handler);

        private:
            template <typename Handler>
            static void * colon(
                Handler const & handler)
            {
                void * ptr = 
                    boost::asio::asio_handler_allocate(sizeof(handler), &handler);
                if (ptr)
                    new (ptr) Handler(handler);
                return ptr;
            }

            template <typename Handler>
            static void invoker(
                void * handler, 
                boost::system::error_code const & ec, 
                size_t bytes_transferred)
            {
                Handler & h = *(Handler *)(handler);
                boost::asio::asio_handler_invoke(boost::asio::detail::bind_handler(h, ec, bytes_transferred), &h);
            }

            template <typename Handler>
            static void deleter(
                void * handler)
            {
                Handler & h = *(Handler *)(handler);
                h.~Handler();
                boost::asio::asio_handler_deallocate(handler, sizeof(h), &h);
            }

        private:
            mutable void * handler_;
            invoker_type invoker_;
            deleter_type deleter_;
        };

        template <typename Handler>
        class StreamHandlerT
            : public StreamHandler
        {
        public:
            StreamHandlerT(
                Handler const & handler)
                : StreamHandler(&StreamHandlerT::invoker, &StreamHandlerT::deleter)
                , handler_(handler)
            {
            }

        private:
            static void invoker(
                StreamHandler const & handler, 
                boost::system::error_code const & ec, 
                size_t bytes_transferred)
            {
                StreamHandlerT const & me = static_cast<StreamHandlerT const &>(handler);
                return me.handler_(ec, bytes_transferred);
            }

            static void deleter(
                StreamHandler const & handler)
            {
                StreamHandlerT const & me = static_cast<StreamHandlerT const &>(handler);
                delete &me;
            }

        private:
            Handler handler_;
        };


    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_STREAM_HANDLER_H_
