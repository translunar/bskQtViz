/*
 ISC License

 Copyright (c) 2016-2017, Autonomous Vehicle Systems Lab, University of Colorado at Boulder

 Permission to use, copy, modify, and/or distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.

 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 */
/*
* strMap.c
*
* University of Colorado, Autonomous Vehicle Systems (AVS) Lab
* Unpublished Copyright (c) 2012-2015 University of Colorado, All Rights Reserved
*/

#include "strMap.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
* Function: strMapHash
* Purpose: Returns a hash code for the provided string
*/
static unsigned long strMapHash(const char *str)
{
    unsigned long hash = 5381;
    int           c;
    while((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

/*
* Function: strMapGetPair
* Purpose: Returns a pair from the bucket that matches the key
*/
static StrMapPair_t *strMapGetPair(StrMapBucket_t *bucket, const char *key)
{
    unsigned int  i;
    unsigned int  n;
    StrMapPair_t *pair;

    n = bucket->count;
    if(n == 0) {
        return NULL;
    }
    pair = bucket->pairs;
    i = 0;
    while(i < n) {
        if(pair->key != NULL) {
            if(strcmp(pair->key, key) == 0) {
                return pair;
            }
        }
        pair++;
        i++;
    }
    return NULL;
}

/*
* Function: strMapAlloc
* Purpose: Allocates a string map
*/
StrMap_t *strMapAlloc(unsigned int capacity)
{
    StrMap_t *map = calloc(1, sizeof(StrMap_t));
    if(map == NULL) {
        return NULL;
    }
    map->count = capacity;
    map->buckets = calloc(map->count, sizeof(StrMapBucket_t));
    if(map->buckets == NULL) {
        free(map);
        return NULL;
    }
    memset(map->buckets, 0, map->count * sizeof(StrMapBucket_t));
    return map;
}

/*
* Function: strMapFree
* Purpose: Frees a string map
*/
void strMapFree(StrMap_t *map)
{
    unsigned int    i;
    unsigned int    j;
    unsigned int    n;
    unsigned int    m;
    StrMapBucket_t *bucket;
    StrMapPair_t   *pair;

    if(map == NULL) {
        return;
    }
    n = map->count;
    bucket = map->buckets;
    i = 0;
    while(i < n) {
        m = bucket->count;
        pair = bucket->pairs;
        j = 0;
        while(j < m) {
            free(pair->key);
            pair++;
            j++;
        }
        free(bucket->pairs);
        bucket++;
        i++;
    }
    free(map->buckets);
    free(map);
}

/*
* Function: strMapGet
* Purpose: Gets a value associated with a key
*   Returns 0 if successful, 1 if not
*/
int strMapGet(const StrMap_t *map, const char *key, double *value)
{
    unsigned int    index;
    StrMapBucket_t *bucket;
    StrMapPair_t   *pair;

    if(map == NULL) {
        return 1;
    }
    if(key == NULL) {
        return 1;
    }
    index = strMapHash(key) % map->count;
    bucket = &(map->buckets[index]);
    pair = strMapGetPair(bucket, key);
    if(pair == NULL) {
        return 1;
    }
    *value = pair->value;
    return 0;
}

/*
* Function: strMapExists
* Purpose: Check if the map contains they key
*   Returns 1 if true, 0 if false
*/
int strMapExists(const StrMap_t *map, const char *key)
{
    unsigned int    index;
    StrMapBucket_t *bucket;
    StrMapPair_t   *pair;

    if(map == NULL) {
        return 0;
    }
    if(key == NULL) {
        return 0;
    }
    index = strMapHash(key) % map->count;
    bucket = &(map->buckets[index]);
    pair = strMapGetPair(bucket, key);
    if(pair == NULL) {
        return 0;
    }
    return 1;
}

/*
* Function: strMapPut
* Purpose: Adds value pair to map
*   Returns 0 if successful, 1 if not
*/
int strMapPut(StrMap_t *map, const char *key, double value)
{
    unsigned int    keyLen;
    unsigned int    index;
    StrMapBucket_t *bucket;
    StrMapPair_t   *tempPairs;
    StrMapPair_t   *pair;
    char           *newKey;

    if(map == NULL) {
        return 1;
    }
    if(key == NULL) {
        return 1;
    }
    keyLen = strlen(key);
    /* Get a pointer to the bucket the key string hashes to */
    index = strMapHash(key) % map->count;
    bucket = &(map->buckets[index]);
    /* Check if key already exists */
    if((pair = strMapGetPair(bucket, key)) != NULL) {
        /* It does, so overwrite value */
        fprintf(stderr, "Warning: Overwriting value for %s in input file\n", pair->key);
        pair->value = value;
        return 0;
    }
    /* Allocate space for a new key and value */
    newKey = calloc(keyLen + 1, sizeof(char));
    if(newKey == NULL) {
        return 1;
    }
    /* Create a key-value pair */
    if(bucket->count == 0) {
        /* The bucket is empty, lazily allocate space for single pair */
        bucket->pairs = calloc(1, sizeof(StrMapPair_t));
        if(bucket->pairs == NULL) {
            free(newKey);
            return 1;
        }
        bucket->count = 1;
    } else {
        /* The bucket is not empty */
        tempPairs = realloc(bucket->pairs, (bucket->count + 1) * sizeof(StrMapPair_t));
        if(tempPairs == NULL) {
            free(newKey);
            return 1;
        }
        bucket->pairs = tempPairs;
        bucket->count++;
    }
    /* Get the last pair in the chain for the bucket */
    pair = &(bucket->pairs[bucket->count - 1]);
    pair->key = newKey;
    pair->value = value;
    /* Copy the new key into the pair */
    strcpy(pair->key, key);
    return 0;
}

/*
* Function: strMapGetCount
* Purpose: Get number of values in map
*/
int strMapGetCount(StrMap_t *map)
{
    unsigned int    i;
    unsigned int    j;
    unsigned int    n;
    unsigned int    m;
    unsigned int    count;
    StrMapBucket_t *bucket;
    StrMapPair_t   *pair;

    if(map == NULL) {
        return 0;
    }
    bucket = map->buckets;
    n = map->count;
    i = 0;
    count = 0;
    while(i < n) {
        pair = bucket->pairs;
        m = bucket->count;
        j = 0;
        while(j < m) {
            count++;
            pair++;
            j++;
        }
        bucket++;
        i++;
    }
    return count;
}