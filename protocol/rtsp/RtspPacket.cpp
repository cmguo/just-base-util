// RtspPacket.cpp

#include "util/Util.h"
#include "util/protocol/rtsp/RtspPacket.h"

namespace util
{
    namespace protocol
    {

        static char const * const rtsp_method_strings[] = {
            "DESCRIBE", 
            "ANNOUNCE", 
            "GET_PARAMETER", 
            "SET_PARAMETER", 
            "OPTIONS", 
            "REDIRECT", 
            "SETUP", 
            "PLAY", 
            "RECORD", 
            "PAUSE", 
            "TEARDOWN", 
            NULL
        };

        CMsgProtocol rtsp_protocol = {
            "RTSP", 
            rtsp_method_strings
        };

    } // namespace protocol
} // namespace util
