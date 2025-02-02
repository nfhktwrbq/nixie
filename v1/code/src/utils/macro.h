#pragma once

#define assert_param(expr) if(!(expr)) {while(1){}}

#define 	SET_BIT(REG, BIT)   ((REG) |= (BIT))
#define 	CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))
#define 	READ_BIT(REG, BIT)   ((REG) & (BIT))
#define 	CLEAR_REG(REG)   ((REG) = (0x0))
#define 	WRITE_REG(REG, VAL)   ((REG) = (VAL))
#define 	READ_REG(REG)   ((REG))
#define 	MODIFY_REG(REG, CLEARMASK, SETMASK)   WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))
#define UNUSED  (void)
#define __weak __attribute__((weak))

#define SET         (1u)
#define RESET       (0u)