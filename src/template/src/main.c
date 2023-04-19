#include "nds.h"

/*
 * These two definitions below are needed so the linker can patch the CRT0 code
 * to execute our own main function.
 */
int main(void);
int (*main_ref)(void) __attribute__ ((section("text_main_fcn_ref"))) = &main;

int main()
{
  ndk_platform_init();
  // Stuff goes here
}
