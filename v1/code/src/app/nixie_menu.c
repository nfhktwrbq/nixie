#include "nixie_menu.h"
#include "app_display.h"

#include "software/datetime.h"
#include "drivers/display.h"
#include "drivers/buttons.h"
#include "hardware/modules/rtc.h"
#include "services/keyboard.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stddef.h>

typedef void (*dt_display_fp)(datetime_s * datetime);

typedef struct menu_item_s
 {
    const char * name;
	struct menu_item_s * next;
	struct menu_item_s * previous;
	struct menu_item_s * parent;
	struct menu_item_s * sibling;
	void_fp select_fp;
	void_fp enter_fp;
} menu_item_s;


typedef struct datetime_item_cfg_s
{
    uint32_t min;
    uint32_t max;
    uint32_t blink_mask;
    uint32_t dot_mask;
    uint32_t sec_en;
    dt_display_fp dt_display;
} datetime_item_cfg_s;

typedef enum datetime_items_e
{
    DT_MIN,
    DT_HOUR,
    DT_MONTH,
    DT_DAY,
    DT_YEAR,
    DT_ITEMS_QTY,
} datetime_items_e;

static void datetime_set(void);
static void clk_clbr_set(void);

static datetime_item_cfg_s dt_cfgs[] =
{
    [DT_MIN] = { .min = 0, .max = 59, .blink_mask = 0x03, .dot_mask = 0x0, .sec_en = true, .dt_display = app_display_time_set },
    [DT_HOUR] = { .min = 0, .max = 23, .blink_mask = 0x0c, .dot_mask = 0x0, .sec_en = true, .dt_display = app_display_time_set },
    [DT_MONTH] = { .min = 0, .max = 12, .blink_mask = 0x03, .dot_mask = 0x2, .sec_en = false, .dt_display = app_display_month_day_set },
    [DT_DAY] = { .min = 0, .max = 31, .blink_mask = 0x0c, .dot_mask = 0x2, .sec_en = false, .dt_display = app_display_month_day_set },
    [DT_YEAR] = { .min = DT_MIN_BASE_YEAS, .max = DT_MIN_BASE_YEAS + 100, .blink_mask = 0x0f, .dot_mask = 0x0, .sec_en = false, .dt_display = app_display_year_set},
};

static menu_item_s time_set_menu = { .name = "1", .select_fp = datetime_set, .enter_fp = NULL, };
static menu_item_s clk_clbr_menu = { .name = "2", .select_fp = clk_clbr_set, .enter_fp = NULL, };
static menu_item_s settings_menu = { .name = "3", .select_fp = NULL, .enter_fp = NULL, };

static menu_item_s * cur_menu = NULL;

static void datetime_change_prepare(datetime_items_e pos, datetime_s * dt)
{
    display_dot_msk_set(dt_cfgs[pos].dot_mask);
    display_blink_msk_set(dt_cfgs[pos].blink_mask);
    display_second_set(dt_cfgs[pos].sec_en);
    dt_cfgs[pos].dt_display(dt);
}

static void datetime_change(datetime_s * dt, bool incr, datetime_items_e pos)
{
    uint32_t val = dt_cfgs[pos].min;
    uint32_t max_val = dt_cfgs[pos].max;
    uint32_t min_val = dt_cfgs[pos].min;
    
    switch (pos)
    {
        case DT_MIN:   val = dt->minute; break;
        case DT_HOUR:  val = dt->hour;   break;
        case DT_DAY:   val = dt->day;    max_val = datetime_days_in_month_get(dt->month, dt->year); break;
        case DT_MONTH: val = dt->month;  break;
        case DT_YEAR:  val = dt->year;   break;
        case DT_ITEMS_QTY:
        default: return;
    }

    if (incr && val < max_val)
    {
        val++;
    }

    if (!incr && val > min_val)
    {
        val--;
    }

    switch (pos)
    {
        case DT_MIN:   dt->minute = (uint8_t)val;  break;
        case DT_HOUR:  dt->hour   = (uint8_t)val;  break;
        case DT_DAY:   dt->day    = (uint8_t)val;  break;
        case DT_MONTH: dt->month  = (uint8_t)val;  break;
        case DT_YEAR:  dt->year   = (uint16_t)val; break;
        case DT_ITEMS_QTY:
        default: return;
    }

    dt_cfgs[pos].dt_display(dt);
}

static void menu_enter(menu_item_s * m)
{
	if (m == NULL)
    {
	  return;
    }

	cur_menu = m;

    DBG_INFO("Enter %s menu\n", m->name);

	if (m->enter_fp != NULL)
    {
        m->enter_fp();
    }
    display_print(m->name);
}


