#include "sdk_keyboard.h"

#define KEYBOARD_ADDR       0xE2

#define PCA_REG_INPUT       0x00
#define PCA_REG_OUTPUT      0x01
#define PCA_REG_POLARITY    0x02
#define PCA_REG_CONFIG      0x03

static I2C_HandleTypeDef *keyboard_i2c = NULL;

static const char keymap[4][3] =
{
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};

static HAL_StatusTypeDef PCA_Write(uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(
        keyboard_i2c,
        KEYBOARD_ADDR,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        &value,
        1,
        100
    );
}

static HAL_StatusTypeDef PCA_Read(uint8_t reg, uint8_t *value)
{
    return HAL_I2C_Mem_Read(
        keyboard_i2c,
        KEYBOARD_ADDR,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        value,
        1,
        100
    );
}

void SDK_Keyboard_Init(I2C_HandleTypeDef *hi2c)
{
    keyboard_i2c = hi2c;

    PCA_Write(PCA_REG_POLARITY, 0x00);

    PCA_Write(PCA_REG_OUTPUT, 0xFF);

    PCA_Write(PCA_REG_CONFIG, 0xFF);
}

static char Keyboard_ScanRaw(void)
{
    if (keyboard_i2c == NULL)
    {
        return 0;
    }

    for (uint8_t row = 0; row < 4; row++)
    {

        uint8_t config = 0xFF;
        config &= ~(1U << row);

        uint8_t output = 0xFF;
        output &= ~(1U << row);

        if (PCA_Write(PCA_REG_OUTPUT, output) != HAL_OK)
        {
            continue;
        }

        if (PCA_Write(PCA_REG_CONFIG, config) != HAL_OK)
        {
            continue;
        }

        for (volatile uint32_t i = 0; i < 100; i++)
        {
            __NOP();
        }

        uint8_t input = 0xFF;

        if (PCA_Read(PCA_REG_INPUT, &input) != HAL_OK)
        {
            continue;
        }

        for (uint8_t col = 0; col < 3; col++)
        {
            uint8_t bit = 4U + col;

            if ((input & (1U << bit)) == 0)
            {
                PCA_Write(PCA_REG_CONFIG, 0xFF);

                return keymap[row][col];
            }
        }
    }

    PCA_Write(PCA_REG_CONFIG, 0xFF);

    return 0;
}

char SDK_Keyboard_GetKey(void)
{
    static char previous_raw = 0;
    static char stable_key = 0;

    static uint32_t change_time = 0;

    static uint8_t reported = 0;

    char raw = Keyboard_ScanRaw();

    uint32_t now = HAL_GetTick();

    if (raw != previous_raw)
    {
        previous_raw = raw;
        change_time = now;
    }

    if ((now - change_time) >= 25)
    {
        if (stable_key != raw)
        {
            stable_key = raw;

            if (stable_key == 0)
            {
                reported = 0;
            }
        }
    }

    if ((stable_key != 0) && (reported == 0))
    {
        reported = 1;

        return stable_key;
    }

    return 0;
}
