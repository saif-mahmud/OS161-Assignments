/*
 * Simple pseudorandom generator.
 *
 * This is from the article "Random Number Generators: Good Ones are
 * Hard to Find", by S.K. Park and K.W. Miller in the October 1988
 * CACM.
 */

#include <types.h>
#include <lib.h>
#include <uio.h>
#include <generic/pseudorand.h>
#include "autoconf.h"

#define MULTIPLIER    16807
#define MODULUS       0x7fffffff
#define K1            127773		/* MODULUS / MULTIPLIER */
#define K2            2836		/* MODULUS % MULTIPLIER */

uint32_t
pseudorand_random(void *devdata)
{
	struct pseudorand_softc *ps = devdata;
	int32_t hi, lo, tmp;

	hi = ps->ps_seed / K1;
	lo = ps->ps_seed % K1;
	tmp = MULTIPLIER * lo - K2 * hi;
	if (tmp <= 0) {
		tmp += MODULUS;
	}
	ps->ps_seed = tmp;

	/*
	 * The seed ranges from 1 through MODULUS-1. 
	 * Return values 0 through MODULUS-2.
	 */
	return ps->ps_seed - 1;
}

uint32_t
pseudorand_randmax(void *devdata)
{
	(void)devdata;

	/* RAND_MAX should be the largest integer we can return. */
	return MODULUS - 2;
}

struct pseudorand_softc *
pseudoattach_pseudorand(int unit)
{
	struct pseudorand_softc *ps = kmalloc(sizeof(struct pseudorand_softc));
	if (ps==NULL) {
		return NULL;
	}

	(void) unit; // unnecessary

	/*
	 * It would be nice if there were a good way to pick the random
	 * seed. (Note: it must be between 1 and MODULUS-1 inclusive; 
	 * do not make it 0.)
	 */

	ps->ps_seed = 305824;

	return ps;
}

/*
 * Access through VFS ("random:").
 *
 * Since the generator doesn't return 32 actual random bits, do
 * 24 at a time.
 */

int
pseudorand_read(void *devdata, struct uio *uio)
{
	uint32_t val;
	uint8_t bytes[3];
	int result;

	while (uio->uio_resid > 0) {
		val = pseudorand_random(devdata);

		/*
		 * Since the maximum value is MODULUS-2, or 0x7ffffffd,
		 * and the generator distributes evenly among the numbers
		 * it returns, it does not actually give us quite random
		 * bits. All of the bits will be very slightly biased.
		 * There's no easy fix for this, so we'll just ignore it.
		 * The strongest effect will be on the outermost bits
		 * (I think - feel free to tell me I'm wrong), so, since
		 * we need to select 24 of the 31 bits anyway, pick the
		 * inside ones.
		 */
		bytes[0] = (val & 0x00000ff0) >> 4;
		bytes[1] = (val & 0x000ff000) >> 12;
		bytes[2] = (val & 0x0ff00000) >> 20;

		result = uiomove(bytes, sizeof(bytes), uio);
		if (result) {
			return result;
		}
	}

	return 0;
}
