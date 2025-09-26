#include <cstdio>
#include <inttypes.h>

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#include "hardware/i2c.h"
#include "si5351.h"

static Si5351 si5351(i2c0, SI5351_BUS_BASE_ADDR, true, 16, 17, 100000);

static int32_t cal_factor = 0;
static int32_t old_cal = 0;

static uint64_t rx_freq;
static const uint64_t target_freq = 1000000000ULL; // 10 MHz, in hundredths of hertz

static int peeked_char = -1;

static void serial_init()
{
    stdio_init_all();
    while (!stdio_usb_connected())
    {
        tight_loop_contents();
    }

    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    setvbuf(stdin, nullptr, _IONBF, 0);
}

static bool serial_available()
{
    if (peeked_char >= 0)
    {
        return true;
    }

    int c = getchar_timeout_us(0);
    if (c == PICO_ERROR_TIMEOUT)
    {
        return false;
    }

    peeked_char = c;
    return true;
}

static int serial_read()
{
    if (peeked_char >= 0)
    {
        int c = peeked_char;
        peeked_char = -1;
        return c;
    }

    int c = getchar_timeout_us(0);
    if (c == PICO_ERROR_TIMEOUT)
    {
        return -1;
    }

    return c;
}

static void flush_input(void)
{
    while (serial_available())
    {
        if (serial_read() < 0)
        {
            break;
        }
    }
}

static void vfo_interface(void)
{
    rx_freq = target_freq;
    cal_factor = old_cal;
    printf("   Up:   r   t  y  u  i   o  p\n");
    printf(" Down:   f   g  h  j  k   l  ;\n");
    printf("   Hz: 0.01 0.1 1 10 100 1K 10k\n");
    while (1)
    {
        if (serial_available())
        {
            int read_value = serial_read();
            if (read_value < 0)
            {
                continue;
            }

            char c = static_cast<char>(read_value);
            switch (c)
            {
                case 'q':
                    flush_input();
                    printf("\n");
                    printf("Calibration factor is %" PRId32 "\n", cal_factor);
                    printf("Setting calibration factor\n");
                    si5351.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
                    si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
                    printf("Resetting target frequency\n");
                    si5351.set_freq(target_freq, SI5351_CLK0);
                    old_cal = cal_factor;
                    return;
                case 'r':
                    rx_freq += 1;
                    break;
                case 'f':
                    rx_freq -= 1;
                    break;
                case 't':
                    rx_freq += 10;
                    break;
                case 'g':
                    rx_freq -= 10;
                    break;
                case 'y':
                    rx_freq += 100;
                    break;
                case 'h':
                    rx_freq -= 100;
                    break;
                case 'u':
                    rx_freq += 1000;
                    break;
                case 'j':
                    rx_freq -= 1000;
                    break;
                case 'i':
                    rx_freq += 10000;
                    break;
                case 'k':
                    rx_freq -= 10000;
                    break;
                case 'o':
                    rx_freq += 100000;
                    break;
                case 'l':
                    rx_freq -= 100000;
                    break;
                case 'p':
                    rx_freq += 1000000;
                    break;
                case ';':
                    rx_freq -= 1000000;
                    break;
                default:
                    continue;
            }

            cal_factor = static_cast<int32_t>(target_freq - rx_freq) + old_cal;
            si5351.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
            si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
            si5351.pll_reset(SI5351_PLLA);
            si5351.set_freq(target_freq, SI5351_CLK0);
            printf("Current difference:%" PRId32 "\n", cal_factor);
        }
    }
}

static void setup(void)
{
    serial_init();

    si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);

    si5351.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
    si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
    si5351.set_freq(target_freq, SI5351_CLK0);
}

static void loop(void)
{
    si5351.update_status();
    if (si5351.dev_status.SYS_INIT == 1)
    {
        printf("Initialising Si5351, you shouldn't see many of these!\n");
        sleep_ms(500);
    }
    else
    {
        printf("\n");
        printf("Adjust until your frequency counter reads as close to 10 MHz as possible.\n");
        printf("Press 'q' when complete.\n");
        vfo_interface();
    }
}

int main()
{
    setup();

    while (true)
    {
        loop();
    }
}

