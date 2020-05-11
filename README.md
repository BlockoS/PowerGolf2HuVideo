# HuVideo extractor for Power Golf 2 - Golfer / パワーゴルフ2 ゴルファー (HCD4056)
# and John Madden Duo CD Football (TGXCD1045)

![03A0A7D0](https://blockos.org/releases/pcengine/HuVideo/PowerGolf2/video_0x03739450.gif)
![0384CE50](https://blockos.org/releases/pcengine/HuVideo/PowerGolf2/video_0x0384CE50.gif)

![0342A090](http://blockos.org/releases/pcengine/HuVideo/Madden/video_0x0342A090.gif)
![0x034CBF70](http://blockos.org/releases/pcengine/HuVideo/Madden/video_0x034CBF70.gif)

## Decoder

### How to build
You only have to compile `huvideo_decode.c` with your favorite C compiler.
```sh
gcc huvideo_decode.c -o huvideo_decode
```

### Usage
```sh
huvideo_decode -o 0x03739450 -g 0 <image> <output_prefix>
```
### Description
This program will extract all video frames of a single HuVideo from a CDROM image and output them as PNG files.

### Parameters
 * `-o/--offset <hex>` (mandatory) specify the offset in byte in the image file.
 * `-g/--game <int>` (optional) specify the game being process (0 for Power Golf 2 - Golfer and 1 for John Madden Duo CD Football).
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

The result can be found here : https://blockos.org/releases/pcengine/HuVideo/PowerGolf2/

## John Madden Duo CD Football
A similar script named `madden_decode.sh` extracts all HuVideo from the track 02 of John Madden Duo CD Football.

The result can be found here : https://blockos.org/releases/pcengine/HuVideo/Madden/

### Parameters
 * `<decoder>` is the HuVideo decoder.
 * `<img>` is the Power Golf 2 CDROM image.
