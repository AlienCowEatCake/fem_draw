// http://www.jonolick.com/code.html
#ifndef JO_IMAGES_H
#define JO_IMAGES_H

#include <QDataStream>
#include <QImage>

typedef struct {
    QDataStream *stream;
    unsigned char palette[0x300];
    short width, height, repeat;
    int numColors, palSize;
    int frame;
} q_jo_gif_t;

// width/height | the same for every frame
// repeat       | 0 = loop forever, 1 = loop once, etc...
// palSize      | must be power of 2 - 1. so, 255 not 256.
q_jo_gif_t jo_gif_start(QDataStream &stream, short width, short height, short repeat = 0, int numColors = 255);

// gif          | the state (returned from jo_gif_start)
// rgba         | the pixels
// delayCsec    | amount of time in between frames (in centiseconds)
// localPalette | true if you want a unique palette generated for this frame (does not effect future frames)
void jo_gif_frame(q_jo_gif_t &gif, const QImage &image, short delayCsec, bool localPalette = false);

// gif          | the state (returned from jo_gif_start)
void jo_gif_end(q_jo_gif_t &gif);

// Returns false on failure
void jo_write_jpg(QDataStream &stream, const QImage &image, int quality = 0);
void jo_write_tga(QDataStream &stream, const QImage &image, bool transparency);

#endif // JO_IMAGES_H
