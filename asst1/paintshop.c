#include <types.h>
#include <lib.h>
#include <synch.h>
#include <test.h>
#include <thread.h>

#include "paintshop.h"
#include "paintshop.h"

/*
 * **********************************************************************
 * YOU ARE FREE TO CHANGE THIS FILE BELOW THIS POINT AS YOU SEE FIT
 *
 */

 // Function Prototype

void init_semaphore(void);
void init_buffer(void);

void paintshop_open(void);
void order_paint(paint_can *can);
void go_home(void);
void * take_order(void);
void fill_order(void *v);
void serve_order(void *v);
void paintshop_close(void);

//======================================================================

void init_semaphore(){

    // binary semaphores
    order_mutex = sem_create("order_mutex", 1);
    delivery_mutex = sem_create("delivery_mutex", 1);
    tints_mutex = sem_create("tints_mutex", 1);

    stuff_exit = sem_create("stuff_exit", 1);
    remaining_customers_mutex = sem_create("remaining_customers_mutex", 1);

    for(int i = 0; i < NCOLOURS; i++)
        access_specific_tints[i] = sem_create("access_specific_tints", 1);;

    // counting semaphores
    ready_cans = sem_create("ready_cans", 0);
    order_buffer_full = sem_create("order_buffer_full", 0);

    delivery_buffer_empty = sem_create("delivery_buffer_empty", NCUSTOMERS);
    order_buffer_empty = sem_create("order_buffer_empty", NCUSTOMERS);

 }

 void init_buffer(){

    int i = 0;

    while(i < NCUSTOMERS){

        order_buffer[i] = NULL;
        delivery_buffer[i] = NULL;

        i++;
    }

 }



/*
 * **********************************************************************
 * FUNCTIONS EXECUTED BY CUSTOMER THREADS
 * **********************************************************************
 */

/*
 * order_paint()
 *
 * Takes one argument specifying the can to be filled. The function
 * makes the can available to staff threads and then blocks until the staff
 * have filled the can with the appropriately tinted paint.
 *
 * The can itself contains an array of requested tints.
 */


void order_paint(paint_can *can)
{

    // decrement counting semaphore for the order_buffer
    P(order_buffer_empty);

    // entering critical region (order_buffer) using binary semaphore
    P(order_mutex);

    for(int i = 0; i < NCUSTOMERS; i++){

        if(order_buffer[i] == NULL){
            order_buffer[i] = can;
            break;
        }

    }

    // exitting critical region (order_buffer)
    V(order_mutex);

    // to permit accessing critical region (order_buffer) via staff threads
    V(order_buffer_full);


    bool is_found = false;

    while(true){

        // decrement the number of ready_cans for delivery
        P(ready_cans);

        // enter critical region (delivery_buffer)
        P(delivery_mutex);

        // searching delivery_buffer for the parameter paint_can
        for(int i = 0; i < NCUSTOMERS; i++){

            if((paint_can *)delivery_buffer[i] == can){
                delivery_buffer[i] = NULL;
                is_found = true;
                break;
            }

        }

        // exit critical region (delivery_buffer)
        V(delivery_mutex);

        // increment # of ready cans
        V(ready_cans);


        if(is_found)
        {
            // increment # of empty slots in the delivery_buffer
            V(delivery_buffer_empty);
            break;
        }

    }

}

/*
 * go_home()
 *
 * This function is called by customers when they go home. It could be
 * used to keep track of the number of remaining customers to allow
 * paint shop staff threads to exit when no customers remain.
 */


void go_home()
{
    P(remaining_customers_mutex);

    remaining_customers--;

    V(remaining_customers_mutex);
}


/*
 * **********************************************************************
 * FUNCTIONS EXECUTED BY PAINT SHOP STAFF THREADS
 * **********************************************************************
 */

