// Base.h

#ifndef _UTIL_STREAM_BASE_H_
#define _UTIL_STREAM_BASE_H_

namespace util
{
    namespace stream
    {

        class Base
        {
        public:
            Base();

            virtual ~Base();

        public:
            boost::asio::io_service & get_io_service()
            {
                return *io_svc_;
            }

        public:
            virtual bool cancel(
                boost::system::error_code & ec);

        public:
            virtual bool set_non_block(
                bool non_block, 
                boost::system::error_code & ec);

            virtual bool set_time_out(
                boost::uint32_t time_out, 
                boost::system::error_code & ec);

        protected:
            void set_io_service(
                boost::asio::io_service & io_svc)
            {
                io_svc_ = &io_svc;
            }

        private:
            boost::asio::io_service * io_svc_;
        };

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_BASE_H_
