///---------------------------------------------------------------
///	
///	陈永健
///	2004.11.30	西安通视数据有限责任公司
///
///	文件用途：
///		MicroWindow 图形类
///---------------------------------------------------------------

#ifndef __MWIN_IMAGE_INCLUDE_20041130__
#define __MWIN_IMAGE_INCLUDE_20041130__

extern "C" {
	#include <device.h>
}

class CMWin_Image
{
public:
	CMWin_Image();
	virtual ~CMWin_Image();
	
public:
	bool LoadFromFile( HDC hDC, LPCSTR lpszFileName, int nFlags=0 );
	bool LoadFromBuffer( HDC hDC, PBYTE pBuffer, int nLen, int nFlags=0 );	
	int GetWidth(){ return m_ImgInfo.width; }
	int GetHeight(){ return m_ImgInfo.height; }
	void Draw( HDC hDC, int x, int y, int nWidthToFit=-1, int nHeightToFit=-1 );
	bool IsValid(){ return (m_nImageID>0); }
	void DeleteObject();

private:
	MWIMAGEINFO	m_ImgInfo;
	int m_nImageID;
};


#endif // __MWIN_IMAGE_INCLUDE_20041130__
