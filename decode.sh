#!/usr/bin/env sh
# Extract all HuVideo from Power Golf 2.
# This scripts only works on redump image (sha512: 064967b341e68c2346880aff17b01eb7110dd507d88b33a4ff157175b3da90e27e20e6c3717a87ba826091ddd2138e341d7945e72fbf23d854c3fe202018915b)
#
# usage:
#   decode.sh decoder image
# with decoder: binary generated form huvideo_decode.c 
#      image  : Power Golf 2 cd image
#
offset_list="0x03739450 0x0384CE50 0x0395E390 0x03A0A7D0 0x03B20690 0x03CF2AD0 0x03E3B210 0x04062610 0x0435AA50 0x0445E310 0x04505DD0 0x045A8F10 0x1112F410 0x11242E10 0x11354350 0x11400790 0x11516650 0x116E8A90 0x118311D0 0x11A585D0 0x11D50A10 0x11E542D0 0x11EFBD90 0x11F9EED0"

if [ ! -f "${1}" ] || [ ! -x "${1}" ]; then
    echo "${1} is not an executable file"
    exit 1
fi

if [ ! -f "${2}" ]; then 
    echo "${2} is not a file"
    exit 1
fi

expected="064967b341e68c2346880aff17b01eb7110dd507d88b33a4ff157175b3da90e27e20e6c3717a87ba826091ddd2138e341d7945e72fbf23d854c3fe202018915b"
checksum=`sha512sum -b "${2}" | cut -d' ' -f 1`

if [ "${checksum}" != "${expected}" ]; then
    echo "${2} invalid checksum"
    exit 1
fi

mkdir -p ./output

for offset in $offset_list; do
    out="./output/video_${offset}.gif"
    ${1} -o ${offset} "${2}" ./output/img_
    ffmpeg -y -i ./output/img_%06d.png ${out}
    rm ./output/img_*.png
done
