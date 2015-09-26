// Based on TGA Writer from http://www.jonolick.com/code.html

#include <QDataStream>
#include <QImage>
#include <cstdio>
#include <cstdlib>
#include <cmath>

using namespace std;

void jo_write_tga(QDataStream &stream, const QImage &image, bool transparency) {
    // Convert QImage to rgb(a)
    int width = image.width();
    int height = image.height();
    int numChannels = transparency ? 4 : 3;
    unsigned char *rgba = (unsigned char *)malloc(width * height * numChannels);
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            unsigned offset = numChannels * (y * width + x);
            QRgb pixel = image.pixel(x, y);
            rgba[offset]   = qRed(pixel);
            rgba[offset+1] = qGreen(pixel);
            rgba[offset+2] = qBlue(pixel);
            if(transparency) rgba[offset+3] = qAlpha(pixel);
        }
    }
    // Header
    stream.writeRawData("\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12);
    stream.writeRawData((const char*)(&width), 2);
    stream.writeRawData((const char*)(&height), 2);
    int bpc = numChannels * 8; // 8 bits per channel
    stream.writeRawData((const char*)(&bpc), 2);
    // Swap RGBA to BGRA if using 3 or more channels
    int remap[4] = {numChannels >= 3 ? 2 : 0, 1, numChannels >= 3 ? 0 : 2, 3};
    char *s = (char *)rgba;
    for(int y = height-1; y >= 0; --y) {
        int i = (y * width) * numChannels;
        for(int x = i; x < i+width*numChannels; x += numChannels) {
            for(int j = 0; j < numChannels; ++j) {
                stream << (quint8)(s[x+remap[j]]);
            }
        }
    }
    // Free rgb(a)
    free(rgba);
}
