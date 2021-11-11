/* * * * * * * * * * * * * 
 * Author : xny
 * Date   : 2021,11,7
 * Item   : alsa udp client
 * Fun    : acquire pcmA data from device and send to remote srv by rtp
 * Release: v1.1
 * Deps   : alsalib
 * * * * * * * * * * * * */

# include "stdio.h"
# include "stdlib.h"
# include "unistd.h"

# include "sys/types.h"
# include "sys/socket.h"
# include "arpa/inet.h"
# include "netinet/in.h"
# include "netinet/udp.h"

# include "pthread.h"
# include "semaphore.h"

# include "alsa_if.h"

# include "g711.h"

# include "rtp.h"
using namespace XY;

/* global value */
char *buf;

sem_t idle;
sem_t ready;
struct pArg{
    int cfd;
    int play_p;
    int chunk;
    int bsize;
    sockaddr_in raddr;
};

// RTP OBJ
RTP Convert;

int socket_create(sockaddr_in *caddr,unsigned int port,const char ipaddr[])
{
    int cfd = socket(AF_INET,SOCK_DGRAM,0);
    if (cfd == -1)
    {
        printf ("create udp socket failed...\r\n");
        exit(-1);
    }
    caddr->sin_family = AF_INET;
    caddr->sin_addr.s_addr = inet_addr(ipaddr);
    caddr->sin_port = htons(port);
    printf ("ip:%s port:%d\r\n",ipaddr,port);
    int ret = bind(cfd,(sockaddr*)caddr,sizeof(sockaddr_in));
    if (ret < 0)
    {
        printf ("bind failed...\r\n");
        close(cfd);
        exit(-1);
    }
    return cfd;
}

void *(sendPthread)(void *arg)
{
    struct pArg *p = (struct pArg *)arg;
    while (1)
    {
        sem_wait(&ready);
        int ret = sendto(p->cfd,Convert.m_buf,Convert.m_len,0,(sockaddr *)&(p->raddr),sizeof(sockaddr_in));
        sem_post(&idle);
        printf ("buf point-> %d \tsend frames : %d\r\n",p->play_p,ret);
        // p->play_p = (p->play_p+((p->chunk)))%(p->bsize);
    }   
}

int main(int argc,char **argv)
{
    if (argc<5)
    {
        printf ("params less:\r\n\
        eg: ./server <local_ip> <local_port> <remote_ip> <remote_port>\r\n");
        return -1;
    }
    sockaddr_in caddr;
    int cfd = socket_create(&caddr,atoi(argv[2]),argv[1]);

    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(argv[3]);
    saddr.sin_port = htons(atoi(argv[4]));
    unsigned int len = sizeof(sockaddr_in);

    unsigned int chunk = 0;
    snd_pcm_t *fd = pcm_device_init_for_record(&chunk);
    // acq buffer
    unsigned int bsize = chunk * SSIZE * R * 10; // buffer for 10s 
    buf = (char*)malloc(sizeof(char)*bsize);
    memset(buf,0,bsize);

    if (fd == NULL)
    {
        printf ("Null Recorder Handler...\r\n");
        exit(-1);
    }

    // pthread send
    pthread_t id;
    struct pArg myarg;
    myarg.cfd = cfd;
    myarg.play_p = 0;
    myarg.raddr = saddr;
    myarg.chunk = chunk;
    myarg.bsize = bsize;

    sem_init(&idle,0,(int)(bsize/(chunk)));
    sem_init(&ready,0,0);

    pthread_create(&id,NULL,sendPthread,&myarg);


    int frame_p = 0; // circle buffer
    while (1)
    {
        char *tmp_buf = (char*)malloc(sizeof(char)*(chunk<<1));
        int frames = snd_pcm_readi(fd,tmp_buf,chunk);
        int ziplen = Linear2Alaw(tmp_buf,chunk<<1,&buf[frame_p]);
        sem_wait(&idle);
        Convert.toStream((unsigned char*)&buf[frame_p],ziplen);
        sem_post(&ready);
        free(tmp_buf);
        printf ("buf point-> %d \tget frames : %d\tziplen : %d\r\n",frame_p,frames,ziplen);
        frame_p = (frame_p+(chunk))%(bsize);
    }
    pcm_device_deinit(fd);
    close(cfd);
    return 0;
}



