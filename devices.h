#ifndef DEVICES_H
#define DEVICES_H

/* title: display device
 * param: void *arg (ignored)
 * function: invoked on a separate thread and peforms the following operations:
 *           - associates device with port 1 
 *           - enters an infinite loop
 *           - the loop 
 *             - gets a charatcer (lower half of a word) from the port
 *             - outputs the character to stdout
 * returns: void *, never returns.
 */
extern void *device_display( void *arg ); 


/* title: keyboard device
 * param: void *arg (ignored)
 * function: invoked on a separate thread and peforms the following operations:
 *           - associates device with port 2 
 *           - enters an infinite loop
 *           - the loop 
 *             - gets a character from stdin
 *             - puts the character (lower half of a word) to the port
 * returns: void *, never returns.
 */
extern void *device_keyboard( void *arg ); 


/* title: random device
 * param: void *arg (ignored)
 * function: invoked on a separate thread and peforms the following operations:
 *           - associates device with port 3 
 *           - enters an infinite loop
 *           - the loop 
 *             - generates a random word using random()
 *             - puts the word to the port
 * returns: void *, never returns.
 */
extern void *device_random( void *arg ); 

#endif
