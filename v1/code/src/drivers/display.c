#include "display.h"

#include "modules/gpio.h"

#include "macro.h"

#define GPIO_DIGIT_0    GPIOB
#define PIN_DIGIT_0     (9)

#define GPIO_DIGIT_9    GPIOB
#define PIN_DIGIT_9     (8)

#define GPIO_DIGIT_8    GPIOB
#define PIN_DIGIT_8     (5)

#define GPIO_DIGIT_7    GPIOB
#define PIN_DIGIT_7     (4)

#define GPIO_DIGIT_6    GPIOA
#define PIN_DIGIT_6     (15)

#define GPIO_DIGIT_5    GPIOA
#define PIN_DIGIT_5     (12)

#define GPIO_DIGIT_4    GPIOA
#define PIN_DIGIT_4     (11)

#define GPIO_DIGIT_3    GPIOA
#define PIN_DIGIT_3     (8)

#define GPIO_DIGIT_2    GPIOB
#define PIN_DIGIT_2     (15)

#define GPIO_DIGIT_1    GPIOB
#define PIN_DIGIT_1     (14)

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


#define MAX_DIGIT       (9)
#define DOT_POS         (10)
#define EMPTY           (0xFF)

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
    { .gpio = GPIO_DIGIT_S, .pin = PIN_DIGIT_S },
};

static output_s anodes[] = 
{
    { .gpio = GPIO_ANODE_0, .pin = PIN_ANODE_0 },
    { .gpio = GPIO_ANODE_1, .pin = PIN_ANODE_1 },
    { .gpio = GPIO_ANODE_2, .pin = PIN_ANODE_2 },
    { .gpio = GPIO_ANODE_3, .pin = PIN_ANODE_3 },
};

static output_s cathodes[] = 
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
};


#define MAX_TEXT_LEN    (8 + 1)

#define ANODES_QTY ARRAY_ITEMS_QTY(anodes)

static struct ctx
{
    uint32_t cur_anode_num;
    uint8_t digits[ANODES_QTY];
    bool dots[ANODES_QTY];
    bool blink[ANODES_QTY];
} ctx;


void display_init_ll(void)
{
    // Enable clocks
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;

    for (uint32_t i = 0; i < ARRAY_ITEMS_QTY(outputs); i++)
    {
        gpio_output_init(outputs[i].gpio, outputs[i].pin, GPIO_MODE_PUSH_PULL, GPIO_SPEED_10_MHZ);
        gpio_reset(outputs[i].gpio, outputs[i].pin);
    }
}

static bool digit_set(uint8_t d, bool dot)
{    
    bool char_set = false;

    for (uint32_t i = 0; i < ARRAY_ITEMS_QTY(cathodes); i++)
    {
        gpio_reset(cathodes[i].gpio, cathodes[i].pin);
    }

    if (d <= MAX_DIGIT)
    {
        gpio_set(cathodes[d].gpio, cathodes[d].pin);
        char_set = true;
    }

    if (dot)
    {
        gpio_set(cathodes[DOT_POS].gpio, cathodes[DOT_POS].pin);
        char_set = true;
    }

    return char_set;
}

void display_turn_off(void)
{
    for (uint32_t i = 0; i < ARRAY_ITEMS_QTY(outputs); i++)
    {
        gpio_reset(outputs[i].gpio, outputs[i].pin);
    }
}

void display_digit_switch(void)
{
    gpio_reset(anodes[ctx.cur_anode_num].gpio, anodes[ctx.cur_anode_num].pin);
    ctx.cur_anode_num = (ctx.cur_anode_num + 1) % ANODES_QTY;
    if (digit_set(ctx.digits[ctx.cur_anode_num], ctx.dots[ctx.cur_anode_num]))
    {
        gpio_set(anodes[ctx.cur_anode_num].gpio, anodes[ctx.cur_anode_num].pin);
    }
}

void display_char_set(char c, uint32_t pos, bool dot, bool blink)
{
    if (pos < ANODES_QTY)
    {
        if (c >= '0' && c <= '9')
        {    
            ctx.digits[pos] = (uint8_t)(c - '0');
        }
        else
        {
            ctx.digits[pos] = EMPTY;
        }
        ctx.dots[pos] = dot;
        ctx.blink[pos] = blink;
    }
}

void display_uint_set(uint32_t d)
{
    for (uint32_t i = 0; i < ANODES_QTY; i++)
    {
        uint8_t digit = d % 10;
        d /= 10;
        if (d)
        {
            ctx.digits[i] = digit;
        }
        else
        {
            ctx.digits[i] = EMPTY;
        }
    }
}

void display_dot_set(uint32_t pos, bool dot)
{
    if (pos < ANODES_QTY)
    {
        ctx.dots[pos] = dot ;
    }
}

void display_second_set(bool en)
{
    if (en)
    { 
        gpio_set(GPIO_DIGIT_S, PIN_DIGIT_S);
    }
    else
    {
        gpio_set(GPIO_DIGIT_S, PIN_DIGIT_S);
    }
}


void display_test_ll(void)
{
    for (uint32_t i = 0; i < ARRAY_ITEMS_QTY(outputs); i++)
    {
        gpio_toggle(outputs[i].gpio, outputs[i].pin);
    }
}



