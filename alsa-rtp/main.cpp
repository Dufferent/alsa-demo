/* * * * * * * * * * * * * 
 * Author : xny
 * Date   : 2021,11,7
 * Item   : alsa test demo
 * Fun    : record save_file replay
 * Release: v1.1
 * Deps   : alsalib
 * * * * * * * * * * * * */

# include "stdio.h"
# include "stdlib.h"
# include "unistd.h"
# include "alsa_if.h"


int main(int argc,char **argv)
{
    int bsize = 0;
    if (argc < 2)
    {
        printf ("Usage : ./demo <times>\r\n");
        exit(-1);
    }
    char *buf = recorder(&bsize,atoi(argv[1]));
    restore_pcm(buf,bsize);
    player(buf,atoi(argv[1]));
    
    return 0;
}