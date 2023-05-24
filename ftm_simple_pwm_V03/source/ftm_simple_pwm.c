/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_ftm.h"

#include "fsl_lptmr.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* The Flextimer base address/channel used for board */
#define BOARD_FTM_BASEADDR FTM0
//#define BOARD_FTM_CHANNEL  kFTM_Chnl_3

#define Board_FTM_2   FTM2
#define BOARD_FTM_CHANNEL  kFTM_Chnl_1

//#define BOARD_FTM_IRQ_NUM FTM2_IRQn
//#define BOARD_FTM_HANDLER FTM2_IRQHandler

/* Interrupt number and interrupt handler for the FTM base address used */
#define FTM_INTERRUPT_NUMBER  FTM0_IRQn
#define FTM_INPUT_CAPTURE_HANDLER      FTM0_IRQHandler

/* FTM channel used for input capture */
#define BOARD_FTM_INPUT_CAPTURE_CHANNEL kFTM_Chnl_1

/* Interrupt to enable and flag to read; depends on the FTM channel pair used */
#define FTM_CHANNEL_INTERRUPT_ENABLE kFTM_Chnl1InterruptEnable
#define FTM_CHANNEL_FLAG             kFTM_Chnl1Flag


/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_CoreSysClk)

//#define FTM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_ScgFircAsyncDiv2Clk)
//#define FTM2_SOURCE_CLOCK  (CLOCK_GetFreq(kCLOCK_CoreSysClk))

#ifndef FTM_PWM_ON_LEVEL
#define FTM_PWM_ON_LEVEL kFTM_HighTrue
#endif

#ifndef DEMO_PWM_FREQUENCY
#define DEMO_PWM_FREQUENCY (4000U)        //4KHz
#endif

//#ifndef DEMO_TIMER_PERIOD_US
///* Set counter period to 1ms */
//#define DEMO_TIMER_PERIOD_US (1000U)
//#endif

#define DEMO_LPTMR_BASE   LPTMR0
#define DEMO_LPTMR_IRQn   PWT_LPTMR0_IRQn
#define LPTMR_HANDLER PWT_LPTMR0_IRQHandler
/* Get source clock for LPTMR driver */
#define LPTMR_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_LpoClk)
/* Define LPTMR microseconds counts value */
#define LPTMR_USEC_COUNT 1000U


/////* Interrupt to enable and flag to read; depends on the FTM channel used for dual-edge capture */
////#define FTM_FIRST_CHANNEL_INTERRUPT_ENABLE  kFTM_Chnl0InterruptEnable
////#define FTM_FIRST_CHANNEL_FLAG              kFTM_Chnl0Flag
////#define FTM_SECOND_CHANNEL_INTERRUPT_ENABLE kFTM_Chnl1InterruptEnable
////#define FTM_SECOND_CHANNEL_FLAG             kFTM_Chnl1Flag

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief delay a while.
 */