/*
 * take_order()
 *
 * This function waits for a new order to be submitted by
 * customers. When submitted, it records the details, and returns a
 * pointer to something representing the order.
 *
 * The return pointer type is void * to allow freedom of representation
 * of orders.
 *
 * The function can return NULL to signal the staff thread it can now
 * exit as their are no customers nor orders left.
 */


void * take_order()
{
    void *can;

    bool is_found = false;

    while(true)
    {

        if(remaining_customers == 0)
        {
            // Entering critical region
            P(stuff_exit);

            can = NULL;

            // Exitting critical region
            V(stuff_exit);

            return can;
        }


        // decrement # of ordered cans
        P(order_buffer_full);

        // Enter critical region (order_buffer)
        P(order_mutex);

        // Iterate through the order_buffer
        for(int i = 0; i < NCUSTOMERS; i++){

            if(order_buffer[i] != NULL){

                is_found = true;

                can = (void *)order_buffer[i];
                order_buffer[i] = NULL;

                break;
            }

        }

        // Exitting critical region (order_buffer)
        V(order_mutex);

        // Access through staff thread
        V(order_buffer_full);

        if(is_found)
        {
            // increment # of empty slots in order_buffer
            V(order_buffer_empty);
            break;
        }
    }

    return can;
}

/*
 * fill_order()
 *
 * This function takes an order generated by take order and fills the
 * order using the mix() function to tint the paint.
 *
 * NOTE: IT NEEDS TO ENSURE THAT MIX HAS EXCLUSIVE ACCESS TO THE TINTS
 * IT NEEDS TO USE TO FILE THE ORDER.
 */

void fill_order(void *v)
{
    // Enter critical region
    P(tints_mutex);

    paint_can *can = ((paint_can *)v);

    for(int i = 0; i < PAINT_COMPLEXITY; i++){
        if(can->requested_colours[i]){
            P(access_specific_tints[can->requested_colours[i]]);
        }
    }

    // Exitting critical region
    V(tints_mutex);

    mix(v);

    for(int i = 0; i < PAINT_COMPLEXITY; i++){
        if(can->requested_colours[i]){
            V(access_specific_tints[can->requested_colours[i]]);
        }
    }

}

/*
 * serve_order()
 *
 * Takes a filled order and makes it available to the waiting customer.
 */


void serve_order(void *v)
{
    // decremnet # of empty slots in delivery_buffer */
    P(delivery_buffer_empty);

    // Enterring critical region (delivery_buffer)
    P(delivery_mutex);

    // Search for an empty space in the delivery_buffer and put the paint_can in it
    for(int i = 0; i < NCUSTOMERS; i++)
    {
        if(delivery_buffer[i] == NULL)
        {
            delivery_buffer[i] = v;
            break;
        }
    }
    // Exitting critical region (delivery_buffer)
    V(delivery_mutex);

    // Increment # of ready_cans to be returned to the customer
    V(ready_cans);
}

/*
 * **********************************************************************
 * INITIALISATION AND CLEANUP FUNCTIONS
 * **********************************************************************
 */


/*
 * paintshop_open()
 *
 * Perform any initialisation you need prior to opening the paint shop to
 * staff and customers
 */

void paintshop_open(){

    remaining_customers = NCUSTOMERS;

    init_semaphore();

    init_buffer();

}

/*
 * paintshop_close()
 *
 * Perform any cleanup after the paint shop has closed and everybody
 * has gone home.
 */


void paintshop_close()
{
    sem_destroy(order_mutex);
    sem_destroy(delivery_mutex);
    sem_destroy(tints_mutex);

    sem_destroy(stuff_exit);

    sem_destroy(remaining_customers_mutex);

    for(int i = 0; i < NCOLOURS; i++)
        sem_destroy(access_specific_tints[i]);

    sem_destroy(ready_cans);
    sem_destroy(order_buffer_full);

    sem_destroy(delivery_buffer_empty);
    sem_destroy(order_buffer_empty);

}

