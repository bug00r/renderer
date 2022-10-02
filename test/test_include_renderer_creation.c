void test_renderer_creation() {
	cRGB_t bgcolor = {0.f, 0.f, 0.f};
	int w = 256;
	int h = 256;
	renderer_t * renderer = renderer_new(w, h, &bgcolor, 1);
	raster_ctx_t *rasterCtx = &renderer->rasterCtx;

	assert(rasterCtx->imgWidth == w);
	assert(rasterCtx->imgHeight == h);
	
	int buffsize = w*h;
	float zbuffervalue = 5.f;
	
	renderer->frameBuffer[buffsize-1].r = 0.5f;
	renderer->frameBuffer[buffsize-1].g = 0.25f;
	renderer->frameBuffer[buffsize-1].b = 0.125f;
	renderer->zBuffer[buffsize-2] = zbuffervalue;
	
	assert(renderer->frameBuffer[buffsize-1].r == 0.5f);
	assert(renderer->frameBuffer[buffsize-1].g == 0.25f);
	assert(renderer->frameBuffer[buffsize-1].b == 0.125f);
	
	assert(renderer->zBuffer[buffsize-2] == zbuffervalue);
	
	assert(rasterCtx->imgWidth == w);
	assert(rasterCtx->imgHeight == h);
	renderer_free(renderer);
}