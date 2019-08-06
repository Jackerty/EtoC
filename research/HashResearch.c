/*************************************************************************************
* File to test different string hashes for hash table implementation.                *
*************************************************************************************/
#include<stdint.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/param.h>
#if (defined(__linux__) && (__GLIBC__ > 2 || (__GLIBC__==2 && __GLIBC_MINOR__ >=25))) || defined(GEN_RANDOM_USE_GET_RANDOM)
	#include<sys/random.h>
#endif /* __NR_getrandom */

#define NUMBER_OF_HASH_FUNCS 5
#define RAND_STRING_MAX 255
#define RECOMENTED_RAND_BUFFER_SIZE 512

/*******************************************
* Random buffer to be used different       *
* locations.                               *
*******************************************/
uint8_t randombuffer[RECOMENTED_RAND_BUFFER_SIZE];
/*******************************************
* Amount of random buffer left.            *
*******************************************/
uint32_t usage;

/*******************************************
* Generate random buffer.                  *
*******************************************/
static inline void genRandom(){
	// Check that one can use getrandom.
	#if (defined(__linux__) && (__GLIBC__ > 2 || (__GLIBC__==2 && __GLIBC_MINOR__ >=25))) || defined(GEN_RANDOM_USE_GET_RANDOM)
		uint32_t buffered=0;

		// Get "actual" randomness.
		buffered+=getrandom(randombuffer,RECOMENTED_RAND_BUFFER_SIZE,GRND_RANDOM);

		// /dev/random doesn't guarantee 512 bytes. 512 bytes is just maximum at time of writing.
		// I don't know why there is limit macro for it...
		// But anyway call /dev/urandom if buffer isn't full.
		if(RECOMENTED_RAND_BUFFER_SIZE>buffered){
			getrandom(randombuffer+buffered,RECOMENTED_RAND_BUFFER_SIZE-buffered,0);
		}
	#else
		//TODO: Not well made... rand should be the last fall back also.
		for(uint32_t i=0;i<RECOMENTED_RAND_BUFFER_SIZE;i++){
			((char *)randombuffer)[i]=random()%256;
		}
	#endif /* defined(__linux__) && (__GLIBC__ > 2 || (__GLIBC__==2 && __GLIBC_MINOR__ >=25)) || defined(GEN_RANDOM_USE_GET_RANDOM) */
	usage=0;
}
/*******************************************
* Generate random string with zero ending. *
* Minimum length of this string is zero    *
* (just null character) and maximum is     *
* defined by RAND_STRING_MAX macro.        *
*                                          *
* Memory for the string has to be given    *
* by a parameter. Make sure that memory is *
* at RAND_STRING_MAX+1 of length!!         *
* Also give random buffer.                 *
*                                          *
* Length of the returning string is not    *
* given to avoid bias to hash algorithms   *
* that know length before the calculation. *
*******************************************/
char *randStr(char *memory){

	// Character strings which we can choose from.
	//
	static char charset[]={'a','b','c','d','e','f','g','h','i','j','k','l','m'
	                      ,'n','o','p','q','r','s','t','u','v','w','x','y','z'
	                      ,'A','B','C','D','E','F','G','H','I','J','K','L','M'
	                      ,'N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
	                      ,'0','1','2','3','4','5','6','7','8','9','.','-','_'
	                      };

	// Generate length of the string.
	if(usage+1>=RECOMENTED_RAND_BUFFER_SIZE) genRandom();
	// TODO: This is bit navy since when length of the string get
	//       large so does change of getting certain string. Hence
	//       string size probablity should scale with length.
	uint8_t lengthofstr=(uint8_t)randombuffer[usage++];

	uint32_t remainder=RECOMENTED_RAND_BUFFER_SIZE-usage;
	// We use random only amount buffer has.
	if(remainder<lengthofstr){
		uint8_t i=0;
		for(;i<remainder;i++) memory[i]=charset[randombuffer[usage++]%sizeof(charset)];
    genRandom();
		for(;i<lengthofstr;i++) memory[i]=charset[randombuffer[usage++]%sizeof(charset)];
	}
	else{
		for(uint8_t i=0;i<lengthofstr;i++) memory[i]=charset[randombuffer[usage++]%sizeof(charset)];
	}

	// Remember put null ending.
	memory[lengthofstr+1]='\0';

	return memory;
}
/**********************************************
* Generate test word combinations randomly.   *
**********************************************/
char *randWords(char *memory){
	return memory;
}

