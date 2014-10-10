// CMsgResponse.h

#ifndef _UTIL_PROTOCOL_CMSG_CMSG_RESPONSE_H_
#define _UTIL_PROTOCOL_CMSG_CMSG_RESPONSE_H_

#include "util/protocol/cmsg/CMsgPacket.h"
#include "util/protocol/cmsg/CMsgProtocol.h"

namespace util
{
    namespace protocol
    {
        class CMsgResponseHead
            : public CMsgHead
        {
        public:
            size_t version;
            size_t err_code;
            std::string err_msg;

        public:
            CMsgResponseHead(
                size_t version, 
                size_t err_code, 
                std::string const & err_msg);

        protected:
            CMsgProtocol const * protocol_;

        private:
            virtual bool get_line(
                std::string & line) const;

            virtual bool set_line(
                std::string const & line);
        };

        class CMsgResponse
            : public CMsgPacket
        {
        protected:
            CMsgResponse(
                CMsgResponseHead & head);

            CMsgResponse(
                CMsgResponse const & r, 
                CMsgResponseHead & head);

        public:
            CMsgResponseHead & head()
            {
                return head_;
            }

            CMsgResponseHead const & head() const
            {
                return head_;
            }

        private:
            CMsgResponseHead & head_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_CMSG_CMSG_RESPONSE_H_
