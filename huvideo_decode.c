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

static uint32_t g_sector_size = 0x0930; // redump seems to be using mode1, so sectors are 2352 bytes long.

enum GameID {
    PowerGolf2,
    Madden
};

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
    uint16_t unknown[4];
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
    // The next 8 bytes are unused.
    fread(&header->unknown, 1, 8, in);

    return 1;
}

// Convert PCE tile vram data to RGB8.
void tile_to_rgb8(uint8_t *rgb, uint8_t *vram, uint8_t *palette, struct header_t *header) {
    uint16_t tile_w = header->width / 8;
    uint16_t tile_h = header->height / 8;
    uint32_t rgb_line_stride = header->width * 3;

    for(int j=0; j<tile_h; j++) {
        for(int i=0; i<tile_w; i++) {
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

// Convert PCE sprite vram data to RGB8 (only supports 32*64 sprite size).
void sprite_to_rgb8(uint8_t *rgb, uint8_t *vram, uint8_t *palette, struct header_t *header) {
    uint16_t sprite_w = header->width / 16;
    uint16_t sprite_h = header->height / 16;
    uint32_t rgb_line_stride = header->width * 3;

    for(int j=0; j<sprite_h; j++) {
        for(int i=0; i<sprite_w; i++) {
            uint16_t *pce_sprite = (uint16_t*)(vram + (i + j*sprite_w) * 0x80);
            
            int u = ((i & 1) * 16) + (j * 32);
            int v = (i >> 1) * 16;
            uint8_t *out_sprite = rgb + (u + v*header->width) * 3;
            for(int y=0; y<16; y++, pce_sprite+=1, out_sprite+=rgb_line_stride) {
                uint16_t w0 = pce_sprite[0];
                uint16_t w1 = pce_sprite[16];
                uint16_t w2 = pce_sprite[32];
                uint16_t w3 = pce_sprite[48];

                uint8_t *out = out_sprite + 15*3;
                for(int x=0; x<16; x++, out-=3) {
                    uint8_t index = (w0&1) | ((w1&1)<<1) | ((w2&1)<<2) | ((w3&1)<<3);
                    out[0] = palette[3*index];
                    out[1] = palette[3*index+1];
                    out[2] = palette[3*index+2];

                    w0 >>= 1;
                    w1 >>= 1;
                    w2 >>= 1;
                    w3 >>= 1;
                }
            }
        }
    }
}

void usage() {
    fprintf(stderr, "huvideo_decode -o/--offset N -g/--game G in output_prefix\n");
}

int main(int argc, char **argv) {
    int c;
    int option_index;

    const struct option options[] = {
        {"offset",  required_argument, 0, 'o' },
        {"game",    optional_argument, 0, 'g' },
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

    int game_id = PowerGolf2;
    
    for(;;) {
        c = getopt_long(argc, argv, "g:o:", options, &option_index);
        if(c < 0) {
            break;
        }
        switch(c) {
            case 'o':
                offset = strtoul(optarg, NULL, 16);
                break;
            case 'g':
                game_id = atoi(optarg);
                break;
            default:
                usage();
                return EXIT_FAILURE;
        }
    }

    if((game_id < 0) || (game_id > Madden)) {
        fprintf(stderr, "Invalid game id. It must be either 0 (Power Golf 2 - Golfer) or 1 (John Madden Duo CD Football).\n");
        usage();
        return EXIT_FAILURE;
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

    int32_t skip_sector_count = 0;
    // Found by trial and error.
    if(game_id == PowerGolf2) {
        skip_sector_count = 8;
    }
    else {
        // Madden
        if((header.width == 0x80) && (header.height == 0x80)) {
            skip_sector_count = header.unknown[1];
        }
        else if((header.width == 0x100) && (header.height == 0x70)) {
            skip_sector_count = 0x4;
        }
        else {
            header.format = SPR;
            skip_sector_count = header.unknown[1];
        }
    }

    // Skip what should have been palettes and tiles data, but ... there's only 1 palette, maybe BAT and crap.
    fseek(in, offset + g_sector_size*skip_sector_count, SEEK_SET);

    // Allocate output filename buffer.
    filename_len = strlen(argv[optind+1]) + 6 + 5; // output_prefix + image number + .png + \0
    filename = (char*)malloc(filename_len);

    // Read tiles.
    img = (uint8_t*)malloc(header.width*header.height*3);
    vram_data_size = header.width*header.height*32/64;
    vram_data = (uint8_t*)malloc(vram_data_size);

    for(int k=0; k<header.frames; k++) {
        size_t remaining;
        uint8_t *ptr = vram_data;
        for(remaining = vram_data_size; remaining>=2048; remaining -= 2048) {
            fread(ptr, 1, 2048, in);
            fseek(in, 0x130, SEEK_CUR);
            ptr += 2048;
        }
        if(remaining) {
            fread(ptr, 1, remaining, in);
            fseek(in, g_sector_size-remaining, SEEK_CUR);
        }

        if(header.format == BG) {
            // Convert from PCE planar vram tile to rgb8.
            tile_to_rgb8(img, vram_data, palette, &header);
        }
        else {
            // Convert from PCE planar sprite tiles to rgb8.
            sprite_to_rgb8(img, vram_data, palette, &header);
        }

        snprintf(filename, filename_len, "%s%06d.png", argv[optind+1], k);
        stbi_write_png(filename, header.width, header.height, 3, img, 0);
    }

    free(filename);
    free(img);
    free(vram_data);

    return EXIT_SUCCESS;
}
