// MineHead.cpp

#include "util/Util.h"
#include "util/protocol/mine/MineHead.hpp"

#include <iostream>

namespace util
{
    namespace protocol
    {

        class TestHead
            : public MineHeadT<TestHead>
        {
        public:
            virtual bool get_line(
                std::string & line) const
            {
                return true;
            }

            virtual bool set_line(
                std::string const & line)
            {
                return true;
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                MineHeadT::serialize(ar);
            }
        };

        static void test()
        {
            TestHead h;
            MineHeadIArchive ia(std::cin);
            ia >> h;
            MineHeadOArchive oa(std::cout);
            oa << h;
        };

    } // namespace protocol
} // namespace util
