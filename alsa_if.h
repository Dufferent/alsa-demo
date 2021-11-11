/* * * * * * * * * * * * * 
 * Author : xny
 * Date   : 2021,11,7
 * Item   : alsa interface
 * Fun    : device_init device_deinit ... 
 * Release: v1.1
 * Deps   : alsalib
 * * * * * * * * * * * * */

# ifndef __ALSA_IF_H
# define __ALSA_IF_H

# include "alsa/asoundlib.h"

# define R 16000       /* sample rate */
# define CHANNEL 1     /* single channel */
# define SSIZE 2       /* sample size 16bit / 8 = 2 Bytes */
# define SWIDTH 16     /* sample lenth */

/* extern fun */
char *recorder(int *sz,unsigned int dtimes);
void player(char* buf,unsigned int ptimes);
void restore_pcm(char *buf,int bsize);
char *read_pcm(const char name[],int *sz);

snd_pcm_t *pcm_device_init_for_record(unsigned int *chunk);
snd_pcm_t* pcm_device_init_for_player(unsigned int *chunk);
void pcm_device_deinit(snd_pcm_t *fd);
# endif