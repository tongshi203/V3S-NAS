#ifndef CV4L2DATAPORT_H
#define CV4L2DATAPORT_H

class CV4l2DataPort
{
public:
    CV4l2DataPort();
    ~CV4l2DataPort();
    bool Initialize(char * VIDEO_DEVICE, int image_width,int image_height,int I_bus_width);
    void Invalid();
    int GetData( unsigned char * buf );

public:
    int video_fd;

private:
    int MyImage_Width;
    int MyImage_Height;
    int MyImage_Size;


};

#endif // V4L2DATAPORT_H
