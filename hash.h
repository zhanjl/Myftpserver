#ifndef HASH_H
#define HASH_H

//hash函数，输入hash表的大小和key值
typedef unsigned int (*hashfunc_t)(unsigned int buckets, void *key, unsigned int keySize);
//hash结点结构
typedef struct hash_node
{
    void *key;              //键
    void *value;            //值
    struct hash_node *prev;
    struct hash_node *next;
} hash_node_t;

//hash表的结构
typedef struct hash
{
    hash_node_t **nodes;    //指向hash表数组的指针
    unsigned buckets;       //数组的大小
    hashfunc_t hash_func;   //hash函数
} hash_t;

//建立一个hash表
hash_t *hash_alloc(unsigned int buckets, hashfunc_t hash_func);
void* hash_lookup_value_by_key(hash_t *hash, void *key, unsigned int keySize);
void hash_add_entry(hash_t *hash, void *key, unsigned int keySize, void *value, unsigned int valueSize);
void hash_free_entry(hash_t *hash, void *key, unsigned int keySize);
void hash_clear_entry(hash_t *hash);
void hash_destroy(hash_t *hash);
#endif  //HASH_H
