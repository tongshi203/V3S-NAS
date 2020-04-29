/** 
    jasperlib.h

*/

#ifndef _JASPERLIB_H_
#define _JASPERLIB_H_
 
#define INTENA_REG (*(volatile int *)0x00500224)
#define INTPOL_REG (*(volatile int *)0x00500220)
#define QUASAR_IRQ 21

#ifdef __cplusplus
extern "C" {
#endif

static inline void disable_quasar_irq()
{
	int reg;
	reg = INTENA_REG;
	reg &= ~(1<<QUASAR_IRQ);
	INTENA_REG = reg;	
}

static inline void enable_quasar_irq()
{
	int reg;
	reg = INTENA_REG;
	reg |= (1<<QUASAR_IRQ);
	INTENA_REG = reg;	
}

// Lock and Unlock Quasar Interrupts
#define QLOCK() disable_quasar_irq();
#define QUNLOCK() enable_quasar_irq();

///////////////////////
//  Memory management 

// Here Buffer is just a single char. This char is the refcount.
// We can enhance if needed

typedef unsigned char Buffer; 

// BufferPool describe the buffers
// Here we have nb_buffers of the same size,
// contiguous in memory starting at start_address
// buffers are always aligned on 32 bit 
typedef struct 
{
	unsigned long *start_address;
	int buffers_size;
	int nb_buffers;
	Buffer *B;  // this is an array of Buffer
} BufferPool;

void init_buffer_pool(BufferPool *pBP, unsigned long *start_address,
		      int buffers_size, int nb_buffers);
int find_free_buffer(BufferPool *pBP, int rua_handle);
void deinit_buffer_pool(BufferPool *pBP);

static inline unsigned long *get_buffer_address(BufferPool *pBP, int idx)
{	
	return pBP->start_address+(idx*pBP->buffers_size)/sizeof(unsigned long);
}

static inline void addref_buffer(Buffer *B)
{
	QLOCK();
	(*B)++;    // B is the pointeur to the refcount
	QUNLOCK();
}

static inline void release_buffer(Buffer *B)
{
	QLOCK();
	(*B)--;    // B is the pointeur to the refcount
	QUNLOCK();
}

static inline unsigned long get_size_used_buffer(BufferPool *pBP) {
	unsigned long size = 0;
	int i;

	QLOCK();
	for(i = 0; i < pBP->nb_buffers; i++)
		if(pBP->B[i])
			size += pBP->buffers_size;
	QUNLOCK();

	return size;
}

feedpacket *getFreeFeedPacket(void);
void nextPacket(FeedPacketQ *pQ);
void FeedPacketQueuesInit(feedpacket *packets, int nbpackets);

#ifdef __cplusplus
}
#endif


#endif // _JASPERLIB_H_
