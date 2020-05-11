#!/usr/bin/env sh
offset_list="0x009F98E0 0x00A90930 0x00BBBBE0 0x00D35010 0x00E58B50 0x00F7C690 0x010A01D0 0x011C3D10 0x01408ED0 0x0152CA10 0x01650550 0x01774090 0x01897BD0 0x01997440 0x01ABAF80 0x01BDEAC0 0x01D87050 0x020ED890 0x022D22D0 0x0236D370 0x024071B0 0x024A1920 0x03730150 0x025CB040 0x026629C0 0x026FBED0 0x02533FF0 0x025CB040 0x026629C0 0x026FBED0 0x027953E0 0x0282DFC0 0x028C6270 0x029609E0 0x029F7A30 0x02A921A0 0x02B26D30 0x02BC0B70 0x02C57290 0x02CF07A0 0x02D877F0 0x02E21F60 0x02EBC6D0 0x02F56510 0x02FF0C80 0x0308B3F0 0x03126490 0x031C02D0 0x0325AA40 0x032F5AE0 0x0338F920 0x0342A090 0x034CBF70 0x03567010 0x035FFBF0 0x036987D0 0x03730150"

if [ ! -f "${1}" ] || [ ! -x "${1}" ]; then
    echo "${1} is not an executable file"
    exit 1
fi

if [ ! -f "${2}" ]; then 
    echo "${2} is not a file"
    exit 1
fi

mkdir -p ./output

for offset in $offset_list; do
    out="./output/video_${offset}.gif"
    ${1} -g 1 -o ${offset} "${2}" ./output/img_
    ffmpeg -y -i ./output/img_%06d.png ${out}
    rm ./output/img_*.png
done
