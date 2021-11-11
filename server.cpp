/* * * * * * * * * * * * * 
 * Author : xny
 * Date   : 2021,11,7
 * Item   : alsa udp server
 * Fun    : recv pcm data from udp and write to alsa device to play
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

/* global value */
char *buf;

sem_t idle;
sem_t ready;
struct pArg{
    snd_pcm_t *dev_fd;
    int play_p;
    int chunk;
    int bsize;
};

int socket_create(sockaddr_in *saddr,unsigned int port,const char ipaddr[])
{
    int sfd = socket(AF_INET,SOCK_DGRAM,0);
    if (sfd == -1)
    {
        printf ("create udp socket failed...\r\n");
        exit(-1);
    }
    saddr->sin_family = AF_INET;
    saddr->sin_addr.s_addr = inet_addr(ipaddr);
    saddr->sin_port = htons(port);
    printf ("ip:%s port:%d\r\n",ipaddr,port);
    int ret = bind(sfd,(sockaddr*)saddr,sizeof(sockaddr_in));
    if (ret == -1)
    {
        printf ("udp socket bind failed...\r\n");
        close(sfd);
        exit(-1);
    }
    return sfd;
}


void *(playPthread)(void *arg)
{
    struct pArg* p = (struct pArg*)arg;
    while (1)
    {
        sem_wait(&ready);
        int ret = snd_pcm_writei(p->dev_fd,&buf[p->play_p],p->chunk);
        sem_post(&idle);
        if (ret == -EPIPE)
            snd_pcm_prepare(p->dev_fd);
        printf ("buf point-> %d \twrite frames : %d\r\n",p->play_p,ret);
        p->play_p = (p->play_p+((p->chunk)<<1))%(p->bsize);
    }
}

int main(int argc,char **argv)
{
    if (argc<3)
    {
        printf ("params less:\r\n\
        eg: ./server <local_ip> <local_port>\r\n");
        return -1;
    }
    sockaddr_in saddr;
    int sfd = socket_create(&saddr,atoi(argv[2]),argv[1]);

    // sockaddr_in raddr;
    // unsigned int len;

    unsigned int chunk = 0;
    snd_pcm_t *fd = pcm_device_init_for_player(&chunk);
    if (fd == NULL)
        printf ("Null Player Handler...\r\n");

    // acq buffer
    unsigned int bsize = chunk * SSIZE * R * 10; // buffer for 10s 
    buf = (char*)malloc(sizeof(char)*bsize);
    memset(buf,0,bsize);

    /* pthread play */
    struct pArg myarg;
    myarg.dev_fd = fd;
    myarg.bsize = bsize;
    myarg.chunk = chunk;
    myarg.play_p = 0;
    pthread_t id;

    sem_init(&idle,0,(int)(bsize/(chunk<<1)));
    sem_init(&ready,0,0);

    pthread_create(&id,NULL,playPthread,&myarg);

    int frame_p = 0;
    while (1)
    {
        sem_wait(&idle);
        int rec = recvfrom(sfd,&buf[frame_p],chunk<<1,0,NULL,NULL);
        sem_post(&ready);
        printf ("buf point-> %d \trec frames : %d\r\n",frame_p,rec);
        frame_p = (frame_p+(chunk<<1))%(bsize);
    }
    pcm_device_deinit(fd);
    return 0;
}



