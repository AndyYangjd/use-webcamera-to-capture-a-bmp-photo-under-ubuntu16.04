#include <unistd.h>         // unix.std =windows.h, contain read, write, getpid
#include <sys/types.h>   // contain size_t, time_t, pid_t, dev_t
#include <sys/stat.h>      // contain all the message of a file
#include <fcntl.h>           //  handle a file
#include <stdio.h>
#include <sys/ioctl.h>     //  set and control the IO of device
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <linux/videodev2.h>

#include "v4l2.h"

int main(void)
{
    FILE *fp1, *fp2;
    BITMAPFILEHEADER  bf;
    BITMAPINFOHEADER bi;

    fp1 = fopen(BMP, "wb");
    if(!fp1)
        {
            printf("open "BMP" error\n");
            return(FALSE);
        }

    fp2 = fopen(YUV, "wb");
    if(!fp2)
        {
            printf("open "YUV" error\n");
            return(FALSE);
        }

    if( init_v4l2() == FALSE )
        return FALSE;

    bi.biSize    = 40;
    bi.biWidth  = IMAGEWIDTH;
    bi.biHeight = IMAGEHEIGHT;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = 0;
    bi.biSizeImage = IMAGEWIDTH * IMAGEHEIGHT * 3;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    bf.bfType = 0x4D42;
    bf.bfSize = 54 + bi.biSizeImage;
    bf.bfReserved = 0;
    bf.bf0ffBits = 54;

    v4l2_grab();
    fwrite(buffers[0].start, 640*480*2, 1, fp2);
    printf("save\tYUV\tOK\n");

    yuyv2_rgb888();
    fwrite(&bf, 14, 1, fp1);
    fwrite(&bi, 40, 1, fp1);
    fwrite(frame_buffer, bi.biSizeImage, 1, fp1);
    printf("save\tBMP\tOK\n");

    fclose(fp1);
    fclose(fp2);
    close_v4l2();

    return TRUE;
}
