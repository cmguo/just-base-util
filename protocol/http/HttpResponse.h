// HttpResponse.h

#ifndef _UTIL_PROTOCOL_HTTP_RESPONSE_H_
#define _UTIL_PROTOCOL_HTTP_RESPONSE_H_

#include "util/protocol/http/HttpPacket.h"
#include "util/protocol/http/HttpError.h"

namespace util
{
    namespace protocol
    {

        class HttpResponseHead
            : public HttpHead
        {
        public:
            size_t version;
            size_t err_code;
            std::string err_msg;

        public:
            HttpResponseHead(
                size_t err_code = http_error::ok);

            HttpResponseHead(
                size_t err_code, 
                std::string const & err_msg);

            HttpResponseHead(
                size_t err_code, 
                std::string const & err_msg, 
                size_t version);

        private:
            virtual bool get_line(
                std::string & line) const;

            virtual bool set_line(
                std::string const & line);
        };

        class HttpResponse
            : public HttpPacket
        {
        public:
            HttpResponse()
                : HttpPacket(head_)
            {
            }

            HttpResponse(
                HttpResponseHead const & head)
                : HttpPacket(head_)
                , head_(head)
            {
            }

            HttpResponse(
                HttpResponse const & r)
                : HttpPacket(head_)
                , head_(r.head())
            {
                HttpPacket::operator =(r);
            }

        public:
            HttpResponseHead & head()
            {
                return head_;
            }

            HttpResponseHead const & head() const
            {
                return head_;
            }

        private:
            HttpResponseHead head_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_HTTP_RESPONSE_H_
