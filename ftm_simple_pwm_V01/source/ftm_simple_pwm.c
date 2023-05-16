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

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* The Flextimer base address/channel used for board */
#define BOARD_FTM_BASEADDR FTM0
#define BOARD_FTM_CHANNEL  kFTM_Chnl_3

/* Interrupt number and interrupt handler for the FTM base address used */
#define FTM_INTERRUPT_NUMBER  FTM0_IRQn
#define FTM_INPUT_CAPTURE_HANDLER      FTM0_IRQHandler

/* FTM channel used for input capture */
#define BOARD_FTM_INPUT_CAPTURE_CHANNEL kFTM_Chnl_1

/* Interrupt to enable and flag to read; depends on the FTM channel pair used */
#define FTM_CHANNEL_INTERRUPT_ENABLE kFTM_Chnl1InterruptEnable
#define FTM_CHANNEL_FLAG             kFTM_Chnl1Flag

/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_CoreSysClk)   //获取频率

#ifndef FTM_PWM_ON_LEVEL
#define FTM_PWM_ON_LEVEL kFTM_HighTrue
#endif

#ifndef DEMO_PWM_FREQUENCY
#define DEMO_PWM_FREQUENCY (4000U)        //4KHz
#endif

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

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool ftmIsrFlag          = false;/*  */
volatile bool brightnessUp        = true; /* Indicate LED is brighter or dimmer */
volatile uint8_t updatedDutycycle = 10U;

/* Record FTM TOF interrupt times */
volatile uint32_t g_timerOverflowInterruptCount = 0u;
volatile uint32_t g_firstChannelOverflowCount   = 0u;
volatile uint32_t g_secondChannelOverflowCount  = 0u;


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

/*********************************************************
***函数名：FTM_LED_HANDLER
***函数功能：定时器中断
***参数：
***返回值：
**********************************************************/
#if 0
void FTM_LED_HANDLER(void)
{
    ftmIsrFlag = true;

    if (brightnessUp)
    {
        /* Increase duty cycle until it reach limited value, don't want to go upto 100% duty cycle
         * as channel interrupt will not be set for 100%
         */
        if (++updatedDutycycle >= 99U)
        {
            updatedDutycycle = 99U;
            brightnessUp     = false;
        }
    }
    else
    {
        /* Decrease duty cycle until it reach limited value */
        if (--updatedDutycycle == 1U)
        {
            brightnessUp = true;
        }
    }

    /* Clear interrupt flag.*/
    FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, FTM_GetStatusFlags(BOARD_FTM_BASEADDR));

    __DSB();
}
#endif

/*********************************************************
***函数名：FTM_INPUT_CAPTURE_HANDLER
***函数功能：定时器0中断
***参数：
***返回值：
**********************************************************/
#if 0
void FTM_INPUT_CAPTURE_HANDLER(void)
{
	  /*定时器状态寄存器*/
    if ((FTM_GetStatusFlags(DEMO_FTM_BASEADDR) & kFTM_TimeOverflowFlag) == kFTM_TimeOverflowFlag)
    {
        /* Clear overflow interrupt flag.*/
        FTM_ClearStatusFlags(DEMO_FTM_BASEADDR, kFTM_TimeOverflowFlag);
        g_timerOverflowInterruptCount++;
    }
    else if (((FTM_GetStatusFlags(DEMO_FTM_BASEADDR) & FTM_FIRST_CHANNEL_FLAG) == FTM_FIRST_CHANNEL_FLAG) &&
             (ftmFirstChannelInterruptFlag == false))
    {
        /* Disable first channel interrupt.*/
        FTM_DisableInterrupts(DEMO_FTM_BASEADDR, FTM_FIRST_CHANNEL_INTERRUPT_ENABLE);
        g_firstChannelOverflowCount  = g_timerOverflowInterruptCount;
        ftmFirstChannelInterruptFlag = true;
    }
    else if ((FTM_GetStatusFlags(DEMO_FTM_BASEADDR) & FTM_SECOND_CHANNEL_FLAG) == FTM_SECOND_CHANNEL_FLAG)
    {
        /* Clear second channel interrupt flag.*/
        FTM_ClearStatusFlags(DEMO_FTM_BASEADDR, FTM_SECOND_CHANNEL_FLAG);
        /* Disable second channel interrupt.*/
        FTM_DisableInterrupts(DEMO_FTM_BASEADDR, FTM_SECOND_CHANNEL_INTERRUPT_ENABLE);
        g_secondChannelOverflowCount  = g_timerOverflowInterruptCount;
        ftmSecondChannelInterruptFlag = true;
    }
    else
    {
    }
    __DSB();
}
#endif


