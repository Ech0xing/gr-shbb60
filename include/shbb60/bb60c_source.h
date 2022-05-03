#ifndef INCLUDED_SHBB60_BB60C_SOURCE_H
#define INCLUDED_SHBB60_BB60C_SOURCE_H

#include <shbb60/api.h>
#include <gnuradio/block.h>



namespace gr {
namespace shbb60 {

    class SHBB60_API bb60c_source : virtual public gr::block
    {
    public:
        typedef std::shared_ptr<bb60c_source> sptr;

        static sptr make(
            float center_freq,
            float ref_level,
            int decimation,
            float filter_bw);
    };

} // namespace shbb60
} // namespace gr



#endif /* INCLUDED_SHBB60_BB60C_SOURCE_H */
