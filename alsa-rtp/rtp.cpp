#include <assert.h>

#include <stdlib.h>

#include <string.h>
#include <strings.h>

#include <arpa/inet.h>

#include "rtp.h"

#include "time.h"
using namespace XY;


// 构造函数
RTP::RTP()
    :   m_codec(rtp_codec_pcma),
        m_size(0),
        m_len(0)
{
    clear();
}


// 析构函数
RTP::~RTP()
{
}


// 重载赋值函数
RTP & RTP::operator = (const RTP &other)
{
    memcpy(&m_header, &(other.m_header), sizeof(RtpHeader));

    m_codec = other.m_codec;

    m_size = other.m_size;
    memcpy(m_data, other.m_data, m_size);

    m_len = other.m_len;
    memcpy(m_buf, other.m_buf, m_len);

    return *this;
}


// 清除所有数据
void RTP::clear()
{
    bzero(&m_header, sizeof(RtpHeader));

    static bool initFlg = false;
    if (!initFlg)
    {
        initFlg = true;
        srand(clock());
    }

    m_header.ssrc       = rand();
    m_header.sequence   = 0;

    m_size = 0;
    m_len  = 0;
}


// 从RTP包中解析数据
bool RTP::fromStream(unsigned char *buf, int len)
{
    if (NULL == buf)
    {
        assert(0);
        return false;
    }

    if ((len <= rtp_header_size) || (len >= 1472)) // 避免超长数据导致内存越界
    {
        assert(0);
        return false;
    }

    m_size = len - rtp_header_size;
    memcpy(m_data, buf + rtp_header_size, m_size);

    return true;
}


// 构造数据的RTP包
void RTP::toStream(unsigned char *buf, int len)
{
    if (NULL == buf)
    {
        assert(0);
        return;
    }

    if ((len <= 0) || (len >= rtp_data_size - rtp_header_size)) // 避免超长数据导致内存越界
    {
        assert(0);
        return;
    }

    memcpy(m_buf + rtp_header_size, buf, len);
    m_len = len + rtp_header_size;

    if (m_header.sequence >= 65535)
    {
        m_header.sequence = 1;
    }
    else
    {
        ++m_header.sequence;
    }

    if (rtp_codec_g729 == m_codec)
    {
        m_header.timestamp += (len << 3);   // len * 8;
    }
    else
    {
        m_header.timestamp += len;
    }

    unsigned int *ui = (unsigned int *) m_buf;
    ui[0] = htonl((2 << 30) | (m_codec << 16) | (m_header.sequence));
    ui[1] = htonl(m_header.timestamp);
    ui[2] = htonl(m_header.ssrc);
}

