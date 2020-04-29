#if !defined(_WIN32) && !defined(WM_CREATE)

#ifndef  __MY_WINDOWS_MSG_H_20080711__
#define  __MY_WINDOWS_MSG_H_20080711__

/* window messages*/
#define WM_NULL                         0x0000
#define WM_CREATE                       0x0001
#define WM_DESTROY                      0x0002
#define WM_MOVE                         0x0003
#define WM_SIZE                         0x0005
#define WM_ACTIVATE                     0x0006
#define WM_SETFOCUS                     0x0007
#define WM_KILLFOCUS                    0x0008
#define WM_ENABLE                       0x000A
#define WM_SETTEXT                      0x000C
#define WM_GETTEXT                      0x000D
#define WM_GETTEXTLENGTH                0x000E
#define WM_PAINT                        0x000F
#define WM_CLOSE                        0x0010
#define WM_QUIT                         0x0012
#define WM_ERASEBKGND                   0x0014
#define WM_SHOWWINDOW                   0x0018
#define WM_SETFONT          		0x0030
#define WM_GETFONT      		0x0031
#define WM_WINDOWPOSCHANGED             0x0047
#define WM_NCCALCSIZE                   0x0083
#define WM_NCHITTEST                    0x0084
#define WM_NCPAINT                      0x0085
#define WM_GETDLGCODE                   0x0087
#define WM_NCMOUSEMOVE                  0x00A0
#define WM_NCLBUTTONDOWN                0x00A1
#define WM_NCLBUTTONUP                  0x00A2
#define WM_NCLBUTTONDBLCLK              0x00A3
#define WM_NCRBUTTONDOWN                0x00A4
#define WM_NCRBUTTONUP                  0x00A5
#define WM_NCRBUTTONDBLCLK              0x00A6
#define WM_KEYFIRST                     0x0100
#define WM_KEYDOWN                      0x0100
#define WM_KEYUP                        0x0101
#define WM_CHAR                         0x0102
#define WM_DEADCHAR                     0x0103	/* notimp*/
#define WM_SYSKEYDOWN                   0x0104	/* nyi*/
#define WM_SYSKEYUP                     0x0105	/* nyi*/
#define WM_SYSCHAR                      0x0106	/* nyi*/
#define WM_SYSDEADCHAR                  0x0107	/* notimp*/
#define WM_KEYLAST                      0x0108
#define WM_COMMAND                      0x0111
#define WM_TIMER                        0x0113
#define WM_HSCROLL                      0x0114
#define WM_VSCROLL                      0x0115

#define WM_MOUSEFIRST                   0x0200
#define WM_MOUSEMOVE                    0x0200
#define WM_LBUTTONDOWN                  0x0201
#define WM_LBUTTONUP                    0x0202
#define WM_LBUTTONDBLCLK                0x0203
#define WM_RBUTTONDOWN                  0x0204
#define WM_RBUTTONUP                    0x0205
#define WM_RBUTTONDBLCLK                0x0206
#define WM_MBUTTONDOWN                  0x0207
#define WM_MBUTTONUP                    0x0208
#define WM_MBUTTONDBLCLK                0x0209
#define WM_MOUSEWHEEL                   0x020A
#define WM_MOUSELAST                    0x020A

#define WM_CARET_CREATE    		0x03E0 /* Microwindows only*/
#define WM_CARET_DESTROY   		0x03E1 /* Microwindows only*/
#define WM_CARET_BLINK      		0x03E2 /* Microwindows only*/
#define WM_FDINPUT                      0x03F0 /* Microwindows only*/
#define WM_FDOUTPUT                     0x03F1 /* Microwindows only*/
#define WM_FDEXCEPT                     0x03F2 /* Microwindows only*/
#define WM_USER                         0x0400

#endif // __MY_WINDOWS_MSG_H_20080711__
#endif // !defined(_WIN32) && !defined(WM_CREATE)


