#include <types.h>
#include <lib.h>
#include <machine/spl.h>
#include <dev.h>
#include "autoconf.h"  // for pseudoconfig

/*
 * Machine-independent device initialization (and cleanup)
 *
 * This is called as early in boot as possible, because until it's
 * called, console I/O doesn't print.
 *
 * machdep_dev_bootstrap() calls the autoconf stuff for the main
 * system bus, which does all the device probes.
 */

void
dev_bootstrap(void)
{
	/* Interrupts should be off. */
	assert(curspl>0);

	kprintf("Device probe...\n");
	machdep_dev_bootstrap();

	/* Interrupts should now have come on. */
	assert(curspl==0);
	
	/* Now initialize pseudo-devices */
	pseudoconfig();

	kprintf("\n");
}
