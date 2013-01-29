// MmspMacToViewerMessage.h 

#ifndef _UTIL_PROTOCOL_MMSP_MMSP_MAC_TO_VIEWER_MESSAGE_H_
#define _UTIL_PROTOCOL_MMSP_MMSP_MAC_TO_VIEWER_MESSAGE_H_

#include "util/protocol/mmsp/MmspMessage.h"
#include "util/protocol/mmsp/MmspString.h"

namespace util
{
    namespace protocol
    {

        struct MmspMacToViewerMessage
        {
            enum IdEnum
            {
                PING                        = 0x0004001b,
                REPORT_CONNECTED_EX         = 0x00040001,
                REPORT_CONNECTED_FUNNEL     = 0x00040002,
                REPORT_DISCONNECTED_FUNNEL  = 0x00040003,
                REPORT_END_OF_STREAM        = 0x0004001e,
                REPORT_FUNNEL_INFO          = 0x00040015,
                REPORT_OPEN_FILE            = 0x00040006,
                REPORT_READ_BLOCK           = 0x00040011,
                REPORT_REDIRECT             = 0x00040022,
                SECURITY_CHALLENGE          = 0x0004001a,
                REPORT_START_PLAYING        = 0x00040005,
                REPORT_START_STRIDING       = 0x0004000a,
                REPORT_STREAM_CHANGE        = 0x00040020,
                REPORT_STREAM_SWITCH        = 0x00040021,
            };
        };

        struct MmspDataPing
            : MmspMessageData<MmspMacToViewerMessage::PING>
        {
            boost::uint32_t dwParam1;
            boost::uint32_t dwParam2;

            MmspDataPing()
                : dwParam1(0x00000000)
                , dwParam2(0x00000000)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                ar & dwParam1;
                ar & dwParam2;
            }
        };

