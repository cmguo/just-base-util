// MmspViewerToMacMessage.h 

#ifndef _UTIL_PROTOCOL_MMSP_MMSP_REQUEST_H_
#define _UTIL_PROTOCOL_MMSP_MMSP_REQUEST_H_

#include "util/protocol/mmsp/MmspMessageData.h"
#include "util/protocol/mmsp/MmspString.h"

namespace util
{
    namespace protocol
    {

        struct MmspViewerToMacMessage
        {
            enum IdEnum
            {
                CANCEL_READ_BLOCK   = 0x00030025,
                CLOSE_FILE          = 0x0003000d,
                CONNECT             = 0x00030001,
                CONNECT_FUNNEL      = 0x00030002,
                FUNNEL_INFO         = 0x00030018,
                LOGGING             = 0x00030032,
                OPEN_FILE           = 0x00030005,
                PONG                = 0x0003001b,
                READ_BLOCK          = 0x00030015,
                SECURITY_RESPONSE   = 0x0003001a,
                START_PLAYING       = 0x00030007,
                START_STRIDING      = 0x00030028,
                STOP_PLAYING        = 0x00030009,
                STREAM_SWITCH       = 0x00030033,
            };
        };

        struct MmspDataCancelReadBlock
            : MmspMessageData<MmspDataCancelReadBlock, MmspViewerToMacMessage::CANCEL_READ_BLOCK>
        {
            boost::uint32_t playIncarnation;

            MmspDataCancelReadBlock()
                : playIncarnation(0)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                ar & playIncarnation;
            }
        };

