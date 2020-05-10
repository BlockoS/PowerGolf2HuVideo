# HuVideo extractor for Power Golf 2 - Golfer / パワーゴルフ2 ゴルファー (HCD4056)

## Decoder

### How to build
You only have to compile `huvideo_decode.c` with your favorite C compiler.
```sh
gcc huvideo_decode.c -o huvideo_decode
```

### Usage
```sh
huvideo_decode -o 0x03739450 <image> <output_prefix>
```
### Description
This program will extract all video frames of a single HuVideo from a CDROM image and output them as PNG files.

### Parameters
 * `-o/--offset <hex>` specify the offset in byte in the image file.
 * `<image>` CDROM image.
 * `<output_prefix>` output files prefix.
 
## Decoder script

### Usage
```sh
decode.sh <decoder> <img>
```

### Description
`decode.sh` is a shell script that will extract all HuVideo from Power Golf 2 CDROM image and save them as animated GIF using `ffmpeg`.
The CDROM image is expected to match the one from the `redump project`.


### Parameters
 * `<decoder>` is the HuVideo decoder.
 * `<img>` is the Power Golf 2 CDROM image.
