/*************************************************************************************
* File to test different string hashes for hash table implementation.                *
*************************************************************************************/
#include<stdint.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#if (defined(__linux__) && (__GLIBC__ > 2 || (__GLIBC__==2 && __GLIBC_MINOR__ >=25))) || defined(GEN_RANDOM_USE_GET_RANDOM)
	#include<sys/random.h>
#endif /* __NR_getrandom */

#define RAND_STRING_MAX 255
#define RECOMENTED_RAND_BUFFER_SIZE 512
#define DEFAULT_HASH_TABLE_SIZE 37

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


int main(){

	genRandom();

	// Test string
	char teststr[RAND_STRING_MAX+1];

	// Number of hash tests
	uint32_t numberoftest=20;

	// Result hash of the test
	uint32_t hash;

	// First test randomly generated string of text.
	for(uint32_t cycle=0;cycle<numberoftest;cycle++){
		printf("String to hash: %s\n",randStr(teststr));

		// Simple plus everything of together and modulate the sum.
		hash=0;
		for(char *str=teststr;*str;str++){
			hash+=((uint32_t)*str);
		}
		hash%=DEFAULT_HASH_TABLE_SIZE;
		printf("Simple sum and modulate: %u\n",hash);

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
		hash%=DEFAULT_HASH_TABLE_SIZE;
		printf("Polynomial rolling: %u\n",hash);

		// We are at the end so put newline into the output.
		puts("\n");
	}

	// Second generate some real words in random order to simulate
	// more closely real life situation.


	return 0;
}
