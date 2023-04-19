#!/bin/bash

# This script will import symbol definitions into an object
# (ELF) file.
#
# The symbol file is a text file that contains a list of symbol
# definitions.
#
# The following example will add a function symbol that is
# located at address 0x02006e2c (ARM9 main RAM) with the
# symbol name ndk_mutex_init:
#
#   ndk_mutex_init=0x02006e2c,function,global
#
# Empty lines and lines that starts with '#' are
# ignored.
#
# Usage: patcher.sh obeject_file symbol_file

while read -r line
do
    if [ -z "$line" ]; then
        continue
    fi

    if [ "#" == "${line:0:1}" ]; then
        continue
    fi

    $DEVKITARM/bin/arm-none-eabi-objcopy --add-symbol $line $1
done < $2
