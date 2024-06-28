/*
 *@Note
 *SDI_Printf routine:
 *It needs to be used with WCH-LinkUtility 1.8.
 *Note:version of WCH-LinkUtility must after 1.8
 *
 */

#include "debug.h"

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
	u8 i = 0;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	Delay_Init();
#if (SDI_PRINT == SDI_PR_OPEN)
	SDI_Printf_Enable();
#else
	USART_Printf_Init(115200);
#endif

	printf("SystemClk:%ld\r\n", SystemCoreClock);

	while (1)
	{
		Delay_Ms(1000);

		printf("-DEBUG-PR-%08x\r\n", i++);
	}
}
