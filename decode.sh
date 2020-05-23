#!/usr/bin/env sh
# Extract all HuVideo from Power Golf 2.
# This scripts only works on redump image (sha512: 064967b341e68c2346880aff17b01eb7110dd507d88b33a4ff157175b3da90e27e20e6c3717a87ba826091ddd2138e341d7945e72fbf23d854c3fe202018915b)
#
# usage:
#   decode.sh decoder image
# with decoder: binary generated form huvideo_decode.c 
#      image  : Power Golf 2 cd image
#
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

${1} "${2}" ./output 2> /dev/null

for i in ./output/* ; do
    if [ -d "${i}" ]; then
        out="${i}.gif"
        ffmpeg -y -i "${i}/%06d.png" "${out}"
        rm -rf "${i}"
    fi
done