        struct MmspDataReportConnectedEx
            : MmspReportMessageHeader
            , MmspMessageData<MmspMacToViewerMessage::REPORT_CONNECTED_EX>
        {
            // MMS_DISABLE_PACKET_PAIR
            // MMS_USE_PACKET_PAIR
            boost::uint32_t playIncarnation;
            boost::uint32_t MacToViewerProtocolRevision;
            boost::uint32_t ViewerToMacProtocolRevision;
            double blockGroupPlayTime;
            boost::uint32_t blockGroupBlocks;
            boost::uint32_t nMaxOpenFiles;
            boost::uint32_t nBlockMaxBytes;
            boost::uint32_t maxBitRate;
            boost::uint32_t cbServerVersionInfo;
            boost::uint32_t cbVersionInfo;
            boost::uint32_t cbVersionUrl;
            boost::uint32_t cbAuthenPackage;
            MmspString ServerVersionInfo;
            MmspString VersionInfo;
            MmspString VersionUrl;
            MmspString AuthenPackage;

            MmspDataReportConnectedEx()
                : playIncarnation(0)
                , MacToViewerProtocolRevision(0x0004000B)
                , ViewerToMacProtocolRevision(0x0003001C)
                , blockGroupPlayTime(1.0)
                , blockGroupBlocks(0x00000001)
                , nMaxOpenFiles(0x00000001)
                , nBlockMaxBytes(0x00008000)
                , maxBitRate(0x00989680)
                , cbServerVersionInfo(0)
                , cbVersionInfo(0)
                , cbVersionUrl(0)
                , cbAuthenPackage(0)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                MmspReportMessageHeader::serialize(ar);

                ar & playIncarnation;
                ar & MacToViewerProtocolRevision;
                ar & ViewerToMacProtocolRevision;
                ar & blockGroupPlayTime;
                ar & blockGroupBlocks;
                ar & nMaxOpenFiles;
                ar & nBlockMaxBytes;
                ar & maxBitRate;
                ar & cbServerVersionInfo;
                ar & cbVersionInfo;
                ar & cbVersionUrl;
                ar & cbAuthenPackage;

                ServerVersionInfo.size(cbServerVersionInfo);
                VersionInfo.size(cbVersionInfo);
                VersionUrl.size(cbVersionUrl);
                AuthenPackage.size(cbAuthenPackage);

                ar & ServerVersionInfo;
                ar & VersionInfo;
                ar & VersionUrl;
                ar & AuthenPackage;
            }
        };

        struct MmspDataReportConnectedFunnel
            : MmspReportMessageHeader
            , MmspMessageData<MmspMacToViewerMessage::REPORT_CONNECTED_FUNNEL>
        {
            boost::uint32_t playIncarnation;
            boost::uint32_t packetPayloadSize;
            MmspString funnelName; // "Funnel Of The Gods"

            MmspDataReportConnectedFunnel()
                : playIncarnation(0x00000000)
                , packetPayloadSize(0x00000000)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                MmspReportMessageHeader::serialize(ar);

                ar & playIncarnation;
                ar & packetPayloadSize;
                ar & funnelName;
            }
        };

        struct MmspDataReportDisconnectedFunnel
            : MmspReportMessageHeader
            , MmspMessageData<MmspMacToViewerMessage::REPORT_DISCONNECTED_FUNNEL>
        {
            boost::uint32_t playIncarnation;

            MmspDataReportDisconnectedFunnel()
                : playIncarnation(0x00000000)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                MmspReportMessageHeader::serialize(ar);

                ar & playIncarnation;
            }
        };

        struct MmspDataReportEndOfStream
            : MmspReportMessageHeader
            , MmspMessageData<MmspMacToViewerMessage::REPORT_END_OF_STREAM>
        {
            boost::uint32_t playIncarnation;

            MmspDataReportEndOfStream()
                : playIncarnation(0x00000000)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                MmspReportMessageHeader::serialize(ar);

                ar & playIncarnation;
            }
        };

        struct MmspDataReportFunnelInfo
            : MmspReportMessageHeader
            , MmspMessageData<MmspMacToViewerMessage::REPORT_FUNNEL_INFO>
        {
            // MMS_DISABLE_PACKET_PAIR
            // MMS_USE_PACKET_PAIR
            // MMS_PACKET_PAIR_TCP_HIGH_ENTROPY
            // MMS_PACKET_PAIR_UDP_HIGH_ENTROPY
            boost::uint32_t playIncarnation;
            boost::uint32_t transportMask;
            boost::uint32_t nBlockFragments;
            boost::uint32_t fragmentBytes;
            boost::uint32_t nCubs;
            boost::uint32_t failedCubs;
            boost::uint32_t nDisks;
            boost::uint32_t decluster;
            boost::uint32_t cubddDatagramSize;
            std::vector<boost::uint8_t> Packet_pair_packet;

           MmspDataReportFunnelInfo()
                : playIncarnation(0x00000000)
                , transportMask(0x00000008)
                , nBlockFragments(0x00000001)
                , fragmentBytes(0x00010000)
                , nCubs(0)
                , failedCubs(0x00000000)
                , nDisks(0x00000001)
                , decluster(0x00000000)
                , cubddDatagramSize(0x00000000)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                MmspReportMessageHeader::serialize(ar);

                ar & playIncarnation;
                ar & transportMask;
                ar & nBlockFragments;
                ar & fragmentBytes;
                ar & nCubs;
                ar & failedCubs;
                ar & nDisks;
                ar & decluster;
                ar & cubddDatagramSize;
            }
        };

        struct MmspDataReportOpenFile
            : MmspReportMessageHeader
            , MmspMessageData<MmspMacToViewerMessage::REPORT_OPEN_FILE>
        {
            // 0x00000001 to 0x000000FE, inclusive.
            boost::uint32_t playIncarnation;
            boost::uint32_t openFileId;
            boost::uint32_t padding;
            boost::uint32_t fileName;
            // FILE_ATTRIBUTE_MMS_CANSTRIDE 0x00800000
            // FILE_ATTRIBUTE_MMS_CANSEEK 0x01000000
            // FILE_ATTRIBUTE_MMS_BROADCAST 0x02000000
            // FILE_ATTRIBUTE_MMS_LIVE 0x04000000
            // FILE_ATTRIBUTE_MMS_PLAYLIST 0x40000000
            boost::uint32_t fileAttributes;
            double fileDuration;
            boost::uint32_t fileBlocks;
            boost::uint8_t unused1[16];
            boost::uint32_t filePacketSize;
            boost::uint64_t filePacketCount;
            boost::uint32_t fileBitRate;
            boost::uint32_t fileHeaderSize;
            boost::uint8_t unused2[36];

            MmspDataReportOpenFile()
                : playIncarnation(0)
                , openFileId(0)
                , padding(0x00000000)
                , fileName(0x00000000)
                , fileAttributes(0x00000000)
                , fileDuration(0.0)
                , fileBlocks(0)
                , filePacketSize(0)
                , filePacketCount(0)
                , fileBitRate(0)
                , fileHeaderSize(0)
            {
                memset(unused1, 0, sizeof(unused1));
                memset(unused2, 0, sizeof(unused2));
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                MmspReportMessageHeader::serialize(ar);

                ar & playIncarnation;
                ar & openFileId;
                ar & padding;
                ar & fileName;
                ar & fileAttributes;
                ar & fileDuration;
                ar & fileBlocks;
                ar & framework::container::make_array(unused1);
                ar & filePacketSize;
                ar & filePacketCount;
                ar & fileBitRate;
                ar & fileHeaderSize;
                ar & framework::container::make_array(unused2);
            }
        };

        struct MmspDataReportReadBlock
            : MmspReportMessageHeader
            , MmspMessageData<MmspMacToViewerMessage::REPORT_READ_BLOCK>
        {
            boost::uint32_t playIncarnation;
            boost::uint32_t playSequence;

            MmspDataReportReadBlock()
                : playIncarnation(0)
                , playSequence(0x00000000)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                MmspReportMessageHeader::serialize(ar);

                ar & playIncarnation;
                ar & playSequence;
            }
        };

        struct MmspDataReportRedirect
            : MmspReportMessageHeader
            , MmspMessageData<MmspMacToViewerMessage::REPORT_REDIRECT>
        {
            boost::uint16_t RedirectType;
            boost::uint16_t cbLocation;
            MmspString wszLocation;

            MmspDataReportRedirect()
                : RedirectType(0)
                , cbLocation(0)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                MmspReportMessageHeader::serialize(ar);

                ar & RedirectType;
                ar & cbLocation;

                wszLocation.size(cbLocation);

                ar & wszLocation;
            }
        };

        struct MmspDataSecurityChallenge
            : MmspMessageData<MmspMacToViewerMessage::SECURITY_CHALLENGE>
        {
            boost::uint32_t playIncarnation;
            boost::uint32_t cookie;
            boost::uint32_t cbToken;
            MmspString pToken;

            MmspDataSecurityChallenge()
                : playIncarnation(0)
                , cookie(0)
                , cbToken(0)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                ar & playIncarnation;
                ar & cookie;
                ar & cbToken;

                pToken.size(cbToken);

                ar & pToken;
            }
        };

        struct MmspDataReportStartPlaying
            : MmspReportMessageHeader
            , MmspMessageData<MmspMacToViewerMessage::REPORT_START_PLAYING>
        {
            boost::uint32_t playIncarnation;
            boost::uint32_t tigerFileId;
            boost::uint32_t unused1;
            boost::uint8_t unused2[12];

            MmspDataReportStartPlaying()
                : playIncarnation(0)
                , tigerFileId(0)
                , unused1(0x00000000)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                MmspReportMessageHeader::serialize(ar);

                ar & playIncarnation;
                ar & tigerFileId;
                ar & unused1;
            }
        };

        struct MmspDataReportStartStriding
            : MmspReportMessageHeader
            , MmspMessageData<MmspMacToViewerMessage::REPORT_START_STRIDING>
        {
            boost::uint32_t playIncarnation;
            boost::uint32_t spare;
            boost::uint8_t unused2[12];

            MmspDataReportStartStriding()
                : playIncarnation(0)
                , spare(0x00000000)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                MmspReportMessageHeader::serialize(ar);

                ar & playIncarnation;
                ar & spare;
            }
        };

        struct MmspDataReportStreamChange
            : MmspReportMessageHeader
            , MmspMessageData<MmspMacToViewerMessage::REPORT_STREAM_CHANGE>
        {
            boost::uint32_t dwTcpHdrIncarnation;
            boost::uint32_t cbPacketSize;
            boost::uint32_t cbHeaderSize;
            boost::uint32_t dwBitRate;
            boost::uint32_t dwStreamId;

            MmspDataReportStreamChange()
                : dwTcpHdrIncarnation(0x000000FF)
                , cbPacketSize(0)
                , cbHeaderSize(0)
                , dwBitRate(0)
                , dwStreamId(0x00000000)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                MmspReportMessageHeader::serialize(ar);

                ar & dwTcpHdrIncarnation;
                ar & cbPacketSize;
                ar & cbHeaderSize;
                ar & dwBitRate;
                ar & dwStreamId;
            }
        };

        struct MmspDataReportStreamSwitch
            : MmspReportMessageHeader
            , MmspMessageData<MmspMacToViewerMessage::REPORT_STREAM_SWITCH>
        {
            MmspDataReportStreamSwitch()
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                MmspReportMessageHeader::serialize(ar);
            }
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMSP_MMSP_MAC_TO_VIEWER_MESSAGE_H_
