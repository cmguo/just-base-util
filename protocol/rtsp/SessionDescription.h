// SessionDescription.h

#ifndef _UTIL_PROTOCOL_PTSP_SDP_INFO_H_
#define _UTIL_PROTOCOL_PTSP_SDP_INFO_H_

#include <boost/intrusive_ptr.hpp>

namespace util
{
    namespace protocol
    {

        class Description
        {
        public:
            Description(
                char const * sequc, 
                char const * flags);

            Description(
                Description const & r);

            ~Description();

        public:
            bool desc_add(
                char key, 
                std::string const & value);

            size_t desc_cnt(
                char key) const;

            bool desc_get(
                char key, 
                std::string & value, 
                size_t idx = 0) const;

            std::string const & desc_get(
                char key, 
                size_t idx = 0) const;

            bool attr_add(
                std::string const & key, 
                std::string const & value);

            size_t attr_cnt(
                std::string const & key) const;

            bool attr_get(
                std::string const & key, 
                std::string & value, 
                size_t idx = 0) const;

            std::string attr_get(
                std::string const & key, 
                size_t idx = 0) const;

        public:
            bool load(
                std::istream & is);

            bool save(
                std::ostream & os) const;

        protected:
            class SectionDefine;

            struct Item;
            class Section;

            struct count_attr;

        protected:
            boost::intrusive_ptr<SectionDefine> declare_child(
                char const * sequc, 
                char const * flags);

            void declare_child(
                boost::intrusive_ptr<SectionDefine> child);

            Description add_child(
                char key);

            bool add_child(
                char key, 
                Description const & child);

            size_t cnt_child(
                char key) const;
            
            Description get_child(
                char key, 
                size_t idx) const;

        private:
            Description(
                boost::intrusive_ptr<Section> root);
        
        private:
            char add_line(
                Section * sec, 
                std::string line);

            Section * create_child(
                Section * sec, 
                char k);

        private:
            boost::intrusive_ptr<Section> root_;
        };

        class TimeDescription;
        class MediaDescription;

        class SessionDescription
            : public Description
        {
        public:
            SessionDescription();

        public:
            size_t time_count() const;

            TimeDescription time_add();

            bool time_add(
                TimeDescription const & desc);

            TimeDescription time(
                size_t idx) const;

        public:
            size_t media_count() const;

            MediaDescription media_add();

            bool media_add(
                MediaDescription const & desc);

            MediaDescription media(
                size_t idx) const;

        private:
            std::vector<TimeDescription> times_;
            std::vector<MediaDescription> medias_;
        };

        class TimeDescription
            : public Description
        {
        public:
            TimeDescription();

        private:
            TimeDescription(
                Description const & desc);

            friend class SessionDescription;
        };

        class MediaDescription
            : public Description
        {
        public:
            MediaDescription();

        public:
            bool set_rtp(
                std::string const & type, 
                int port, 
                int fmt);

            bool add_rtpmap(
                int fmt, 
                std::string const & codec, 
                int clock, 
                std::string const & parameters);

            bool add_fmtp(
                int fmt, 
                std::string const & parameters);

            bool get_rtp(
                std::string & type,
                int & port, 
                int & fmt);

            bool get_rtpmap(
                int fmt, // input 0 for the only one
                std::string & codec,
                int & clock, 
                std::string & parameters);

            bool get_fmtp(
                int fmt, // input 0 for the only one
                std::string & parameters);

        private:
            MediaDescription(
                Description const & desc);

            friend class SessionDescription;
        };

        inline std::istream & operator>>(
            std::istream & is, 
            Description & desc)
        {
            desc.load(is);
            return is;
        }

        inline std::ostream & operator<<(
            std::ostream & os, 
            Description const & desc)
        {
            desc.save(os);
            return os;
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_PTSP_SDP_INFO_H_

