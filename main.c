/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the PSoC 6 MCU Cryptography: True
* Random Number Generation Example for ModusToolbox.
*
* Related Document: See Readme.md
*
*******************************************************************************
* (c) 2019-2020, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Macros
********************************************************************************/
/* Macro for the maximum value of the random number generated in bits */
#define MAX_TRNG_VAL                     (0x7F)

#define PASSWORD_LENGTH                  (8u)
#define ASCII_ALPHANUMERIC_LOWER_LIMIT   (33u)
#define ASCII_ALPHANUMERIC_UPPER_LIMIT   (127u)
#define ASCII_RETURN_CARRIAGE            (0x0D)

#define SCREEN_HEADER "\r\n__________________________________________________"\
           "____________________________\r\n*\tPSoC 6 MCU Cryptography: "\
           "True Random Number Generation\r\n*\r\n*\tThis code example "\
           "demonstrates generating a One-Time Password (OTP)\r\n*\tusing the"\
           " True Random Number generation feature of PSoC 6 MCU\r\n*\t"\
           "cryptography block\r\n*\r\n*\tUART Terminal Settings\tBaud Rate:"\
           "115200 bps 8N1 \r\n*"\
           "\r\n__________________________________________________"\
           "____________________________\r\n\n"

#define SCREEN_HEADER1 "\r\n================================================="\
           "=============================\r\n"

/* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
#define CLEAR_SCREEN         "\x1b[2J\x1b[;H"

/*******************************************************************************
* Function Prototypes
********************************************************************************/
void generate_password();

/*******************************************************************************
* Global Variables
********************************************************************************/

/* Uart object used for reading character from terminal */
extern cyhal_uart_t cy_retarget_io_uart_obj;

/* Variable for storing character read from terminal */
uint8_t uart_read_value;

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for CM4 CPU. It configures the Crypto block to 
* generate random number and convert it to a alpha-numeric password. A new 
* password is generated every time  the user presses the Enter key.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    
    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();
    
    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, \
                                 CY_RETARGET_IO_BAUDRATE);
    
    /* retarget-io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf(CLEAR_SCREEN);

    printf(SCREEN_HEADER);

    printf("Press the Enter key to generate password\r\n");

    for(;;)
    {
        /* Check if 'Enter' key was pressed */
        if (cyhal_uart_getc(&cy_retarget_io_uart_obj, &uart_read_value, 1) == CY_RSLT_SUCCESS)
        {
            if (uart_read_value == ASCII_RETURN_CARRIAGE)
            {
                /* Generate a password of 8 characters in length */
                generate_password();
            }
        }
    }
}

/*******************************************************************************
* Function Name: generate_password
********************************************************************************
* Summary: This function generates a 8 character long password
*
* Parameters:
*  None
*
* Return
*  void
*
*******************************************************************************/
void generate_password()
{
    int8_t index;
    uint32_t random_val;
    
    /* Array to hold the generated password. Array size is inclusive of
       string NULL terminating character */
    uint8_t password[PASSWORD_LENGTH + 1];

    cy_rslt_t result;

    cyhal_trng_t trng_obj;

    /* Initialize the TRNG generator block*/
    result = cyhal_trng_init(&trng_obj);

    if (result == CY_RSLT_SUCCESS)
    {
        for (index = 0; index < PASSWORD_LENGTH; index++)
        {
            /* Generate a random number and truncate to a 7 bit number */
            random_val = cyhal_trng_generate(&trng_obj) & MAX_TRNG_VAL;

            /* Check if the generated random number is in the range of alpha-numeric,
            special characters ASCII codes. If not, convert to that range */
            if (random_val < ASCII_ALPHANUMERIC_LOWER_LIMIT)
            {
                random_val += ASCII_ALPHANUMERIC_LOWER_LIMIT;
            }
            else if (random_val >= ASCII_ALPHANUMERIC_UPPER_LIMIT)
            {
                random_val -= ASCII_ALPHANUMERIC_LOWER_LIMIT;
            }
            password[index] = random_val;
        }
        /* Terminate the password with end of string character */
        password[index] = '\0';

        /* Display the generated password on the UART Terminal */
        printf("One-Time Password: %s\r\n\n",password);
        printf("Press the Enter key to generate new password\r\n");
        printf(SCREEN_HEADER1);

        /* Free the TRNG generator block */
        cyhal_trng_free(&trng_obj);
    }
}

/* [] END OF FILE */