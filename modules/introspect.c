/* Author: Tobi Vollebregt */

#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <mach/hardware.h>
#include <mach/platform.h>

/* From drivers/tty/serial/lpc32xx_hs.c */
#define LPC32XX_HSUART_RATE(x)          io_p2v(x + 0x10)
#define LPC32XX_ADC_SELECT              io_p2v(LPC32XX_ADC_BASE + 0x04)
#define LPC32XX_ADC_CTRL                io_p2v(LPC32XX_ADC_BASE + 0x08)
#define LPC32XX_ADC_VALUE               io_p2v(LPC32XX_ADC_BASE + 0x48)

static const char* to_string(u32 value)
{
	static char buf[64];
	u32 i, j;

	for (i = j = 0; i < 32; ++i) {
		buf[j++] = (value & (1 << (31 - i))) ? '1' : '0';
		if ((i & 7) == 7) buf[j++] = ' ';
	}
	buf[--j] = 0;
	return buf;
}

int init_module(void)
{
	u32 tmp, x, y;

	/* GPIO MUX states */
	printk(KERN_INFO "P_MUX_STATE  = %s\n", to_string(__raw_readl(LPC32XX_GPIO_P_MUX_STATE)));
	printk(KERN_INFO "P0_MUX_STATE = %s\n", to_string(__raw_readl(LPC32XX_GPIO_P0_MUX_STATE)));
	printk(KERN_INFO "P1_MUX_STATE = %s\n", to_string(__raw_readl(LPC32XX_GPIO_P1_MUX_STATE)));
	printk(KERN_INFO "P2_MUX_STATE = %s\n", to_string(__raw_readl(LPC32XX_GPIO_P2_MUX_STATE)));
	printk(KERN_INFO "P3_MUX_STATE = %s\n", to_string(__raw_readl(LPC32XX_GPIO_P3_MUX_STATE)));

	/* ADC */
	printk(KERN_INFO "ADC_SELECT     = %s\n", to_string(__raw_readl(LPC32XX_ADC_SELECT)));
	printk(KERN_INFO "ADC_CTRL       = %s\n", to_string(__raw_readl(LPC32XX_ADC_CTRL)));
	printk(KERN_INFO "ADC_VALUE      = %s\n", to_string(__raw_readl(LPC32XX_ADC_VALUE)));
	printk(KERN_INFO "ADC_CLK_CTRL   = %s\n", to_string(__raw_readl(LPC32XX_CLKPWR_ADC_CLK_CTRL)));
	printk(KERN_INFO "ADC_CLK_CTRL_1 = %s\n", to_string(__raw_readl(LPC32XX_CLKPWR_ADC_CLK_CTRL_1)));

	/* Standard UART */
	tmp = __raw_readl(LPC32XX_CLKPWR_UART3_CLK_CTRL);
	x = (tmp >> 8) & 0xff;
	y = tmp & 0xff;
	printk(KERN_INFO "CLKPWR_UART3_CLK_CTRL = 0x%08x (%s * %d/%d)\n", tmp,
		(tmp & _BIT(16)) ? "HCLK" : "PERIPH_CLK", x, y);

		/* Set DLAB in U3LCR */
	__raw_writel(__raw_readl(LPC32XX_UART_LCR(LPC32XX_UART3_BASE)) | _BIT(7),
		LPC32XX_UART_LCR(LPC32XX_UART3_BASE));

		/* Read Divisor Latch */
	tmp = __raw_readl(LPC32XX_UART_DLL_FIFO(LPC32XX_UART3_BASE)) |
		(__raw_readl(LPC32XX_UART_DLM_IER(LPC32XX_UART3_BASE)) << 8);

		/* Clear DLAB in U3LCR */
	__raw_writel(__raw_readl(LPC32XX_UART_LCR(LPC32XX_UART3_BASE)) & ~_BIT(7),
		LPC32XX_UART_LCR(LPC32XX_UART3_BASE));

	printk(KERN_INFO "UART3_DL = 0x%04x (PERIPH_CLK * %d/%d * 1 / (16 * %d) = %d)\n",
		tmp, x, y, tmp, LPC32XX_MAIN_OSC_FREQ * x / y / (16 * tmp));

	/* High speed UART */
	tmp = __raw_readl(LPC32XX_HSUART_RATE(LPC32XX_HS_UART2_BASE));
	printk(KERN_INFO "HSUART2_RATE = %d (PERIPH_CLK / (%d * 14) = %d)\n",
		tmp, tmp + 1, LPC32XX_MAIN_OSC_FREQ / ((tmp + 1) * 14));

	return 0;
}

void cleanup_module(void)
{
}

MODULE_LICENSE("GPL");
