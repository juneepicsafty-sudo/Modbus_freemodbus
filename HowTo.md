# How to Integrate FreeMODBUS

### Step 1: Add FreeMODBUS Source Files to Your Project

First, you need to obtain the FreeMODBUS library source code and add it to your project structure. A good practice is to place third-party libraries in a dedicated folder.

1.  **Download FreeMODBUS**: If you haven't already, download the latest version from the official FreeMODBUS website.
2.  **Directory Structure**: Create a new directory for FreeMODBUS within your `Middlewares/Third_Party` folder. For example: `Middlewares/Third_Party/FreeMODBUS`.
3.  **Copy Files**: Copy the FreeMODBUS source files into this new directory. The core files you'll need are:
    *   `modbus/` (containing `mb.c`, `mbcrc.c`, etc.)
    *   `modbus/include/` (containing `mb.h`, `mbconfig.h`, `mbport.h`, etc.)
    *   `modbus/rtu/` (containing `mbrtu.c`, `mbrtu.h`)
    *   `modbus/tcp/` (if you need Modbus TCP)
    *   `modbus/ascii/` (if you need Modbus ASCII)
    *   `port/` (This contains platform-specific implementations. You will need to create your own port files for your STM32 target).

### Step 2: Configure FreeMODBUS

FreeMODBUS is highly configurable. The main configuration file is `mbconfig.h`.

In `mbconfig.h`, you will enable or disable the Modbus modes you need (RTU, ASCII, TCP) and set device-specific parameters.

```
#define MB_RTU_ENABLED      (1)
#define MB_ASCII_ENABLED    (0)
#define MB_TCP_ENABLED      (0)
#define MB_SLAVE_ADDR       (1) // Set your desired Modbus slave address
```

### Step 3: Implement the Porting Layer

This is the most critical part of the integration. FreeMODBUS uses a porting layer to abstract the hardware (timers, UART) and the operating system (critical sections, event handling). You need to implement the functions defined in `mbport.h`.

Create new files for your port, for example, `portserial.c`, `porttimer.c`, and `portevent.c` for RTOS integration.

#### 3.1. Serial Port (`portserial.c`)

This file will contain the implementation for UART communication. Your project already uses UARTs (`usart.h`, `usart_task.c`), so you can adapt your existing drivers.

You need to implement:

*   `BOOL xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )`: Initialize the UART peripheral for Modbus communication.
*   `void vMBPortClose( void )`: Disable the UART.
*   `void vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )`: Enable/disable UART transmitter and receiver.
*   `BOOL xMBPortSerialGetByte( CHAR * pucByte )`: Read a byte from the UART receive buffer. This should be non-blocking.
*   `BOOL xMBPortSerialPutByte( CHAR cByte )`: Write a byte to the UART transmit buffer. This should also be non-blocking.

The interrupt service routines (ISRs) for the UART will call FreeMODBUS callback functions:

*   In the UART RX ISR: `pxMBFrameCBByteReceived()`
*   In the UART TX ISR: `pxMBFrameCBTransmitterEmpty()`

#### 3.2. Timer Port (`porttimer.c`)

FreeMODBUS RTU requires a timer to detect the 1.5 and 3.5 character time gaps for frame delimitation. You can use one of the STM32's hardware timers.

You need to implement:

*   `BOOL xMBPortTimersInit( USHORT usTim1S5Timerout50us )`: Initialize a hardware timer. The `usTim1S5Timerout50us` parameter gives the timeout in 50µs steps.
*   `void vMBPortTimersEnable( void )`: Start the timer.
*   `void vMBPortTimersDisable( void )`: Stop the timer.

The timer ISR will call `pxMBPortCBTimerExpired()`.

#### 3.3. Event/RTOS Port (`portevent.c`)

This file handles the integration with FreeRTOS. FreeMODBUS is event-driven and designed to run in a single task.

You need to implement:

*   `BOOL xMBPortEventInit( void )`: Create an event queue (like `xQueueCreate`) for FreeMODBUS events.
*   `BOOL xMBPortEventPost( eMBEventType eEvent )`: Post an event to the queue from an ISR or task.
*   `BOOL xMBPortEventGet( eMBEventType * eEvent )`: Block and wait for an event from the queue. This will be called from the main Modbus polling task.

### Step 4: Create the Modbus Task

You need a dedicated FreeRTOS task that will run the Modbus stack.

