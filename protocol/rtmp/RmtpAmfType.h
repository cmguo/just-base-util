// RtmpAmfType.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_AMF_TYPE_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_AMF_TYPE_H_

namespace util
{
    namespace protocol
    {

        struct RtmpAmfType // AMF Data Type
        {
            enum Enum {
                NUMBER      = 0x00,
                BOOL        = 0x01,
                STRING      = 0x02,
                OBJECT      = 0x03,
                MOVIECLIP   = 0x04,
                _NULL       = 0x05,
                UNDEFINED   = 0x06,
                REFERENCE   = 0x07,
                MIXEDARRAY  = 0x08,
                OBJECT_END  = 0x09,
                ARRAY       = 0x0a,
                DATE        = 0x0b,
                LONG_STRING = 0x0c,
                UNSUPPORTED = 0x0d,
            };
        };

        struct RtmpAmfString
        {
            boost::uint16_t StringLength;
            std::string StringData;

            RtmpAmfString()
                : StringLength(0)
            {
            }

            RtmpAmfString(
                char const * str)
                : StringLength(strlen(str))
                , StringData(str)
            {
            }

            RtmpAmfString(
                std::string const & str)
                : StringLength(str.size())
                , StringData(str)
            {
            }

            friend bool operator==(
                RtmpAmfString const & l, 
                RtmpAmfString const & r)
            {
                return l.StringData == r.StringData;
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & StringLength;
                util::serialization::serialize_collection(ar, StringData, StringLength);
            }
        };

        struct RtmpAmfDate
        {
            double DateTime;
            boost::int16_t LocalDateTimeOffset;

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & DateTime;
                ar & LocalDateTimeOffset;
            }
        };

        struct RtmpAmfLongString
        {
            boost::uint32_t StringLength;
            std::vector<boost::uint8_t> StringData;

            RtmpAmfLongString()
                : StringLength(0)
            {
            }

            RtmpAmfLongString(
                std::vector<boost::uint8_t> const & data)
                : StringLength(data.size())
                , StringData(data)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & StringLength;
                util::serialization::serialize_collection(ar, StringData, StringLength);
            }
        };

        struct RtmpAmfObjectProperty;

        static inline bool FLV_Property_End(
            RtmpAmfObjectProperty const & Property);

        struct RtmpAmfObject
        {
            std::vector<RtmpAmfObjectProperty> ObjectProperties;

            SERIALIZATION_SPLIT_MEMBER();

            template <typename Archive>
            void load(
                Archive & ar)
            {
                RtmpAmfObjectProperty Property;
                ObjectProperties.clear();
                while (ar & Property) {
                    ObjectProperties.push_back(Property);
                    if (FLV_Property_End(Property))
                        break;
                }
            }

            template <typename Archive>
            void save(
                Archive & ar) const
            {
                for (size_t i = 0; i < ObjectProperties.size(); ++i) {
                    ar & ObjectProperties[i];
                }
                ar & RtmpAmfObjectProperty();
            }
        };

        struct RtmpAmfECMAArray
        {
            boost::uint32_t ECMAArrayLength;
            std::vector<RtmpAmfObjectProperty> Variables;

            SERIALIZATION_SPLIT_MEMBER();

            template <typename Archive>
            void load(
                Archive & ar)
            {
                ar & ECMAArrayLength;
                RtmpAmfObjectProperty Property;
                Variables.clear();
                while (ar & Property) {
                    Variables.push_back(Property);
                    if (FLV_Property_End(Property))
                        break;
                }
            }

            template <typename Archive>
            void save(
                Archive & ar) const
            {
                boost::uint32_t ECMAArrayLength = Variables.size();
                ar & ECMAArrayLength;
                for (size_t i = 0; i < Variables.size(); ++i) {
                    ar & Variables[i];
                }
                ar & RtmpAmfObjectProperty();
            }
        };

        struct RtmpAmfValue;

        struct RtmpAmfStrictArray
        {
            boost::uint32_t StrictArrayLength;
            std::vector<RtmpAmfValue> StrictArrayValue;

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & StrictArrayLength;
                util::serialization::serialize_collection(ar, StrictArrayValue, StrictArrayLength);
            }
        };

        struct RtmpAmfValue
        {
            boost::uint8_t Type;
            union {
                double Double;
                boost::uint64_t _Double;
                boost::uint8_t Bool;
                boost::uint16_t MovieClip;
                boost::uint16_t Null;
                boost::uint16_t Undefined;
                boost::uint16_t Reference;
                boost::uint16_t ObjectEndMarker;
                RtmpAmfDate Date;
            };
            RtmpAmfString String;
            RtmpAmfLongString LongString;
            RtmpAmfObject Object;
            RtmpAmfECMAArray ECMAArray;
            RtmpAmfStrictArray StrictArray;

            RtmpAmfValue(
                RtmpAmfType::Enum type = RtmpAmfType::UNDEFINED)
                : Type(type)
            {
            }

            RtmpAmfValue(
                double num)
                : Type(RtmpAmfType::NUMBER)
                , Double(num)
            {
            }

            //RtmpAmfValue(
            //    bool b)
            //    : Type(RtmpAmfType::BOOL)
            //    , Bool(b)
            //{
            //}

            RtmpAmfValue(
                char const * str)
                : Type(RtmpAmfType::STRING)
                , String(str)
            {
            }

            RtmpAmfValue(
                std::string const & str)
                : Type(RtmpAmfType::STRING)
                , String(str)
            {
            }

            RtmpAmfValue(
                std::vector<boost::uint8_t> const & data)
                : Type(RtmpAmfType::LONG_STRING)
                , LongString(data)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & Type;
                switch (Type) {
                    case RtmpAmfType::NUMBER:
                        ar & _Double;
                        break;
                    case RtmpAmfType::BOOL:
                        ar & Bool;
                        break;
                    case RtmpAmfType::STRING:
                        ar & String;
                        break;
                    case RtmpAmfType::OBJECT:
                        ar & Object;
                        break;
                    case RtmpAmfType::MOVIECLIP:
                        ar.fail();
                        break;
                    case RtmpAmfType::_NULL:
                        break;
                    case RtmpAmfType::UNDEFINED:
                        break;
                    case RtmpAmfType::REFERENCE:
                        ar & Reference;
                        break;
                    case RtmpAmfType::MIXEDARRAY:
                        ar & ECMAArray;
                        break;
                    case RtmpAmfType::OBJECT_END:
                        break;
                    case RtmpAmfType::ARRAY:
                        ar & StrictArray;
                        break;
                    case RtmpAmfType::DATE:
                        ar & Date;
                        break;
                    case RtmpAmfType::LONG_STRING:
                        ar & LongString;
                        break;
                    default:
                        ar.fail();
                        break;
                }
            }
        };

        struct RtmpAmfObjectProperty
        {
            RtmpAmfString PropertyName;
            RtmpAmfValue PropertyData;

            RtmpAmfObjectProperty() 
                : PropertyData(RtmpAmfType::OBJECT_END)
            {
            }

            template <typename T>
            RtmpAmfObjectProperty(
                std::string const & name, 
                T const & value)
                : PropertyName(name)
                , PropertyData(value)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & PropertyName
                    & PropertyData;
            }
        };

        static inline bool FLV_Property_End(
            RtmpAmfObjectProperty const & Property)
        {
            return Property.PropertyData.Type == RtmpAmfType::OBJECT_END;
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_AMF_TYPE_H_
