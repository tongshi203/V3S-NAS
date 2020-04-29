/***************************************************************************
 *	Author     : yuanlii@29bbs.net			_________________  *
 *	Date       : 01/10/2005 17:25:16		|    | This file | *
 *	File name  : mylibb.c				| vi |	powered  | *
 *	Description: Buffer functions source to replace	|____|___________| *
 *			the stdio functions				   *
 ***************************************************************************/
#include "mylibb.h"

/* End of file character.
 *    Some things throughout the library rely on this being -1.  */
#ifndef EOF
# define EOF (-1)
#endif


/* The possibilities for the third argument to `fseek'.
 *    These values should not be changed.  */
#define SEEK_SET	0	/* Seek from beginning of file.  */
#define SEEK_CUR	1	/* Seek from current position.  */
#define SEEK_END	2	/* Seek from end of file.  */

void
binit(void *in, int size, buffer_t *dest)/*{{{*/
{
	dest->start = in;
	dest->offset = 0;
	dest->size = size;
}/*}}}*/
 
int
bseek(buffer_t *buffer, int offset, int whence)/*{{{*/
{
	int current;

	switch(whence) {
	case SEEK_SET:
		if (offset >= buffer->size || offset < 0)
			return(-1);
		buffer->offset = offset;
		return(0);

	case SEEK_CUR:
		current = buffer->offset + offset;
		if (current >= buffer->size || current < 0)
			return(-1);
		buffer->offset = current;
		return(0);

	case SEEK_END:
		if (offset >= buffer->size || offset > 0)
			return(-1);
		buffer->offset = (buffer->size - 1) - offset;
		return(0);

	default:
		return(-1);
	}
}/*}}}*/
   
int
bread(buffer_t *buffer, void *dest, int size)/*{{{*/
{
	int copysize = size;

	if (buffer->offset == buffer->size)
		return(0);

	if (buffer->offset + size > buffer->size) 
		copysize = (buffer->size - buffer->offset);

	memcpy((void *)dest, (void *)(buffer->start + buffer->offset), copysize);

	buffer->offset += copysize;
	return(copysize);
}/*}}}*/
 
int
bgetc(buffer_t *buffer)/*{{{*/
{
	int ch;

	if (buffer->offset == buffer->size) 
		return(EOF);

	ch = *((unsigned char *) (buffer->start + buffer->offset));
	buffer->offset++;
	return(ch);
}/*}}}*/
 
char *
bgets(buffer_t *buffer, char *dest, int size)/*{{{*/
{
	int i,o;
	int copysize = size - 1;

	if (buffer->offset == buffer->size) 
		return(0);

	if (buffer->offset + copysize > buffer->size) 
		copysize = buffer->size - buffer->offset;

	for(o=0, i=buffer->offset; i < buffer->offset + copysize; i++, o++) {
		dest[o] = *((char *) (buffer->start + i));
		if (dest[o] == '\n')
			break;
	}

	buffer->offset = i + 1;
	dest[o + 1] = 0;

	return(dest);
}/*}}}*/
 
int
beof(buffer_t *buffer)/*{{{*/
{
	return (buffer->offset == buffer->size);
}/*}}}*/

/* by yuanlii@29bbs.net, 07/13/2005 15:14:08 */
int
bpos(buffer_t *buffer)
{
	return buffer->offset;
}

int
bsize(buffer_t *buffer_t)
{
	return buffer_t->size;
}
/* by yuanlii@29bbs.net */