/*********************************************
* Please look at the main for description.   *
*********************************************/
uint32_t bobjenkins_hash(const void *key,uint32_t initval){

	#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))
	// Reversible mix that saves all of the info.
	#define mix(a,b,c) \
	{                  \
	  a-=c;            \
	  a^=rot(c,4);     \
		c+=b;            \
		b-=a;            \
		b^=rot(a,6);     \
		a+=c;            \
		c -= b;          \
		c ^= rot(b, 8);  \
		b += a;          \
		a -= c;          \
		a ^= rot(c,16);  \
		c += b;          \
		b -= a;          \
		b ^= rot(a,19);  \
		a += c;          \
		c -= b;          \
		c ^= rot(b, 4);  \
		b += a;          \
	}
	// mix that changes c totally different?
	#define finalmix(a,b,c) \
	{                    \
		c ^= b;            \
		c -= rot(b,14);    \
		a ^= c;            \
		a -= rot(c,11);    \
		b ^= a;            \
		b -= rot(a,25);    \
		c ^= b;            \
		c -= rot(b,16);    \
		a ^= c;            \
		a -= rot(c,4);     \
		b ^= a;            \
		b -= rot(a,14);    \
		c ^= b;            \
		c -= rot(b,24);    \
	}

	#if __BYTE_ORDER==__LITTLE_ENDIAN

	// Internal state
	uint32_t a,b,c;
	uint32_t length=strlen(key);

	a=b=c=0xdeadbeef+length+initval;

	// There is some  Mac Powerbook G4 need???
	// Variable u is used to make sure
	// number of bytes can be read one time.
	union{const void *ptr;size_t i;} u={.ptr=key};

	if((u.i & 0x3)==0){
		// Handle 32 bit at time.
		const uint32_t *k=(const uint32_t *)key;

		// Mix the state when there is more then 12 bytes of information
		while(length>12){
			a+=k[0];
			b+=k[1];
			c+=k[2];
			mix(a,b,c);
			length-=12;
			k+=3;
		}

		// Handle what is left.
		// Note that k[2]&0xffffff reads beyond strings boundaries.
		// This is fine according to jenkins... Well we are masking
		// it off anyway so any heap and rodata we would have there
		// least exist (as padding if notting else) so no segment vault.
		// Valgrind may still throw something...
		switch(length){
			case 12:
				c+=k[2];
				b+=k[1];
				a+=k[0];
				break;
			case 11:
				c+=k[2]&0xffffff;
				b+=k[1];
				a+=k[0];
				break;
			case 10:
				c+=k[2]&0xffff;
				b+=k[1];
				a+=k[0];
				break;
			case 9:
				c+=k[2]&0xff;
				b+=k[1];
				a+=k[0];
				break;
			case 8:
				b+=k[1];
				a+=k[0];
				break;
			case 7:
				b+=k[1]&0xffffff;
				a+=k[0];
				break;
			case 6:
				b+=k[1]&0xffff;
				a+=k[0];
				break;
			case 5:
				b+=k[1]&0xff;
				a+=k[0];
				break;
			case 4:
				a+=k[0];
				break;
			case 3:
				a+=k[0]&0xffffff;
				break;
			case 2:
				a+=k[0]&0xffff;
				break;
			case 1:
				a+=k[0]&0xff;
				break;
			// Zero length strings don't need additional mixing.
			case 0:
				return c;
		}
	}
	else if((u.i & 0x1)==0){
		// Handle 16 bit at the time.
		const uint16_t *k=(const uint16_t *)key;
		const uint8_t  *k8;

		while(length>12){
      a += k[0] + (((uint32_t)k[1])<<16);
      b += k[2] + (((uint32_t)k[3])<<16);
      c += k[4] + (((uint32_t)k[5])<<16);
      mix(a,b,c);
      length -= 12;
      k += 6;
    }

    k8 = (const uint8_t *)k;
    switch(length){
    case 12:
			c+=k[4]+(((uint32_t)k[5])<<16);
			b+=k[2]+(((uint32_t)k[3])<<16);
			a+=k[0]+(((uint32_t)k[1])<<16);
			break;
    case 11:
			c+=((uint32_t)k8[10])<<16;
			/* fall through */
    case 10:
			c+=k[4];
			b+=k[2]+(((uint32_t)k[3])<<16);
			a+=k[0]+(((uint32_t)k[1])<<16);
			break;
    case 9:
			c+=k8[8];
			/* fall through */
    case 8:
			b+=k[2]+(((uint32_t)k[3])<<16);
			a+=k[0]+(((uint32_t)k[1])<<16);
			break;
    case 7:
			b+=((uint32_t)k8[6])<<16;
			/* fall through */
    case 6:
			b+=k[2];
			a+=k[0]+(((uint32_t)k[1])<<16);
			break;
    case 5:
			b+=k8[4];
			/* fall through */
    case 4:
			a+=k[0]+(((uint32_t)k[1])<<16);
			break;
    case 3:
			a+=((uint32_t)k8[2])<<16;
			/* fall through */
    case 2:
			a+=k[0];
			break;
    case 1:
			a+=k8[0];
			break;
    case 0:
    	// Zero length strings don't need additional mixing.
			return c;
    }
	}
	else{
		// Handle byte at the time.
		const uint8_t *k = (const uint8_t *)key;
		while(length>12){
			a += k[0];
			a += ((uint32_t)k[1])<<8;
			a += ((uint32_t)k[2])<<16;
			a += ((uint32_t)k[3])<<24;
			b += k[4];
			b += ((uint32_t)k[5])<<8;
			b += ((uint32_t)k[6])<<16;
			b += ((uint32_t)k[7])<<24;
			c += k[8];
			c += ((uint32_t)k[9])<<8;
			c += ((uint32_t)k[10])<<16;
			c += ((uint32_t)k[11])<<24;
			mix(a,b,c);
			length -= 12;
			k += 12;
    }

    switch(length){
		case 12:
			c+=((uint32_t)k[11])<<24;
			/* fall through */
    case 11:
    	c+=((uint32_t)k[10])<<16;
    	/* fall through */
    case 10:
    	c+=((uint32_t)k[9])<<8;
    	/* fall through */
    case 9:
			c+=k[8];
			/* fall through */
    case 8:
			b+=((uint32_t)k[7])<<24;
			/* fall through */
    case 7:
    	b+=((uint32_t)k[6])<<16;
    	/* fall through */
    case 6:
    	b+=((uint32_t)k[5])<<8;
    	/* fall through */
    case 5:
    	b+=k[4];
    	/* fall through */
    case 4:
    	a+=((uint32_t)k[3])<<24;
    	/* fall through */
    case 3:
    	a+=((uint32_t)k[2])<<16;
    	/* fall through */
    case 2:
    	a+=((uint32_t)k[1])<<8;
    	/* fall through */
    case 1:
    	a+=k[0];
			break;
    case 0:
    	return c;
    }

	}

	#else /* __BYTE_ORDER==__LITTLE_ENDIAN */
		// BIG ENDIEAN MACHINE!!

		uint32_t a,b,c;
		uint32_t length=strlen(key);
		a=b=c=0xdeadbeef+length+initval;
		union{const void *ptr;size_t i;} u={key};

		if((u.i & 0x3)==0){
			const uint32_t *k=(const uint32_t *)key;

			while(length>12){
				a += k[0];
				b += k[1];
				c += k[2];
				mix(a,b,c);
				length -= 12;
				k += 3;
			}

      // Handle rest of the string.
      // k[2]<<8 reads beyond strings but as noted above we mask it off and we
      // probably have memory there anyway so how cares since malloc allocates
      // chunks of power of two and rodata section has aligment to 32 bits
      // probably. Valgrind may not like.
			switch(length){
			case 12:
				c+=k[2];
				b+=k[1];
				a+=k[0];
				break;
			case 11:
				c+=k[2]&0xffffff00;
				b+=k[1];
				a+=k[0];
				break;
			case 10:
				c+=k[2]&0xffff0000;
				b+=k[1];
				a+=k[0];
				break;
			case 9:
				c+=k[2]&0xff000000;
				b+=k[1];
				a+=k[0];
				break;
			case 8:
				b+=k[1];
				a+=k[0];
				break;
			case 7:
				b+=k[1]&0xffffff00;
				a+=k[0];
				break;
			case 6:
				b+=k[1]&0xffff0000;
				a+=k[0];
				break;
			case 5:
				b+=k[1]&0xff000000;
				a+=k[0];
				break;
			case 4:
				a+=k[0];
				break;
			case 3:
				a+=k[0]&0xffffff00;
				break;
			case 2:
				a+=k[0]&0xffff0000;
				break;
			case 1:
				a+=k[0]&0xff000000;
				break;
			// We don't need any additional mixing.
			case 0:
				return c;
			}
		}
		else{
			const uint8_t *k=(const uint8_t *)key;

			// Note that shifts are opposited comapared to
			// little endian one byte at the time.
			while(length>12){
				a += ((uint32_t)k[0])<<24;
				a += ((uint32_t)k[1])<<16;
				a += ((uint32_t)k[2])<<8;
				a += ((uint32_t)k[3]);
				b += ((uint32_t)k[4])<<24;
				b += ((uint32_t)k[5])<<16;
				b += ((uint32_t)k[6])<<8;
				b += ((uint32_t)k[7]);
				c += ((uint32_t)k[8])<<24;
				c += ((uint32_t)k[9])<<16;
				c += ((uint32_t)k[10])<<8;
				c += ((uint32_t)k[11]);
				mix(a,b,c);
				length -= 12;
				k += 12;
			}

			switch(length){
			case 12:
				c+=k[11];
				/* fall through */
			case 11:
				c+=((uint32_t)k[10])<<8;
				/* fall through */
			case 10:
				c+=((uint32_t)k[9])<<16;
				/* fall through */
			case 9:
				c+=((uint32_t)k[8])<<24;
				/* fall through */
			case 8:
				b+=k[7];
				/* fall through */
			case 7:
				b+=((uint32_t)k[6])<<8;
				/* fall through */
			case 6:
				b+=((uint32_t)k[5])<<16;
				/* fall through */
			case 5:
				b+=((uint32_t)k[4])<<24;
				/* fall through */
			case 4:
				a+=k[3];
				/* fall through */
			case 3:
				a+=((uint32_t)k[2])<<8;
				/* fall through */
			case 2:
				a+=((uint32_t)k[1])<<16;
				/* fall through */
			case 1:
				a+=((uint32_t)k[0])<<24;
				break;
			case 0:
				return c;
			}
		}

	#endif /* __BYTE_ORDER==__LITTLE_ENDIAN */

  finalmix(a,b,c);
  return c;

	#undef rot
	#undef mix
	#undef finalmix
}

