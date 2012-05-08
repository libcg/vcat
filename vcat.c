/*
vcat -- Outputs a video on a 256-color enabled terminal with UTF-8 locale
Andreas Textor <textor.andreas@googlemail.com>
Clément Guérin <geecko.dev@free.fr>

Copyright 2010 Andreas Textor. All rights reserved.
Copyright 2012 Clément Guérin. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Clément Guérin ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Andreas Textor OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <getopt.h>
#include <malloc.h>
#include <unistd.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#define VERSION "0.2"

static bool     interrupt = false;
static uint32_t colors[] = {
    // ANSI
    0x000000, 0x800000, 0x008000, 0x808000, 0x000080, 0x800080,
    0x008080, 0xc0c0c0, 0x808080, 0xff0000, 0x00ff00, 0xffff00,
    0x0000ff, 0xff00ff, 0x00ffff, 0xffffff,
    // Color
    0x000000, 0x00005f, 0x000087, 0x0000af, 0x0000d7, 0x0000ff,
    0x005f00, 0x005f5f, 0x005f87, 0x005faf, 0x005fd7, 0x005fff,
    0x008700, 0x00875f, 0x008787, 0x0087af, 0x0087d7, 0x0087ff,
    0x00af00, 0x00af5f, 0x00af87, 0x00afaf, 0x00afd7, 0x00afff,
    0x00d700, 0x00d75f, 0x00d787, 0x00d7af, 0x00d7d7, 0x00d7ff,
    0x00ff00, 0x00ff5f, 0x00ff87, 0x00ffaf, 0x00ffd7, 0x00ffff,
    0x5f0000, 0x5f005f, 0x5f0087, 0x5f00af, 0x5f00d7, 0x5f00ff,
    0x5f5f00, 0x5f5f5f, 0x5f5f87, 0x5f5faf, 0x5f5fd7, 0x5f5fff,
    0x5f8700, 0x5f875f, 0x5f8787, 0x5f87af, 0x5f87d7, 0x5f87ff,
    0x5faf00, 0x5faf5f, 0x5faf87, 0x5fafaf, 0x5fafd7, 0x5fafff,
    0x5fd700, 0x5fd75f, 0x5fd787, 0x5fd7af, 0x5fd7d7, 0x5fd7ff,
    0x5fff00, 0x5fff5f, 0x5fff87, 0x5fffaf, 0x5fffd7, 0x5fffff,
    0x870000, 0x87005f, 0x870087, 0x8700af, 0x8700d7, 0x8700ff,
    0x875f00, 0x875f5f, 0x875f87, 0x875faf, 0x875fd7, 0x875fff,
    0x878700, 0x87875f, 0x878787, 0x8787af, 0x8787d7, 0x8787ff,
    0x87af00, 0x87af5f, 0x87af87, 0x87afaf, 0x87afd7, 0x87afff,
    0x87d700, 0x87d75f, 0x87d787, 0x87d7af, 0x87d7d7, 0x87d7ff,
    0x87ff00, 0x87ff5f, 0x87ff87, 0x87ffaf, 0x87ffd7, 0x87ffff,
    0xaf0000, 0xaf005f, 0xaf0087, 0xaf00af, 0xaf00d7, 0xaf00ff,
    0xaf5f00, 0xaf5f5f, 0xaf5f87, 0xaf5faf, 0xaf5fd7, 0xaf5fff,
    0xaf8700, 0xaf875f, 0xaf8787, 0xaf87af, 0xaf87d7, 0xaf87ff,
    0xafaf00, 0xafaf5f, 0xafaf87, 0xafafaf, 0xafafd7, 0xafafff,
    0xafd700, 0xafd75f, 0xafd787, 0xafd7af, 0xafd7d7, 0xafd7ff,
    0xafff00, 0xafff5f, 0xafff87, 0xafffaf, 0xafffd7, 0xafffff,
    0xd70000, 0xd7005f, 0xd70087, 0xd700af, 0xd700d7, 0xd700ff,
    0xd75f00, 0xd75f5f, 0xd75f87, 0xd75faf, 0xd75fd7, 0xd75fff,
    0xd78700, 0xd7875f, 0xd78787, 0xd787af, 0xd787d7, 0xd787ff,
    0xd7af00, 0xd7af5f, 0xd7af87, 0xd7afaf, 0xd7afd7, 0xd7afff,
    0xd7d700, 0xd7d75f, 0xd7d787, 0xd7d7af, 0xd7d7d7, 0xd7d7ff,
    0xd7ff00, 0xd7ff5f, 0xd7ff87, 0xd7ffaf, 0xd7ffd7, 0xd7ffff,
    0xff0000, 0xff005f, 0xff0087, 0xff00af, 0xff00d7, 0xff00ff,
    0xff5f00, 0xff5f5f, 0xff5f87, 0xff5faf, 0xff5fd7, 0xff5fff,
    0xff8700, 0xff875f, 0xff8787, 0xff87af, 0xff87d7, 0xff87ff,
    0xffaf00, 0xffaf5f, 0xffaf87, 0xffafaf, 0xffafd7, 0xffafff,
    0xffd700, 0xffd75f, 0xffd787, 0xffd7af, 0xffd7d7, 0xffd7ff,
    0xffff00, 0xffff5f, 0xffff87, 0xffffaf, 0xffffd7, 0xffffff,
    // Grey
    0x080808, 0x121212, 0x1c1c1c, 0x262626, 0x303030, 0x3a3a3a,
    0x444444, 0x4e4e4e, 0x585858, 0x606060, 0x666666, 0x767676,
    0x808080, 0x8a8a8a, 0x949494, 0x9e9e9e, 0xa8a8a8, 0xb2b2b2,
    0xbcbcbc, 0xc6c6c6, 0xd0d0d0, 0xdadada, 0xe4e4e4, 0xeeeeee
};

// Find an xterm color value that matches an BGRA color
uint8_t rgb2xterm(uint32_t* pixel) {
    uint32_t c, match = 0;
    uint32_t r, g, b, d, dist;

    dist = 0xFFFFFFFF;
    for(c = 0; c < 256; c++) {
        r = ((0xff0000 & colors[c]) >> 16) - ((0xff0000 & *pixel) >> 16);
        g = ((0x00ff00 & colors[c]) >> 8)  - ((0x00ff00 & *pixel) >> 8 );
        b =  (0x0000ff & colors[c])        -  (0x0000ff & *pixel);
        d = r * r + g * g + b * b;
        if (d <= dist) {
            dist = d;
            match = c;
        }
    }

    return match;
}

void print_usage() {
    printf("vcat (" VERSION ") outputs a video on a 256-color enabled terminal "
           "with UTF-8 locale.\n"
           "Usage: vcat [OPTION]... videofile\n"
           "    -w | --width   -- Set width\n"
           "    -h | --height  -- Set height\n"
           "    -k | --keep    -- Keep aspect ratio\n"
           "    videofile      -- The video to print.\n");
}

int getNextFrame(AVFormatContext *fctx, AVCodecContext *cctx, int id,
                 AVFrame *frame) {
    static AVPacket packet;
    static int      bytesRemaining = 0;
    static uint8_t *rawData;
    static bool     first = true;
    int             bytesDecoded;
    int             finished;

    // First time we're called, set packet.data to NULL to indicate it
    // doesn't have to be freed
    if (first) {
        first = false;
        packet.data = NULL;
    }

    // Decode packets until we have decoded a complete frame
    for (;;) {
        // Work on the current packet until we have decoded all of it
        while (bytesRemaining > 0) {
            // Decode the next chunk of data
            bytesDecoded = avcodec_decode_video2(cctx, frame, &finished,
                                                 &packet);

            // Was there an error?
            if (bytesDecoded < 0) {
                fprintf(stderr, "Error while decoding frame\n");
                return 0;
            }

            bytesRemaining -= bytesDecoded;
            rawData += bytesDecoded;

            // Did we finish the current frame? Then we can return
            if (finished)
                return 1;
        }

        // Read the next packet, skipping all packets that aren't for this
        // stream
        do {
            // Free old packet
            if (packet.data != NULL)
                av_free_packet(&packet);

            // Read new packet
            if (av_read_packet(fctx, &packet) < 0)
                goto loop_exit;
        } while (packet.stream_index != id);

        bytesRemaining = packet.size;
        rawData = packet.data;
    }

loop_exit:

    // Decode the rest of the last frame
    bytesDecoded = avcodec_decode_video2(cctx, frame, &finished, &packet);

    // Free last packet
    if (packet.data != NULL)
        av_free_packet(&packet);

    return finished;
}

void printFrame(AVFrame *frame, AVCodecContext *cctx,
                uint32_t rwidth, uint32_t rheight,
                uint32_t width, uint32_t height) {
    uint8_t   c1;
    uint8_t   c2;
    uint32_t *p;

    for (uint32_t y = 0; y < 2*rheight; y += 2) {
        printf("\033[%d;%dH", 1+(height-rheight+y)/2, 1+(width-rwidth)/2);
        // Draw a horizontal line. Each console line consists of two
        // pixel lines in order to keep the pixels square (most console
        // fonts have two times the height for the width of each character)
        for (uint32_t x = 0; x < rwidth; x++) {
            p = (uint32_t*)(frame->data[0] + (y+1)*frame->linesize[0]) + x;
            c1 = rgb2xterm(p);

            p = (uint32_t*)(frame->data[0] + y*frame->linesize[0]) + x;
            c2 = rgb2xterm(p);

            printf("\x1b[38;5;%dm\x1b[48;5;%dm▄", c1, c2);
        }
    }
}

void sigint(int param)
{
    interrupt = true;
}

int main(int argc, char* argv[]) {
    static bool        display_help = false;
    static bool        keep_ratio = false;
    int                c;
    int                width = 80;
    int                height = 24;
    uint32_t           rwidth;
    uint32_t           rheight;
    AVFormatContext   *fctx = NULL;
    int                id;
    AVCodecContext    *cctx;
    AVCodec           *pCodec;
    AVFrame           *frame; 
    AVFrame           *frameRGB;
    uint8_t           *buffer;
    struct SwsContext *sctx;

    signal(SIGINT, sigint);

    for(;;) {
        static struct option long_options[] = {
            {"keep",   no_argument,       0, 'k'},     
            {"width",  required_argument, 0, 'w'},
            {"height", required_argument, 0, 'h'},
            {0, 0, 0, 0}
        };

        c = getopt_long (argc, argv, "w:h:k", long_options, NULL);

        if (c == -1)
            break;

        switch (c) {
            case 'w':
                width = atoi(optarg);
                if (width <= 0) {
                    fprintf(stderr, "Invalid width\n");
                    return -1;
                }
                break;

            case 'h':
                height = atoi(optarg);
                if (height <= 0) {
                    fprintf(stderr, "Invalid height\n");
                    return -1;
                }
                break;

            case 'k':
                keep_ratio = true;
                break;

            case '?':
                display_help = true;
                break;

            default:
                abort();
        }
    }

    if (display_help == true || optind == argc || optind < argc - 1) {
        print_usage();
        return 0;
    }

    // Register all formats and codecs
    av_register_all();

    // Open video file
    if (avformat_open_input(&fctx, argv[optind], NULL, NULL))
        return -1;

    // Retrieve stream information
    if (avformat_find_stream_info(fctx, NULL) < 0)
        return -1;

    // Find the first video stream
    id = -1;
    for (int i=0; i<fctx->nb_streams; i++)
        if (fctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            id = i;
            break;
        }

    if (id < 0)
        return -1;

    // Get a pointer to the codec context for the video stream
    cctx = fctx->streams[id]->codec;

    // Find the decoder for the video stream
    pCodec = avcodec_find_decoder(cctx->codec_id);
    if (pCodec == NULL)
        return -1;

    // Inform the codec that we can handle truncated bitstreams -- i.e.,
    // bitstreams where frame boundaries can fall in the middle of packets
    if (pCodec->capabilities & CODEC_CAP_TRUNCATED)
        cctx->flags |= CODEC_FLAG_TRUNCATED;

    // Open codec
    if (avcodec_open2(cctx, pCodec, NULL) < 0)
        return -1;

    // Determine the real width and height of video output
    if (keep_ratio) {
        if ((float)width / height / 2 > (float)cctx->width / cctx->height) {
            rwidth  = 2 * height * cctx->width / cctx->height;
            rheight = height;
        }
        else {
            rwidth  = width;
            rheight = width * cctx->height / cctx->width / 2;
        }
    }
    else {
        rwidth  = width;
        rheight = height;
    }

    // Get an image convert context
    sctx = sws_getContext(cctx->width, cctx->height, cctx->pix_fmt,
                          rwidth, 2*rheight, PIX_FMT_BGRA,
                          SWS_BILINEAR, NULL, NULL, NULL);
    if (sctx == NULL)
        return -1;

    // Allocate video frames
    frame    = avcodec_alloc_frame();
    frameRGB = avcodec_alloc_frame();
    if (frame == NULL || frameRGB == NULL)
        return -1;

    // Determine required buffer size and allocate buffer
    buffer = malloc(avpicture_get_size(PIX_FMT_BGRA, rwidth, 2*rheight));

    // Assign appropriate parts of buffer to image planes in frameRGB
    avpicture_fill((AVPicture*)frameRGB, buffer, PIX_FMT_BGRA,
                   rwidth, 2*rheight);

    // Clear the terminal only once
    printf("\033[2J");

    // Print all the frames
    while (getNextFrame(fctx, cctx, id, frame) && !interrupt) {
        sws_scale(sctx, (const uint8_t* const*)frame->data, frame->linesize,
                  0, cctx->height, frameRGB->data, frameRGB->linesize);
            
        printFrame(frameRGB, cctx, rwidth, rheight, width, height);
    }
    
    // Reset the color
    printf("\x1b[0m\n");
    
    // Free the convert context
    sws_freeContext(sctx);
    
    // Free the RGB image
    free(buffer);
    av_free(frameRGB);

    // Free the YUV frame
    av_free(frame);

    // Close the codec
    avcodec_close(cctx);

    // Close the video file
    avformat_close_input(&fctx);

    return 0;
}

