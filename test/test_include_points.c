void test_render_points(Renderer * renderer, bool isperspective) {
	Vec3 p = { 0.0f, 0.0f, 0.f };
	Mesh * points = create_point3(&p);
	
	render_mesh(renderer, points);
	
	#ifdef output
		char * filename = create_string("build/__%u_point_%uxMSAA.ppm", isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_point_%uxMSAA.ppm", isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#else
		(void)(isperspective);
	#endif
	

	free_mesh(points);
}