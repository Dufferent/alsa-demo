/* * * * * * * * * * * * * 
 * Author : xny
 * Date   : 2021,11,7
 * Item   : alsa interface
 * Fun    : device_init device_deinit ... 
 * Release: v1.1
 * Deps   : alsalib
 * * * * * * * * * * * * */

/* sondcard.h based on oss framework */
/* alsoundlib.h based on alsa framework */

// linux normal lib
# include "stdio.h"
# include "stdlib.h"
# include "unistd.h"
# include "fcntl.h"
# include "sys/types.h"
# include "sys/stat.h"
// alsa interface
# include "alsa_if.h"

char *recorder(int *sz,unsigned int dtimes)
{
    snd_pcm_t *fd;
    char devname[32] = "plughw:0,0";
    snd_pcm_open(&fd,devname,SND_PCM_STREAM_CAPTURE,0); // Recording
    /* parmaters set */
    snd_pcm_hw_params_t *hwdsc;
    snd_pcm_hw_params_alloca(&hwdsc);
    int ret = snd_pcm_hw_params_any(fd,hwdsc);
    if (ret < 0)
    {
      printf ("device Init Failed...\r\n");
      exit(-1);
    }
    /* ch==2 : LR LR LR ... */
    /* ch==1 : D D D D  ... */
    unsigned int arg;
    int dir = 0;
    snd_pcm_hw_params_set_access(fd,hwdsc,SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_channels(fd,hwdsc,CHANNEL);
    snd_pcm_hw_params_set_format(fd,hwdsc,SND_PCM_FORMAT_S16_LE);
    arg = R;
    snd_pcm_hw_params_set_rate_near(fd,hwdsc,&arg,&dir);

    /* Apply HW parameter settings to */
    /* PCM device and prepare device  */
    if (snd_pcm_hw_params(fd, hwdsc) < 0) {
      fprintf(stderr, "Error setting HW params.\n");
      exit(-1);
    }

    unsigned int get;
    snd_pcm_hw_params_get_access(hwdsc,(snd_pcm_access_t *)&get);
    printf ("ACCESS MODE:%d\r\n",get);
    snd_pcm_hw_params_get_channels(hwdsc,&get);
    printf ("CHANNEL    :%d\r\n",get);
    snd_pcm_hw_params_get_format(hwdsc,(snd_pcm_format_t *)&get);
    printf ("FORMAT     :%d\r\n",get);
    snd_pcm_hw_params_get_period_size(hwdsc,(snd_pcm_uframes_t *)&get,&dir);
    printf ("FRAMES     :%d dir:%d\r\n",get,dir);
    unsigned int chunk = get;
    snd_pcm_hw_params_get_rate(hwdsc,&get,&dir);
    printf ("Rate       :%d dir:%d\r\n",get,dir);

    /* Recording */
    unsigned int psize = sizeof(char)*R*SSIZE;
    unsigned int bsize = psize*dtimes;
    char *buf = (char*)malloc(bsize);
    memset(buf,0,bsize);
    unsigned int bufct = 0;
    printf ("Recording...\r\n");
    while (bufct < bsize)
    {
      snd_pcm_readi(fd,&buf[bufct],chunk);
      bufct += (chunk<<1);
    }
    snd_pcm_close(fd);
    *sz = bsize;
    return buf;
}

void player(char* buf,unsigned int ptimes)
{
    snd_pcm_t *fd;
    char devname[32] = "plughw:0,0";
    snd_pcm_open(&fd,devname,SND_PCM_STREAM_PLAYBACK,0); // Recording
    /* parmaters set */
    snd_pcm_hw_params_t *hwdsc;
    snd_pcm_hw_params_alloca(&hwdsc);
    int ret = snd_pcm_hw_params_any(fd,hwdsc);
    if (ret < 0)
    {
      printf ("device Init Failed...\r\n");
      exit(-1);
    }
    /* ch==2 : LR LR LR ... */
    /* ch==1 : D D D D  ... */
    unsigned int arg;
    int dir = 0;
    snd_pcm_hw_params_set_access(fd,hwdsc,SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_channels(fd,hwdsc,CHANNEL);
    snd_pcm_hw_params_set_format(fd,hwdsc,SND_PCM_FORMAT_S16_LE);
    arg = R;
    snd_pcm_hw_params_set_rate_near(fd,hwdsc,&arg,&dir);

    /* Apply HW parameter settings to */
    /* PCM device and prepare device  */
    if (snd_pcm_hw_params(fd, hwdsc) < 0) {
      fprintf(stderr, "Error setting HW params.\n");
      exit(-1);
    }

    unsigned int get;
    snd_pcm_hw_params_get_access(hwdsc,(snd_pcm_access_t *)&get);
    printf ("ACCESS MODE:%d\r\n",get);
    snd_pcm_hw_params_get_channels(hwdsc,&get);
    printf ("CHANNEL    :%d\r\n",get);
    snd_pcm_hw_params_get_format(hwdsc,(snd_pcm_format_t *)&get);
    printf ("FORMAT     :%d\r\n",get);
    snd_pcm_hw_params_get_period_size(hwdsc,(snd_pcm_uframes_t *)&get,&dir);
    printf ("FRAMES     :%d dir:%d\r\n",get,dir);
    unsigned int chunk = get;
    snd_pcm_hw_params_get_rate(hwdsc,&get,&dir);
    printf ("Rate       :%d dir:%d\r\n",get,dir);

    /* Recording */
    unsigned int psize = sizeof(char)*R*SSIZE;
    unsigned int bsize = psize*ptimes;
    unsigned int bufct = 0;
    printf ("Playing...\r\n");
    while (bufct < bsize)
    {
      int rec = snd_pcm_writei(fd,&buf[bufct],chunk);
      if (rec < 0)
        snd_pcm_prepare(fd);
      bufct += (chunk<<1);
    }
    snd_pcm_close(fd);
}

void restore_pcm(char *buf,int bsize)
{
    int restore = open("./record.pcm",O_CREAT|O_RDWR,S_IRWXU);
    if (restore < 0)
    {
      printf ("pcm file create failed...\r\n");
      exit(-1);
    }
    write(restore,buf,bsize);
    close(restore);
    system("chmod +rwx ./record.pcm");
}

char *read_pcm(const char name[],int *sz)
{
  int fd = open(name,O_RDONLY);
  char *buf = (char*)malloc(1);
  int rec;
  do {
    char tmp;
    rec = read(fd,&tmp,1);
    if (rec > 0)
    {
      buf = (char*)realloc(buf,((*sz)++)+1);
    }
  }while (rec > 0);
  return buf;
}

snd_pcm_t *pcm_device_init_for_record(unsigned int *chunk)
{
    snd_pcm_t *fd_op;
    snd_pcm_hw_params_t *hw_op;
    char devname[32] = "plughw:0,0";
    snd_pcm_open(&fd_op,devname,SND_PCM_STREAM_CAPTURE,0); // Recording
    snd_pcm_hw_params_alloca(&hw_op);
    int ret = snd_pcm_hw_params_any(fd_op,hw_op);
    if (ret < 0)
    {
      printf ("device Init Failed...\r\n");
      exit(-1);
    }
    /* ch==2 : LR LR LR ... */
    /* ch==1 : D D D D  ... */
    unsigned int arg;
    int dir = 0;
    snd_pcm_hw_params_set_access(fd_op,hw_op,SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_channels(fd_op,hw_op,CHANNEL);
    snd_pcm_hw_params_set_format(fd_op,hw_op,SND_PCM_FORMAT_S16_LE);
    arg = R;
    snd_pcm_hw_params_set_rate_near(fd_op,hw_op,&arg,&dir);
    snd_pcm_uframes_t val = 960;
    snd_pcm_hw_params_set_period_size_near(fd_op,hw_op,&val,&dir);

    /* Apply HW parameter settings to */
    /* PCM device and prepare device  */
    if (snd_pcm_hw_params(fd_op, hw_op) < 0) {
      fprintf(stderr, "Error setting HW params.\n");
      exit(-1);
    }

    unsigned int get;
    snd_pcm_hw_params_get_access(hw_op,(snd_pcm_access_t *)&get);
    printf ("ACCESS MODE:%d\r\n",get);
    snd_pcm_hw_params_get_channels(hw_op,&get);
    printf ("CHANNEL    :%d\r\n",get);
    snd_pcm_hw_params_get_format(hw_op,(snd_pcm_format_t *)&get);
    printf ("FORMAT     :%d\r\n",get);
    snd_pcm_hw_params_get_period_size(hw_op,(snd_pcm_uframes_t *)&get,&dir);
    printf ("FRAMES     :%d dir:%d\r\n",get,dir);
    *chunk = get;
    snd_pcm_hw_params_get_rate(hw_op,&get,&dir);
    printf ("Rate       :%d dir:%d\r\n",get,dir);

    return fd_op;
}

snd_pcm_t* pcm_device_init_for_player(unsigned int *chunk)
{
    snd_pcm_t *fd_op;
    snd_pcm_hw_params_t *hw_op;
    char devname[32] = "plughw:0,1";
    snd_pcm_open(&fd_op,devname,SND_PCM_STREAM_PLAYBACK,0); // Recording
    snd_pcm_hw_params_alloca(&hw_op);
    int ret = snd_pcm_hw_params_any(fd_op,hw_op);
    if (ret < 0)
    {
      printf ("device Init Failed...\r\n");
      exit(-1);
    }
    /* ch==2 : LR LR LR ... */
    /* ch==1 : D D D D  ... */
    unsigned int arg;
    int dir = 0;
    snd_pcm_hw_params_set_access(fd_op,hw_op,SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_channels(fd_op,hw_op,CHANNEL);
    snd_pcm_hw_params_set_format(fd_op,hw_op,SND_PCM_FORMAT_S16_LE);
    arg = R;
    snd_pcm_hw_params_set_rate_near(fd_op,hw_op,&arg,&dir);
    snd_pcm_uframes_t val = 960;
    snd_pcm_hw_params_set_period_size_near(fd_op,hw_op,&val,&dir);
    /* Apply HW parameter settings to */
    /* PCM device and prepare device  */
    if (snd_pcm_hw_params(fd_op, hw_op) < 0) {
      fprintf(stderr, "Error setting HW params.\n");
      exit(-1);
    }

    unsigned int get;
    snd_pcm_hw_params_get_access(hw_op,(snd_pcm_access_t *)&get);
    printf ("ACCESS MODE:%d\r\n",get);
    snd_pcm_hw_params_get_channels(hw_op,&get);
    printf ("CHANNEL    :%d\r\n",get);
    snd_pcm_hw_params_get_format(hw_op,(snd_pcm_format_t *)&get);
    printf ("FORMAT     :%d\r\n",get);
    snd_pcm_hw_params_get_period_size(hw_op,(snd_pcm_uframes_t *)&get,&dir);
    printf ("FRAMES     :%d dir:%d\r\n",get,dir);
    *chunk = get;
    snd_pcm_hw_params_get_rate(hw_op,&get,&dir);
    printf ("Rate       :%d dir:%d\r\n",get,dir);

    return fd_op;
}

void pcm_device_deinit(snd_pcm_t *fd)
{
    snd_pcm_close(fd);
}