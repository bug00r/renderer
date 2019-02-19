renderer_t * create_renderer(int width, int height,
									vec3_t *from, vec3_t* to, 
									float zoom, 
									float left, float right, 
									float top, float bottom, 
									float near, float far,
									cRGB_t *bgcolor,
									unsigned int samplestep) {
    renderer_t *renderer = renderer_new(width, height, bgcolor,samplestep);
	config_camera(&renderer->camera, from, to, zoom * left, zoom * right, zoom * top, zoom * bottom, near, far);
	return renderer;
}

renderer_t * create_test_base_renderer(unsigned int samplestep) {
	cRGB_t  bgcolor = {0.0f, 0.0f, 0.0f};
	vec3_t from = { 1.f, 1.0f, 2.0f };
	vec3_t to = { 0.0f, 0.0f, 0.0f };
	renderer_t *renderer = create_renderer(
									512 /*width*/, 512/*height*/,
									&from, &to, 
									0.5f /* zoom */,
									-2.f /* left */, 2.f /*right*/, 
									2.f /* top */, -2.f /*bottom*/, 
									1.f /* near */, 100.f /*far*/,
									&bgcolor,
									samplestep);
	return renderer;
}

renderer_t * create_renderer_perspective(int width, int height,
									vec3_t *from, vec3_t* to, 
									float zoom, 
									float left, float right, 
									float top, float bottom, 
									float near, float far,
									cRGB_t *bgcolor,
									unsigned int samplestep) {
    renderer_t *renderer = renderer_new(width, height, bgcolor, samplestep);
	renderer->projection = RP_PERSPECTIVE;
	config_camera_perspective(&renderer->camera, from, to, zoom * left, zoom * right, zoom * top, zoom * bottom, near, far);
	return renderer;
}

renderer_t * create_test_base_renderer_perspective(unsigned int samplestep) {
	cRGB_t  bgcolor = {0.0f, 0.0f, 0.0f};
	vec3_t from = { 1.f, 1.0f, 1.f };
	vec3_t to = { 0.0f, 0.0f, 0.0f };
	renderer_t *renderer = create_renderer_perspective(
									512 /*width*/, 512/*height*/,
									&from, &to, 
									1.0f /* zoom */,
									-2.f /* left */, 2.f /*right*/, 
									2.f /* top */, -2.f /*bottom*/, 
									1.f /* near */, 5.f /*far*/,
									&bgcolor,
									samplestep);
	return renderer;
}