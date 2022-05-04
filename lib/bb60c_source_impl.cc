#include <gnuradio/io_signature.h>
#include "bb60c_source_impl.h"

#define PRINT_OVERFLOWS



namespace gr {
namespace shbb60 {

    bb60c_source::sptr
    bb60c_source::make(
        float center_freq,
        float ref_level,
        int decimation,
        float filter_bw)
    {
        return gnuradio::make_block_sptr<bb60c_source_impl>(
            center_freq,
            ref_level,
            decimation,
            filter_bw);
    }


    bb60c_source_impl::bb60c_source_impl(
        float center_freq,
        float ref_level,
        int decimation,
        float filter_bw)
        : gr::block(
            "bb60c_source",
            gr::io_signature::make(0, 0, 0),
            gr::io_signature::make(1, 1, sizeof(gr_complex)))
    {
        printf("[*] bb60c_source_ipml: center_freq =    %0.f\n", center_freq);
        printf("[*] bb60c_source_ipml: ref_level =      %0.f\n", ref_level);
        printf("[*] bb60c_source_ipml: decimation =     %d\n", decimation);
        printf("[*] bb60c_source_ipml: filter_bw =      %0.f\n", filter_bw);

        bbStatus status;
        m_handle = -1;
        int serials[8] = {0};
        int count = 0;

        printf("[*] bb60c_source_ipml: Signal Hound BB60C API version: %s\n", bbGetAPIVersion());

        bbGetSerialNumberList(serials, &count);
        if (count < 1)
        { 
            fprintf(stderr, "[!] no BB60 devices found\n");
            exit(EXIT_FAILURE);
        }

        printf("[*] bb60c_source_ipml: attempting to open device with serial number %d\n", serials[0]);

        status = bbOpenDeviceBySerialNumber(&m_handle, serials[0]);
        if (status != bbNoError)
        {
            fprintf(stderr, "[!] bb60c_source_ipml: bbOpenDeviceBySerialNumber() failed\n");
            fprintf(stderr, "%s\n", bbGetErrorString(status));
            exit(EXIT_FAILURE);
        }

        // BNC ports (only configured once)
        status = bbConfigureIO(m_handle, 0, 0);
        if (status != bbNoError)
        {
            fprintf(stderr, "[!] bb60c_source_ipml: bbConfigureIO() failed\n");
            fprintf(stderr, "%s\n", bbGetErrorString(status));
            bbAbort(m_handle);
            exit(EXIT_FAILURE);
        }

        printf("[*] bb60c_source_ipml: opened device\n");

        set_center_freq(center_freq);
        set_ref_level(ref_level);
        set_decimation(decimation);
        set_filter_bw(filter_bw);

        m_should_reconfigure = true;
    }


    bb60c_source_impl::~bb60c_source_impl()
    {
    }


    bool bb60c_source_impl::stop()
    {
        bbCloseDevice(m_handle);
        return true;
    }


    void bb60c_source_impl::set_center_freq(
        float center_freq)
    {
        const std::lock_guard<std::mutex> lock(m_mutex);
        m_center_freq = center_freq;
        m_should_reconfigure = true;
    }


    void bb60c_source_impl::set_ref_level(
        float ref_level)
    {
        const std::lock_guard<std::mutex> lock(m_mutex);
        m_ref_level = ref_level;
        m_should_reconfigure = true;
    }


    void bb60c_source_impl::set_decimation(
        int decimation)
    {
        const std::lock_guard<std::mutex> lock(m_mutex);
        m_decimation = decimation;
        m_should_reconfigure = true;
    }


    void bb60c_source_impl::set_filter_bw(
        float filter_bw)
    {
        const std::lock_guard<std::mutex> lock(m_mutex);
        m_filter_bw = filter_bw;
        m_should_reconfigure = true;
    }


    void bb60c_source_impl::reconfigure()
    {
        const std::lock_guard<std::mutex> lock(m_mutex);

        bbStatus status;

        status = bbConfigureIQCenter(m_handle, m_center_freq);
        if (status != bbNoError)
        {
            fprintf(stderr, "[!] bb60c_source_ipml: bbConfigureIQCenter() failed\n");
            fprintf(stderr, "%s\n", bbGetErrorString(status));
            bbAbort(m_handle);
            exit(EXIT_FAILURE);
        }

        status = bbConfigureLevel(m_handle, m_ref_level, BB_AUTO_ATTEN);
        if (status != bbNoError)
        {
            fprintf(stderr, "[!] bb60c_source_ipml: bbConfigureLevel() failed\n");
            fprintf(stderr, "%s\n", bbGetErrorString(status));
            bbAbort(m_handle);
            exit(EXIT_FAILURE);
        }

        status = bbConfigureIQ(m_handle, m_decimation, m_filter_bw);
        if (status != bbNoError)
        {
            fprintf(stderr, "[!] bb60c_source_ipml: bbConfigureIQ() failed\n");
            fprintf(stderr, "%s\n", bbGetErrorString(status));
            bbAbort(m_handle);
            exit(EXIT_FAILURE);
        }

        status = bbConfigureIQDataType(m_handle, bbDataType32fc);
        if (status != bbNoError)
        {
            fprintf(stderr, "[!] bb60c_source_ipml: bbConfigureIQDataType() failed\n");
            fprintf(stderr, "%s\n", bbGetErrorString(status));
            bbAbort(m_handle);
            exit(EXIT_FAILURE);
        }

        status = bbInitiate(m_handle, BB_STREAMING, BB_STREAM_IQ);
        if (status != bbNoError)
        {
            fprintf(stderr, "[!] bb60c_source_ipml: bbInitiate() failed\n");
            fprintf(stderr, "%s\n", bbGetErrorString(status));
            bbAbort(m_handle);
            exit(EXIT_FAILURE);
        }

        m_should_reconfigure = false;
    }


    int bb60c_source_impl::general_work(
        int noutput_items,
        gr_vector_int &ninput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
        if (m_should_reconfigure == true)
        {
            reconfigure();
        }

        bbIQPacket pkt;
        pkt.iqData = (void*)output_items[0];
        pkt.iqCount = noutput_items;
        pkt.triggers = NULL;
        pkt.triggerCount = 0;
        pkt.purge = BB_FALSE;

        bbStatus status = bbGetIQ(m_handle, &pkt);
        if (status != bbNoError)
        {
            fprintf(stderr, "[!] bbGetIQ() failed\n");
            fprintf(stderr, "%s\n", bbGetErrorString(status));
            bbAbort(m_handle);
            exit(EXIT_FAILURE);
        }

#ifdef PRINT_OVERFLOWS
        if (pkt.dataRemaining > 0)
        {
            printf("O"); fflush(stdout);
        }
#endif // PRINT_OVERFLOWS

        return noutput_items;
    }

} /* namespace shbb60 */
} /* namespace gr */
