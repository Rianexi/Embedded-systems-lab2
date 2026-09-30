#include "main.h"
#include "gpio.h"
#include "i2c.h"

#include "sdk_keyboard.h"
#include "sdk_oled.h"

#include <stdint.h>
#include <stdio.h>

typedef enum
{
    CALC_FIRST,

    CALC_SECOND,

    CALC_RESULT

} CalculatorState;

static int32_t operand1 = 0;

static int32_t operand2 = 0;

static int32_t calc_result = 0;

static char calc_operation = 0;

static CalculatorState calc_state =
    CALC_FIRST;

static uint8_t operation_selected = 0;

static uint8_t calc_error = 0;

void SystemClock_Config(void);

static void Calculator_Reset(void);

static void Calculator_NextOperation(void);

static void Calculator_Calculate(void);

static void Calculator_Key(char key);

static void Calculator_Draw(void);

static void Calculator_Reset(void)
{
    operand1 = 0;

    operand2 = 0;

    calc_result = 0;

    calc_operation = 0;

    calc_state = CALC_FIRST;

    operation_selected = 0;

    calc_error = 0;
}

static void Calculator_NextOperation(void)
{
    if (calc_operation == 0)
    {
        calc_operation = '+';
    }

    else if (calc_operation == '+')
    {
        calc_operation = '-';
    }

    else if (calc_operation == '-')
    {
        calc_operation = '*';
    }

    else if (calc_operation == '*')
    {
        calc_operation = '/';
    }

    else
    {
        calc_operation = '+';
    }
}

static void Calculator_Calculate(void)
{
    calc_error = 0;

    switch (calc_operation)
    {
        case '+':
        {
            calc_result =
                operand1 + operand2;

            break;
        }

        case '-':
        {
            calc_result =
                operand1 - operand2;

            break;
        }

        case '*':
        {
            calc_result =
                operand1 * operand2;

            break;
        }

        case '/':
        {

            if (operand2 == 0)
            {
                calc_result = 0;

                calc_error = 1;
            }

            else
            {

                calc_result =
                    operand1 / operand2;
            }

            break;
        }

        default:
        {
            calc_result = 0;

            calc_error = 1;

            break;
        }
    }

    calc_state = CALC_RESULT;
}

static void Calculator_Key(char key)
{

    if ((key >= '0') &&
        (key <= '9'))
    {
        uint8_t digit =
            (uint8_t)(key - '0');

        if (calc_state == CALC_RESULT)
        {
            Calculator_Reset();
        }

        if ((calc_state == CALC_FIRST) &&
            (operation_selected != 0))
        {
            calc_state = CALC_SECOND;
        }

        if (calc_state == CALC_FIRST)
        {

            if (operand1 <= 200000000)
            {
                operand1 =
                    operand1 * 10
                    + digit;
            }
        }

        else if (calc_state == CALC_SECOND)
        {
            if (operand2 <= 200000000)
            {
                operand2 =
                    operand2 * 10
                    + digit;
            }
        }

        return;
    }

    if (key == '*')
    {

        if (calc_state == CALC_RESULT)
        {
            Calculator_Reset();
        }

        if (calc_state == CALC_FIRST)
        {
            Calculator_NextOperation();

            operation_selected = 1;
        }

        return;
    }

    if (key == '#')
    {
        if ((calc_state == CALC_SECOND) &&
            (calc_operation != 0))
        {
            Calculator_Calculate();
        }

        return;
    }
}

static void Calculator_Draw(void)
{
    char line1[24];

    char line2[24];

    char line3[24];

    SDK_OLED_Clear();

    if (calc_operation == 0)
    {
        snprintf(
            line1,
            sizeof(line1),
            "%ld",
            (long)operand1
        );
    }

    else
    {
        snprintf(
            line1,
            sizeof(line1),
            "%ld %c",
            (long)operand1,
            calc_operation
        );
    }

    if ((calc_state == CALC_SECOND) ||
        (calc_state == CALC_RESULT))
    {
        snprintf(
            line2,
            sizeof(line2),
            "%ld =",
            (long)operand2
        );
    }

    else
    {
        line2[0] = '\0';
    }

    if (calc_state == CALC_RESULT)
    {

        if (calc_error != 0)
        {
            snprintf(
                line3,
                sizeof(line3),
                "ERR"
            );
        }

        else
        {
            snprintf(
                line3,
                sizeof(line3),
                "%ld",
                (long)calc_result
            );
        }
    }

    else
    {
        line3[0] = '\0';
    }

    SDK_OLED_SetCursor(
        4,
        0
    );

    SDK_OLED_PutString(
        line1
    );

    SDK_OLED_SetCursor(
        4,
        2
    );

    SDK_OLED_PutString(
        line2
    );

    SDK_OLED_DrawHLine(
        3,
        123,
        31
    );

    SDK_OLED_SetCursor(
        4,
        5
    );

    SDK_OLED_PutString(
        line3
    );

    SDK_OLED_Update();
}

int main(void)
{

    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();

    MX_I2C1_Init();

    volatile HAL_StatusTypeDef oled_status;

    volatile HAL_StatusTypeDef keyboard_status;

    oled_status =
        HAL_I2C_IsDeviceReady(
            &hi2c1,
            0x78,
            3,
            100
        );

    keyboard_status =
        HAL_I2C_IsDeviceReady(
            &hi2c1,
            0xE2,
            3,
            100
        );

    SDK_Keyboard_Init(
        &hi2c1
    );

    SDK_OLED_Init(
        &hi2c1
    );

    Calculator_Reset();

    Calculator_Draw();

    while (1)
    {

        char key =
            SDK_Keyboard_GetKey();

        if (key != 0)
        {
            Calculator_Key(
                key
            );

            Calculator_Draw();
        }

        HAL_Delay(5);

    }

}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct =
        {0};

    RCC_ClkInitTypeDef RCC_ClkInitStruct =
        {0};

    __HAL_RCC_PWR_CLK_ENABLE();

    __HAL_PWR_VOLTAGESCALING_CONFIG(
        PWR_REGULATOR_VOLTAGE_SCALE3
    );

    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSI;

    RCC_OscInitStruct.HSIState =
        RCC_HSI_ON;

    RCC_OscInitStruct.HSICalibrationValue =
        RCC_HSICALIBRATION_DEFAULT;

    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(
            &RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource =
        RCC_SYSCLKSOURCE_HSI;

    RCC_ClkInitStruct.AHBCLKDivider =
        RCC_SYSCLK_DIV1;

    RCC_ClkInitStruct.APB1CLKDivider =
        RCC_HCLK_DIV1;

    RCC_ClkInitStruct.APB2CLKDivider =
        RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct,
            FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

void Error_Handler(void)
{

    __disable_irq();

    while (1)
    {
    }

}

#ifdef USE_FULL_ASSERT

void assert_failed(
    uint8_t *file,
    uint32_t line
)
{

}

#endif