void delay(void);
void PWM_Init(void);
void Input_capture_init(void);
void TIM2_Init(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool ftmIsrFlag          = false;/*  */
volatile bool brightnessUp        = true; /* Indicate LED is brighter or dimmer */
volatile uint8_t updatedDutycycle = 20U;

/* Record FTM TOF interrupt times */
volatile uint32_t g_timerOverflowInterruptCount = 0u;
volatile uint32_t g_firstChannelOverflowCount   = 0u;
volatile uint32_t g_secondChannelOverflowCount  = 0u;
volatile uint32_t milisecondCounts = 0U;

uint32_t captureVal;


uint32_t capture1Val;
uint32_t capture2Val;
ftm_pwm_level_select_t pwmLevel = FTM_PWM_ON_LEVEL;
float pulseWidth;     //脉冲宽度
float Frequency;      //频率

uint16_t capturecount;

uint8_t displayFlag;
uint8_t step1 = 0;
uint16_t motor100ms = 0;
uint8_t firstupflag = 1;

unsigned int motorcounter = 0;
unsigned int intcounter = 0;
unsigned int testtimer = 0;

uint8_t err;
uint8_t current_rpm;
uint8_t target_rpm = 10;  // 

#define BOARD_SW_GPIO GPIOD
#define BOARD_SW_GPIO_PIN 16U

/*******************************************************************************
 * Code
 ******************************************************************************/
void delay(void)
{
    volatile uint32_t i = 0U;
    for (i = 0U; i < 80000U; ++i)
    {
        __asm("NOP"); /* delay */
    }
}


volatile uint32_t lptmrCounter = 0U;

/*******************************************************************************
 * Code
 ******************************************************************************/



/*********************************************************
***函数名：PWM_Init
***函数功能：
***参数：
***返回值：
**********************************************************/
void PWM_Init(void)
{
		ftm_config_t ftmInfo;
    ftm_chnl_pwm_signal_param_t ftmParam;
    ftm_pwm_level_select_t pwmLevel = FTM_PWM_ON_LEVEL;
	  
	/* Fill in the FTM config struct with the default settings */
    FTM_GetDefaultConfig(&ftmInfo);
	
    /* Calculate the clock division based on the PWM frequency to be obtained */
	  /* 根据要获得的PWM频率计算时钟分频 */
    ftmInfo.prescale = FTM_CalculateCounterClkDiv(Board_FTM_2, DEMO_PWM_FREQUENCY, FTM_SOURCE_CLOCK);
		
	  /* Initialize FTM module */
    FTM_Init(Board_FTM_2, &ftmInfo);
	 
    /* Configure ftm params with frequency 4kHZ */
    ftmParam.chnlNumber            = BOARD_FTM_CHANNEL;
    ftmParam.level                 = pwmLevel;
    ftmParam.dutyCyclePercent      = updatedDutycycle;      //设置占空比
    ftmParam.firstEdgeDelayPercent = 0U;
    ftmParam.enableComplementary   = false;
    ftmParam.enableDeadtime        = false;
		
		/**/
	  FTM_SetupPwm(Board_FTM_2, &ftmParam, 1U, kFTM_CenterAlignedPwm, DEMO_PWM_FREQUENCY, FTM_SOURCE_CLOCK);  //配置PWM信号参数
		
		FTM_StartTimer(Board_FTM_2, kFTM_SystemClock);
	
}

/*********************************************************
***函数名：Input_capture_init
***函数功能：
***参数：
***返回值：
**********************************************************/
void Input_capture_init(void)
{
	
	  ftm_config_t ftmInfo1;
	
		FTM_GetDefaultConfig(&ftmInfo1);
	
    /* Initialize FTM module */
    FTM_Init(BOARD_FTM_BASEADDR, &ftmInfo1);

    /* Setup dual-edge capture on a FTM channel pair */
    FTM_SetupInputCapture(BOARD_FTM_BASEADDR, BOARD_FTM_INPUT_CAPTURE_CHANNEL, kFTM_RisingEdge, 0);

    /* Set the timer to be in free-running mode */
    FTM_SetTimerPeriod(BOARD_FTM_BASEADDR, 0xffff);  //0x15f9);

    /* Enable channel interrupt when the second edge is detected */
    FTM_EnableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);

    /* Enable at the NVIC */
    EnableIRQ(FTM_INTERRUPT_NUMBER);

    FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);
	
}

/*********************************************************
***函数名：
***函数功能：
***参数：
***返回值：
**********************************************************/
#if 0
void TIM2_Init(void)
{
	  ftm_config_t ftmInfo2;
	
	/* Fill in the FTM config struct with the default settings */
    FTM_GetDefaultConfig(&ftmInfo2);
    /* Calculate the clock division based on the timer period frequency to be obtained */
	  /* 根据要获得的定时器周期频率计算时钟分频 */
    ftmInfo2.prescale =
        FTM_CalculateCounterClkDiv(Board_FTM_2, 1000000U / DEMO_TIMER_PERIOD_US, FTM2_SOURCE_CLOCK);
    ;

    /* Initialize FTM module */
    FTM_Init(Board_FTM_2, &ftmInfo2);

    /* Set timer period */
    FTM_SetTimerPeriod(Board_FTM_2,USEC_TO_COUNT(DEMO_TIMER_PERIOD_US, FTM2_SOURCE_CLOCK / (1U << ftmInfo2.prescale)));

    FTM_EnableInterrupts(Board_FTM_2, kFTM_TimeOverflowInterruptEnable);

    EnableIRQ(BOARD_FTM_IRQ_NUM);

    FTM_StartTimer(Board_FTM_2, kFTM_SystemClock);
}
#endif

