
/* Includes ------------------------------------------------------------------*/
#include <string.h>

#include "stm32f7xx_ll_dma.h"
#include "stm32f7xx_ll_rcc.h"
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_system.h"
#include "stm32f7xx_ll_exti.h"
#include "stm32f7xx_ll_cortex.h"
#include "stm32f7xx_ll_utils.h"
#include "stm32f7xx_ll_pwr.h"
#include "stm32f7xx_ll_usart.h"
#include "stm32f7xx_ll_gpio.h"

#include "stm32f7xx_hal_cortex.h"

#include "os.h"

#define LD1_GPIO_PIN 		LL_GPIO_PIN_0
#define LD1_GPIO_PORT 		GPIOB
#define LD2_GPIO_PIN 		LL_GPIO_PIN_7
#define LD2_GPIO_PORT 		GPIOB
#define LD3_GPIO_PIN 		LL_GPIO_PIN_14
#define LD3_GPIO_PORT 		GPIOB

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Board_Led_Init(void);
extern uint32_t SystemCoreClock;
extern void UCOS_III_Init(void);

/**
 * @brief  CPU L1-Cache enable.
 * @param  None
 * @retval None
 */
static void CPU_CACHE_Enable(void)
{
	/* Enable I-Cache */
	SCB_EnableICache();

	/* Enable D-Cache */
	SCB_EnableDCache();
}

void Test1_Task_Func(void *p_arg)
{
    (void)p_arg;
    OS_ERR os_err;

    while(1)
    {
		LL_GPIO_TogglePin(LD1_GPIO_PORT,LD1_GPIO_PIN);
        OSTimeDly(100,OS_OPT_TIME_DLY,&os_err);
    }
}

void Test2_Task_Func(void *p_arg)
{
    (void)p_arg;
    OS_ERR os_err;

    while(1)
    {

		LL_GPIO_TogglePin(LD2_GPIO_PORT,LD2_GPIO_PIN);
        OSTimeDly(100,OS_OPT_TIME_DLY,&os_err);
    }
}

void Test3_Task_Func(void *p_arg)
{
    (void)p_arg;
    OS_ERR os_err;

    while(1)
    {

		LL_GPIO_TogglePin(LD3_GPIO_PORT,LD3_GPIO_PIN);
        OSTimeDly(100,OS_OPT_TIME_DLY,&os_err);
    }
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	CPU_CACHE_Enable();

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */

	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);

	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	/* System interrupt init*/

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	Board_Led_Init();
	LL_GPIO_SetOutputPin(LD1_GPIO_PORT,LD1_GPIO_PIN);

	UCOS_III_Init();
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
static void SystemClock_Config(void)
{
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_7);
	while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_7){}

	LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
	LL_PWR_EnableOverDriveMode();
	LL_RCC_HSI_SetCalibTrimming(16);
	LL_RCC_HSI_Enable();
	while (LL_RCC_HSI_IsReady() != 1){} /* Wait till HSI is ready */

	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_8, 216, LL_RCC_PLLP_DIV_2);
	LL_RCC_PLL_Enable();
	while (LL_RCC_PLL_IsReady() != 1){} /* Wait till PLL is ready */

	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
	/* Wait till System clock is ready */
	while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL){}

	LL_SetSystemCoreClock(216000000);
	LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_SYSCLK);
}

static void Board_Led_Init(void)
{
	LL_GPIO_InitTypeDef gpioConfig;
	memset(&gpioConfig, 0, sizeof(gpioConfig));

	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	gpioConfig.Mode = LL_GPIO_MODE_OUTPUT;
	gpioConfig.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	gpioConfig.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	gpioConfig.Pull = LL_GPIO_PULL_UP;

	gpioConfig.Pin = LD1_GPIO_PIN;
	LL_GPIO_Init(LD1_GPIO_PORT, &gpioConfig);

	gpioConfig.Pin = LD2_GPIO_PIN;
	LL_GPIO_Init(LD2_GPIO_PORT, &gpioConfig);

	gpioConfig.Pin = LD3_GPIO_PIN;
	LL_GPIO_Init(LD3_GPIO_PORT, &gpioConfig);
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
static void Error_Handler(void)
{
	__disable_irq();
	while (1)
	{

	}
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	Error_Handler();
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
