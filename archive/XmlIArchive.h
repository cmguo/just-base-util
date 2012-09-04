// // XmlIArchive.h

#ifndef _UTIL_ARCHIVE_XML_I_ARCHIVE_H_
#define _UTIL_ARCHIVE_XML_I_ARCHIVE_H_

#include "util/archive/StreamIArchive.h"

#include <framework/string/Parse.h>

#include <boost/optional.hpp>

#include <iterator>

#include <tinyxml/tinyxml.h>

namespace util
{
    namespace archive
    {

        template<class T>
        struct AbnormalCollection
        {
            AbnormalCollection(
                T & t)
                : t(t)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar);
                
            T & t;
        };

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class XmlIArchive
            : public StreamIArchive<XmlIArchive<_Elem, _Traits>, _Elem, _Traits>
        {
            friend class StreamIArchive<XmlIArchive<_Elem, _Traits>, _Elem, _Traits>;
        public:
            XmlIArchive(
                std::basic_istream<_Elem, _Traits> & is)
                : StreamIArchive<XmlIArchive<_Elem, _Traits>, _Elem, _Traits>(*is.rdbuf())
            {
                load_xml();
            }

            XmlIArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : StreamIArchive<XmlIArchive<_Elem, _Traits>, _Elem, _Traits>(buf)
            {
                load_xml();
            }

        public:
            /// 从流中读出变量
            template<class T>
            void load(
                T & t)
            {
                Value & v = value_stack_.back();
                char const * value = NULL;
                switch (v.type) {
                    case Value::t_elem:
                        value = v.elem->GetText();
                        break;
                    case Value::t_attr:
                        value = v.attr;
                        break;
                    case Value::t_count:
                        t = (T)v.count;
                        return;
                    default:
                        assert(false);
                }
                if (value == NULL || framework::string::parse2(value, t)) {
                    this->state(2);
                }
            }

            /// 从流中读出标准库字符串
            void load(
                std::string & t)
            {
                Value & v = value_stack_.back();
                char const * value = NULL;
                switch (v.type) {
                    case Value::t_elem:
                        value = v.elem->GetText();
                        break;
                    case Value::t_attr:
                        value = v.attr;
                        break;
                    default:
                        assert(false);
                        return;
                }
                if (value == NULL) {
                    this->state(2);
                } else {
                    t = value;
                }
            }

            /// 从流中读出可选值
            template<class T>
            void load(
                boost::optional<T> & t)
            {
                Value & v = value_stack_.back();
                if (v.type == Value::t_none) {
                    t.reset();
                } else {
                    t.reset(T());
                    (*this) >> t.get();
                }
            }

            using StreamIArchive<XmlIArchive<_Elem, _Traits>, _Elem, _Traits>::load;

            void start_abnormal_collection()
            {
                Value & vp = value_stack_.back();
                Value v;
                v.type = Value::t_set2;
                v.attr = vp.elem->Value();
                vp.elem = NULL;
                value_stack_.push_back(v);
            }

            template<class T>
            AbnormalCollection<T> const abnormal_collection(
                T & t)
            {
                return AbnormalCollection<T>(t);
            }

            void load_start(
                std::string const & name)
            {
                Value & vp = value_stack_.back();
                assert(vp.type == Value::t_elem || vp.type == Value::t_set || vp.type == Value::t_set2);
                Value v;
                if (vp.type == Value::t_elem) {
                    TiXmlElement * elem = vp.elem->FirstChildElement(name.c_str());
                    if (elem) {
                        v.type = Value::t_elem;
                        v.elem = elem;
                    } else {
                        char const * attr = vp.elem->Attribute(name.c_str());
                        if (attr) {
                            v.type = Value::t_attr;
                            v.attr = attr;
                        }
                    }
                    if (v.type == Value::t_none) {
                        if (name == "count") {
                            v.type = Value::t_count;
                            v.count = 0;
                            for (TiXmlNode * node = vp.elem->FirstChild(); 
                                node; 
                                node = node->NextSibling(), ++v.count);
                            Value v2;
                            v2.type = Value::t_set;
                            value_stack_.push_back(v2);
                        }
                    } else {
                        if (name == "count") {
                            v.type = Value::t_count;
                            v.count = 0;
                            for (TiXmlNode * node = elem->NextSibling(); 
                                node; 
                                node = node->NextSibling(), ++v.count);
                            Value v2;
                            v2.type = Value::t_set;
                            v2.item = elem; // 从count节点的下一个开始
                            value_stack_.push_back(v2);
                        }
                    }
                } else if (vp.type == Value::t_set) {
                    assert(name == "item");
                    v.type = Value::t_elem;
                    Value & vpp = *(&vp - 1);
                    vp.item = vpp.elem->IterateChildren(vp.item);
                    if (vp.item)
                        v.elem = vp.item->ToElement();
                    if (v.elem == NULL) {
                        v.type = Value::t_none;
                    }
                } else { // vp.type == Value::t_set2
                    Value & vpp = *(&vp - 1);
                    Value & vppp = *(&vpp - 1);
                    if (name == "count") {
                        v.type = Value::t_count;
                        v.count = 0;
                        for (TiXmlNode * node = vppp.elem->FirstChild(vp.attr); 
                            node; 
                            node = node->NextSibling(vp.attr), ++v.count);
                    } else {
                        assert(name == "item");
                        vpp.item = vppp.elem->IterateChildren(vp.attr, vpp.item);
                        if (vpp.item) {
                            v.type = Value::t_elem;
                            v.elem = vpp.item->ToElement();
                        }
                    }
                }
                //if (v.type == Value::t_none) {
                //    this->state(2);
                //}
                value_stack_.push_back(v);
            }

            void load_end(
                std::string const & name)
            {
                int t = value_stack_.back().type;
                value_stack_.pop_back();
                if (t == Value::t_set || t == Value::t_set2) {
                    value_stack_.pop_back();
                }
            }

        private:
            void load_xml()
            {
                std::string str;
                std::basic_istream<_Elem, _Traits> is(this->buf_);
                std::istream_iterator<char, _Elem, _Traits> beg(is);
                std::istream_iterator<char, _Elem, _Traits> end;
                is >> std::noskipws;
                std::copy(beg, end, std::back_inserter(str));
                doc_.Parse(str.c_str(), NULL);
                if (doc_.Error())
                    this->state(1);
                else {
                    Value v;
                    v.type = Value::t_elem;
                    v.elem = doc_.RootElement();
                    value_stack_.push_back(v);
                }
            }

        private:
            TiXmlDocument doc_;
            struct Value
            {
                Value()
                    : type(t_none)
                    , elem(NULL)
                {
                }

                enum TypeEnum
                {
                    t_none, 
                    t_elem, 
                    t_attr, 
                    t_set, 
                    t_set2, 
                    t_count, 
                    t_item, 
                } type;
                union {
                    TiXmlElement * elem;
                    char const * attr;
                    size_t count;
                    TiXmlNode * item;
                };
            };
            std::vector<Value> value_stack_;
        };

        template <typename T>
        template <typename Archive>
        void AbnormalCollection<T>::serialize(
            Archive & ar)
        {
            ar.start_abnormal_collection();
            ar & t;
        }

    } // namespace archive
} // namespace util

#endif // _UTIL_ARCHIVE_XML_I_ARCHIVE_H_