void nixie_menu_enter(void)
{
    cur_menu = &time_set_menu;
    menu_enter(cur_menu);
}

static void datetime_set(void)
{
    buttons_e btn;
    datetime_s datetime;
    uint32_t current_pos = 0;

    datetime_from_timestamp(rtc_datetime_get(), &datetime);

    display_blink_set(current_pos, true);
    display_second_set(true);
    bool dt_changed = false;
    datetime_change_prepare(current_pos, &datetime);

    for (;;)
    {
        if (keyboard_key_is_pressed(&btn))
        {
            switch (btn)
            {
                case BUTTON_OK:
                    if (dt_changed)
                    {
                        datetime.second = 0;
                        rtc_datetime_set(datetime_to_timestamp(&datetime));
                    }
                    display_blink_msk_set(0);
                    display_dot_msk_set(0);
                    return;
                break;

                case BUTTON_LEFT:
                    if (current_pos < DT_ITEMS_QTY)
                    {
                        current_pos++;
                        DBG_INFO("Pos %u\n", current_pos);
                        datetime_change_prepare(current_pos, &datetime);
                    }
                break;

                case BUTTON_RIGHT:
                    if (current_pos > 0)
                    {
                        current_pos--;
                        DBG_INFO("Pos %u\n", current_pos);
                        datetime_change_prepare(current_pos, &datetime);
                    }
                break;
                
                case BUTTON_DOWN:
                case BUTTON_UP:
                {
                    bool incr = btn == BUTTON_UP ? true : false;
                    datetime_change(&datetime, incr, current_pos);
                    vTaskDelay(pdMS_TO_TICKS(800));
                    while (!keyboard_key_is_released())
                    {
                        vTaskDelay(pdMS_TO_TICKS(200));
                        datetime_change(&datetime, incr, current_pos);
                    }
                    dt_changed = true;
                }
                break;

                case BUTTONS_QTY:
                default: 
                break;
            }
        }
    }
}

static void clk_clbr_set(void)
{
    buttons_e btn;

    uint32_t cal = rtc_calibration_get();
    display_uint_set(cal);

    bool changed = false;

    for (;;)
    {
        if (keyboard_key_is_pressed(&btn))
        {
            switch (btn)
            {
                case BUTTON_OK:
                    if (changed)
                    {
                        rtc_calibration_set(cal);
                    }
                    return;
                break;
                
                case BUTTON_DOWN:
                case BUTTON_UP:
                {
                    bool incr = btn == BUTTON_UP ? true : false;
                    do
                    {
                        if (incr)
                        {
                            cal++;
                        }
                        else                    
                        {
                            if (cal)
                            {
                                cal--;
                            }
                        }
                        display_uint_set(cal);
                        vTaskDelay(pdMS_TO_TICKS(200));
                    }
                    while (!keyboard_key_is_released());
                    changed = true;
                }
                break;

                case BUTTON_LEFT:
                case BUTTON_RIGHT:
                case BUTTONS_QTY:
                default:
                break;
            }
        }
    }
}

void nixie_menu_init(void)
{
    time_set_menu.next = &clk_clbr_menu;
    time_set_menu.parent = NULL; 
    time_set_menu.previous = &settings_menu;
    time_set_menu.sibling = NULL;

    clk_clbr_menu.next = &settings_menu;
    clk_clbr_menu.parent = NULL;
    clk_clbr_menu.previous = &time_set_menu;
    clk_clbr_menu.sibling = NULL;

    settings_menu.next = &time_set_menu;
    settings_menu.parent = NULL;
    settings_menu.previous = &time_set_menu;
    settings_menu.sibling = NULL;
}

void nixie_menu_handle(void)
{
    nixie_menu_enter();
    for (;;)
    {
        buttons_e btn;
        if (keyboard_key_is_pressed(&btn))
        {
            switch (btn)
            {
                case BUTTON_OK:
                    if (cur_menu->select_fp)
                    {
                        cur_menu->select_fp();
                    }
                    menu_enter(cur_menu);
                break;

                case BUTTON_UP:
                    menu_enter(cur_menu->previous);
                break;

                case BUTTON_DOWN:
                    menu_enter(cur_menu->next);
                break;

                case BUTTON_LEFT:
                    // sibling
                break;

                case BUTTON_RIGHT:
                    return;
                break;

                case BUTTONS_QTY:
                default:
                break;
            }            
        }
    }
}