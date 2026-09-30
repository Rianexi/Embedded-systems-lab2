#ifndef INC_SDK_KEYBOARD_H_
#define INC_SDK_KEYBOARD_H_

#include "i2c.h"

void SDK_Keyboard_Init(I2C_HandleTypeDef *hi2c);
char SDK_Keyboard_GetKey(void);

#endif /* INC_SDK_KEYBOARD_H_ */
