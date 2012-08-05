/* Author: Tobi Vollebregt */

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <mach/hardware.h>
#include <mach/platform.h>

#define LPC32XX_ADC_SELECT              io_p2v(LPC32XX_ADC_BASE + 0x04)
#define LPC32XX_ADC_CTRL                io_p2v(LPC32XX_ADC_BASE + 0x08)
#define LPC32XX_ADC_VALUE               io_p2v(LPC32XX_ADC_BASE + 0x48)

#define MOD_NAME "lpc32xx-adc"

static const char *to_string(u32 value);
static void dump_regs(void);

/* Platform driver */

static struct platform_device *lpc32xx_adc_device;

static int __init lpc32xx_adc_probe(struct platform_device *pdev) {
	printk(KERN_INFO "lpc32xx_adc_probe\n");

	lpc32xx_adc_device = pdev;

	return 0;
}

static int __devexit lpc32xx_adc_remove(struct platform_device *pdev) {
	printk(KERN_INFO "lpc32xx_adc_remove\n");

	lpc32xx_adc_device = NULL;

	return 0;
}

static struct platform_driver lpc32xx_adc_driver = {
	.driver         = {
		.name   = MOD_NAME,
		.owner  = THIS_MODULE,
	},
	.remove = __devexit_p(lpc32xx_adc_remove),
};

/* Module */

static int __init lpc32xx_adc_init(void) {
	struct platform_device *pdev;

	printk(KERN_INFO "lpc32xx_adc_init\n");

	pdev = platform_create_bundle(&lpc32xx_adc_driver, lpc32xx_adc_probe, NULL, 0, NULL, 0);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);
	return 0;
}

static void __exit lpc32xx_adc_exit(void) {
	printk(KERN_INFO "lpc32xx_adc_exit\n");

	platform_device_unregister(lpc32xx_adc_device);
	platform_driver_unregister(&lpc32xx_adc_driver);
}

module_init(lpc32xx_adc_init);
module_exit(lpc32xx_adc_exit);

MODULE_AUTHOR("Tobi Vollebregt <tobivollebregt@gmail.com>");
MODULE_DESCRIPTION("LPC32XX ADC Driver");
MODULE_LICENSE("GPL");

/* Utilities */

static const char *to_string(u32 value) {
	static char buf[64];
	u32 i, j;

	for (i = j = 0; i < 32; ++i) {
		buf[j++] = (value & (1 << (31 - i))) ? '1' : '0';
		if ((i & 7) == 7) buf[j++] = ' ';
	}
	buf[--j] = 0;
	return buf;
}

static void dump_regs(void) {
	printk(KERN_INFO "ADC_SELECT     = %s\n", to_string(__raw_readl(LPC32XX_ADC_SELECT)));
	printk(KERN_INFO "ADC_CTRL       = %s\n", to_string(__raw_readl(LPC32XX_ADC_CTRL)));
	printk(KERN_INFO "ADC_VALUE      = %s\n", to_string(__raw_readl(LPC32XX_ADC_VALUE)));
	printk(KERN_INFO "ADC_CLK_CTRL   = %s\n", to_string(__raw_readl(LPC32XX_CLKPWR_ADC_CLK_CTRL)));
	printk(KERN_INFO "ADC_CLK_CTRL_1 = %s\n", to_string(__raw_readl(LPC32XX_CLKPWR_ADC_CLK_CTRL_1)));
}
