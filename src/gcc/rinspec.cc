/*
 * The lang_specific_driver processes flags passed to the
 * compiler. This is currently not used.
 */
void lang_specific_driver(struct cl_decoded_option**, unsigned int*, int*) {}
int lang_specific_pre_link(void) { return 0; }

// Not used.
int lang_specific_extra_outfiles = 0;
