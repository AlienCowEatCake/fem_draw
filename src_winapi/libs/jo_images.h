// http://www.jonolick.com/code.html
#ifndef JO_IMAGES_H
#define JO_IMAGES_H

#include <stdio.h>

#if defined UNICODE || defined _UNICODE
#include <cwchar>
#endif

// =============================================================================

// Returns false on failure
#if defined UNICODE || defined _UNICODE
extern bool jo_write_jpg(const wchar_t *filename, const void *data, int width, int height, int comp, int quality);
#else
extern bool jo_write_jpg(const char *filename, const void *data, int width, int height, int comp, int quality);
#endif

// =============================================================================

typedef struct {
    FILE *fp;
    unsigned char palette[0x300];
    short width, height, repeat;
    int numColors, palSize;
    int frame;
} jo_gif_t;

// width/height	| the same for every frame
// repeat       | 0 = loop forever, 1 = loop once, etc...
// palSize		| must be power of 2 - 1. so, 255 not 256.
#if defined UNICODE || defined _UNICODE
extern jo_gif_t jo_gif_start(const wchar_t *filename, short width, short height, short repeat, int palSize);
#else
extern jo_gif_t jo_gif_start(const char *filename, short width, short height, short repeat, int palSize);
#endif

// gif			| the state (returned from jo_gif_start)
// rgba         | the pixels
// delayCsec    | amount of time in between frames (in centiseconds)
// localPalette | true if you want a unique palette generated for this frame (does not effect future frames)
extern void jo_gif_frame(jo_gif_t *gif, unsigned char *rgba, short delayCsec, bool localPalette);

// gif          | the state (returned from jo_gif_start)
extern void jo_gif_end(jo_gif_t *gif);

// =============================================================================

// Returns false on failure
#if defined UNICODE || defined _UNICODE
extern bool jo_write_tga(const wchar_t *filename, void *rgba, int width, int height, int numChannels);
#else
extern bool jo_write_tga(const char *filename, void *rgba, int width, int height, int numChannels);
#endif

#endif // JO_IMAGES_H
