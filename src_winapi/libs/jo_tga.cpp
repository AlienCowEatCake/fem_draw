// http://www.jonolick.com/code.html
#ifndef JO_INCLUDE_TGA_H
#define JO_INCLUDE_TGA_H

#include <stdio.h>

void jo_write_tga(const char *filename, void *rgba, int width, int height, int numChannels);

#endif

#ifndef JO_TGA_HEADER_FILE_ONLY

#if defined(_MSC_VER) && _MSC_VER >= 0x1400
#define _CRT_SECURE_NO_WARNINGS // suppress warnings about fopen()
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void jo_write_tga(const char *filename, void *rgba, int width, int height, int numChannels) {
    FILE *fp = fopen(filename, "wb");
    if(!fp) {
        return;   
    }
    // Header
    fwrite("\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12, 1, fp);
    fwrite(&width, 2, 1, fp);
    fwrite(&height, 2, 1, fp);
    int bpc = numChannels * 8; // 8 bits per channel
    fwrite(&bpc, 2, 1, fp);
    // Swap RGBA to BGRA if using 3 or more channels
    int remap[4] = {numChannels >= 3 ? 2 : 0, 1, numChannels >= 3 ? 0 : 2, 3};
    char *s = (char *)rgba;
    for(int y = height-1; y >= 0; --y) 	{
	    int i = (y * width) * numChannels;
	    for(int x = i; x < i+width*numChannels; x += numChannels) {
		    for(int j = 0; j < numChannels; ++j) {
			    fputc(s[x+remap[j]], fp);
		    }
	    }
    }
    fclose(fp);
}
#endif
