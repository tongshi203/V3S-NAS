#ifndef _ERROR_H_
#define _ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif
int GdError(const char *format, ...);

int GdErrorNull(const char *format, ...);
#ifdef __cplusplus
}
#endif
#endif/*_ERROR_H_*/
