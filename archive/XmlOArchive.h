// // XmlOArchive.h

#ifndef _UTIL_ARCHIVE_XML_O_ARCHIVE_H_
#define _UTIL_ARCHIVE_XML_O_ARCHIVE_H_

#include "util/archive/StreamOArchive.h"

#include <framework/string/Format.h>

#include <tinyxml/tinyxml.h>

#include <fstream>

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class XmlOArchive
            : public StreamOArchive<XmlOArchive<_Elem, _Traits>, _Elem, _Traits>
        {
            friend class StreamOArchive<XmlOArchive<_Elem, _Traits>, _Elem, _Traits>;
        public:
            XmlOArchive(
                std::basic_istream<_Elem, _Traits> & is)
                : StreamOArchive<XmlOArchive<_Elem, _Traits>, _Elem, _Traits>(*is.rdbuf())
            {
                init_xml();
            }

            XmlOArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : StreamOArchive<XmlOArchive<_Elem, _Traits>, _Elem, _Traits>(buf)
            {
                init_xml();
            }

            ~XmlOArchive()
            {
                save_xml();
            }

        public:
            /// 从流中读出变量
            template<class T>
            void save(
                T const & t)
            {
                TiXmlElement * elem = stack_.back();
                std::string str;
                framework::string::format2(str, t);
                TiXmlText * text = new TiXmlText(str.c_str());
                elem->LinkEndChild(text);
            }

            void save(
                std::string const & str)
            {
                TiXmlElement * elem = stack_.back();
                TiXmlText * text = new TiXmlText(str.c_str());
                elem->LinkEndChild(text);
            }

            using StreamOArchive<XmlOArchive<_Elem, _Traits>, _Elem, _Traits>::save;

            void save_start(
                std::string const & name)
            {
                TiXmlElement * parent = stack_.back();
                TiXmlElement * elem = new TiXmlElement(name.c_str());
                parent->LinkEndChild(elem);
                stack_.push_back(elem);
            }

            void save_end(
                std::string const & name)
            {
                stack_.pop_back();
            }

        private:
            void init_xml()
            {
                doc_.LinkEndChild(new TiXmlDeclaration("1.0", "utf-8", "yes"));
                doc_.LinkEndChild(new TiXmlElement("root"));
                stack_.push_back(doc_.RootElement());
            }

            void save_xml()
            {
                std::string file_name("xml_o_archive_" + framework::string::format(size_t(this)) + ".xml");
                doc_.SaveFile(file_name.c_str());
                {
                    std::ifstream ifs(file_name.c_str(), std::ios::binary);
                    std::basic_ostream<_Elem, _Traits> os(this->buf_);
                    os << ifs.rdbuf();
                }
                ::unlink(file_name.c_str());
            }

        private:
            TiXmlDocument doc_;
            std::vector<TiXmlElement *> stack_;
        };

    } // namespace archive
} // namespace util

#endif // _UTIL_ARCHIVE_XML_O_ARCHIVE_H_
