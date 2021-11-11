// u-law, A-law and linear PCM conversions

#include <string.h>

#include "g711.h"


#define SIGN_BIT    (0x80)  // Sign bit for a A-law byte
#define QUANT_MASK  (0xf)   // Quantization field mask
#define NSEGS       (8)     // Number of A-law segments
#define SEG_SHIFT   (4)     // Left shift for segment number
#define SEG_MASK    (0x70)  // Segment field mask
#define BIAS        (0x84)  // Bias for linear code

static short seg_end[8] = { 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF };

// copy from CCITT G.711 specifications
unsigned char _u2a[128] = {     // u- to A-law conversions
      1,   1,   2,   2,   3,   3,   4,   4,
      5,   5,   6,   6,   7,   7,   8,   8,
      9,  10,  11,  12,  13,  14,  15,  16,
     17,  18,  19,  20,  21,  22,  23,  24,
     25,  27,  29,  31,  33,  34,  35,  36,
     37,  38,  39,  40,  41,  42,  43,  44,
     46,  48,  49,  50,  51,  52,  53,  54,
     55,  56,  57,  58,  59,  60,  61,  62,
     64,  65,  66,  67,  68,  69,  70,  71,
     72,  73,  74,  75,  76,  77,  78,  79,
     81,  82,  83,  84,  85,  86,  87,  88,
     89,  90,  91,  92,  93,  94,  95,  96,
     97,  98,  99, 100, 101, 102, 103, 104,
    105, 106, 107, 108, 109, 110, 111, 112,
    113, 114, 115, 116, 117, 118, 119, 120,
    121, 122, 123, 124, 125, 126, 127, 128
};

unsigned char _a2u[128] = {     // A- to u-law conversions
      1,   3,   5,   7,   9,  11,  13,  15,
     16,  17,  18,  19,  20,  21,  22,  23,
     24,  25,  26,  27,  28,  29,  30,  31,
     32,  32,  33,  33,  34,  34,  35,  35,
     36,  37,  38,  39,  40,  41,  42,  43,
     44,  45,  46,  47,  48,  48,  49,  49,
     50,  51,  52,  53,  54,  55,  56,  57,
     58,  59,  60,  61,  62,  63,  64,  64,
     65,  66,  67,  68,  69,  70,  71,  72,
     73,  74,  75,  76,  77,  78,  79,  79,
     80,  81,  82,  83,  84,  85,  86,  87,
     88,  89,  90,  91,  92,  93,  94,  95,
     96,  97,  98,  99, 100, 101, 102, 103,
    104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 126, 127
};


int search(int val, short *table, int size)
{
    for (int i=0; i<size; i++)
    {
        if (val <= *table++)
        {
            return i;
        }
    }

    return size;
}


// linear2alaw() - Convert a 16-bit linear PCM value to 8-bit A-law
/*
    Linear Input Code   Compressed Code
    -----------------   ---------------
    0000000wxyza        000wxyz
    0000001wxyza        001wxyz
    000001wxyzab        010wxyz
    00001wxyzabc        011wxyz
    0001wxyzabcd        100wxyz
    001wxyzabcde        101wxyz
    01wxyzabcdef        110wxyz
    1wxyzabcdefg        111wxyz
*/

#if 1

unsigned char linear2alaw(short pcm_val)
{
    int mask = 0xD5;
    if (pcm_val < 0)
    {
        mask    = 0x55;
        pcm_val = ~pcm_val;
    }

    int seg = search(pcm_val, seg_end, 8);
    if (seg >= 8)
    {
        return (0x7F ^ mask);
    }

    unsigned char aval = seg << SEG_SHIFT;
    if (seg < 2)
    {
        aval |= (pcm_val >> 4) & QUANT_MASK;
    }
    else
    {
        aval |= (pcm_val >> (seg + 3)) & QUANT_MASK;
    }

    return (aval ^ mask);
}

#else

unsigned char linear2alaw(short pcm)
{
    int sign = (pcm & 0x8000) >> 8;
    if (sign != 0)
    {
        pcm = -pcm;
    }
    if (pcm > 32635)
    {
        pcm = 32635;
    }

    int exponent = 7;
    for (int expMask = 0x4000; ((pcm & expMask) == 0) && (exponent > 0); exponent--, expMask >>= 1)
    {
    }

    int mantissa = (pcm >> ((exponent == 0) ? 4 : (exponent + 3))) & 0x0f;
    unsigned char alaw = (unsigned char)(sign | exponent << 4 | mantissa);

    return (unsigned char) (alaw ^ 0xD5);
}

