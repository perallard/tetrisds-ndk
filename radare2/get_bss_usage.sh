#!/bin/bash

#bss_start=0x020a5380
#bss_end=0x0213a160

bss_start=0x0205a0d0
bss_end=0x020a5380

text_start=0x02000e94
text_end=0x02026500

# line format: 0x020268c4 -> 0x02026910  DATA
r2 -p tetris_ds -q -c 'axq' | grep DATA |
while read -r line
do
  arr=($line)

  if (( ${arr[0]} >= $text_start )) && (( ${arr[0]} < $text_end )); then
    echo "${arr[2]}" >> tmp_bss_usage.txt
  fi
done

maxaddr=0x0

r2 -p tetris_ds -q -c 'pf x @@.tmp_bss_usage.txt' > tmp_bss_usage2.txt

while read -r line
do
  arr=($line)

  if (( ${arr[2]} >= $bss_start )) && (( ${arr[2]} < $bss_end )); then
    echo "data ref: ${arr[2]}"
    if (( ${arr[2]} > $maxaddr )); then
      maxaddr=${arr[2]}
    fi
  fi
done < tmp_bss_usage2.txt

echo "Max bss address: ${maxaddr}"

rm tmp_bss_usage.txt
rm tmp_bss_usage2.txt
