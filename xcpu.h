#ifndef XCPU_H
#define XCPU_H


#define X_MAX_REGS 16                 /* maximum size of the data stack */


/* CPU context struct */

#define X_STATE_COND_FLAG     0x0001 /* Last comparison result       */
#define X_STATE_DEBUG_ON      0x0002 /* CPU is is debug mode         */
#define X_STATE_IN_EXCEPTION  0x0004 /* CPU is handling an exception */

#define X_STACK_REG           15     /* stack register */

typedef struct xcpu_context {        
  unsigned short regs[X_MAX_REGS];    /* general register file */
  unsigned short pc;                  /* program counter */
  unsigned short state;               /* state register */
  unsigned short itr;                 /* interrupt table register */
  unsigned short id;                  /* cpu identifier */
  unsigned short num;                 /* number of cpus */
} xcpu;


/* title: execute next instruction
 * param: pointer to CPU context
 * function: performs instruction indexed by the program counter 
 * returns: 1 if successful, 0 if not
 */
extern int xcpu_execute( xcpu *c ); 

enum {
 X_E_INTR,    /* Interrupt has occurred */
 X_E_TRAP,    /* Trap has occurred */
 X_E_FAULT,   /* Fault has occurred */
 X_E_LAST
};

/* title: generate an exception (interrupt, trap, or fault) 
 * param: pointer to CPU context, type of exception 
 *        (X_E_INTR, X_E_TRAP, X_E_FAULT)
 * function: generates the exception, pushes current state register and 
             pc on stack, jumps to handler, sets the X_STATE_IN_EXCEPTION 
 *           bit in state register
 * returns: 1 if successful, 0 if not
 */
extern int xcpu_exception( xcpu *c, unsigned int ex ); 

/* title: print state of CPU and current instruction to be executed.
 * param: pointer to CPU context
 * function: outputs instruction (as encoded) and the state of all registers
 *           to the specified output file in the CPU context.
 * returns: void
 */
extern void xcpu_print( xcpu *c );

#endif
