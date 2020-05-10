/*
 * HuVideo decoder for Power Golf 2 - Golfer / パワーゴルフ2 ゴルファー
 * Licensed under Public Domain
 * No warranty implied
 * Use at your own risk
 * 2020 - Vincent Cruz
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

enum HuVideoFormat {
    BG = 0,
    SPR
};

struct header_t {
    uint16_t frames;
    uint16_t width;
    uint16_t height;
    uint8_t flag;
    uint8_t format;

};

/* This part is based upon the source code found in Power Golf 2 and Beyond Shadowgate. */
int decode_header(FILE *in, struct header_t *header) {
    static const char magic[16] = "HuVIDEO         ";
    
    size_t n_read;
    uint8_t buffer[16];

    n_read = fread(buffer, 1, 16, in);
    if(n_read != 16) {
        fprintf(stderr, "failed to read header ID.\n");
        return 0;
    }
    if(memcmp(buffer, magic, 16)) {
        fprintf(stderr, "invalid header ID.\n");
        return 0;
    }
    n_read = fread(&header->frames, 1, 2, in);
    if(n_read != 2) {
        fprintf(stderr, "failed to read frame count.\n");
        return 0;
    }
    n_read = fread(&header->width, 1, 2, in);
    if(n_read != 2) {
        fprintf(stderr, "failed to read frame width.\n");
        return 0;
    }
    if((header->width < 1) || (header->width >512)) {
        fprintf(stderr, "invalid frame width.\n");
        return 0;
    }
    n_read = fread(&header->height, 1, 2, in);
    if(n_read != 2) {
        fprintf(stderr, "failed to read frame height.\n");
        return 0;
    }
    if((header->height < 1) || (header->height >512)) {
        fprintf(stderr, "invalid frame height.\n");
        return 0;
    }
    n_read = fread(&header->flag, 1, 1, in);
    if(n_read != 1) {
        fprintf(stderr, "failed to read palette flag.\n");
        return 0;
    }
    n_read = fread(&header->format, 1, 1, in);
    if(n_read != 1) {
        fprintf(stderr, "failed to read format.\n");
        return 0;
    }
    if(header->format > 1) {
        fprintf(stderr, "invalid format.\n");
        return 0;
    }
    return 1;
}

// Convert PCE tile vram data to RGB8.
void tile_to_rgb8(uint8_t *rgb, uint8_t *vram, uint8_t *palette, struct header_t *header) {
    uint16_t tile_w = header->width / 8;
    uint16_t tile_h = header->height / 8;
    uint32_t rgb_line_stride = header->width * 3;

    for(int j=0; j<tile_w; j++) {
        for(int i=0; i<tile_h; i++) {
            uint8_t *pce_tile = vram + (i + j*tile_w) * 32;
            uint8_t *out_tile = rgb + (i + j*header->width) * 8 * 3;
            for(int y=0; y<8; y++, pce_tile+=2, out_tile+=rgb_line_stride) {
                uint8_t b0 = pce_tile[0];
                uint8_t b1 = pce_tile[1];
                uint8_t b2 = pce_tile[16];
                uint8_t b3 = pce_tile[17];

                uint8_t *out = out_tile + 7*3;
                for(int x=0; x<8; x++, out-=3) {
                    uint8_t index = (b0&1) | ((b1&1)<<1) | ((b2&1)<<2) | ((b3&1)<<3);
                    out[0] = palette[3*index];
                    out[1] = palette[3*index+1];
                    out[2] = palette[3*index+2];

                    b0 >>= 1;
                    b1 >>= 1;
                    b2 >>= 1;
                    b3 >>= 1;
                }
            }
        }
    }
}

void usage() {
    fprintf(stderr, "huvideo_decode -o/--offset N in output_prefix\n");
}

int main(int argc, char **argv) {
    int c;
    int option_index;

    const struct option options[] = {
        {"offset",  required_argument, 0, 'o' },
        {0,         0,                 0,  0 }
    };

    FILE *in;
    size_t input_length;
    uint64_t offset;
    struct header_t header;

    uint8_t palette[16*3];
    uint8_t buffer[0x20];

    uint8_t *vram_data;
    size_t vram_data_size; 
    size_t n_read;
    uint8_t *img;

    size_t filename_len;
    char *filename;

    for(;;) {
        c = getopt_long(argc, argv, "o:", options, &option_index);
        if(c < 0) {
            break;
        }
        switch(c) {
            case 'o':
                offset = strtoul(optarg, NULL, 16);
                break;
            default:
                usage();
                return EXIT_FAILURE;
        }
    }

    if((optind + 2) > argc) {
        usage();
        return EXIT_FAILURE;
    }

    in = fopen(argv[optind],"rb");
    if(in == NULL) {
        fprintf(stderr, "failed to open %s: %s\n", argv[optind], strerror(errno));
        return EXIT_FAILURE;
    }

    // Compute file size.
    fseek(in, 0, SEEK_END);
    input_length = ftell(in);
    fseek(in, 0, SEEK_SET);
    input_length -= ftell(in);

    // Read  Huvideo header.
    fseek(in, offset, SEEK_SET);
    if(!decode_header(in, &header)) {
        return EXIT_FAILURE;
    }
    
    // Apart from the header the rest is game specific.

    // Read palette
    fseek(in, offset + 0x20, SEEK_SET);
    n_read = fread(buffer, 1, 0x20, in);
    if(n_read != 0x20) {
        fprintf(stderr, "failed to read palette\n");
        return EXIT_FAILURE;
    }

    // [todo] use a fixed LUT instead.
    for(int i=0; i<16; i++) {
        palette[i*3  ] = 255 * ((buffer[2*i] >> 3) & 0x7) / 7;
        palette[i*3+1] = 255 * (((buffer[2*i] >> 6) & 0x07) | ((buffer[2*i+1] & 0x07) << 2)) / 7;
        palette[i*3+2] = 255 * (buffer[2*i] & 0x07) / 7;
    }

    // Let's skip background attribute table (BAT) data as the frames are stored as-is and only use 1 palette.
    // The offset is bigger than the expected BAT data (which would have been header.width/8*header.height/8*2 = 0x200).
    // Even if it were the BAT for the whole animation, it doesn't match.
    // Worse! In some video (still un Power Golf 2), there's what seems to be left-overs from the coder hard drive (fragments of source code, or bits of file system).
    fseek(in, offset + 0x24c0, SEEK_SET);

    // Allocate output filename buffer.
    filename_len = strlen(argv[optind+1]) + 6 + 5; // output_prefix + image number + .png + \0
    filename = (char*)malloc(filename_len);

    // Read tiles.
    img = (uint8_t*)malloc(header.width*header.height*3);
    vram_data_size = header.width*header.height/64 * 32;
    vram_data = (uint8_t*)malloc(vram_data_size);

    for(int k=0; k<header.frames; k++) {
        for(int j=0; j<16; j+=4) {
            size_t n = 4*16*32;
            fread(vram_data+j*16*32, 1, n, in);
            fseek(in, 0x130, SEEK_CUR);             // I have no idea what those 304 bytes are about.
        }

        // Convert from PCE planar vram tile to rgb8.
        tile_to_rgb8(img, vram_data, palette, &header);

        snprintf(filename, filename_len, "%s%06d.png", argv[optind+1], k);
        stbi_write_png(filename, header.width, header.height, 3, img, 0);
    }

    free(filename);
    free(img);
    free(vram_data);

    return EXIT_SUCCESS;
}
