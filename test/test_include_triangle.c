void test_render_triangle(Renderer * renderer, bool isperspective) {
	
	Vec3 p1 = { -0.75f, -0.75f, 0.75f };
	Vec3 p2 = { 0.75f, -0.55f, 0.25f };
	Vec3 p3 = { 0.05f, 0.75f, -0.75f };
	
	//1. add triangle
	Mesh * triangle = create_triangle3(&p1, &p2, &p3);
	
	render_mesh(renderer, triangle);
	
	#ifdef output
		char * filename = create_string("build/__%u_triangle_%uxMSAA.ppm", 
										isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
	#else
		(void)(isperspective);
	#endif
	free_mesh(triangle);
}