// MmspMessage.h

#ifndef _UTIL_PROTOCOL_MMSP_MMSP_MESSAGE_H_
#define _UTIL_PROTOCOL_MMSP_MMSP_MESSAGE_H_

namespace util
{
    namespace protocol
    {

        struct MmspMessageHeader
        {
        public:
            boost::uint32_t chunkLen;              // 当前位置到结束的块的个数 X 8 byte
            boost::uint32_t MID;                    // 命令方向和ID

        public:
            MmspMessageHeader(
                boost::uint32_t id = 0)
                : chunkLen(0)
                , MID(id)
            {
            }

            static boost::uint32_t chunk_size(
                boost::uint32_t & n)
            {
                boost::uint32_t align_size = n & 0x07;
                align_size = (align_size == 0) ? 0 : 8 - align_size;
                align_size += n;
                n = align_size - n;
                return align_size / 8;
            }

            static boost::uint8_t const HEAD_SIZE = 8;

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & chunkLen;
                ar & MID;
            }
        };

        struct MmspReportMessageHeader
        {
            boost::uint32_t hr;

            MmspReportMessageHeader()
                : hr(0x00000000)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & hr;
            }
        };

        template <
            boost::uint32_t ID
        >
        struct MmspMessageData
        {
            static boost::uint32_t const StaticId = ID;
        };

        class MmspMessage
            : MmspMessageHeader
        {
        public:
            MmspMessage();

            ~MmspMessage();

        public:
            template <typename T>
            T & get()
            {
                if (!is<T>()) {
                    reset();
                    construct<T>();
                }
                return as<T>();
            }

            void reset();

            bool empty() const;

            template <typename T>
            bool is() const
            {
                return destroy_ == (destroy_t)&MmspMessage::destroy<T>;
            }

            template <typename T>
            T const & as() const
            {
                assert(is<T>());
                return *(T const *)data_;
            }

            template <typename T>
            T & as()
            {
                assert(is<T>());
                return *(T *)data_;
            }

        public:
            MmspMessageHeader const & header() const
            {
                return *this;
            }

            boost::uint32_t id() const
            {
                return MID;
            }

        private:
            template <typename T>
            void construct()
            {
                new (data_)T;
                MID = T::StaticId;
                destroy_ = &MmspMessage::destroy<T>;
            }

            template <typename T>
            void destroy()
            {
                ((T *)data_)->~T();
                MID = 0;
                destroy_ = NULL;
            }

        private:
            typedef void (MmspMessage::* destroy_t)();
            destroy_t destroy_;
            char data_[256];
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMSP_MMSP_MESSAGE_H_
