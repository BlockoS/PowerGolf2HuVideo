#!/usr/bin/env sh
if [ ! -f "${1}" ] || [ ! -x "${1}" ]; then
    echo "${1} is not an executable file"
    exit 1
fi

if [ ! -f "${2}" ]; then 
    echo "${2} is not a file"
    exit 1
fi

mkdir -p ./output

${1} -g 1 "${2}" ./output 2> /dev/null

for i in ./output/* ; do
    if [ -d "${i}" ]; then
        out="${i}.gif"
        ffmpeg -y -i "${i}/%06d.png" "${out}"
        rm -rf "${i}"
    fi
done