void Lptmr_Init(void)
{
	  lptmr_config_t lptmrConfig;
	
	  LPTMR_GetDefaultConfig(&lptmrConfig);

    /* Initialize the LPTMR */
    LPTMR_Init(DEMO_LPTMR_BASE, &lptmrConfig);

    /*
     * Set timer period.
     * Note : the parameter "ticks" of LPTMR_SetTimerPeriod should be equal or greater than 1.
     */
    LPTMR_SetTimerPeriod(DEMO_LPTMR_BASE, USEC_TO_COUNT(LPTMR_USEC_COUNT, LPTMR_SOURCE_CLOCK));

    /* Enable timer interrupt */
    LPTMR_EnableInterrupts(DEMO_LPTMR_BASE, kLPTMR_TimerInterruptEnable);

    /* Enable at the NVIC */
    EnableIRQ(DEMO_LPTMR_IRQn);

    /* Start counting */
    LPTMR_StartTimer(DEMO_LPTMR_BASE);
}

/*********************************************************
***函数名：
***函数功能： 1000us中断1次
***参数：
***返回值：
**********************************************************/
#if 0
void BOARD_FTM_HANDLER(void)
{
    /* Clear interrupt flag.*/
    FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, kFTM_TimeOverflowFlag);
    ftmIsrFlag = true;
	
	  motorcounter = intcounter ;
	
	  intcounter = 0;
	
//		testtimer ++;
//	  if(testtimer >= 1000)
//		{
//			testtimer = 0;
//			PRINTF("1s\r\n");
//		}
    __DSB();
}
#endif

/*********************************************************
***函数名：LPTMR_HANDLER
***函数功能：
***参数：
***返回值：
**********************************************************/
void LPTMR_HANDLER(void)
{
	uint8_t err;
    LPTMR_ClearStatusFlags(DEMO_LPTMR_BASE, kLPTMR_TimerCompareFlag);
	
	  testtimer ++;
	
	  if(testtimer >= 100)
		{
			testtimer = 0;
			motorcounter = intcounter ;
			intcounter = 0;
			
//			motor100ms = motorcounter;
			
		 PRINTF(" 转速 = %d\r\n",motorcounter);
			
		 current_rpm = motorcounter;
		
			
//    //速度微调
//    err=current_rpm-target_rpm;
//    if(err>5)
//    {
//        if(updatedDutycycle>=1)updatedDutycycle-=1;
//        else updatedDutycycle=0;
//    }
//    else if(err<5)
//    {
//        updatedDutycycle+=1;
//        if(updatedDutycycle>=100)updatedDutycycle=100;
//    }
			

		}
    /*
     * Workaround for TWR-KV58: because write buffer is enabled, adding
     * memory barrier instructions to make sure clearing interrupt flag completed
     * before go out ISR
     */
    __DSB();
    __ISB();
}


/*********************************************************
***函数名：FTM_INPUT_CAPTURE_HANDLER
***函数功能：输入捕获中断
***参数：
***返回值：
**********************************************************/
#if 1
void FTM_INPUT_CAPTURE_HANDLER(void)
{
//	ftmIsrFlag = true;
//	if (((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & FTM_CHANNEL_FLAG) == FTM_CHANNEL_FLAG))
//		{

//			FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, FTM_CHANNEL_FLAG);
//			if(firstupflag)
//			{
//				firstupflag = 0;
//				step1 = 0;
//			}
//			else
//			{
//				step1 = 1;
//			}			
//		}	
    
	  ftmIsrFlag = true;
    if ((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & kFTM_TimeOverflowFlag) == kFTM_TimeOverflowFlag)
		{
				/* Clear overflow interrupt flag.*/
				FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, kFTM_TimeOverflowFlag);
				g_timerOverflowInterruptCount++;  //中断次数
		}
		else if(((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & FTM_CHANNEL_FLAG) == FTM_CHANNEL_FLAG))
		{
			FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, FTM_CHANNEL_FLAG);
				if(firstupflag)
				{
					firstupflag = 0;
					step1 = 0;
				}
				else
				{
					step1 = 1;
				}			
		}
		else
		{
			
		}
		