#endif


// alaw2linear() - Convert an A-law value to 16-bit linear PCM
short alaw2linear(unsigned char a_val)
{
    a_val  ^= 0x55;

    int t   = (a_val & QUANT_MASK) << 4;
    int seg = (a_val & SEG_MASK)   >> SEG_SHIFT;

    switch (seg)
    {
        case 0:
            t += 8;
            break;

        case 1:
            t += 0x108;
            break;

        default:
            t += 0x108;
            t <<= seg - 1;
            break;
    }

    return ((a_val & SIGN_BIT) ? t : -t);
}


// linear2ulaw() - Convert a linear PCM value to u-law
unsigned char linear2ulaw(short pcm_val)
{
    int mask = 0;
    if (pcm_val < 0)
    {
        pcm_val = BIAS - pcm_val;
        mask = 0x7F;
    }
    else
    {
        pcm_val += BIAS;
        mask = 0xFF;
    }

    int seg = search(pcm_val, seg_end, 8);
    if (seg >= 8)
    {
        return (0x7F ^ mask);
    }

    unsigned char uval = (seg << 4) | ((pcm_val >> (seg + 3)) & 0xF);

    return (uval ^ mask);
}


// ulaw2linear() - Convert a u-law value to 16-bit linear PCM
short ulaw2linear(unsigned char u_val)
{
    u_val = ~u_val;

    int t = ((u_val & QUANT_MASK) << 3) + BIAS;
    t <<= ((unsigned)u_val & SEG_MASK) >> SEG_SHIFT;

    return ((u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS));
}


// A-law to u-law conversion
unsigned char alaw2ulaw(unsigned char aval)
{
    aval &= 0xff;
    return ((aval & 0x80) ? (0xFF ^ _a2u[aval ^ 0xD5]) : (0x7F ^ _a2u[aval ^ 0x55]));
}


// u-law to A-law conversion
unsigned char ulaw2alaw(unsigned char uval)
{
    uval &= 0xff;
    return ((uval & 0x80) ? (0xD5 ^ (_u2a[0xFF ^ uval] - 1)) : (0x55 ^ (_u2a[0x7F ^ uval] - 1)));
}




int Alaw2Linear(unsigned char *inStr, int inStrLen, char *outStr)
{
    int j = 0;

    unsigned char *alaw = (unsigned char *) inStr;
    for (int i=0; i<inStrLen; i++, j+=2)
    {
        short linear = alaw2linear(alaw[i]);
        memcpy(outStr+j, &linear, 2);
    }

    return j;
}


int Linear2Alaw(char *inStr, int inStrLen, char *outStr)
{
    int i = 0;

    short *linear = (short *) inStr;
    for (; i < inStrLen / 2; i++)
    {
        unsigned char alaw = linear2alaw(linear[i]);
        memcpy(outStr+i, &alaw, sizeof(unsigned char));
    }

    return i;
}


int Ulaw2Linear(unsigned char *inStr, int inStrLen, char *outStr)
{
    int j = 0;

    unsigned char *alaw = (unsigned char *) inStr;
    for (int i=0; i<inStrLen; i++, j+=2)
    {
        short linear = ulaw2linear(alaw[i]);
        memcpy(outStr+j, &linear, 2);
    }

    return j;
}


int Linear2Ulaw(char *inStr, int inStrLen, char *outStr)
{
    int i = 0;

    short *linear = (short *) inStr;
    for (; i < inStrLen / 2; i++)
    {
        unsigned char alaw = linear2ulaw(linear[i]);
        memcpy(outStr+i, &alaw, sizeof(unsigned char));
    }

    return i;
}


int Alaw2Ulaw(char *inStr, int inStrLen, char *outStr)
{
    int i = 0;

    unsigned char *alaw = (unsigned char *) inStr;
    for (; i<inStrLen; i++)
    {
        unsigned char ulaw = alaw2ulaw(alaw[i]);
        memcpy(outStr+i, &ulaw, sizeof(unsigned char));
    }

    return i;
}


int Ulaw2Alaw(char *inStr, int inStrLen, char *outStr)
{
    int i = 0;

    unsigned char *ulaw = (unsigned char *) inStr;
    for (; i<inStrLen; i++)
    {
        unsigned char alaw = ulaw2alaw(ulaw[i]);
        memcpy(outStr+i, &alaw, sizeof(unsigned char));
    }

    return i;
}

