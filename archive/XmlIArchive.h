// // XmlIArchive.h

#ifndef _UTIL_ARCHIVE_XML_I_ARCHIVE_H_
#define _UTIL_ARCHIVE_XML_I_ARCHIVE_H_

#include "util/archive/StreamIArchive.h"

#include <framework/string/Parse.h>

#include <iterator>

#include <tinyxml/tinyxml.h>

namespace util
{
    namespace archive
    {

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

            /// 从流中读出变标准库字符串
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

            using StreamIArchive<XmlIArchive<_Elem, _Traits>, _Elem, _Traits>::load;

            void load_start(
                std::string const & name)
            {
                Value & vp = value_stack_.back();
                assert(vp.type == Value::t_elem || vp.type == Value::t_set);
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
                            size_t c = 0;
                            for (TiXmlNode * node = vp.elem->FirstChild(); 
                                node; 
                                node = node->NextSibling(), ++c);
                            v.type = Value::t_count;
                            v.count = c;
                        } else {
                            v.type = Value::t_none;
                        }
                    }
                } else {
                    assert(name == "item");
                    v.type = Value::t_elem;
                    Value & vpp = *(&vp - 1);
                    vp.item = vpp.elem->IterateChildren(vp.item);
                    if (vp.item)
                        v.elem = vp.item->ToElement();
                    if (v.elem == NULL) {
                        v.type = Value::t_none;
                    }
                }
                if (v.type == Value::t_none) {
                    this->state(2);
                }
                value_stack_.push_back(v);
            }

            void load_end(
                std::string const & name)
            {
                Value & v = value_stack_.back();
                if (v.type == Value::t_count) {
                    v.type = Value::t_set;
                    v.item = NULL;
                } else {
                    value_stack_.pop_back();
                    if (v.type == Value::t_set) {
                        value_stack_.pop_back();
                    }
                }
            }

        private:
            void load_xml()
            {
                std::string str;
                std::basic_istream<_Elem, _Traits> is(&this->buf_);
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

    } // namespace archive
} // namespace util

#endif // _UTIL_ARCHIVE_XML_I_ARCHIVE_H_
