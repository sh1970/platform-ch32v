#if defined(CH32V00X)
#include <ch32v00x.h>
#elif defined(CH32V10X)
#include <ch32v10x.h>
#elif defined(CH32V20X)
#include <ch32v20x.h>
#elif defined(CH32V30X) || defined(CH32V31X)
#include <ch32v30x.h>
#elif defined(CH32L10X)
#include <ch32l103.h>
#endif
#include <debug.h>

/* PC1 */
#define BLINKY_GPIO_PORT    GPIOC
#define BLINKY_GPIO_PIN     GPIO_Pin_1
#if defined(CH32L10X)
#define BLINKY_CLOCK        RCC_PB2Periph_GPIOC
#else
#define BLINKY_CLOCK        RCC_APB2Periph_GPIOC
#endif

extern "C" void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
extern "C" void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

class LEDBlinker {
private:
    GPIO_TypeDef* gpio_port;
    uint32_t gpio_pin;
    uint32_t rcc_clock;
public:
    LEDBlinker(GPIO_TypeDef* gpio_port, uint32_t gpio_pin, uint32_t rcc_clock) {
        this->gpio_port = gpio_port;
        this->gpio_pin = gpio_pin;
        this->rcc_clock = rcc_clock;
    }

    void begin() {
#if defined(CH32L10X)
        RCC_PB2PeriphClockCmd(this->rcc_clock, ENABLE);
#else
        RCC_APB2PeriphClockCmd(this->rcc_clock, ENABLE);
#endif
        GPIO_InitTypeDef GPIO_InitStructure = {0};
        GPIO_InitStructure.GPIO_Pin = this->gpio_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(this->gpio_port, &GPIO_InitStructure);
    }

    void blink(int time_ms) {
        GPIO_WriteBit(this->gpio_port, this->gpio_pin, BitAction::Bit_SET);
        Delay_Ms(time_ms);
        GPIO_WriteBit(this->gpio_port, this->gpio_pin, BitAction::Bit_RESET);
        Delay_Ms(time_ms);
    }
};

LEDBlinker blinker(BLINKY_GPIO_PORT, BLINKY_GPIO_PIN, BLINKY_CLOCK);

int main(void)
{
    #ifdef NVIC_PriorityGroup_2
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
#else
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
#endif
    SystemCoreClockUpdate();
    Delay_Init();
    
    blinker.begin();
    while (1)
    {
        blinker.blink(1000);
    }
}

void NMI_Handler(void) {}
void HardFault_Handler(void)
{
    while (1)
    {
    }
}
