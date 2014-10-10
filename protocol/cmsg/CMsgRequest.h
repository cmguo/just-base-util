// CMsgRequest.h

#ifndef _UTIL_PROTOCOL_CMSG_CMSG_REQUEST_H_
#define _UTIL_PROTOCOL_CMSG_CMSG_REQUEST_H_

#include "util/protocol/cmsg/CMsgPacket.h"
#include "util/protocol/cmsg/CMsgProtocol.h"

namespace util
{
    namespace protocol
    {

        class CMsgRequestHead
            : public CMsgHead
        {
        public:
            CMsgRequestHead(
                size_t method, 
                std::string const & path, 
                size_t version);

        public:
            size_t method;
            std::string path;
            size_t version;

        protected:
            CMsgProtocol const * protocol_;

        private:
            virtual bool get_line(
                std::string & line) const;

            virtual bool set_line(
                std::string const & line);
        };

        class CMsgRequest
            : public CMsgPacket
        {
        protected:
            CMsgRequest(
                CMsgRequestHead & head);

            CMsgRequest(
                CMsgRequest const & r, 
                CMsgRequestHead & head);

        public:
            CMsgRequestHead & head()
            {
                return head_;
            }

            CMsgRequestHead const & head() const
            {
                return head_;
            }

        private:
            CMsgRequestHead & head_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_CMSG_CMSG_REQUEST_H_
