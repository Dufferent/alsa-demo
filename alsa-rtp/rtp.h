#ifndef rtpH
#define rtpH


/*
rtp payload type RFC1890
      PT         encoding      audio/video    clock rate    channels
                 name          (A/V)          (Hz)          (audio)
      _______________________________________________________________
      0          PCMU          A              8000          1
      1          1016          A              8000          1
      2          G721          A              8000          1
      3          GSM           A              8000          1
      4          G723          A              8000          1
      5          DVI4          A              8000          1
      6          DVI4          A              16000         1
      7          LPC           A              8000          1
      8          PCMA          A              8000          1
      9          G722          A              8000          1
      10         L16           A              44100         2
      11         L16           A              44100         1
      12         unassigned    A
      13         unassigned    A
      14         MPA           A              90000        (see text)
      15         G728          A              8000          1
      18         G729
      16--23     unassigned    A
      24         unassigned    V
      25         CelB          V              90000
      26         JPEG          V              90000
      27         unassigned    V
      28         nv            V              90000
      29         unassigned    V
      30         unassigned    V
      31         H261          V              90000
      32         MPV           V              90000
      33         MP2T          AV             90000
      34--71     unassigned    ?
      72--76     reserved      N/A            N/A           N/A
      77--95     unassigned    ?
      96--127    dynamic       ?
*/


enum EnumRtpCodec
{
    rtp_codec_pcmu  = 0,
    rtp_codec_g721  = 2,
    rtp_codec_gsm   = 3,
    rtp_codec_g723  = 4,
    rtp_codec_pcma  = 8,
    rtp_codec_g722  = 9,
    rtp_codec_g729  = 18,
    rtp_codec_h263  = 34,
    rtp_codec_2833  = 96,   // 101
    rtp_codec_ilbc  = 98,
    rtp_codec_h264  = 99,   // 104
};


struct RtpHeader
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
        unsigned int    count:4;        // csrc count
        unsigned int    extension:1;    // header extension flag
        unsigned int    padding:1;      // padding flag - for encryption
        unsigned int    version:2;      // protocol version

        unsigned int    type:7;         // payload type
        unsigned int    marker:1;       // marker bit - for profile
        unsigned int    sequence:16;    // sequence number of this packet
#elif __BYTE_ORDER == __BIG_ENDIAN
        unsigned int    version:2;      // protocol version
        unsigned int    padding:1;      // padding flag - for encryption
        unsigned int    extension:1;    // header extension flag
        unsigned int    count:4;        // csrc count

        unsigned int    marker:1;       // marker bit - for profile
        unsigned int    type:7;         // payload type
        unsigned int    sequence:16;    // sequence number of this packet
#else
    #error "Problem in <endian.h>"
#endif
        unsigned int    timestamp;      // timestamp of this packet
        unsigned int    ssrc;           // source of packet
};




namespace XY
{

    // RTP包
    class RTP
    {
        enum EnumRtpSize
        {
            rtp_header_size = 12,   // RTP包的包头长度
            rtp_data_size   = 1472 - rtp_header_size
        };

    public:
        RTP();
        virtual ~RTP();

    public:
        RTP &   operator =  (const RTP &other);             // 重载赋值函数

        void    clear       ();                             // 清除所有数据

        bool    fromStream  (unsigned char *buf, int len);  // 从RTP包中解析数据
        void    toStream    (unsigned char *buf, int len);  // 构造数据的RTP包

    public:
        void *  rtpBuf      ()                  { return m_buf; }
        int     rtpLen      ()                  { return m_len; }

        void *  dataBuf     ()                  { return m_data; }
        int     dataLen     ()                  { return m_size; }

    public:
        int     codec       ()                  { return m_codec;  }
        void    codec       (int codec)         { m_codec = codec; }

    public:
        RtpHeader       m_header;               // RTP数据包头
        int             m_codec;                // RTP编码类型
        unsigned char   m_data[rtp_data_size];  // RTP数据内容
        size_t          m_size;                 // RTP数据长度
        unsigned char   m_buf[rtp_data_size];   // RTP数据缓冲区
        int             m_len;                  // RTP缓冲区长度
    };

}   // namespace XY


#endif

