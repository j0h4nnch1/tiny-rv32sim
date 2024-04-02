#include "device/uart.h"
#include <cstdint>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>

uint32_t uart::HandleControlLoad(uint32_t addy)
{
	// Emulating a 8250 / 16550 UART
	if( addy == 0x10000005 )
		return 0x60 | this->IsKBHit();
	else if( addy == 0x10000000 && this->IsKBHit() )
		return this->ReadKBByte();
	return 0;
}

uint32_t uart::HandleControlStore( uint32_t addy, uint32_t val )
{
	if( addy == 0x10000000 ) //UART 8250 / 16550 Data Buffer
	{
		printf( "%c", val );
		fflush( stdout );
	}
	return 0;
}

int uart::IsKBHit(){
	if( is_eofd ) return -1;
	int byteswaiting;
	ioctl(0, FIONREAD, &byteswaiting);
	if( !byteswaiting && write( fileno(stdin), 0, 0 ) != 0 ) { is_eofd = 1; return -1; } // Is end-of-file for 
	return !!byteswaiting;
}

int uart::ReadKBByte(){
	if( is_eofd ) return 0xffffffff;
	char rxchar = 0;
	// printf("read a key\n");
	int rread = read(fileno(stdin), (char*)&rxchar, 1);

	if( rread > 0 ) // Tricky: getchar can't be used with arrow keys.
		return rxchar;
	else
		return -1;
}

int32_t uart::HandleOtherCSRRead( uint8_t * image, uint16_t csrno)
{
	if( csrno == 0x140 )
	{
		if( !IsKBHit() ) return -1;
		return ReadKBByte();
	}
	return 0;
}

void uart::HandleOtherCSRWrite( uint8_t * image, uint16_t csrno, uint32_t value )
{
	if( csrno == 0x136 )
	{
		printf( "%d", value ); fflush( stdout );
	}
	if( csrno == 0x137 )
	{
		printf( "%08x", value ); fflush( stdout );
	}
	else if( csrno == 0x138 )
	{
		//Print "string"
		uint32_t ptrstart = value;
		uint32_t ptrend = ptrstart;
		if( ptrstart >= MEM_RANGE )
			printf( "invalid ptr (%08x)\n", value );
		while( ptrend < MEM_RANGE )
		{
			if( image[ptrend] == 0 ) break;
			ptrend++;
		}
		if( ptrend != ptrstart )
			fwrite( image + ptrstart, ptrend - ptrstart, 1, stdout );
	}
	else if( csrno == 0x139 )
	{
		putchar( value ); fflush( stdout );
	}
}