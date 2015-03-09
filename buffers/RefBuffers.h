// RefBuffers.h

#ifndef _UTIL_BUFFERS_REF_BUFFERS_H_
#define _UTIL_BUFFERS_REF_BUFFERS_H_

namespace util
{
    namespace buffers
    {

        template <
            typename BufferSequence
        >
        class RefBuffers
        {
        public:
            typedef typename BufferSequence::const_iterator const_iterator;

        public:
            RefBuffers(
                BufferSequence const & buffers)
                : buffers_(buffers)
            {
            }

            const_iterator begin() const
            {
                return buffers_.begin();
            }

            const_iterator end() const
            {
                return buffers_.end();
            }

        private:
            BufferSequence const & buffers_;
        };

        template <
            typename BufferSequence
        >
        RefBuffers<BufferSequence> const ref_buffers(
            BufferSequence const & buffers)
        {
            return RefBuffers<BufferSequence>(buffers);
        }

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_REF_BUFFERS_H_

