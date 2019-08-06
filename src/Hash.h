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
*     // VALUE KEY MAP TO                             *
*     int value;                                      *
*   }                                                 *
*                                                     *
* Important thing is there is linked list pointer and *
* key string are same offset in the structure when    *
* given to addition function.                         *
* You can also use HASH_TABLE_ENTRY_HEADER macro to   *
* makes sure that needed members are there.           *
******************************************************/
#incldue<stdint.h>

/******************************************************
* Hash table entry header macro is to make programmer *
* not f'up entries.                                   *
******************************************************/
#define HASH_TABLE_ENTRY_HEADER struct HashEntry *next;char *key;

/******************************************************
* Hash table entry header.                            *
* Any data you want to map to key you have to add     *
* after this structure.                               *
******************************************************/
typedef struct HashEntry{
	struct HashEntry *next;
	char *key;
}HashEntry;
/******************************************************
* Hash table is array of linked listed entries of     *
* length. Length may changes if too many conclicts    *
* have happened.                                      *
******************************************************/
typedef struct HashTable{
	struct HashEntry **entries;
	int32_t length;
	uint32_t conflicts;
}HashTable;

/******************************************************
* Initialize the hash map table.                      *
* Does not allocate memory HashTable structure only   *
* inners!                                             *
******************************************************/
void initHashTable(HashTable *table,uint32_t initializesize);
/******************************************************
* Function adds hash entry to the given table.        *
* Added entries should be dynamicly allocated.        *
******************************************************/
void addHashTableEntry(HashTable *table,HashEntry *entry);
/******************************************************
* Functions gets entry from the hash table.           *
******************************************************/
HashEntry *getHashTableEntry(HashTable *table,char *key);
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
* optimazations.                                      *
******************************************************/
uint32_t hashBobJenkins(const char *key,uint32_t initval);
