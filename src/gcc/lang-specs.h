/*
 * Specifies the compiler to use for the Rinto programming language.
 * Modifies default_compilers in gcc/gcc/gcc.cc to override default
 * GCC errors.
 */
{".rin",  "@rinto", 0, 1, 0},
{"@rinto",  "grin %i %(cc1_options) %{!fsyntax-only:%(invoke_as)}", 0, 1, 0},
