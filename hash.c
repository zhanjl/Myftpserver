#include "hash.h"
#include <string.h>
#include <stdlib.h>
//找到hash表的入口点
static hash_node_t **hash_get_bucket(hash_t *hash, void *key, unsigned int keySize);
//找到对应的结点
static hash_node_t  *hash_get_node_by_key(hash_t *hash, void *key, unsigned int keySize);
//释放某个hash表项的所有结点
static void hash_clear_bucket(hash_t *hash, unsigned int index);

hash_t* hash_alloc(unsigned int buckets, hashfunc_t hash_func)
{
    hash_t *ph = (hash_t*)malloc(sizeof(hash_t));
    ph->buckets = buckets;
    ph->nodes = (hash_node_t**)malloc(buckets * sizeof(hash_t*));
    ph->hash_func = hash_func;
    
    memset(ph->nodes, 0, buckets * sizeof(hash_t*));
    return ph;
}

void* hash_lookup_value_by_key(hash_t *hash, void *key, unsigned int keySize)
{

    hash_node_t *pnode = hash_get_node_by_key(hash, key, keySize);
    if (pnode == NULL)
        return NULL;

    return pnode->value;

}

void hash_add_entry(hash_t *hash, void *key, unsigned int keySize, void *value, unsigned int valueSize)
{
    hash_node_t *new_node = (hash_node_t*)malloc(sizeof(hash_node_t));
    new_node->key = malloc(keySize);
    memcpy(new_node->key, key, keySize);
    new_node->value = malloc(valueSize);
    memcpy(new_node->value, value, valueSize);

    hash_node_t **p_entry = hash_get_bucket(hash, key, keySize);
    if (*p_entry == NULL) {
        *p_entry = new_node;
        new_node->next = new_node->prev = NULL;
        return;
    }

    hash_node_t *prev = *p_entry;
    while (prev->next) 
        prev = prev->next;

    prev->next = new_node;
    new_node->prev = prev;
    new_node->next = NULL;
    return;
}

void hash_free_entry(hash_t *hash, void *key, unsigned int keySize)
{
    hash_node_t *pnode = hash_get_node_by_key(hash, key, keySize);
    if (pnode == NULL)
       return;
    if (pnode->prev)
       pnode->prev->next = pnode->next;
    
    if (pnode->next)
       pnode->next->prev = pnode->prev;
    
    free(pnode->value); 
    free(pnode->key); 
    free(pnode);
}

void hash_clear_entry(hash_t *hash)
{
    int i;
    for (i = 0; i < hash->buckets; i++)
       hash_clear_bucket(hash, i); 
}


void hash_destroy(hash_t *hash)
{
    hash_destroy(hash);
    free(hash->nodes);
    free(hash);
}

static hash_node_t **hash_get_bucket(hash_t *hash, void *key, unsigned int keySize)
{
    int offset;
    offset = hash->hash_func(hash->buckets, key, keySize);
    return &(hash->nodes[offset]);
}

static hash_node_t *hash_get_node_by_key(hash_t *hash, void *key, unsigned int keySize)
{
    hash_node_t *pnode = *hash_get_bucket(hash, key, keySize);
    
    while (pnode) {
        if (memcmp(pnode->key, key, keySize) == 0)
            break;
        pnode = pnode->next;
    }

    return pnode;
}

static void hash_clear_bucket(hash_t *hash, unsigned int index)
{
    hash_node_t *cur, *next;
    cur = hash->nodes[index];
    hash->nodes[index] = NULL;
    while (cur) {
        next = cur->next;
        free(cur->value);
        free(cur->key);
        free(cur);
        cur = next;
    }
}
