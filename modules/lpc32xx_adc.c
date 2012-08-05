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
#define LPC32XX_SIC1_RSR                io_p2v(0x4000C004)

#define LPC32XX_ADC_SELECT_AD_IN_MASK   0x30
#define LPC32XX_ADC_CTRL_AD_PDN_CTRL    _BIT(2)
#define LPC32XX_ADC_CTRL_AD_STROBE      _BIT(1)
#define LPC32XX_ADC_VALUE_MASK          0x3ff
#define LPC32XX_SIC1_RSR_ADC_INT	_BIT(7)

#define MOD_NAME "lpc32xx-adc"

static const char *to_string(u32 value);
static void dump_regs(void);

/* Core */

static int adc(int source) {
	u32 tmp;

	/* Select ADC source */
	tmp = __raw_readl(LPC32XX_ADC_SELECT);
	tmp &= ~LPC32XX_ADC_SELECT_AD_IN_MASK;
	__raw_writel(tmp | (source << 4), LPC32XX_ADC_SELECT);

	/* Start AD conversion */
	__raw_writel(LPC32XX_ADC_CTRL_AD_PDN_CTRL | LPC32XX_ADC_CTRL_AD_STROBE,
		LPC32XX_ADC_CTRL);

	/* Busyloop until the conversion is complete
	   (20..5000 iterations depending on ADC clock speed) */
	while ((__raw_readl(LPC32XX_SIC1_RSR) & LPC32XX_SIC1_RSR_ADC_INT) == 0) ;

	/* Read value and mask out reserved (undefined) bits */
	return __raw_readl(LPC32XX_ADC_VALUE) & LPC32XX_ADC_VALUE_MASK;
}

/* Platform device */

static ssize_t show_adc_value(struct device *dev, struct device_attribute *attr, char *buf);

static DEVICE_ATTR(adin0, S_IRUGO, show_adc_value, NULL);
static DEVICE_ATTR(adin1, S_IRUGO, show_adc_value, NULL);
static DEVICE_ATTR(adin2, S_IRUGO, show_adc_value, NULL);

static struct device_attribute *dev_attrs[] = {
	&dev_attr_adin0,
	&dev_attr_adin1,
	&dev_attr_adin2,
	NULL,
};

static ssize_t show_adc_value(struct device *dev, struct device_attribute *attr, char *buf) {
	int i;

	for (i = 0; dev_attrs[i] != NULL; ++i) {
		if (attr == dev_attrs[i]) {
			return scnprintf(buf, PAGE_SIZE, "%d", adc(i));
		}
	}

	return -EINVAL;
}

/* Platform driver */

static struct platform_device *lpc32xx_adc_device;

static int __init lpc32xx_adc_probe(struct platform_device *pdev) {
	int i, err;

	printk(KERN_INFO "lpc32xx_adc_probe\n");
	dump_regs(); /* pre-init register state */

	/* Check if touch screen is enabled, bail out if it is */
	if ((__raw_readl(LPC32XX_ADC_CTRL) & _BIT(0)) != 0)
		return -EBUSY;

	/* Set ADC negative and positive reference voltage
	   (other settings in bits 9:6 are undefined) */
	__raw_writel(__raw_readl(LPC32XX_ADC_SELECT) | _BIT(9) | _BIT(7),
		LPC32XX_ADC_SELECT);

	/* Select ADC clock source (RTC) and divider (n/a) */
	/* TODO: Using PERIPH_CLOCK and divider may speed up AD conversion */
	__raw_writel(0, LPC32XX_CLKPWR_ADC_CLK_CTRL_1);

	/* Enable ADC clock */
	__raw_writel(_BIT(0), LPC32XX_CLKPWR_ADC_CLK_CTRL);

	/* Power on ADC */
	__raw_writel(_BIT(2), LPC32XX_ADC_CTRL);

	/* Register device attributes */
	for (i = 0; dev_attrs[i] != NULL; ++i) {
		err = device_create_file(&pdev->dev, dev_attrs[i]);
		if (err != 0) {
			while (--i >= 0)
				device_remove_file(&pdev->dev, dev_attrs[i]);
			return err;
		}
	}

	lpc32xx_adc_device = pdev;

	dump_regs(); /* post-init register state */
	return 0;
}

static int __devexit lpc32xx_adc_remove(struct platform_device *pdev) {
	printk(KERN_INFO "lpc32xx_adc_remove\n");

	/* Power down ADC */
	__raw_writel(0, LPC32XX_ADC_CTRL);

	/* Disable ADC clock */
	__raw_writel(0, LPC32XX_CLKPWR_ADC_CLK_CTRL);

	lpc32xx_adc_device = NULL;

	dump_regs(); /* post-remove register state */
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
