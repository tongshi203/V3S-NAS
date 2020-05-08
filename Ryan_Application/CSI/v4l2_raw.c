#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <string.h>


#define u8 unsigned char
#define  LOGD(...)  do {printf(__VA_ARGS__);printf("\n");} while (0)
#define DBG(fmt, args...) LOGD("%s:%d, " fmt, __FUNCTION__, __LINE__, ##args);
#define ASSERT(b) \
do \
{ \
    if (!(b)) \
    { \
        LOGD("error on %s:%d", __FUNCTION__, __LINE__); \
        return 0; \
    } \
} while (0)

#define VIDEO_DEVICE "/dev/video0"
#define IMAGE_WIDTH 188
#define IMAGE_HEIGHT 1
#define IMAGE_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT*2)
#define BUFFER_COUNT 4 //buffer count

int cam_fd = -1;
struct v4l2_buffer video_buffer[BUFFER_COUNT];
u8* video_buffer_ptr[BUFFER_COUNT];
u8 buf[IMAGE_SIZE];
u8 str_buf[188];;
const unsigned char Lookup_Table[256]={
0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};
int cam_open()
{
    cam_fd = open(VIDEO_DEVICE, O_RDWR);  //| O_NONBLOCK);//打开摄像头

    if (cam_fd >= 0) return 0;
    else return -1;
}

int cam_close()
{
    close(cam_fd);//关闭摄像头

    return 0;
}

int cam_select(int index)
{
    int ret;

    int input = index;
    ret = ioctl(cam_fd, VIDIOC_S_INPUT, &input);//设置输入源
    return ret;
}

int cam_init()
{
    int i;
    int ret;
    struct v4l2_format format;

    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//帧类型，用于视频捕获设备
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_SGRBG12;;//V4L2_PIX_FMT_JPEG;////8bit raw格式
    format.fmt.pix.width = IMAGE_WIDTH;//分辨率
    format.fmt.pix.height = IMAGE_HEIGHT;
    ret = ioctl(cam_fd, VIDIOC_TRY_FMT, &format);//设置当前格式
    if (ret != 0)
    {
        DBG("ioctl(VIDIOC_TRY_FMT) failed %d(%s)", errno, strerror(errno));
        return ret;
    }

    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	//format.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;//V4L2_PIX_FMT_SGRBG12;//V4L2_PIX_FMT_JPEG;////8bit raw格式
    ret = ioctl(cam_fd, VIDIOC_S_FMT, &format);//设置当前格式
    if (ret != 0)
    {
        DBG("ioctl(VIDIOC_S_FMT) failed %d(%s)", errno, strerror(errno));
        return ret;
    }

    struct v4l2_requestbuffers req;
    req.count = BUFFER_COUNT;//缓冲帧个数
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//缓冲帧数据格式
    req.memory = V4L2_MEMORY_MMAP;//内存映射方式
    ret = ioctl(cam_fd, VIDIOC_REQBUFS, &req);//申请缓冲区
    if (ret != 0)
    {
        DBG("ioctl(VIDIOC_REQBUFS) failed %d(%s)", errno, strerror(errno));
        return ret;
    }
    DBG("req.count: %d", req.count);
    if (req.count < BUFFER_COUNT)
    {
        DBG("request buffer failed");
        return ret;
    }

    struct v4l2_buffer buffer;
    memset(&buffer, 0, sizeof(buffer));
    buffer.type = req.type;
    buffer.memory = V4L2_MEMORY_MMAP;
    for (i=0; i<req.count; i++)
    {
        buffer.index = i;
        ret = ioctl (cam_fd, VIDIOC_QUERYBUF, &buffer);//获取缓冲帧地址
        if (ret != 0)
        {
            DBG("ioctl(VIDIOC_QUERYBUF) failed %d(%s)", errno, strerror(errno));
            return ret;
        }
        DBG("buffer.length: %d", buffer.length);
        DBG("buffer.m.offset: %d", buffer.m.offset);
        video_buffer_ptr[i] = (u8*) mmap(NULL, buffer.length, PROT_READ|PROT_WRITE, MAP_SHARED, cam_fd, buffer.m.offset);//内存映射
        if (video_buffer_ptr[i] == MAP_FAILED)
        {
            DBG("mmap() failed %d(%s)", errno, strerror(errno));
            return -1;
        }

        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = i;
        ret = ioctl(cam_fd, VIDIOC_QBUF, &buffer);//把缓冲帧放入队列中
        if (ret != 0)
        {
            DBG("ioctl(VIDIOC_QBUF) failed %d(%s)", errno, strerror(errno));
            return ret;
        }
    }

    int buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(cam_fd, VIDIOC_STREAMON, &buffer_type);//启动数据流
    if (ret != 0)
    {
        DBG("ioctl(VIDIOC_STREAMON) failed %d(%s)", errno, strerror(errno));
        return ret;
    }

    DBG("cam init done.");

    return 0;
}

int cam_get_image(u8* out_buffer, int out_buffer_size)
{
    int ret;
    struct v4l2_buffer buffer;

    memset(&buffer, 0, sizeof(buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = BUFFER_COUNT;
    ret = ioctl(cam_fd, VIDIOC_DQBUF, &buffer);//从队列中取出一帧
    if (ret != 0)
    {
        DBG("ioctl(VIDIOC_DQBUF) failed %d(%s)", errno, strerror(errno));
        return ret;
    }

    if (buffer.index < 0 || buffer.index >= BUFFER_COUNT)
    {
        DBG("invalid buffer index: %d", buffer.index);
        return ret;
    }

    DBG("dequeue done, index: %d", buffer.index);
    memcpy(out_buffer, video_buffer_ptr[buffer.index], IMAGE_SIZE);//缓冲帧数据拷贝出来
    DBG("copy done.");

    ret = ioctl(cam_fd, VIDIOC_QBUF, &buffer);//缓冲帧放入队列
    if (ret != 0)
    {
        DBG("ioctl(VIDIOC_QBUF) failed %d(%s)", errno, strerror(errno));
        return ret;
    }
    DBG("enqueue done.");

    return 0;
}

int main()
{
    int i;
    int ret;

    ret = cam_open();
    ASSERT(ret==0);

    ret = cam_select(0);
    ASSERT(ret==0);

    ret = cam_init();
    ASSERT(ret==0);

    int count = 0;
    while (1)
    {
        ret = cam_get_image(buf, IMAGE_SIZE);
        ASSERT(ret==0);

        char tmp[64] = {"---\n"};
        for (i=0; i<16; i++)
            sprintf(&tmp[strlen(tmp)], "%02x ", Lookup_Table[buf[i*2]]);
        LOGD("%s", tmp);
        for (i=0; i<188; i++)
        {
            str_buf[i] = Lookup_Table[buf[2*i]];
        }
        char filename[32];
        sprintf(filename, "/mnt/mmcblk0p1/%05d.raw", count++);
        int fd = open(filename,O_WRONLY|O_CREAT);//00700);//保存图像数据
        if (fd >= 0)
        {

            write(fd, str_buf, 188);
            close(fd);
        }
        else
        {
            LOGD("open() failed: %d(%s)", errno, strerror(errno));
        }
    }

    ret = cam_close();
    ASSERT(ret==0);

    return 0;
}
