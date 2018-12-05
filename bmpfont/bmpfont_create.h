#ifndef BMPFONT_CREATE_H_
# define BMPFONT_CREATE_H_

#ifdef __cplusplus
extern "C" {
#endif

/************************
**** API for plugins ****
************************/

// Called at the beginning of the program.
// The return value will be given to all the other functions in the obj parameter.
// NULL means the function failed. If you don't have any object to return, you can just return (void*)1.
void *graphics_init();

// Called for every plugin-specific argument.
// Return 1 if you handled the argument correctly, or 0 if you have an error.
// If name is "--help", you should ignore value, list the arguments you recognize, and return 0.
// If name is NULL, you won't receive any more options. Return 0 if you miss some require option
// at this point.
// You should be displaying error messages, the caller won't do it.
int graphics_consume_option(void *obj, const char *name, const char *value);

// If you want to support some options with binary parameters for when bmpfont is used as a library,
// you can optionnaly implement this function.
// It have the same semantincs as graphics_consume_options, but it doesn't need to support "--help".
// These options can't be accessed from the command line, so you shouldn't list them in your --help message.
// Instead, document them in your source code.
int graphics_consume_option_binary(void *obj, const char *name, void *value, size_t value_size);

// This function should draw the character c into dest.
// Dest is an array of 256x256 32bpp pixels, filled with black.
// After the call, w and h should contain the width and height of the character in dest.
void graphics_put_char(void *obj, WCHAR c, BYTE **dest, int *w, int *h);

// Called after all calls to the graphics_* functions. Use it to free the things allocated in graphics_init.
void graphics_free(void *obj);


/***************************
**** Internal functions ****
***************************/
// You can call these functions if you use bmpfont as a library
// instead of using the main executable.

// Initialize the bmpfont_create library
void *bmpfont_init();

// Call this for every argument. For example:
// bmpfont_consume_option(bmpfont, "--format", "packed_rgba");
// This function has the same semantics as the command-line interface
// (internally, the command-line tool just passes every option-value pair to this function in order).
// Return 1 on success and 0 on failure.
// If name is "--help", value is ignored and can be NULL.
int bmpfont_add_option(void *bmpfont, const char *name, const char *value);

// Some plugins may support some binary (non-string) options when bmpfont is used as a library.
// For example, the GDI+ plugin can load a font from memory instead of loading it from a file name.
// See the plugins documentation for more details.
// bmpfont itself supports these binary options:
// * --chars_list
//   value is an array of booleans (char*). value_size is the size of this array.
//   Every entry in this array represents an unicode code point. Entry 0 is \0,
//   entry 0x41 is the letter A (U+0041), entry 0xABCD is U+ABCD etc.
//   For every possible 16-bits code point, a non-zero value means to include this character
//   in the font.
//   By default, every character between U+0020 (space) and U+FFFE are included.
//   If the array is smaller than 65535, the missing entries are assumed to be zero.
//   The array is copied inside the function, you can free it after calling this function.
// * --out-buffer
//   value is a pointer to a pointer (BYTE**). value_size must be sizeof(void*).
//   When bmpfont_run returns, *value will contain the generated bitmap file.
//   Calling bmpfont_free will free the memory in *value.
// * --out-size
//   value is a pointer to a size_t (size_t*). value_size must be sizeof(size_t).
//   When bmpfont_run returns, *value will contain the size of the generated bitmap size.
int bmpfont_add_option_binary(void *bmpfont, const char *name, void *value, size_t value_size);

// Generate a bmp font
int bmpfont_run(void *bmpfont);

// Free the bmpfont library
void bmpfont_free(void *bmpfont);

#ifdef __cplusplus
}
#endif

#endif /* !BMPFONT_CREATE_H_ */
