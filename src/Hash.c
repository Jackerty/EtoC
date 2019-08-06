/*************************************
* See Hash.h for module description. *
*************************************/
#include"Hash.h"

/*************************************
* This calculates nearest power of   *
* two sum bigger then given number.  *
* This is used to keep table in not  *
* recalculating and other small      *
* optimazations.                     *
*************************************/
static inline void genNearestBiggerFF(uint32_t number){
	//
	uint32_t ite=0x80000000;
	while(ite>number)ite>>1;
	return number;
}
/*************
* See Hash.h *
*************/
void initHashTable(HashTable *table,uint32_t initializesize){
	getNearestBiggerFF();
	table->conflicts=0;
	table->length=initializesize;
	table->entries=calloc(sizeof(HashEntry*),initializesize);
}
/*************
* See Hash.h *
*************/
void addHashTableEntry(HashTable *table,HashEntry *entry){
	uint32_t hash=hashBobJenkins(entry->key,0);
	// Use rather then moduĺe for speed.
	// Works only if table length is 0b000...11111.
	hash&=table->length;
}
/*************
* See Hash.h *
*************/
HashEntry *getHashTableEntry(HashTable *table,char *key){
	;
}
/*************
* See Hash.h *
*************/
void destroyHashTable(HashTable *table){
	for(int32_t i=table->length;i>=0;i--){
		HashEntry *freestruct=table->entries[i];
		HashEntry *next;
		while(freestruct){
			next=freestruct;
			free(freestruct);
			freestruct=next;
		}
	}
}
/*************
* See Hash.h *
*************/
uint32_t hashBobJenkins(const void *key,uint32_t initval){

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