//    else if (((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & FTM_CHANNEL_FLAG) == FTM_CHANNEL_FLAG) &&
//             (ftmFirstChannelInterruptFlag == false))
//    {
//        /* Disable first channel interrupt.*/
//        FTM_DisableInterrupts(DEMO_FTM_BASEADDR, FTM_FIRST_CHANNEL_INTERRUPT_ENABLE);
//        g_firstChannelOverflowCount  = g_timerOverflowInterruptCount;
//        ftmFirstChannelInterruptFlag = true;
//    }
//    else if ((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & FTM_CHANNEL_FLAG) == FTM_SECOND_CHANNEL_FLAG)
//    {
//        /* Clear second channel interrupt flag.*/
//        FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, FTM_SECOND_CHANNEL_FLAG);
//        /* Disable second channel interrupt.*/
//        FTM_DisableInterrupts(BOARD_FTM_BASEADDR, FTM_SECOND_CHANNEL_INTERRUPT_ENABLE);
//        g_secondChannelOverflowCount  = g_timerOverflowInterruptCount;
//        ftmSecondChannelInterruptFlag = true;
//    }
//    else
//    {
//    }		
	
    __DSB();
}
#endif


void GPIO_INPUT_CONFIG(void)
{
	
	
	gpio_pin_config_t PD16_config = {
        kGPIO_DigitalInput,
        0,
    };
	
	GPIO_PinInit(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN, &PD16_config);	
}




/*!
 * @brief Main function
 */
int main(void)
{  
    float time;
	 
	
    /* Board pin, clock, debug console init */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
	
	  PRINTF("\r\n Test beginning... \r\n");
	
	//	  TIM2_Init();
	
	  Lptmr_Init();
	
	  PWM_Init();

		Input_capture_init();
	  
	
    while (1)
    {

//			if(true == ftmIsrFlag)
//			{
//				ftmIsrFlag = false;
//			  
//			}
			
			#if 1
			if(true == ftmIsrFlag)
			{
				
				ftmIsrFlag = false;	
				
				switch(step1)
				{
					case 0:
					{
						capture1Val = FTM_GetInputCaptureValue(BOARD_FTM_BASEADDR, BOARD_FTM_INPUT_CAPTURE_CHANNEL);
						capture1Val = g_timerOverflowInterruptCount * 65539 + capture1Val;
            						
					}
					case 1:
					{
						capture2Val = FTM_GetInputCaptureValue(BOARD_FTM_BASEADDR, BOARD_FTM_INPUT_CAPTURE_CHANNEL);
						capture2Val = g_timerOverflowInterruptCount * 65539 + capture2Val;
						captureVal = capture2Val - capture1Val;
						capture1Val = capture2Val;
						
//						PRINTF("capture = %d \r\n", captureVal);
						
						if(captureVal > 8000)
						{
							intcounter++;
						}
						
					}
				}
				
				
//						capturecount++;	
////				if(capturecount < 2)
////				{
////					g_firstChannelOverflowCount  = g_timerOverflowInterruptCount;
////					
//////					capture1Val = FTM_GetInputCaptureValue(BOARD_FTM_BASEADDR, (ftm_chnl_t)(BOARD_FTM_INPUT_CAPTURE_CHANNEL * 2));
////					
////					capture1Val = FTM_GetInputCaptureValue(BOARD_FTM_BASEADDR, BOARD_FTM_INPUT_CAPTURE_CHANNEL);
////					
////				}
////				else if (capturecount >= 2)	
////				{
////					g_secondChannelOverflowCount  = g_timerOverflowInterruptCount;
////					
////					capture2Val = FTM_GetInputCaptureValue(BOARD_FTM_BASEADDR, BOARD_FTM_INPUT_CAPTURE_CHANNEL);
////					
//////			    capture2Val = FTM_GetInputCaptureValue(BOARD_FTM_BASEADDR, (ftm_chnl_t)(BOARD_FTM_INPUT_CAPTURE_CHANNEL * 2 + 1));
////					
////					captureVal = capture2Val - capture1Val;
////					
////					intcounter++;
////					
//////					if(captureVal >= 1)
//////					{
//////						intcounter++;
//////					}
////					
//////					PRINTF("capture = %d \r\n", captureVal);
////					
////					capturecount = 0;
////					
////					displayFlag = 1;
////				}
				
				
				
			}
			#endif
						
    }
}