int main(){

	genRandom();

	// Test string
	char teststr[RAND_STRING_MAX+1];

	// Collision arrays collect information on
	// what collisions happen for each tested
	// hash function on different sized hash
	// table. Number of certain indexes appearing
	// are stored at that column index of the row
	// of the hash. Hashes appear same order as they
	// appear in the code below.
	uint32_t collision32[NUMBER_OF_HASH_FUNCS][32];
	memset(collision32,0,sizeof(collision32));
	uint32_t collision41[NUMBER_OF_HASH_FUNCS][41];
	memset(collision41,0,sizeof(collision41));
	uint32_t collision61[NUMBER_OF_HASH_FUNCS][61];
	memset(collision61,0,sizeof(collision61));
	uint32_t collision64[NUMBER_OF_HASH_FUNCS][64];
	memset(collision64,0,sizeof(collision64));
	uint32_t collision79[NUMBER_OF_HASH_FUNCS][79];
	memset(collision79,0,sizeof(collision79));
	uint32_t collision127[NUMBER_OF_HASH_FUNCS][127];
	memset(collision127,0,sizeof(collision127));

	// Number of hash tests
	uint32_t numberoftest=40;

	// Result hash of the test
	uint32_t hash;
	// Result is hash would be salted
	// (given some chaning initialize state).
	uint32_t saltedhash;

	// Special variable for Bob Jenkin's hash.
	// Look down for what this is used.
	uint32_t bobjenkins_prevhash=0;


	// First test randomly generated string of text.
	for(uint32_t cycle=0;cycle<numberoftest;cycle++){
		printf("String to hash: %s\n",randStr(teststr));

		// Simple plus everything of together and modulate the sum.
		hash=0;
		for(char *str=teststr;*str;str++){
			hash+=((uint32_t)*str);
		}
		printf("Simple sum and modulate: %u\n",hash);

		collision32[0][hash%32]++;
		collision41[0][hash%41]++;
		collision61[0][hash%61]++;
		collision64[0][hash%64]++;
		collision79[0][hash%79]++;
		collision127[0][hash%127]++;

		// Calculate polynomial rolling hash function.
		// https://cp-algorithms.com/string/string-hashing.html
		// This function works by having prime number that is
		// exponents are used coefficient for strings character
		// starting at one. This prime should be near number of character
		// in input alphabet.
    hash=0;
    uint32_t polyrollprime=73;
    uint32_t coeff=1;
    for(char *str=teststr;*str;str++){
			hash+=((uint32_t)*str)*coeff;
			coeff*=polyrollprime;
    }

		printf("Polynomial rolling: %u\n",hash);

		collision32[1][hash%32]++;
		collision41[1][hash%41]++;
		collision61[1][hash%61]++;
		collision64[1][hash%64]++;
		collision79[1][hash%79]++;
		collision127[1][hash%127]++;

		// Calculate Paul Hsieh's hash
		// From https://burtleburtle.net/bob/hash/doobs.html
		// Really doesn't explain what happens.
		{
			#define get16bits(d) (*((const uint8_t *)(d)))
			// Initialize by size of the string.
			// This has benefit of different size strings
			// that have same ending are more like to be different.
			uint32_t len=strlen(teststr);
			hash=len;
			uint32_t tmp;
			int32_t rem=len%3;
			len>>=2;
			char *data=teststr;
			// Loop and operations on hash
			while(len>0){
				hash+=get16bits(data);
				tmp=(get16bits(data+2)<<11)^hash;
				hash=(hash<<16)^tmp;
				data+=2*sizeof(uint16_t);
				hash+=hash>>11;
				len--;
			}

			// Handle remainder that part which
			// needed because previos loop needs
			// four bytes at the time.
			switch(rem){
			case 3:
				hash+=get16bits(data);
				hash^=hash<<16;
				hash^=data[sizeof(uint16_t)]<<18;
				hash+=hash>>11;
				break;
			case 2:
				hash+=get16bits(data);
				hash^=hash<<11;
				hash+=hash>>17;
				break;
			case 1:
				hash+=*data;
				hash^=hash<<10;
				hash+=hash>>1;
				break;
			}

			// Comment here says:
			//   "Force "avalanching" of final 127 bits".
			// This phase probably makes sure 32 bit integer is used more equally??
			hash^=hash<<3;
			hash+=hash>>5;
			hash^=hash<<4;
			hash+=hash>>17;
			hash^=hash<<25;
			hash+=hash>>6;
		}

		printf("Paul Hsieh's: %u\n",hash);

		collision32[2][hash%32]++;
		collision41[2][hash%41]++;
		collision61[2][hash%61]++;
		collision64[2][hash%64]++;
		collision79[2][hash%79]++;
		collision127[2][hash%127]++;

		// Bob Jenkins lookup3.c hashlittle (comes from little endian)
		// From: burtleburtle.net/bob/c/lookup3.c
		// Jenkins recomends power of 2 table size and claims one that
		// for 2^32 table there is on avarage one collision.

		// interesting thing about this hash is he recoments that previous generated hash
		// would be used as initialize for next one when generating multiple hashes.
		// I will probably test both here so that I know collision difference. It isn't
		// smart to be carry string + salt in compiler program because I get strings from
		// the source code and have to match salt by other hash table. Maybe namespace
		// type where namespace has same salt to make it's inner symbols to pop-out.

		hash=bobjenkins_hash(teststr,0);
		saltedhash=bobjenkins_hash(teststr,bobjenkins_prevhash);
		printf("Bob Jenkins Lookup3 (normal,salted): %u %u\n",hash,saltedhash);

		collision32[3][hash%32]++;
		collision41[3][hash%41]++;
		collision61[3][hash%61]++;
		collision64[3][hash%64]++;
		collision79[3][hash%79]++;
		collision127[3][hash%127]++;

		collision32[4][saltedhash%32]++;
		collision41[4][saltedhash%41]++;
		collision61[4][saltedhash%61]++;
		collision64[4][saltedhash%64]++;
		collision79[4][saltedhash%79]++;
		collision127[4][saltedhash%127]++;

		bobjenkins_prevhash=saltedhash;

		// We are at the end so put newline into the output.
		puts("\n");
	}

	// Print out array results.
	for(uint32_t hfunc=0;hfunc<NUMBER_OF_HASH_FUNCS;hfunc++){
		uint32_t numbercoll=0;
		uint32_t numbercollindex=0;
		for(uint32_t i=0;i<32;i++){
			if(collision32[hfunc][i]>1){
				numbercoll+=collision32[hfunc][i];
				numbercollindex++;
			}
		}
		printf("Collision32 (collisions,collision index) %d,%d\n",numbercoll,numbercollindex);

		numbercoll=0;
		numbercollindex=0;
		for(uint32_t i=0;i<41;i++){
			if(collision41[hfunc][i]>1){
				numbercoll+=collision41[hfunc][i];
				numbercollindex++;
			}
		}
		printf("Collision41 (collisions,collision index) %d,%d\n",numbercoll,numbercollindex);

		numbercoll=0;
		numbercollindex=0;
		for(uint32_t i=0;i<61;i++){
			if(collision61[hfunc][i]>1){
				numbercoll+=collision61[hfunc][i];
				numbercollindex++;
			}
		}
		printf("Collision61 (collisions,collision index) %d,%d\n",numbercoll,numbercollindex);

		numbercoll=0;
		numbercollindex=0;
		for(uint32_t i=0;i<64;i++){
			if(collision64[hfunc][i]>1){
				numbercoll+=collision64[hfunc][i];
				numbercollindex++;
			}
		}
		printf("Collision64 (collisions,collision index) %d,%d\n",numbercoll,numbercollindex);

		numbercoll=0;
		numbercollindex=0;
		for(uint32_t i=0;i<79;i++){
			if(collision79[hfunc][i]>1){
				numbercoll+=collision79[hfunc][i];
				numbercollindex++;
			}
		}
		printf("Collision79 (collisions,collision index) %d,%d\n",numbercoll,numbercollindex);

		numbercoll=0;
		numbercollindex=0;
		for(uint32_t i=0;i<126;i++){
			if(collision127[hfunc][i]>1){
				numbercoll+=collision127[hfunc][i];
				numbercollindex++;
			}
		}
		printf("collision127 (collisions,collision index) %d,%d\n",numbercoll,numbercollindex);

		puts("\n");
	}

	// Second generate some real words in random order to simulate
	// more closely real life situation.


	return 0;
}
