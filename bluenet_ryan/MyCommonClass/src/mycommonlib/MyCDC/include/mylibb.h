/***************************************************************************
 *	Author     : yuanlii@29bbs.net			_________________  *
 *	Date       : 01/10/2005 17:24:23		|    | This file | *
 *	File name  : mylibb.h				| vi |	powered  | *
 *	Description: Buffer struct and function header	|____|___________| *
 *			to replace the stdio functions			   *
 ***************************************************************************/

#ifndef _MYLIBB_H
#define _MYLIBB_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****** Tools for buffer operation ******/
typedef struct { 	/* structure for reading images from buffer   */
	void *start; 	/* The pointer to the beginning of the buffer */
	int offset;	/* The current offset with in the buffer      */
	int size;	/* The total size of the buffer		      */
} buffer_t;

void binit(void *in, int size, buffer_t *dest);
int bseek(buffer_t *buffer, int offset, int whence);
int bread(buffer_t *buffer, void *dest, int size);
int bgetc(buffer_t *buffer);
char * bgets(buffer_t *buffer, char *dest, int size);
int beof(buffer_t *buffer);
int bpos(buffer_t *buffer);
int bsize(buffer_t *buffer_t);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif//_MYLIBB_H
