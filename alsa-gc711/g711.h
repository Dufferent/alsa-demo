#ifndef g711H
#define g711H


unsigned char   linear2ulaw (short pcm_val);
short           ulaw2linear (unsigned char u_val);
unsigned char   linear2alaw (short pcm_val);
short           alaw2linear (unsigned char a_val);

unsigned char   alaw2ulaw   (unsigned char);
unsigned char   ulaw2alaw   (unsigned char);


int Alaw2Linear (unsigned char *, int, char *);
int Ulaw2Linear (unsigned char *, int, char *);

int Alaw2Ulaw   (char *, int, char *);
int Ulaw2Alaw   (char *, int, char *);
int Linear2Alaw (char *, int, char *);
int Linear2Ulaw (char *, int, char *);


#endif

