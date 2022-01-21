#ifndef TEXTURE_CACHE_H
#define TEXTURE_CACHE_H

#include "dl_list.h"
#include "texture.h"

typedef struct {
    dl_list_t *textures;
} texture_cache_t;

texture_cache_t* texture_cache_new();
void texture_cache_free(texture_cache_t **cache);

/* registers a texture and returns stored ID. The cache took ownership of texture.
   If cache will be free'd the memory for each cach in it will be free'd too */
int texture_cache_register(texture_cache_t *cache, texture_t *texture);
texture_t* texture_cache_get(texture_cache_t *cache, unsigned int id);

#endif