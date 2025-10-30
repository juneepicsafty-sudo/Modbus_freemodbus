/* 
 * FreeModbus Libary: A portable Modbus implementation for Modbus ASCII/RTU.
 */

#include "stdlib.h"
#include "string.h"

#include "port.h"

#include "mb.h"
#include "mbproto.h"

#define BITS_UCHAR      8U

void
xMBUtilSetBits( UCHAR * ucByteBuf, USHORT usBitOffset, UCHAR ucNBits,
                UCHAR ucValue )
{
    USHORT          usWordBuf;
    USHORT          usMask;
    USHORT          usByteOffset;
    USHORT          usNPreBits;
    USHORT          usValue = ucValue;

    assert( ucNBits <= 8 );
    assert( ( size_t )BITS_UCHAR == sizeof( UCHAR ) * 8 );

    usByteOffset = ( USHORT )( ( usBitOffset ) / BITS_UCHAR );
    usNPreBits = ( USHORT )( usBitOffset - usByteOffset * BITS_UCHAR );
    usValue <<= usNPreBits;
    usMask = ( USHORT )( ( 1 << ( USHORT ) ucNBits ) - 1 );
    usMask <<= usBitOffset - usByteOffset * BITS_UCHAR;
    usWordBuf = ucByteBuf[usByteOffset];
    usWordBuf |= ucByteBuf[usByteOffset + 1] << BITS_UCHAR;
    usWordBuf = ( USHORT )( ( usWordBuf & ( ~usMask ) ) | usValue );
    ucByteBuf[usByteOffset] = ( UCHAR )( usWordBuf & 0xFF );
    ucByteBuf[usByteOffset + 1] = ( UCHAR )( usWordBuf >> BITS_UCHAR );
}

UCHAR
xMBUtilGetBits( UCHAR * ucByteBuf, USHORT usBitOffset, UCHAR ucNBits )
{
    USHORT          usWordBuf;
    USHORT          usMask;
    USHORT          usByteOffset;
    USHORT          usNPreBits;

    usByteOffset = ( USHORT )( ( usBitOffset ) / BITS_UCHAR );
    usNPreBits = ( USHORT )( usBitOffset - usByteOffset * BITS_UCHAR );
    usMask = ( USHORT )( ( 1 << ( USHORT ) ucNBits ) - 1 );
    usWordBuf = ucByteBuf[usByteOffset];
    usWordBuf |= ucByteBuf[usByteOffset + 1] << BITS_UCHAR;
    usWordBuf >>= usNPreBits;
    usWordBuf &= usMask;
    return ( UCHAR ) usWordBuf;
}

eMBException
prveMBError2Exception( eMBErrorCode eErrorCode )
{
    eMBException    eStatus;

    switch ( eErrorCode )
    {
        case MB_ENOERR:
            eStatus = MB_EX_NONE;
            break;

        case MB_ENOREG:
            eStatus = MB_EX_ILLEGAL_DATA_ADDRESS;
            break;

        case MB_ETIMEDOUT:
            eStatus = MB_EX_SLAVE_BUSY;
            break;

        default:
            eStatus = MB_EX_SLAVE_DEVICE_FAILURE;
            break;
    }

    return eStatus;
}


