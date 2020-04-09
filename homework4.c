#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "homework4.h"

typedef enum {first, second, third, final} fsmVar;

int main(void)
{
    char rChar = 0xFF;
    char *response = "\n\n\r2534 is the best course in the curriculum!\r\n\n";
    bool ender = true;

    // Stops the Watchdog timer.
    initBoard();
    // Configures the UART for 9600 baud, 8-bit payload (LSB first), no parity, 1 stop bit.
    eUSCI_UART_ConfigV1 uartConfig = {
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     19,
     8,
     0xD6,
     EUSCI_A_UART_NO_PARITY,
     EUSCI_A_UART_LSB_FIRST,
     EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE,
     EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION,
     EUSCI_A_UART_8_BIT_LEN
    };

    // Makes sure Tx AND Rx pins of EUSCI_A0 work for UART and not as regular GPIO pins.
    GPIO_setAsPeripheralModuleFunctionInputPin(
            GPIO_PORT_P1, GPIO_PIN2,
            GPIO_PRIMARY_MODULE_FUNCTION);

    GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_P1, GPIO_PIN3,
            GPIO_PRIMARY_MODULE_FUNCTION);

    // Initializes EUSCI_A0
    UART_initModule(EUSCI_A0_BASE, &uartConfig);


    // Enables EUSCI_A0
    UART_enableModule(EUSCI_A0_BASE);


    while(1)
    {
        // Checks the receive interrupt flag to see if a received character is available.
       if (UART_getInterruptStatus (EUSCI_A0_BASE,EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)==EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
       {
           // Sets the char variable to the input from the UART
           rChar = UART_receiveData(EUSCI_A0_BASE);

           // Checks interrupt flag for transmission
           if (UART_getInterruptStatus(EUSCI_A0_BASE,EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG) == EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG)
           {
               // Echoes to the console
               UART_transmitData(EUSCI_A0_BASE,rChar);

               // Sends variable to state machine for processing.
               if(charFSM(rChar))
               {
                   // State machine triggered complete '2534'. Resetting variables to allow display of response
                   int i = 0;
                   ender = true;

                   // Begins loop for printing response
                   while(ender)
                   {
                       // Checks transmission flag before sending the char at location[i]. Only increments i if successful to avoid skipping letters
                       if (UART_getInterruptStatus(EUSCI_A0_BASE,EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG) == EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG)
                       {
                           // Transmits data and increments i
                           UART_transmitData(EUSCI_A0_BASE,response[i]);
                           i++;
                       }

                       // Once full response sent, stop while loop.
                       if(i == 48)
                           ender = false;
                   }
               }
           }
       }
    }
}

void initBoard()
{
    WDT_A_hold(WDT_A_BASE);
}

// FSM for detecting character sequence.
bool charFSM(char rChar)
{
    // Sets variables to appropriate values
    bool finished = false;
    static fsmVar fsmChar = first;
    // Begins State Machine
    switch(fsmChar)
    {
        // If first char is 2, move on.
        case(first):
            if(rChar == 50)
                fsmChar = second;
            break;
        // If second char is 5, move on. Else, restart
        case(second):
            if(rChar == 53)
                fsmChar = third;
            else if (rChar != 50)
                fsmChar = first;
            break;
        // If third char is 3, move on. Else, restart
        case(third):
            if(rChar == 51)
                fsmChar = final;
            else if (rChar != 50)
                fsmChar = first;
            else
                fsmChar = second;
            break;
        // If final char is 4, output true. Else, restart
        case(final):
            fsmChar = first;
            if(rChar == 52)
                finished = true;
            else if (rChar == 50)
                fsmChar = second;
            break;
    }

    // Returns whether FSM detected '2534' or not
    return finished;
}
