/*
 Interleaving is "easiest" if the same number of bits is used per symbol as for FEC
 Chosen mode "spreading 8, low rate" has 6 bits per symbol, so use 4:6 FEC

 More spreading needs higher frequency resolution and longer time on air, increasing drift errors.
 Want higher bandwidth when using more spreading, which needs more CPU and a better FFT.

 Six bit Hamming can only correct long runs of drift errors when not using interleaving. Interleaving defeats the point of using Gray code and puts multiple bit errors into single FEC blocks. Hardware decoding uses RSSI to detect the symbols most likely to be damaged, so that individual bits can be repaired after de-interleaving. 

 Using Implicit Mode: explicit starts with a 4:8 block and seems to have a different whitening sequence.
*/

// Six bits per symbol, six chars per block
void LoRaDemod::interleave6(char* inout, int size)
{
	int i, j;
	char in[6 * 2];
	short s;

	for (j = 0; j < size; j+=6) {
		for (i = 0; i < 6; i++)
			in[i] = in[i + 6] = inout[i + j];
		// top bits are swapped
		for (i = 0; i < 6; i++) {
			s = (32 & in[2 + i]) | (16 & in[1 + i]) | (8 & in[3 + i])
				| (4 & in[4 + i]) | (2 & in[5 + i]) | (1 & in[6 + i]);
			// bits are also  rotated
			s = (s << 3) | (s >> 3);
			s &= 63;
			s = (s >> i) | (s << (6 - i));
			inout[i + j] = s & 63;
		}
	}
}

short LoRaDemod::toGray(short num)
{
        return (num >> 1) ^ num;
}

// Ignore the FEC bits, just extract the data bits
void LoRaDemod::hamming6(char* c, int size)
{
	int i;

	for (i = 0; i < size; i++) {
		c[i] = ((c[i] & 1)<<3) | ((c[i] & 2)<<0) | ((c[i] & 4)>>0) | ((c[i] & 8)>>3);
		i++;
		c[i] = ((c[i] & 1)<<2) | ((c[i] & 2)<<2) | ((c[i] & 4)>>1) | ((c[i] & 8)>>3);
		i++;
		c[i] = ((c[i] &32)>>2) | ((c[i] & 2)<<1) | ((c[i] & 4)>>1) | ((c[i] & 8)>>3);
		i++;
		c[i] = ((c[i] & 1)<<3) | ((c[i] & 2)<<1) | ((c[i] & 4)>>1) | ((c[i] & 8)>>3);
		i++;
		c[i] = ((c[i] & 1)<<3) | ((c[i] & 2)<<1) | ((c[i] & 4)>>1) | ((c[i] &16)>>4);
		i++;
		c[i] = ((c[i] & 1)<<3) | ((c[i] & 2)<<1) | ((c[i] & 4)>>2) | ((c[i] & 8)>>2);
	}
	c[i] = 0;
}

// data whitening (4 bit)
void LoRaDemod::whiten(char* inout, int size)
{
	const char otp[] = {
		"DHOOPJA^EHAOCIOEFJIGLJPDNDADIGCJPGGNOOGJECLAMFALDCMABBBGIIIBCHPKHFNOAJICCBOGENKOOJFDICLBODPDEDAGDIMEBJAGKIMBKHOLEHKKOBEDKCPBGDOCHBHCNAAFJKBBIGINDPMKBFBOJIKBGHHKGEDGMIGGJFMJCFLGMJC"
	};
	int i, maxchars;

	maxchars = sizeof( otp );
	if (size < maxchars)
		maxchars = size;
	for (i = 0; i < maxchars; i++)
		inout[i] ^= (otp[i] - 65);
}

