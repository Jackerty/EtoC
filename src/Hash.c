/*************************************
* See Hash.h for module description. *
*************************************/
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include"Hash.h"

	/*************************************
	* This calculates nearest power of   *
	* two sum bigger then given number.  *
	* This is used to keep table in not  *
	* recalculating and other small      *
	* optimazations.                     *
	*************************************/
	static inline int32_t genNearestBiggerFF(int32_t number){
		// For example if number is 0b01011001
		// then after while loop is 0b10000000
		// as 0b01000000 is still less then number
		// then we take minus one causing ite
		// to be 0b01111111.
		// Only problem will be if number is more
		// then (INT32_MAX>>1)<<1. We have to
		// give INT32_MAX in this special case.
		
		if(number<=(INT32_MAX>>1)){
			int32_t ite=0x1;
			while(ite<number)ite<<=1;
			ite--;
			return ite;
		}
		else return INT32_MAX;
	}
	/*************
	* See Hash.h *
	*************/
	uint8_t initHashTable(HashTable *table,int32_t initializesize){
		
		table->collision=0;
		// Note that length is stored in minus one format meaning that
		// true length is plus one value stored.
		// This is done for performance gain.
		table->length=genNearestBiggerFF(initializesize);
		table->buckets=calloc(sizeof(HashEntry*),((uint32_t)table->length)+1);
		if(table->buckets) return 0;
		else return 1;
	}
	/*************************************
	* Add entry to bucket hash value     *
	* reduce to.                         *
	* Can increments collision count but *
	* doesn't check if resize should be  *
	* done.                              *
	*************************************/
	static void addToBucket(HashTable *table,HashEntry *entry){

		// Use rather then moduĺe for speed.
		// Works only if table length is 0b000...11111.
		uint32_t index=entry->hash&table->length;
		HashEntry *bucket=table->buckets[index];

		// Just add to beging of the bucket pointer
		// and if there was already something at
		// the bucket increment collision.
		if(bucket){
			entry->next=bucket;
			table->collision++;
		}
		bucket=entry;
	}
	/*************************************
	* Resize the hashmap.                *
	* This function recalculas collision *
	* amount but doesn't check is there  *
	* too many collisions.               *
	*************************************/
	static void resizeHashTableUp(HashTable *table){
		
		// Number of collisions should be recalculated
		// during resize.
		table->collision=0;

		// Get store of old bucket array before allocating
		// new one. Use calloc to allocate since we need
		// zero initialized area.
		HashEntry **oldbuckets=table->buckets;
		int32_t oldlength=table->length+1;		
		// This operation "doubles" the size of the table.
		// We have to add one since length is in minus one format
		// meaning that to get true length you should add one.
		table->length=(table->length<<1)+1;
		table->buckets=calloc(sizeof(HashEntry*),((uint32_t)table->length)+1);

		// Go through all the hashmap entries and use entries' hash member
		// to recalculate modulo with out rehashing. 
		for(int32_t i=oldlength;i>=0;i++){	
			HashEntry *ite=oldbuckets[i];			
			while(ite){
				addToBucket(table,ite);
				
				ite=ite->next;
			}
		}

		// Free the old bucket array.
		free(oldbuckets);
	}
	/*************
	* See Hash.h *
	*************/
	void addHashTableEntry(HashTable *table,HashEntry *entry){
		
		entry->hash=hashBobJenkins(entry->key,0);

		addToBucket(table,entry);

		// If there is more then half of the length 
		// (minus one format doesn't matter) of collision
		// then we reshash.
		if(table->collision>table->length/2) resizeHashTableUp(table);
	}
	/*************
	* See Hash.h *
	*************/
	HashEntry *getHashTableEntry(HashTable *table,void *key){
		
		uint32_t hash=hashBobJenkins(key,0);
		// Use rather then moduĺe for speed.
		// Works only if table length is 0b000...11111.
		hash&=table->length;
		HashEntry *bucket=table->buckets[hash];
		
		// Find correct place in the bucket.
		// If not found return NULL.
		// We consider key to same if it same hash 
		// (full not moduled) and if strcmp of
		// the keys are the same. Hash compare makes
		// it fast to check "obviously" wrong cases
		// and strcmp makes sure that we don't think
		// collision is a correct entry.
		while(bucket){
			if(bucket->hash==hash && strcmp(bucket->key,key)==0){
				return bucket;
			}
			bucket=bucket->next;
		}

		return 0;
	}
	/*************
	* See Hash.h *
	*************/
	void destroyHashTable(HashTable *table){
		for(int32_t i=table->length;i>=0;i--){
			HashEntry *freestruct1=table->buckets[i];
			HashEntry *freestruct2;

			while(freestruct1){
				freestruct2=freestruct1->next;
				free(freestruct1);
				// Rather then setting freestruct1
				// back to freestruct2 we set 
				// freestruct1 as next free since
				// we save in instructions.
				// (Rather then step equal step equal...
				// it is step1 step2 step1 step2 ...)
				// If sentence doesn't matter since
				// same info would be checked in while 
				// anyway.
				if(freestruct2){
					freestruct1=freestruct2->next;
					free(freestruct2);
				}
				else break;
			}
		}
		free(table->buckets);
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
