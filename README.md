# tetrisds-ndk

Reverse out the nitro developer kit from a game, so custom cart images can be created
that can accesses ROM data just like a commercial game would do. To eliminate the
need for DLDI patching when running from flash carts.

## Requirements and setup

To be able to do this a ROM dump of a commercial game is needed. I decided to
go with:

Tetris DS (NTR-ATRP-EUR)  
MD5 340856a9c56cc6cb6cd0966b38f5624f

Use gnu md5sum to generate the MD5 sum.

Depends on devkitPro for cross compilation and creating NDS images.
See: https://devkitpro.org/wiki/Getting_Started

Requires GNU make >= 4.3
See: https://www.gnu.org/software/make/

Your environment needs two variables to be defined in order to build the
projects in the /src folder:

1. TETRIS_DS_ROM: the location of the Tetris DS ROM dump
2. NDK_DIR: the location of the src/nitro folder in this repo

See: How to export environment variables in linux.

## Notes

The font used by the src/util project was found here: https://github.com/dhepper/font8x8

## Radare2

## Contribute

Use private function file section template to unclutter the modules. See template below.
```
/*========================= PRIVATE FUNCTIONS =================================
 * The heuristic for 'private' functions is that they are only called from
 * other functions in this module/logical unit. They are kept here for
 * reference so we don't end up reversing the same function over and over again.
 */
```

## TODO