        struct MmspDataCloseFile
            : MmspMessageData<MmspDataCloseFile, MmspViewerToMacMessage::CLOSE_FILE>
        {
            boost::uint32_t playIncarnation;
            boost::uint32_t openFileId;

            MmspDataCloseFile()
                : playIncarnation(0)
                , openFileId(0)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                ar & playIncarnation;
                ar & openFileId;
            }
        };

        struct MmspDataConnect
            : MmspMessageData<MmspDataConnect, MmspViewerToMacMessage::CONNECT>
        {
            boost::uint32_t playIncarnation; // MMS_DISABLE_PACKET_PAIR or MMS_USE_PACKET_PAIR
            boost::uint32_t MacToViewerProtocolRevision;
            boost::uint32_t ViewerToMacProtocolRevision;
            MmspString subscriberName;

            MmspDataConnect()
                : playIncarnation(0)
                , MacToViewerProtocolRevision(0)
                , ViewerToMacProtocolRevision(0)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                ar & playIncarnation;
                ar & MacToViewerProtocolRevision;
                ar & ViewerToMacProtocolRevision;
                ar & subscriberName;
            }
        };

        struct MmspDataConnectFunnel
            : MmspMessageData<MmspDataConnectFunnel, MmspViewerToMacMessage::CONNECT_FUNNEL>
        {
            boost::uint32_t playIncarnation;
            boost::uint32_t maxBlockBytes;
            boost::uint32_t maxFunnelBytes;
            boost::uint32_t maxBitRate;
            boost::uint32_t funnelMode;
            MmspString funnelName;

            MmspDataConnectFunnel()
                : playIncarnation(0x00000000)
                , maxBlockBytes(0xFFFFFFFF)
                , maxFunnelBytes(0x00000000)
                , maxBitRate(0x00989680)
                , funnelMode(0x00000002)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                ar & playIncarnation;
                ar & maxBlockBytes;
                ar & maxFunnelBytes;
                ar & maxBitRate;
                ar & funnelMode;
                ar & funnelName;
            }
        };

        struct MmspDataFunnelInfo
            : MmspMessageData<MmspDataConnectFunnel, MmspViewerToMacMessage::FUNNEL_INFO>
        {
            // MMS_DISABLE_PACKET_PAIR
            // MMS_USE_PACKET_PAIR
            // MMS_PACKET_PAIR_TCP_HIGH_ENTROPY
            // MMS_PACKET_PAIR_UDP_HIGH_ENTROPY
            boost::uint32_t playIncarnation;

            MmspDataFunnelInfo()
                : playIncarnation(0x00000000)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                ar & playIncarnation;
            }
        };

        struct MmspDataOpenFile
            : MmspMessageData<MmspDataOpenFile, MmspViewerToMacMessage::OPEN_FILE>
        {
            // 0x00000001 to 0x000000FE, inclusive.
            boost::uint32_t playIncarnation;
            boost::uint32_t spare;
            boost::uint32_t token;
            boost::uint32_t cbtoken;
            MmspString fileName;
            MmspString tokenData;

            MmspDataOpenFile()
                : playIncarnation(0)
                , spare(0x00000000)
                , token(0x00000000)
                , cbtoken(0x00000000)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                ar & playIncarnation;
                ar & spare;
                ar & token;
                ar & cbtoken;

                if (token) {
                    fileName.size(token / 2);
                    tokenData.size(cbtoken / 2);
                }

                ar & fileName;
                ar & tokenData;
            }
        };

        struct MmspDataPong
            : MmspMessageData<MmspDataPong, MmspViewerToMacMessage::PONG>
        {
            boost::uint32_t dwParam1;
            boost::uint32_t dwParam2;

            MmspDataPong()
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

        struct MmspDataReadBlock
            : MmspMessageData<MmspDataReadBlock, MmspViewerToMacMessage::READ_BLOCK>
        {
            boost::uint32_t openFileId;
            boost::uint32_t fileBlockId;
            boost::uint32_t offset;
            boost::uint32_t length;
            boost::uint32_t flags;
            boost::uint32_t padding;
            double tEarliest;
            double tDeadline;
            // 0x00000001 to 0x000000FE, inclusive.
            boost::uint32_t playIncarnation;
            boost::uint32_t playSequence;

            MmspDataReadBlock()
                : openFileId(0x00000000)
                , fileBlockId(0x00000000)
                , offset(0x00000000)
                , length(0x00000000)
                , flags(0x00000000)
                , padding(0x00000000)
                , tEarliest(0.0)
                , tDeadline(3600.0)
                , playIncarnation(0)
                , playSequence(0x00000000)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                ar & openFileId;
                ar & fileBlockId;
                ar & offset;
                ar & length;
                ar & flags;
                ar & padding;
                ar & tEarliest;
                ar & tDeadline;
                ar & playIncarnation;
                ar & playSequence;
            }
        };

        struct MmspDataSecurityResponse
            : MmspMessageData<MmspDataSecurityResponse, MmspViewerToMacMessage::SECURITY_RESPONSE>
        {
            boost::uint32_t playIncarnation;
            boost::uint32_t cookie;
            boost::uint32_t cbToken;
            MmspString pToken;

            MmspDataSecurityResponse()
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

        struct MmspDataStartPlaying
            : MmspMessageData<MmspDataStartPlaying, MmspViewerToMacMessage::START_PLAYING>
        {
            boost::uint32_t openFileId;
            boost::uint32_t padding;
            double position;
            boost::uint32_t asfOffset;
            boost::uint32_t locationId;
            boost::uint32_t frameOffset;
            // 0x00000001 to 0x000000FE, inclusive.
            boost::uint32_t playIncarnation;
            boost::uint32_t dwAccelBandwidth; // optional
            boost::uint32_t dwAccelDuration; // optional
            boost::uint32_t dwLinkBandwidth; // optional

            MmspDataStartPlaying()
                : openFileId(0)
                , padding(0x00000000)
                , position(0.0)
                , asfOffset(0x00000000)
                , locationId(0x00000000)
                , frameOffset(0x00000000)
                , playIncarnation(0x00000000)
                , dwAccelBandwidth(0)
                , dwAccelDuration(0)
                , dwLinkBandwidth(0)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                ar & openFileId;
                ar & padding;
                ar & position;
                ar & asfOffset;
                ar & locationId;
                ar & frameOffset;
                ar & playIncarnation;

                if (ar) {
                    ar & dwAccelBandwidth;
                    ar & dwAccelDuration;
                    ar & dwLinkBandwidth;
                    ar.clear();
                }
            }
        };

        struct MmspDataStartStriding
            : MmspMessageData<MmspDataStartStriding, MmspViewerToMacMessage::START_STRIDING>
        {
            boost::uint32_t openFileId;
            boost::uint32_t padding;
            double position;
            boost::uint32_t asfOffset;
            boost::uint32_t locationId;
            boost::uint32_t frameOffset;
            // 0x00000001 to 0x000000FE, inclusive.
            boost::uint32_t playIncarnation;
            double rate;

            MmspDataStartStriding()
                : openFileId(0)
                , padding(0x00000000)
                , position(0.0)
                , asfOffset(0x00000000)
                , locationId(0x00000000)
                , frameOffset(0x00000000)
                , playIncarnation(0x00000000)
                , rate(0.0)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                ar & openFileId;
                ar & padding;
                ar & position;
                ar & asfOffset;
                ar & locationId;
                ar & frameOffset;
                ar & playIncarnation;
                ar & rate;
            }
        };

        struct MmspDataStopPlaying
            : MmspMessageData<MmspDataStopPlaying, MmspViewerToMacMessage::STOP_PLAYING>
        {
            boost::uint32_t openFileId;
            // 0x00000001 to 0x000000FE, inclusive.
            boost::uint32_t playIncarnation;

            MmspDataStopPlaying()
                : openFileId(0)
                , playIncarnation(0x00000000)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                ar & openFileId;
                ar & playIncarnation;
            }
        };

        struct MmspDataStreamSwitch
            : MmspMessageData<MmspDataStreamSwitch, MmspViewerToMacMessage::STREAM_SWITCH>
        {
            boost::uint32_t cStreamEntries;

            struct STREAM_SWITCH_ENTRY
            {
                boost::uint16_t wSrcStreamNumber;
                boost::uint16_t wDstStreamNumber;
                boost::uint16_t wThinningLevel;

                STREAM_SWITCH_ENTRY()
                    : wSrcStreamNumber(0)
                    , wDstStreamNumber(0)
                    , wThinningLevel(0)
                {
                }

                template <typename Archive>
                void serialize(Archive & ar)
                {
                    ar & wSrcStreamNumber;
                    ar & wDstStreamNumber;
                    ar & wThinningLevel;
                }
            };

            std::vector<STREAM_SWITCH_ENTRY> aStreamEntries;

            MmspDataStreamSwitch()
                : cStreamEntries(0)
            {
            }

            template <typename Archive>
            void serialize(Archive & ar)
            {
                ar & cStreamEntries;
                util::serialization::serialize_collection(ar, aStreamEntries, cStreamEntries);
            }
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMSP_MMSP_REQUEST_H_
