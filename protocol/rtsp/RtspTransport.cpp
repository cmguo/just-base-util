// UdpTransport.cpp

#include "util/Util.h"
#include "util/protocol/rtsp/RtspTransport.h"
#include "util/protocol/rtsp/rtp/RtpUdpSocket.h"
#include "util/protocol/rtsp/rtp/RtpTcpSocket.h"

#include <framework/string/Slice.h>
#include <framework/string/Join.h>
using namespace framework::string;

namespace util
{
    namespace protocol
    {

        struct find_parameter
        {
            find_parameter(
                std::string const & find)
                : find_(find)
            {
            }

            bool operator()(
                std::string const & r)
            {
                return r.compare(0, find_.size(), find_) == 0;
            }

        private:
            std::string const find_;
        };

        bool create_transport(
            util::stream::Dual *& rtp_socket, 
            RtspSocket & rtsp_socket, 
            std::string const & in_transport, 
            std::string & out_transport, 
            boost::system::error_code & ec)
        {
            std::vector<std::string> vec;
            slice<std::string>(in_transport, std::back_inserter(vec), ";");
            if (vec[0] == "RTP/AVP" || vec[0] == "RTP/AVP/UDP") {
                if (rtp_socket == NULL)
                    rtp_socket = new RtpUdpSocket(rtsp_socket);
                RtpUdpSocket * rtp_udp_socket = (RtpUdpSocket *)rtp_socket;
                std::vector<std::string>::iterator iter = 
                    std::find_if(vec.begin(), vec.end(), find_parameter("client_port="));
                boost::uint16_t ports[2] = {0, 0};
                if (rtp_udp_socket->create_port(ports, ec)) {
                    if (iter == vec.end()) {
                        out_transport = join(ports,  ports + 2, "-", "client_port=");
                    } else {
                        out_transport = join(ports,  ports + 2, "-", "server_port=");
                        ++iter;
                    }
                    vec.insert(iter, out_transport);
                }
            } else {
                if (rtp_socket == NULL)
                    rtp_socket = new RtpTcpSocket(rtsp_socket);
                RtpTcpSocket * rtp_tcp_socket = (RtpTcpSocket *)rtp_socket;
                std::vector<std::string>::iterator iter = 
                    std::find_if(vec.begin(), vec.end(), find_parameter("interleaved="));
                boost::uint8_t interleaveds[2] = {0, 0};
                if (rtp_tcp_socket->create_port(interleaveds, ec)) {
                    out_transport = join(interleaveds, interleaveds + 2, "-", "interleaved=");
                    if (iter == vec.end()) {
                        vec.insert(iter, out_transport);
                    } else {
                        *iter = out_transport;
                    }
                }
            }
            if (!ec) {
                out_transport = join(vec.begin(), vec.end(), ";", "", "");
            }
            return !ec;
        }

        bool connect_transport(
            util::stream::Dual * rtp_socket, 
            std::string const & transport, 
            boost::system::error_code & ec)
        {
            std::vector<std::string> vec_t;
            slice<std::string>(transport, std::back_inserter(vec_t), ",", "", "");
            std::vector<std::string> vec;
            slice<std::string>(vec_t[0], std::back_inserter(vec), ";");
            if (vec[0] == "RTP/AVP" || vec[0] == "RTP/AVP/UDP") {
                RtpUdpSocket * rtp_udp_socket = (RtpUdpSocket *)rtp_socket;
                std::vector<std::string>::iterator iter = 
                    std::find_if(vec.begin(), vec.end(), find_parameter("server_port="));
                if (iter == vec.end()) {
                    iter = std::find_if(vec.begin(), vec.end(), find_parameter("client_port="));
                }
                if (iter != vec.end()) {
                    boost::uint16_t peer_ports[2] = {0, 0};
                    slice<boost::uint16_t>(iter->substr(12), peer_ports, "-");
                    rtp_udp_socket->connect_port(peer_ports, ec);
                }
            } else {
                RtpTcpSocket * rtp_tcp_socket = (RtpTcpSocket *)rtp_socket;
                std::vector<std::string>::iterator iter = 
                    std::find_if(vec.begin(), vec.end(), find_parameter("interleaved="));
                if (iter != vec.end()) {
                    boost::uint8_t interleaveds[2] = {0, 0};
                    slice<boost::uint16_t>(*iter, interleaveds, "-", "interleaved=");
                    rtp_tcp_socket->connect_port(interleaveds, ec);
                }
            }
            return !ec;
        }

    } // namespace protocol
} // namespace util
