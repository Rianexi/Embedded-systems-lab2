#ifndef INC_SDK_OLED_H_
#define INC_SDK_OLED_H_

#include "i2c.h"

#include <stdint.h>

void SDK_OLED_Init(I2C_HandleTypeDef *hi2c);

void SDK_OLED_Clear(void);

void SDK_OLED_Update(void);

void SDK_OLED_SetCursor(uint8_t x, uint8_t page);

void SDK_OLED_PutChar(char c);

void SDK_OLED_PutString(const char *str);

void SDK_OLED_DrawHLine(uint8_t x1,
                        uint8_t x2,
                        uint8_t y);

#endif /* INC_SDK_OLED_H_ */
