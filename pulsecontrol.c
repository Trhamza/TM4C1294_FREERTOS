#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"

#include "FreeRTOS.h"

uint32_t ui32ADCValue = 0U;

void vTask1(void *pvParameters);
void vTask2(void *pvParameters);
const char *pvTask1  = "Task_ADC is running.";
const char *pvTask2  = "Task_PWM is running.";

int main(void)
{
	/* Configure CPU CLOCK */ 
    uint32_t ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240), 120000000);
	
	/*  Enable PWM Output */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
	
	/* Configure K7 Pins */
    GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_7);
	
	/* PWM output */
    GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_7, 0x00U);
	
	/* Clock Enable */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	
	/* Read Pot Pin */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	
	/* Cofigure ADC */
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_7);
    ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PIOSC | ADC_CLOCK_RATE_HALF, 2);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH4 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 3);
    ADCIntClear(ADC0_BASE, 3);
	
	/* Create Task */
    xTaskCreate(vTask1, "Task_ADC", configMINIMAL_STACK_SIZE, (void*)pvTask1, 1, NULL);
    xTaskCreate(vTask2, "Task_PWM", configMINIMAL_STACK_SIZE, (void*)pvTask2, 1, NULL);
    vTaskStartScheduler();

    while(1);
}

void vTask1(void *pvParameters)
{
    ADCProcessorTrigger(ADC0_BASE, 3);
    while(!ADCIntStatus(ADC0_BASE, 3, false));
    ADCIntClear(ADC0_BASE, 3);
    ADCSequenceDataGet(ADC0_BASE, 3, &ui32ADCValue);
}

void vTask2(void *pvParameters)
{
    HWREG(GPIO_PORTK_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = GPIO_PIN_7;
    HWREG(GPIO_PORTK_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = 0x00;
    
    for(int i = 0U; i < (1000 * ui32ADCValue); i++);
}
