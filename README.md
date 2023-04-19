# tetrisds-nds

## Goal 

To be able to access ROM data just like a commercial game (ROM dump).

## Requirements and setup

Tetris DS (NTR-ATRP-EUR)  
MD5 340856a9c56cc6cb6cd0966b38f5624f  

Depends on devkitPro for cross compilation and creating NDS images.  
See: https://devkitpro.org/wiki/Getting_Started

Requires GNU make >= 4.3
See: https://www.gnu.org/software/make/

Environment needs two variables to be defined:

1. TETRIS_DS_ROM: the location of the Tetris DS ROM dump
2. NDK_DIR: the location of the nitro folder in this repo

The font used by the src/util project was found here: https://github.com/dhepper/font8x8

## Radare2

## Contribute

Use private function file section template to unclutter the modules. See template below.

/*========================= PRIVATE FUNCTIONS =================================
 * The heuristic for 'private' functions is that they are only called from
 * other functions in this module/logical unit. They are kept here for
 * reference so we don't end up reversing the same function over and over agin.
 */

## TODO
