/* ###################################################################
 **     Filename    : main.c
 **     Project     : test1
 **     Processor   : MK22FN512VLH12
 **     Version     : Driver 01.01
 **     Compiler    : GNU C Compiler
 **     Date/Time   : 2018-03-25, 00:20, # CodeGen: 0
 **     Abstract    :
 **         Main module.
 **         This module contains user's application code.
 **     Settings    :
 **     Contents    :
 **         No public methods
 **
 ** ###################################################################*/
/*!
 ** @file main.c
 ** @version 01.01
 ** @brief
 **         Main module.
 **         This module contains user's application code.
 */
/*!
 **  @addtogroup main_module main module documentation
 **  @{
 */
/* MODULE main */


/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "pin_mux.h"
#include "RED.h"
#include "BitIoLdd1.h"
#include "TU1.h"
#include "Green.h"
#include "BitIoLdd2.h"
#include "BLUE.h"
#include "BitIoLdd3.h"
/* Including shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "PDD_Includes.h"
#include "Init_Config.h"
#include "task_schdlr.h"

/* User includes (#include below this line is not maintained by Processor Expert) */
uint32_t comp = 0;
void Task_led_blink()
{
  /* Write your code here ... */
  BLUE_SetVal();
  Green_SetVal();
  if(comp == 0)
    {
      RED_SetVal();
      comp = 1;
    }
  else
    {
      RED_ClrVal();
      comp = 0;
    }

}

volatile uint32_t count = 0, count1 = 0;

volatile uint32_t *DWT_CONTROL1 = (uint32_t *)0xE0001000;
volatile uint32_t *DWT_CYCCNT1 = (uint32_t *)0xE0001004;
volatile uint32_t *DEMCR1 = (uint32_t *)0xE000EDFC;

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */
  Scheduler_Init();
  /* For example: for(;;) { } */

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  BLUE_SetVal();
  Green_SetVal();

  /* For example: for(;;) { } */
  while(1)
    {
      count = 0;
      *DWT_CYCCNT1 = 0;
      // enable cycle counter
      *DWT_CONTROL1 = *DWT_CONTROL1 | 1 ;
      RED_SetVal();
      count1 = 0;
      count1 = count1 +2;
      count1 = count1 +2;
      count1 = count1 +2;
      count1 = count1 +2;
      RED_ClrVal();
      //read the counts
      count = *DWT_CYCCNT1;
      //Actual execution time = count * Time for One System Clock

      count = 0;
      *DWT_CYCCNT1 = 0;
      // enable cycle counter
      *DWT_CONTROL1 = *DWT_CONTROL1 | 1 ;
      count1 = 0;
      count1 = count1 +2;
      count1 = count1 +2;
      count1 = count1 +2;
      count1 = count1 +2;
      RED_SetVal();
      //read the counts
      count = *DWT_CYCCNT1;
      //Actual execution time = count * Time for One System Clock
    }

  while(1);

  FCS_TASK_SCHDLR_addTask((FCS_TaskCode_t) &Task_led_blink, NULL, 1000, FCS_TASKPRIORITY_Normal);

  // Run the dispatcher.
  FCS_TASK_SCHDLR_dispatcher();


  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
 ** @}
 */
/*
 ** ###################################################################
 **
 **     This file was created by Processor Expert 10.5 [05.21]
 **     for the Freescale Kinetis series of microcontrollers.
 **
 ** ###################################################################
 */
