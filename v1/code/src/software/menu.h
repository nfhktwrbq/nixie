#pragma once

#include "types.h"

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
