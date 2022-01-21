#include "texture_cache.h"

static void __tex_cache_texture_free_wrapper(void **data, void *eachdata)
{
	(void)(eachdata);

	texture_t *texture = (texture_t *)*data;
	texture_free(texture);
}

texture_cache_t* texture_cache_new()
{
    texture_cache_t* newcache = malloc(sizeof(texture_cache_t));
    newcache->textures = dl_list_new();
    return newcache;
}

void texture_cache_free(texture_cache_t **cache)
{
    if ( cache != NULL && *cache != NULL )
    {
        texture_cache_t *to_delete = *cache;
        if ( to_delete->textures != NULL )
		{
			dl_list_each_data(to_delete->textures, NULL, __tex_cache_texture_free_wrapper);
		}

        dl_list_free(&to_delete->textures);
        free(to_delete);
    } 
}

int texture_cache_register(texture_cache_t *cache, texture_t *texture)
{
    dl_list_append(cache->textures, texture);
    return cache->textures->cnt - 1;
}

texture_t* texture_cache_get(texture_cache_t *cache, unsigned int id)
{
    if ( cache != NULL)
    {
        return dl_list_get(cache->textures, id);
    }

    return NULL;   
}
