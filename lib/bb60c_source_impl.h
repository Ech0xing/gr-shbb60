#ifndef INCLUDED_SHBB60_BB60C_SOURCE_IMPL_H
#define INCLUDED_SHBB60_BB60C_SOURCE_IMPL_H

#include <shbb60/bb60c_source.h>

#include <bb_api.h>



// temporary BB60C constants
#define CENTER_FREQ         2425000000.0    // Hz
#define REF_LEVEL           -10.0           // dBm
#define DOWNSAMPLE_FACTOR   4               // 40 MS/s / 4 = 10 MS/s
#define FILTER_BW           8000000         // 8.0 MHz



namespace gr {
namespace shbb60 {

class bb60c_source_impl : public bb60c_source
{
    public:
        bb60c_source_impl(
            float center_freq,
            float ref_level,
            int decimation,
            float filter_bw);

        ~bb60c_source_impl();

        bool stop() override;

        int general_work(
            int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) override;

    private:
        float m_center_freq;
        float m_ref_level;
        int m_decimation;

        int m_handle;
};

} // namespace shbb60
} // namespace gr



#endif /* INCLUDED_SHBB60_BB60C_SOURCE_IMPL_H */
