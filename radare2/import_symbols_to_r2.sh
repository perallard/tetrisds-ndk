#!/bin/bash

# Only run this script on a newly created radare2 project. It will import all
# symbols into the project. It does not however check if the symbols are
# already refined.

# $1 path to symbols.txt
# $2 radare project name

commands=""

while read -r line
do
  if [ "${line:0:1}" == "#" ] || [ -z "$line" ]; then
    continue
  fi

  IFS='=' key_value_pair=($line)
  IFS=',' value_list=(${key_value_pair[1]})

  sym_name="${key_value_pair[0]}"
  sym_address="${value_list[0]}"

  if [ "${value_list[1]}" == "function" ]; then
    # check if it's a thumb encoded function (it will have an odd address)
    if (( $sym_address % 2 == 1 )); then
        sym_address=$(( sym_address - 1 ))
        commands="$commands s $sym_address ; e asm.bits=16 ; af $sym_name ; e asm.bits=32 ;"
    else
      commands="$commands af $sym_name $sym_address ;"
    fi
  elif [ "${value_list[1]}" == "object" ]; then
    commands="$commands f $sym_name = $sym_address ;"
  fi
done < $1

r2 -p $2 -q -c ''"$commands"' Ps'
#echo "TEST: $commands :TEST"