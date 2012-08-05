/* Author: Tobi Vollebregt */

#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <mach/hardware.h>
#include <mach/platform.h>

static u32 old_mux_state;

int init_module(void)
{
	old_mux_state = __raw_readl(LPC32XX_GPIO_P2_MUX_STATE);
	__raw_writel(_BIT(3), LPC32XX_GPIO_P2_MUX_SET);
	return 0;
}

void cleanup_module(void)
{
	if ((old_mux_state & _BIT(3)) == 0) {
		__raw_writel(_BIT(3), LPC32XX_GPIO_P2_MUX_CLR);
	}
}

MODULE_LICENSE("GPL");
