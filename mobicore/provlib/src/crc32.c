#include <gdmcprovlib.h>
#include <string.h>

static _u32 crctable[256];

_u32 GDPROVAPI CalcCRC32 ( const _u8 *data, _u32 length )
{
  _u32 crc = 0xFFFFFFFF;

  while (length--)
    crc = (crc>>8) ^ crctable[(crc&0xFF) ^ *data++];

  return crc ^ 0xFFFFFFFF;
}

static _u32 GDPROVAPI reflect ( _u32 refl, _u8 c )
{
  int     i;
  _u32    value = 0;

  // Swap bit 0 for bit 7, bit 1 For bit 6, etc....
  for (i = 1; i < (c + 1); i++)
  {
    if (refl & 1)
      value |= (1 << (c - i));
    refl >>= 1;
  }

  return value;
}

void GDPROVAPI InitCRCTable ( void )
{
  int       i,j;

  memset(crctable,0,sizeof(crctable));

  for (i = 0; i <= 0xFF; i++)
  {
    crctable[i] = reflect(i, 8) << 24;

    for (j = 0; j < 8; j++)
      crctable[i] = (crctable[i] << 1)
                  ^ ((crctable[i] & (1 << 31)) ? 0x04C11DB7 : 0);

    crctable[i] = reflect(crctable[i], 32);
  }
}