/*********************************************************
***函数名：FTM_INPUT_CAPTURE_HANDLER
***函数功能：输入捕获中断
***参数：
***返回值：
**********************************************************/
#if 1
void FTM_INPUT_CAPTURE_HANDLER(void)
{
	
		ftmIsrFlag = true;
    if ((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & FTM_CHANNEL_FLAG) == FTM_CHANNEL_FLAG)  //获取FTM状态标志。
    {
			/* Clear interrupt flag.*/
			FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, FTM_CHANNEL_FLAG);
    }
    //ftmIsrFlag = true;
    __DSB();
}
#endif


/*!
 * @brief Main function
 */
int main(void)
{  
	  uint32_t captureVal;
    ftm_config_t ftmInfo;
		ftm_dual_edge_capture_param_t edgeParam; 
    ftm_chnl_pwm_signal_param_t ftmParam;
	
	  uint32_t capture1Val;
    uint32_t capture2Val;
    ftm_pwm_level_select_t pwmLevel = FTM_PWM_ON_LEVEL;
	  float pulseWidth;    //脉冲宽度

    /* Board pin, clock, debug console init */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Fill in the FTM config struct with the default settings */
    FTM_GetDefaultConfig(&ftmInfo);
	
    /* Calculate the clock division based on the PWM frequency to be obtained */
	  /* 根据要获得的PWM频率计算时钟分频 */
    ftmInfo.prescale = FTM_CalculateCounterClkDiv(BOARD_FTM_BASEADDR, DEMO_PWM_FREQUENCY, FTM_SOURCE_CLOCK);
		
				/* Initialize FTM module */
    FTM_Init(BOARD_FTM_BASEADDR, &ftmInfo);
		
		/* Enable at the NVIC */
    EnableIRQ(FTM_INTERRUPT_NUMBER); 

    /* Configure ftm params with frequency 40kHZ */
    ftmParam.chnlNumber            = BOARD_FTM_CHANNEL;
    ftmParam.level                 = pwmLevel;
    ftmParam.dutyCyclePercent      = updatedDutycycle;      //设置占空比
    ftmParam.firstEdgeDelayPercent = 0U;
    ftmParam.enableComplementary   = false;
    ftmParam.enableDeadtime        = false;
    
		if (kStatus_Success !=
        FTM_SetupPwm(BOARD_FTM_BASEADDR, &ftmParam, 1U, kFTM_CenterAlignedPwm, DEMO_PWM_FREQUENCY, FTM_SOURCE_CLOCK))  //配置PWM信号参数
    {
        PRINTF("\r\nSetup PWM fail, please check the configuration parameters!\r\n");
        return -1;
    }		
			
		/* 开启计数 */
    FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);
		
    /* Setup dual-edge capture on a FTM channel pair */
    FTM_SetupInputCapture(BOARD_FTM_BASEADDR, BOARD_FTM_INPUT_CAPTURE_CHANNEL, kFTM_RisingEdge, 0);  //上升沿捕获

    /* Set the timer to be in free-running mode */
    FTM_SetTimerPeriod(BOARD_FTM_BASEADDR, 0xFFFF);

    /* Enable channel interrupt when the second edge is detected */
    FTM_EnableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);
			 

    while (ftmIsrFlag != true)     //当ftmIsrFlag = true时，进入while循环
    {
			
    }
		
		
    captureVal = FTM_GetInputCaptureValue(BOARD_FTM_BASEADDR, BOARD_FTM_INPUT_CAPTURE_CHANNEL);
		
    PRINTF("\r\nCapture value C(n)V=%x\r\n", captureVal);


    while (1)
    {
			
//			if(true == ftmIsrFlag)
//			{
//				
//				ftmIsrFlag = false;
//				
//				captureVal = FTM_GetInputCaptureValue(BOARD_FTM_BASEADDR, BOARD_FTM_INPUT_CAPTURE_CHANNEL);
//		
//				PRINTF("\r\nCapture value C(n)V=%x\r\n", captureVal);
//			}
			
			#if 0
        /* Use interrupt to update the PWM dutycycle */
        if (true == ftmIsrFlag)
        {
            /* Disable interrupt to retain current dutycycle for a few seconds */
            FTM_DisableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);

            ftmIsrFlag = false;

            /* Disable channel output before updating the dutycycle */
            FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, BOARD_FTM_CHANNEL, 0U);

            /* Update PWM duty cycle */
            if (kStatus_Success !=
                FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, BOARD_FTM_CHANNEL, kFTM_CenterAlignedPwm, updatedDutycycle))
            {
                PRINTF("Update duty cycle fail, the target duty cycle may out of range!\r\n");
            }

            /* Software trigger to update registers */
            FTM_SetSoftwareTrigger(BOARD_FTM_BASEADDR, true);

            /* Start channel output with updated dutycycle */
            FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, BOARD_FTM_CHANNEL, pwmLevel);

            /* Delay to view the updated PWM dutycycle */
            delay();

            /* Enable interrupt flag to update PWM dutycycle */
            FTM_EnableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);
        }
			#endif
				
    }
}
