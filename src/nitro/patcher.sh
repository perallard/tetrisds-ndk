#!/bin/bash

# This script is used to patch the ARM9 firmware binary
#
# It will search the object file for sections that have '_patch'
# as a suffix in their section names. The sections VMA address
# is then used to calculate the start address for the patch in
# the ARM9 firmare binary file. And SIZE is used to determine
# how many bytes to write.
#
# Usage: patcher.sh elf_object_file arm9_firmware_file

ARM9_FIRMWARE_ADDRESS=0x02000000

"$DEVKITARM"/bin/arm-none-eabi-objdump -h "$1" | grep "_patch" |
while read -r line
do
  section_info=($line)

  name=${section_info[1]}
  offset=$(( 16#${section_info[3]} - ARM9_FIRMWARE_ADDRESS ))
  size=$(( 16#${section_info[2]} ))

  echo "writing patch: $name of size $size at 0x${section_info[3]}"

  temp_file=$(mktemp)

  "$DEVKITARM"/bin/arm-none-eabi-objcopy -O binary -j "$name" "$1" "$temp_file"

  dd if="$temp_file" of="$2" bs=1 count=$size seek=$offset conv=notrunc 2> /dev/null

  rm "$temp_file"
done
