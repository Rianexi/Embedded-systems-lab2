#include "sdk_oled.h"

#include <string.h>

#define OLED_ADDR       0x78

#define OLED_WIDTH      128
#define OLED_HEIGHT     64
#define OLED_PAGES      8

static I2C_HandleTypeDef *oled_i2c = NULL;

static uint8_t framebuffer[OLED_WIDTH * OLED_PAGES];

static uint8_t cursor_x = 0;
static uint8_t cursor_page = 0;

static const uint8_t font_0[5] =
{
    0x3E, 0x51, 0x49, 0x45, 0x3E
};

static const uint8_t font_1[5] =
{
    0x00, 0x42, 0x7F, 0x40, 0x00
};

static const uint8_t font_2[5] =
{
    0x42, 0x61, 0x51, 0x49, 0x46
};

static const uint8_t font_3[5] =
{
    0x21, 0x41, 0x45, 0x4B, 0x31
};

static const uint8_t font_4[5] =
{
    0x18, 0x14, 0x12, 0x7F, 0x10
};

static const uint8_t font_5[5] =
{
    0x27, 0x45, 0x45, 0x45, 0x39
};

static const uint8_t font_6[5] =
{
    0x3C, 0x4A, 0x49, 0x49, 0x30
};

static const uint8_t font_7[5] =
{
    0x01, 0x71, 0x09, 0x05, 0x03
};

static const uint8_t font_8[5] =
{
    0x36, 0x49, 0x49, 0x49, 0x36
};

static const uint8_t font_9[5] =
{
    0x06, 0x49, 0x49, 0x29, 0x1E
};

static const uint8_t font_plus[5] =
{
    0x08, 0x08, 0x3E, 0x08, 0x08
};

static const uint8_t font_minus[5] =
{
    0x08, 0x08, 0x08, 0x08, 0x08
};

static const uint8_t font_equal[5] =
{
    0x14, 0x14, 0x14, 0x14, 0x14
};

static const uint8_t font_mul[5] =
{
    0x22, 0x14, 0x08, 0x14, 0x22
};

static const uint8_t font_div[5] =
{
    0x20, 0x10, 0x08, 0x04, 0x02
};

static const uint8_t font_E[5] =
{
    0x7F, 0x49, 0x49, 0x49, 0x41
};

static const uint8_t font_R[5] =
{
    0x7F, 0x09, 0x19, 0x29, 0x46
};

static const uint8_t font_space[5] =
{
    0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t *GetGlyph(char c)
{
    switch (c)
    {
        case '0':
            return font_0;

        case '1':
            return font_1;

        case '2':
            return font_2;

        case '3':
            return font_3;

        case '4':
            return font_4;

        case '5':
            return font_5;

        case '6':
            return font_6;

        case '7':
            return font_7;

        case '8':
            return font_8;

        case '9':
            return font_9;

        case '+':
            return font_plus;

        case '-':
            return font_minus;

        case '=':
            return font_equal;

        case '*':
            return font_mul;

        case '/':
            return font_div;

        case 'E':
            return font_E;

        case 'R':
            return font_R;

        case ' ':
        default:
            return font_space;
    }
}

static HAL_StatusTypeDef OLED_Command(uint8_t command)
{
    uint8_t packet[2];

    packet[0] = 0x00;
    packet[1] = command;

    return HAL_I2C_Master_Transmit(
        oled_i2c,
        OLED_ADDR,
        packet,
        2,
        100
    );
}

void SDK_OLED_Init(I2C_HandleTypeDef *hi2c)
{
    oled_i2c = hi2c;

    HAL_Delay(100);

    OLED_Command(0xAE);

    OLED_Command(0xD5);
    OLED_Command(0x80);

    OLED_Command(0xA8);
    OLED_Command(0x3F);

    OLED_Command(0xD3);
    OLED_Command(0x00);

    OLED_Command(0x40);

    OLED_Command(0x8D);
    OLED_Command(0x14);

    OLED_Command(0x20);
    OLED_Command(0x00);

    OLED_Command(0xA1);

    OLED_Command(0xC8);

    OLED_Command(0xDA);
    OLED_Command(0x12);

    OLED_Command(0x81);
    OLED_Command(0x7F);

    OLED_Command(0xD9);
    OLED_Command(0xF1);

    OLED_Command(0xDB);
    OLED_Command(0x40);

    OLED_Command(0xA4);

    OLED_Command(0xA6);

    OLED_Command(0x2E);

    OLED_Command(0xAF);

    SDK_OLED_Clear();

    SDK_OLED_Update();
}

void SDK_OLED_Clear(void)
{
    memset(
        framebuffer,
        0,
        sizeof(framebuffer)
    );

    cursor_x = 0;
    cursor_page = 0;
}

void SDK_OLED_SetCursor(uint8_t x, uint8_t page)
{
    if (x >= OLED_WIDTH)
    {
        x = 0;
    }

    if (page >= OLED_PAGES)
    {
        page = 0;
    }

    cursor_x = x;
    cursor_page = page;
}

void SDK_OLED_PutChar(char c)
{
    const uint8_t *glyph = GetGlyph(c);

    if ((cursor_x + 6) >= OLED_WIDTH)
    {
        return;
    }

    uint16_t offset =
        ((uint16_t)cursor_page * OLED_WIDTH)
        + cursor_x;

    for (uint8_t i = 0; i < 5; i++)
    {
        framebuffer[offset++] = glyph[i];
    }

    framebuffer[offset] = 0x00;

    cursor_x += 6;
}

void SDK_OLED_PutString(const char *str)
{
    while (*str != '\0')
    {
        SDK_OLED_PutChar(*str);

        str++;
    }
}

void SDK_OLED_DrawHLine(uint8_t x1,
                        uint8_t x2,
                        uint8_t y)
{
    if (y >= OLED_HEIGHT)
    {
        return;
    }

    if (x1 >= OLED_WIDTH)
    {
        return;
    }

    if (x2 >= OLED_WIDTH)
    {
        x2 = OLED_WIDTH - 1;
    }

    if (x2 < x1)
    {
        uint8_t temp = x1;

        x1 = x2;
        x2 = temp;
    }

    uint8_t page = y / 8;

    uint8_t bit = y % 8;

    for (uint8_t x = x1; x <= x2; x++)
    {
        framebuffer[
            ((uint16_t)page * OLED_WIDTH) + x
        ] |= (1U << bit);
    }
}

void SDK_OLED_Update(void)
{
    if (oled_i2c == NULL)
    {
        return;
    }

    for (uint8_t page = 0; page < OLED_PAGES; page++)
    {

        OLED_Command(0x21);
        OLED_Command(0);
        OLED_Command(127);

        OLED_Command(0x22);
        OLED_Command(page);
        OLED_Command(page);

        uint8_t packet[129];

        packet[0] = 0x40;

        memcpy(
            &packet[1],
            &framebuffer[
                ((uint16_t)page * OLED_WIDTH)
            ],
            OLED_WIDTH
        );

        HAL_I2C_Master_Transmit(
            oled_i2c,
            OLED_ADDR,
            packet,
            sizeof(packet),
            500
        );
    }
}
