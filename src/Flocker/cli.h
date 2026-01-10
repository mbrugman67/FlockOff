/********************************************************************
 * cli.h
 ********************************************************************
 * Externs for command line handler.
 * Call setupCLI() once to get things allocated and ready to go.
 *
 * updateCLI() needs to be called pretty regularly to keep the RX
 * and TX buffers processed and to do the things.
 *
 * holdCLI() can be used to temporarily disable command line
 * processing.  For example, if a process is using the serial
 * RX buffer, hold the CLI to keep it from stealing the buffer.
 *******************************************************************/
#ifndef CLI_H_
#define CLI_H_

bool setupCLI();
void updateCLI();
void holdCLI(bool hold);

#endif  // CLI_H_
