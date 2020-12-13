/******************************************************
* Hash table and functions to do hashing.             *
* Hash table consist two data structures one called   *
* HashTable which is the handler for certain hash map *
* and HashEntry which is structure which keys map to. *
* HashEntry doesn't have any specific map to type but *
* rather programmer has to add it after the           *
* structure. For example string to integer hash table *
* would look like:                                    *
*                                                     *
*   struct HashEntryInt{                              *
*     // HEADER                                       *
*     struct HashEntry *next;                         *
*     char *key;                                      *
*     uint32_t hash;                                  *
*     // VALUE KEY MAPPED TO                          *
*     int value;                                      *
*   }                                                 *
*                                                     *
* Important thing is there is header members are same *
* offset in the structure when given to addition      *
* function. Note that because last member of header   *                       
* is 32 bit long it means that you should put one 32  *
* bit type before 64 bit data so that there isn't 32  *
* bits of padding in 64 bit systems.                  *
*                                                     *
* You can also use HASH_TABLE_ENTRY_HEADER macro to   *
* makes sure that needed members are there.           *
******************************************************/
#ifndef _HASH_H_
#define _HASH_H_

#include<stdint.h>


/******************************************************
* Hash table entry header macro is to make programmer *
* not f'up entries.                                   *
******************************************************/
#define HASH_TABLE_ENTRY_HEADER \
	struct HashEntry *next; \
	char *key; \
	uint32_t hash

/******************************************************
* Hash table entry header.                            *
* Any data you want to map to key you have to add     *
* after this structure.                               *
******************************************************/
typedef struct HashEntry{
	HASH_TABLE_ENTRY_HEADER;
}HashEntry;
/******************************************************
* Hash table is array of linked listed entries of     *
* length. Length may changes if too many conclicts    *
* have happened. Also length maybe stored in minus    *
* one format meaning that to get true length you have *
* to add one.                                         *
******************************************************/
typedef struct HashTable{
	struct HashEntry **buckets;
	int32_t length;
	uint32_t collision;
}HashTable;

/******************************************************
* Initialize the hash map table.                      *
* Does not allocate memory HashTable structure only   *
* inners!                                             *
******************************************************/
uint8_t initHashTable(HashTable *table,int32_t initializesize);
/******************************************************
* Function adds hash entry to the given table.        *
* Added entries should be dynamicly allocated.        *
******************************************************/
void addHashTableEntry(HashTable *table,HashEntry *entry);
/******************************************************
* Functions gets entry from the hash table.           *
******************************************************/
HashEntry *getHashTableEntry(HashTable *table,void *key);
/******************************************************
* Destroy hash table structure inners! Does not free  *
* structure it self!                                  *
******************************************************/
void destroyHashTable(HashTable *table);
/******************************************************
* Bob Jenkin's hash function given in:                *
* http://burtleburtle.net/bob/c/lookup3.c             *
*                                                     *
* Function is combination of Little and big endian    *
* functions that optimize for little and big endian   *
* via macro check.                                    *
******************************************************/
uint32_t hashBobJenkins(const void *key,uint32_t initval);

#endif /* _HASH_H_ */