1.  Create a new task function, for example `prvModbus_Task`.
2.  In this task:
    *   Initialize the Modbus stack: `eMBInit( MB_RTU, 0x01, 1, 9600, MB_PAR_NONE );` (example for RTU).
    *   Enable the Modbus stack: `eMBEnable();`.
    *   Enter an infinite loop that calls `eMBPoll();`. `eMBPoll()` will internally call `xMBPortEventGet()` and block until a Modbus event occurs.

Here is a skeleton for the Modbus task:

```
#include "mb.h"

static void prvModbus_Task(void *pvParameters)
{
    eMBErrorCode eStatus;

    // Initialize the Modbus stack for RTU mode
    eStatus = eMBInit(MB_RTU, 0x01, 1, 9600, MB_PAR_NONE);
    if (eStatus != MB_ENOERR) {
        // Error handling
    }

    // Enable the Modbus stack
    eStatus = eMBEnable();
    if (eStatus != MB_ENOERR) {
        // Error handling
    }

    for (;;)
    {
        // eMBPoll will block until a new event is received
        // by the porting layer's xMBPortEventGet().
        eMBPoll();
    }
}

void modbus_init(void)
{
    OS_TASK_CREATE("Modbus Task",
                   prvModbus_Task,
                   NULL,
                   configMINIMAL_STACK_SIZE + 512, // Adjust stack size as needed
                   OS_TASK_PRIORITY_NORMAL,
                   NULL); // Add task handle if needed
}
```

### Step 5: Implement Modbus Register Callbacks

FreeMODBUS acts as a slave and needs callback functions to read from or write to your application's data model (the Modbus registers). These functions are where you connect Modbus to your application's variables.

You need to implement these functions, typically in a file like `user_mb_app.c`:

*   `eMBRegInputCB(...)`: Handles Read Input Registers.
*   `eMBRegHoldingCB(...)`: Handles Read/Write Holding Registers.
*   `eMBRegCoilsCB(...)`: Handles Read/Write Coils.
*   `eMBRegDiscreteCB(...)`: Handles Read Discrete Inputs.

For example, for holding registers:

```
// Example holding registers
static USHORT usRegHoldingBuf[REG_HOLDING_NREGS];

eMBErrorCode eMBRegHoldingCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode)
{
    eMBErrorCode eStatus = MB_ENOERR;
    int iRegIndex;

    if ((usAddress >= REG_HOLDING_START) && (usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS))
    {
        iRegIndex = (int)(usAddress - REG_HOLDING_START);
        switch (eMode)
        {
        case MB_REG_READ:
            while (usNRegs > 0)
            {
                *pucRegBuffer++ = (UCHAR)(usRegHoldingBuf[iRegIndex] >> 8);
                *pucRegBuffer++ = (UCHAR)(usRegHoldingBuf[iRegIndex] & 0xFF);
                iRegIndex++;
                usNRegs--;
            }
            break;

        case MB_REG_WRITE:
            while (usNRegs > 0)
            {
                usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }
            break;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}
```

### Step 6: Integration with Your Build System

Finally, you need to add all the new source files (`.c`) to your build system (e.g., Makefile, CubeIDE project settings) and add the necessary include paths for the compiler to find the header files.

*   **Include Paths**: Add the path to `FreeMODBUS/modbus/include` and your new porting directory.
*   **Source Files**: Add `mb.c`, `mbrtu.c`, `portserial.c`, `porttimer.c`, `portevent.c`, and your `user_mb_app.c` to the list of files to be compiled.

### Summary of Integration

1.  **File Placement**: Add FreeMODBUS source code to your project tree.
2.  **Configuration**: Edit `mbconfig.h` to suit your needs.
3.  **Porting Layer**: Implement hardware and OS-specific functions for UART, Timer, and RTOS events. This is the most significant part of the work.
4.  **Modbus Task**: Create a FreeRTOS task that initializes and polls the Modbus stack.
5.  **Application Callbacks**: Implement the callback functions that link the Modbus registers to your application's data.
6.  **Build System**: Update your project/makefile to include the new files and paths.

By following these steps, you can successfully integrate FreeMODBUS into your `Faro-FreeRTOS-Modbus_working` project. Given the complexity of your existing project, the key will be to carefully integrate the porting layer with your existing peripheral drivers and RTOS constructs. Good luck!