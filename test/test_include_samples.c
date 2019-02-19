void test_samples() {
	#ifdef debug
		printf(">> test samples\n");
	#endif	
	
	samples_base_t * samples_base = create_samples_base(4);
	#ifdef debug
		print_samples_base(samples_base);
	#endif	
	
	samples_computed_t * samples_computed = create_samples_computed(4);
	#ifdef debug
		print_samples_computed(samples_computed);
	#endif	
	
	samples_result_t result;
	result.samples = samples_base;
	result.computed = samples_computed;
	
	#ifdef debug
		print_samples_result(&result);
	#endif
	
	free_samples_base(samples_base);
	free_samples_computed(samples_computed);
	#ifdef debug
		printf("<< test samples\n");
	#endif
}