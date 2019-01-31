#ifndef PAINTSHOP_H
#define PAINTSHOP_H

#include "paintshop_driver.h"

typedef struct paintcan paint_can;
typedef struct semaphore _semaphore;

int remaining_customers;

// buffers (critical regions)
paint_can *order_buffer[NCUSTOMERS];
void *delivery_buffer[NCUSTOMERS];

// binary semaphores
_semaphore *order_mutex;
_semaphore *delivery_mutex;
_semaphore *tints_mutex;

_semaphore *remaining_customers_mutex;

_semaphore *stuff_exit;

_semaphore *access_specific_tints[NCOLOURS];

// counting semaphores

_semaphore *ready_cans;
_semaphore *order_buffer_full;

_semaphore *delivery_buffer_empty;
_semaphore *order_buffer_empty;

#endif



