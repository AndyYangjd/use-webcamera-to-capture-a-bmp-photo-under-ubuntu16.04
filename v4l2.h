#ifndef  V4L2_H
#define V4L2_H

#define TRUE  1
#define FALSE 0

#define FREE(x)     if( (x) ) { free( (x) ); (x) = NULL; }

#define FILE_VIDEO  "/dev/video0"

#define BMP         	"/home/andy/Desktop/image_bmp.bmp"
#define YUV			"/home/andy/Desktop/image_yuv.yuv"

#define IMAGEWIDTH   640
#define IMAGEHEIGHT 480

int                                         fd;
struct v4l2_capability           cap;
struct v4l2_input                  input, g_input;
struct v4l2_output                output, g_output;
struct v4l2_fmtdesc             fmtdesc;
struct v4l2_format               fmt, fmtack;
struct v4l2_streamparm      setfps;
struct v4l2_requestbuffers  req;
struct v4l2_buffer                buf;
enum v4l2_buf_type           type;
unsigned char frame_buffer [IMAGEWIDTH * IMAGEHEIGHT *3];

struct buffer
{
    void* start;
    unsigned int length;
}* buffers;

int init_v4l2(void);
int v4l2_grab(void);
int close_v4l2(void);

void yuyv2_rgb888(void);

typedef unsigned char   BYTE;         // the size of char is 1
typedef unsigned short  WORD;       // the size of short is 2
typedef unsigned int      DWORD;    // the size  of int is 4, not use long(8 bytes)

#pragma pack(1)

typedef struct tagBITMAPFILEHEADER    // must be 14 bytes
{
    WORD     bfType;              // the flag of bmp, value is "BM"
    DWORD  bfSize;               // size BMP file, uint is bytes
    DWORD  bfReserved;      //  0
    DWORD  bf0ffBits;           // must be 54, offsets = bmp file header size + bmp info size + color palette size
}BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER    // must be 40 bytes
{
    DWORD       biSize;                       // must be 0x0028, equal 40
    DWORD       biWidth;                    // unit is pixel
    DWORD       biHeight;                   // unit is pixel
    WORD         biPlanes;                   // must be 1
    WORD         biBitCount;                // the bit of every pixel
    DWORD       biCompression;
    DWORD       biSizeImage;
    DWORD       biXPelsPerMeter;
    DWORD       biYPelsPerMeter;
    DWORD       biClrUsed;
    DWORD       biClrImportant;
}BITMAPINFOHEADER;

typedef struct tagRGBQUAD
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
}RGBQUAD;

int openVideo(void);
int closeVideo(void);

void getVideoData(unsigned char* data, int size);


#endif // V4L2_H
