/*
 * Attachment code for having the generic random device use the
 * pseudo-random device.
 */

#include <types.h>
#include <lib.h>
#include <generic/random.h>
#include <generic/pseudorand.h>
#include "autoconf.h"

struct random_softc *
attach_random_to_pseudorand(int randomno, struct pseudorand_softc *ls)
{
	struct random_softc *rs = kmalloc(sizeof(struct random_softc));
	if (rs==NULL) {
		return NULL;
	}

	(void)randomno;  // unused

	rs->rs_devdata = ls;
	rs->rs_random = pseudorand_random;
	rs->rs_randmax = pseudorand_randmax;
	rs->rs_read = pseudorand_read;

	return rs;
}
