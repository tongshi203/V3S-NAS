// GenDVBCRC.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


/*
 * Calculate the value to XOR into the shifted CRC register for the given index
 * NumBits should be the "width" of the chunk being operated on eg: 4 or 8. Poly
 * is the polynomial to use eg 0x1021
 */
unsigned short GenCRC16TableEntry( const unsigned short index, const short NumBits, const unsigned short Poly )
{
	int i;
	unsigned short Ret;
		
	// Prepare the initial setup of the register so the index is at the
	// top most bits.
	Ret = index;
	Ret <<= 16 - NumBits;

	for( i = 0; i < NumBits; i++ ) {
		if( Ret & 0x8000 )
			Ret = (Ret << 1) ^ Poly;
		else 
			Ret = Ret << 1;
	}
	
	return Ret;
}

/*
 * Calculate the value to XOR into the shifted CRC register for the given index
 * NumBits should be the "width" of the chunk being operated on eg: 4 or 8. Poly
 * is the polynomial to use eg 0x1021
 */
unsigned long GenCRC32TableEntry( const unsigned short index, const short NumBits, const unsigned long Poly )
{
	int i;
	unsigned long Ret;
		
	// Prepare the initial setup of the register so the index is at the
	// top most bits.
	Ret = index;
	Ret <<= 32 - NumBits;

	for( i = 0; i < NumBits; i++ ) {
		if( Ret & 0x80000000 )
			Ret = (Ret << 1) ^ Poly;
		else 
			Ret = Ret << 1;
	}
	
	return Ret;
}

/*
 * The following Defines are used to configure the table generator
 */
#define NUM_BITS 8			// Width of message chunk each iteration of the CRC algorithm
//#define POLYNOMIAL 0x1021	// The Polynomial (0x1021 is the CCITT standard one)
#define POLYNOMIAL 0x11021	// The Polynomial (0x1021 is the CCITT standard one)
#define POLYNOMIAL32	0x4c11db7

int main(int argc, char* argv[])
{
	unsigned long i, Count, te;
	
	// Setup the values to compute the table
	Count = 1 << NUM_BITS;		// Number of entries in the table

	printf("/********************************************************************\n");
	printf("*                                                                   *\n");
	printf("*              CCITT CRC generator                                  *\n");
	printf("*                                                                   *\n");
	printf("*              Chen Yongjian                                        *\n");
	printf("*                                                                   *\n");
	printf("********************************************************************/\n\n");
	
	// Generate the WORD width table
	printf("/* CRC16(0x%04X) Lookup table for %u bits per iteration. Full WORD per entry */\r\n", POLYNOMIAL, NUM_BITS );
	printf("static unsigned short s_awCRC16_Lookup[%u] = {", Count );

	for( i = 0; i < Count; i++ ) {
		if( (i % 8) == 0 )
			printf("//%d\r\n\t",i);

		te = GenCRC16TableEntry( (unsigned short)i, NUM_BITS, POLYNOMIAL );
		printf("0x%04X", te);

		if( i+1 != Count )
			printf(", ");
	}
	printf("\r\n};\r\n\r\n", Count );

	// Generate the WORD width table
	printf("/* CRC32(0x%08X) Lookup table for %u bits per iteration. Full DWORD per entry */\r\n", POLYNOMIAL32, NUM_BITS );
	printf("unsigned short s_adwCRC32_Lookup[%u] = {", Count );

	for( i = 0; i < Count; i++ ) {
		if( (i % 8) == 0 )
			printf("//%d\r\n\t",i);

		te = GenCRC32TableEntry( i, NUM_BITS, POLYNOMIAL32 );
		printf("0x%08X", te);

		if( i+1 != Count )
			printf(", ");
	}
	printf("\r\n};\r\n\r\n", Count );

	printf("unsigned short CCITT_GetCRC16(unsigned char * pBuf, int nLen, unsigned short wLastCRC=0xFFFF)\n");
	printf("{\n");
	printf("    for(int i=0; i<nLen; i++)\n");
	printf("    {\n");
	printf("         wLastCRC=(wLastCRC<<8)^s_awCRC16_Lookup[(wLastCRC>>8)^(*pBuf++)];\n");
	printf("    }\n");
	printf("    return wLastCRC;\n");
	printf("}\n\n\n");

	printf("unsigned short CCITT_GetCRC32(unsigned char * pBuf, int nLen, unsigned long dwLastCRC=0xFFFFFFFF)\n");
	printf("{\n");
	printf("    for(int i=0; i<nLen; i++)\n");
	printf("    {\n");
	printf("         dwLastCRC=(dwLastCRC<<8)^s_adwCRC32_Lookup[(dwLastCRC>>24)^(*pBuf++)];\n");
	printf("    }\n");
	printf("    return dwLastCRC;\n");
	printf("}\n");

	return 0;
}

