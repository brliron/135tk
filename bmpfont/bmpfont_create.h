#ifndef BMPFONT_CREATE_H_
# define BMPFONT_CREATE_H_

#ifdef __cplusplus
extern "C" {
#endif

// Called at the beginning of the program.
// The return value will be given to all the other functions in the obj parameter.
// NULL means the function failed. If you don't have any object to return, just return (void*)1 or font.
void* graphics_init(int ac, char* const* av);

// Called after all calls to the graphics_* functions. Use it to free the things allocated in graphics_init.
void graphics_free(void* obj);

// Print an help message.
void graphics_help();

// This function should draw the character c into dest.
// Dest is an array of 256x256 32bpp pixels, filled with black.
// After the call, w and h should contain the width and height of the character in dest.
void graphics_put_char(void* obj, WCHAR c, BYTE** dest, int* w, int* h);

#ifdef __cplusplus
}
#endif

#endif /* !BMPFONT_CREATE_H_ */
