int alaw2linear(unsigned char alawbyte)
{
  int sign, mantissa, exponent;

  alawbyte ^= 0x55;
  sign = alawbyte & 0x80;
  mantissa = alawbyte & 0x0f;
  exponent = (alawbyte & 0x70) >> 4;
  if (exponent > 0) mantissa |= 0x10;
  mantissa = (mantissa << 4) + 0x0008;
  if (exponent > 1) mantissa <<= exponent - 1;
  return sign ? mantissa : -mantissa;
}

unsigned char linear2alaw(int sample)
{
  int sign, mantissa, exponent;
  static unsigned char table[128] =
  {
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, 
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6, 
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, 
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, 
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, 
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
  };

  sign = (sample >> 8) & 0x80;
  if (sign) sample = ~sample;
  sample >>= 4;
  exponent = table[sample >> 4];
  if (exponent > 1) sample >>= exponent - 1;
  return ((sample & 0x000f) | (exponent << 4) | (sign ^ 0x80)) ^ 0x0055;
}

