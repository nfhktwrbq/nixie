#include "modules/gpio.h"

#include "macro.h"

#define GPIO_DIGIT_0    GPIOB
#define PIN_DIGIT_0     (9)

#define GPIO_DIGIT_1    GPIOB
#define PIN_DIGIT_1     (8)

#define GPIO_DIGIT_2    GPIOB
#define PIN_DIGIT_2     (5)

#define GPIO_DIGIT_3    GPIOB
#define PIN_DIGIT_3     (4)

#define GPIO_DIGIT_4    GPIOA
#define PIN_DIGIT_4     (15)

#define GPIO_DIGIT_5    GPIOA
#define PIN_DIGIT_5     (12)

#define GPIO_DIGIT_6    GPIOA
#define PIN_DIGIT_6     (11)

#define GPIO_DIGIT_7    GPIOA
#define PIN_DIGIT_7     (8)

#define GPIO_DIGIT_8    GPIOB
#define PIN_DIGIT_8     (15)

#define GPIO_DIGIT_9    GPIOB
#define PIN_DIGIT_9     (14)

#define GPIO_DIGIT_DOT  GPIOB
#define PIN_DIGIT_DOT   (13)

#define GPIO_ANODE_0    GPIOB
#define PIN_ANODE_0     (12)

#define GPIO_ANODE_1    GPIOB
#define PIN_ANODE_1     (11)

#define GPIO_ANODE_2    GPIOB
#define PIN_ANODE_2     (10)

#define GPIO_ANODE_3    GPIOB
#define PIN_ANODE_3     (2)

#define GPIO_DIGIT_S    GPIOB
#define PIN_DIGIT_S     (1)

typedef struct output_s
{
    GPIO_TypeDef* gpio;
    uint8_t pin;
} output_s;

static output_s outputs[] = 
{
    { .gpio = GPIO_DIGIT_0, .pin = PIN_DIGIT_0 },
    { .gpio = GPIO_DIGIT_1, .pin = PIN_DIGIT_1 },
    { .gpio = GPIO_DIGIT_2, .pin = PIN_DIGIT_2 },
    { .gpio = GPIO_DIGIT_3, .pin = PIN_DIGIT_3 },
    { .gpio = GPIO_DIGIT_4, .pin = PIN_DIGIT_4 },
    { .gpio = GPIO_DIGIT_5, .pin = PIN_DIGIT_5 },
    { .gpio = GPIO_DIGIT_6, .pin = PIN_DIGIT_6 },
    { .gpio = GPIO_DIGIT_7, .pin = PIN_DIGIT_7 },
    { .gpio = GPIO_DIGIT_8, .pin = PIN_DIGIT_8 },
    { .gpio = GPIO_DIGIT_9, .pin = PIN_DIGIT_9 },
    { .gpio = GPIO_DIGIT_DOT, .pin = PIN_DIGIT_DOT },
    { .gpio = GPIO_ANODE_0, .pin = PIN_ANODE_0 },
    { .gpio = GPIO_ANODE_1, .pin = PIN_ANODE_1 },
    { .gpio = GPIO_ANODE_2, .pin = PIN_ANODE_2 },
    { .gpio = GPIO_ANODE_3, .pin = PIN_ANODE_3 },
    { .gpio = GPIO_DIGIT_S, .pin = PIN_DIGIT_S }
};

void display_init_ll(void)
{
    // Enable clocks
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;

    for (uint32_t i = 0; i < ARRAY_ITEMS_QTY(outputs); i++)
    {
        gpio_output_init(outputs[i].gpio, outputs[i].pin, GPIO_MODE_PUSH_PULL, GPIO_SPEED_10_MHZ);
    }
}

void display_test_ll(void)
{
    for (uint32_t i = 0; i < ARRAY_ITEMS_QTY(outputs); i++)
    {
        gpio_toggle(outputs[i].gpio, outputs[i].pin);
    }
}