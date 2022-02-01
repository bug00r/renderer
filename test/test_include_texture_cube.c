void test_render_texture_cube(unsigned int samplestep) {
	bool isperspective = false;
	unsigned int w_ = 512;
	unsigned int h_ = 512;
	texture_t * texture_fractals = texture_new(w_,h_);
	
	mandelbrot_t *mb = mandelbrot_new(w_, h_);
	mb->minreal = -2.f;//-1.3f;
	mb->maxreal = 0.5f;//-1.f;
	mb->minimag = -1.f;//-.3f;
	mb->maximag = 1.f;//0.f;
	mb->cntiterations = 20;
	create_mandelbrot(mb);
	
	mandelbrot_to_texture(mb, texture_fractals, mandelbrot_color_line_int_rgb);
	
	cRGB_t bgcolor = {0.f, 0.0f, 0.f};
	renderer_t * renderer_active_tex = renderer_new(w_, h_, &bgcolor, samplestep);
    
	int texId = texture_cache_register(renderer_active_tex->texture_cache, texture_fractals);
	(void)(texId);

	mandelbrot_free(mb);
	
	vec3_t _from = { 0.5f, 0.34f, 1.f };
	vec3_t _to = { 0.0f, 0.0f, 0.0f };
	float zoom = .25f;
	float bottom = 1.f;
	float left = 1.f;
	config_camera(&renderer_active_tex->camera, &_from, &_to, zoom * -left, zoom * left, zoom * bottom, zoom * -bottom, 1.f, 5.f);
	
	scene_t * texscene = scene_create_texture_test();
	
	render_scene(renderer_active_tex, texscene);
	
	#ifdef output
		char * filename = create_string("build/__%u_texture_cube_%uxMSAA.ppm", isperspective, renderer_active_tex->samplestep);
		renderer_output_ppm(renderer_active_tex, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_texture_cube_%uxMSAA.ppm", isperspective, renderer_active_tex->samplestep);
		renderer_output_z_buffer_ppm(renderer_active_tex, filename);
		free(filename);
	#else
		(void)(isperspective);
	#endif

	free_scene(texscene);	
	renderer_free(renderer_active_tex);
}

/**
    renderer_t *renderer = renderer_new(width, height, bgcolor);
	renderer->projection = RP_PERSPECTIVE;
	config_camera_perspective(&renderer->camera, from, to, zoom * left, zoom * right, zoom * top, zoom * bottom, near, far);
*/
void test_render_texture_cube_perspective(unsigned int samplestep) {
	bool isperspective = true;
	unsigned int w_ = 512;
	unsigned int h_ = 512;
	texture_t * texture_fractals = texture_new(w_,h_);
	
	mandelbrot_t *mb = mandelbrot_new(w_, h_);
	mb->minreal = -2.f;//-1.3f;
	mb->maxreal = 0.5f;//-1.f;
	mb->minimag = -1.f;//-.3f;
	mb->maximag = 1.f;//0.f;
	mb->cntiterations = 20;
	create_mandelbrot(mb);
	
	mandelbrot_to_texture(mb, texture_fractals, mandelbrot_color_line_int_rgb);
	
	cRGB_t bgcolor = {0.f, 0.0f, 0.f};
	renderer_t * renderer_active_tex = renderer_new(w_, h_, &bgcolor, samplestep);
	renderer_active_tex->projection = RP_PERSPECTIVE;

	int texId = texture_cache_register(renderer_active_tex->texture_cache, texture_fractals);
	(void)(texId);
    
	mandelbrot_free(mb);
	
	vec3_t _from = { 0.17f, 0.27f, .2f };
	vec3_t _to = { 0.0f, 0.0f, 0.0f };
	float zoom = 1.f;
	float bottom = 1.f;
	float left = 1.f;
	config_camera_perspective(&renderer_active_tex->camera, &_from, &_to, zoom * -left, zoom * left, zoom * bottom, zoom * -bottom, 1.f, 5.f);
	
	scene_t * texscene = scene_create_texture_test();
	
	render_scene(renderer_active_tex, texscene);
	
	#ifdef output
		char * filename = create_string("build/__%u_texture_cube_%uxMSAA.ppm", isperspective, renderer_active_tex->samplestep);
		renderer_output_ppm(renderer_active_tex, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_texture_cube_%uxMSAA.ppm", isperspective, renderer_active_tex->samplestep);
		renderer_output_z_buffer_ppm(renderer_active_tex, filename);
		free(filename);
	#else
		(void)(isperspective);
	#endif

	free_scene(texscene);	
	renderer_free(renderer_active_tex);
}