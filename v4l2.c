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

// open and initialize the device
int init_v4l2(void)
 {
     if( (fd =open(FILE_VIDEO, O_RDWR)) ==-1 )
     {
         printf("Error opening V4l2 interface.\n");
         return  FALSE;
     }

     if( ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1 )
     {
         printf("Error opening device %s: unable to query device capbilities.\n", FILE_VIDEO);
         return FALSE;
     }
     else {
         printf("driver:\t\t%s\n", cap.driver);
         printf("card:\t\t%s\n", cap.card);
         printf("bus_info:\t\t%s\n", cap.bus_info);
         printf("version:\t\t%d\n", cap.version);
         printf("capabilities:\t%x\n", cap.capabilities);
         printf("device capbilities:\t%x\n", cap.device_caps);

         if( (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE )
             printf("Device %s: supports capture.\n", FILE_VIDEO);
         if( (cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING )
             printf("Device %s: supports streaming.\n", FILE_VIDEO);
      }

     while( ioctl(fd, VIDIOC_ENUMINPUT, &input) != -1 )
     {
         printf("input:\t%d", input.index +1);
         printf("\t%s", input.name);
         printf("\t%d, non-tuner video input, such as camera sensor.", input.type);
         printf("\t%d\n", input.status);
         input.index ++;
     }

     while( ioctl(fd, VIDIOC_ENUMOUTPUT, &output) != -1 )
     {
         printf("output:\t%d", output.index +1);
         printf("\t%s", output.name);
         printf("\t%d\n", output.type);
         input.index ++;
     }

     if( ioctl(fd, VIDIOC_G_INPUT, &g_input) ==0 )
         printf("The current input is:\t%d\t%s\n", g_input.index +1, g_input.name);

     if( ioctl(fd, VIDIOC_G_OUTPUT, &g_output) ==0 )
         printf("The current input is:\t%d\t%s\n", g_output.index +1, g_output.name);

     fmtdesc.index = 0;
     fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     printf("Support format:\n");
     while( ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1)
     {
         printf("\t%d\t%s\n",fmtdesc.index+1, fmtdesc.description);
         fmtdesc.index ++;
     }

     fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
     fmt.fmt.pix.height = IMAGEHEIGHT;
     fmt.fmt.pix.width  = IMAGEWIDTH;
     fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

     if( ioctl(fd, VIDIOC_S_FMT, &fmt) == -1 )
     {
         printf("Unable to set format.\n");
         return FALSE;
     }

     if( ioctl(fd, VIDIOC_G_FMT, &fmt) == -1 )
     {
         printf("Unable to get format.\n");
         return FALSE;
     }
     else{
         printf("fmt.type:\t\t%d\n", fmt.type);
         printf("pix.height:\t\t%d\n", fmt.fmt.pix.height);
         printf("pix.width :\t\t%d\n", fmt.fmt.pix.width);
         printf("pix.field:\t\t%d\n", fmt.fmt.pix.field);
     }

     setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     setfps.parm.capture.timeperframe.numerator = 10;
     setfps.parm.capture.timeperframe.denominator = 10;

     if( ioctl(fd, VIDIOC_S_PARM, &setfps) == 0 )
         printf("streaming parameters true.\n");

     printf("init %s \t[OK]\n", FILE_VIDEO);
     return TRUE;
 }

int v4l2_grab(void)
{
    unsigned int n_buffers;

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if( ioctl(fd, VIDIOC_REQBUFS, &req) == -1 )
        printf("Request for buffers error.\n");

    buffers = malloc(req.count * sizeof(*buffers));
    if( !buffers )
    {
        printf("Out of memory.\n");
        return FALSE;
    }

    for(n_buffers =0; n_buffers <req.count; n_buffers ++)
    {
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        if( ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1)
        {
            printf("Query buff error.\n");
            return FALSE;
        }

        buffers[n_buffers].length = buf.length;

        buffers[n_buffers].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if( buffers[n_buffers].start == MAP_FAILED )
        {
            printf("buffer map error.\n");
            return FALSE;
        }
    }

    for( n_buffers =0; n_buffers <req.count; n_buffers ++)
    {
        buf.index = n_buffers;
        ioctl(fd, VIDIOC_QBUF, &buf);
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMON, &type);

    ioctl(fd, VIDIOC_DQBUF, &buf);

    printf("grab yuyv OK\n");
    return TRUE;
}

// close the device
int close_v4l2(void)
{
    if( fd != -1)
    {
        close( fd );
        return TRUE;
    }
    return FALSE;
}

void yuyv2_rgb888(void)
{
    int                      i, j;
    unsigned char   y1, y2, u, v;
    int r1, g1, b1, r2, g2, b2;
    char* pointer;

    pointer = buffers[0].start;

    for(i =0; i< 480; i++)
    {
        for(j =0; j< 320; j++)
        {
            y1 = *(pointer + (i * 320 + j) * 4);
            u   = *(pointer + (i * 320 + j) * 4 + 1);
            y2 = *(pointer + (i * 320 + j) * 4 + 2);
            v   = *(pointer + (i * 320 + j) * 4 + 3);

            r1  = y1 + 1.042 * (v - 128);
            g1 = y1 - 0.34414 * (u - 128) - 0.71414 * (v - 128);
            b1 = y1 + 1.772 * (u - 128);

            r2  = y2 + 1.042 * (v - 128);
            g2 = y2 - 0.34414 * (u - 128) - 0.71414 * (v - 128);
            b2 = y2 + 1.772 * (u - 128);

            if(r1 > 255)
                r1 = 255;
            else if(r1 < 0)
                r1 = 0;

            if(b1 > 255)
                b1 = 255;
            else if(b1 < 0)
                b1 = 0;

            if(g1 > 255)
                g1 = 255;
            else if(g1 < 0)
                g1 = 0;

            if(r2 > 255)
                r2 = 255;
            else if(r2 < 0)
                r2 = 0;

            if(b2 > 255)
                b2 = 255;
            else if(b2 < 0)
                b2 = 0;

            if(g2 > 255)
                g2 = 255;
            else if(g2 < 0)
                g2 = 0;

            *(frame_buffer + ( (480 -1 -i)* 320 + j )*6      ) = (unsigned char)b1;
            *(frame_buffer + ( (480 -1 -i)* 320 + j )*6 + 1) = (unsigned char)g1;
            *(frame_buffer + ( (480 -1 -i)* 320 + j )*6 + 2) = (unsigned char)r1;
            *(frame_buffer + ( (480 -1 -i)* 320 + j )*6 + 3) = (unsigned char)b2;
            *(frame_buffer + ( (480 -1 -i)* 320 + j )*6 + 4) = (unsigned char)g2;
            *(frame_buffer + ( (480 -1 -i)* 320 + j )*6 + 5) = (unsigned char)r2;
        }
    }
    printf("change to RGB OK  \n");
}
