#ifndef XDEV_H
#define XDEV_H

/* title: associate port with device
 * param: unsigned short port   : port to be associated with device
 * function: associate an unuserd port id with a device
 * returns: 1 if successful, 0 otherwise
 *          failures may occur if port is already associated or
 *          if memory could not be allocated or initialized.
 */
extern int xdev_associate_port( unsigned short port ); 


/* title: synchronous put of data to port by device
 * param: unsigned short data   : word of data to be put to port
 *        unsigned short port   : port to which the data should be put
 * function: put data to specified port by device and block until CPU
 *           receives data (performs inp or inpa)
 * returns: 1 if successful, 0 otherwise
 *          failures may occur if port is not already associated with a device
 */
extern int xdev_dev_put( unsigned short data, unsigned short port );


/* title: synchronous get of data from port by device
 * param: unsigned short port   : port from which the data should be gotten
 *        unsigned short *data  : pointer to where gotten data should be written
 * function: get data from specified port.  If no data is available,
 *           block until data is available (until a CPU performs outp or outpa).
 *           The data should be written to the specified location.
 * returns: 1 if successful, 0 otherwise
 *          failures may occur if port is not already associated with a device
 */
extern int xdev_dev_get( unsigned short port, unsigned short *data );



/* title: synchronous output to port by CPU
 * param: unsigned short data   : word of data to be sent to port
 *        unsigned short port   : port to which the data should be sent
 * function: output data to specified port and block until associated device 
 *           receives data
 * returns: 1 if successful, 0 otherwise
 *          failures may occur if port is not already associated with a device
 */
extern int xdev_outp_sync( unsigned short data, unsigned short port );


/* title: asynchronous output to port by CPU
 * param: unsigned short data   : word of data to be sent to port
 *        unsigned short port   : port to which the data should be sent
 * function: output data to specified port and return immediately
 * returns: 1 if successful, 0 otherwise
 *          failures may occur if port is not already associated with a device
 */
extern int xdev_outp_async( unsigned short data, unsigned short port );


/* title: synchronous input from port by CPU
 * param: unsigned short port   : port from which the data should be received
 *        unsigned short *data  : pointer to where received data should be 
 *                                written 
 * function: receive data from specified port.  If no data is available,
 *           block until data is available.  The data should be written to
 *           the specified location.
 * returns: 1 if successful, 0 otherwise
 *          failures may occur if port is not already associated with a device
 */
extern int xdev_inp_sync( unsigned short port, unsigned short *data );


/* title: poll port for input by CPU
 * param: unsigned short port   : port from which the data should be polled
 *        unsigned short *data  : pointer to where received data should be 
 *                                written 
 * function: poll specified port.  If data is available, receive data from
 *           the port.  Return immediately regardless of whether data was
 *           received.
 * returns: 1 if data was received, 0 otherwise
 *          failures may occur if port is not already associated with a device
 *          or no data was available.
 */
extern int xdev_inp_poll( unsigned short port, unsigned short *data );

#endif
