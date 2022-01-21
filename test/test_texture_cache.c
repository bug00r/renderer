#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "renderer.h"
#include "scene_builder.h"
#include "texture.h"
#include "texture_cache.h"

int 
main() {
	#ifdef debug
		printf("Start test texture cache\n");
	#endif	
	
	texture_cache_t* cache = texture_cache_new();

	texture_t * tex1 = texture_new(20, 20);
	texture_t * tex2 = texture_new(30, 30);
	texture_t * tex3 = texture_new(40, 40);

	int id = texture_cache_register(cache, tex1);
	assert(id == 0);

	id = texture_cache_register(cache, tex2);
	assert(id == 1);

	id = texture_cache_register(cache, tex3);
	assert(id == 2);

	texture_t *texture = texture_cache_get(cache, 500);
	assert(texture == NULL);

	texture = texture_cache_get(cache, 1);
	assert(texture == tex2);
	assert(texture->width == 30);
	assert(texture->height == 30);

	texture = texture_cache_get(cache, 2);
	assert(texture == tex3);
	assert(texture->width == 40);
	assert(texture->height == 40);

	texture = texture_cache_get(cache, 0);
	assert(texture == tex1);
	assert(texture->width == 20);
	assert(texture->height == 20);

	texture_cache_free(&cache);

	#ifdef debug
		printf("End test texture cache\n");
	#endif	
	
	return 0;
}