/* Author: Tobi Vollebregt */

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <mach/irqs.h>
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
#define LPC32XX_SIC1_RSR_ADC_INT        _BIT(7)

#define LPC32XX_ADC_CLK_MAX_SPEED       4500000

/* Size of small buffer filled in ISR before putting the data into the kfifo
   and waking up processes.  512 bytes appears to be the fastest configuration. */
#define BUFFER_SIZE                     512

/* 4096 bytes ~ 5ms single channel buffering at the highest speed.
   Note: this speed is never reached due to limits on interrupt latency etc.! */
#define FIFO_SIZE                       4096

#define MOD_NAME                        "lpc32xx-adc"
#define DEVICE_NAME                     "adc"

/* Core */

static DEFINE_SPINLOCK(adc_lock);
static int pclk_rate;
static int osc_32KHz_rate;

static int __init adc_init(void) {
	struct clk *pclk;
	struct clk *osc_32KHz;

	/* Check if touch screen is enabled, bail out if it is */
	if ((__raw_readl(LPC32XX_ADC_CTRL) & _BIT(0)) != 0) {
		printk(KERN_ERR "Error: Touch screen is using A/D converter\n");
		return -EBUSY;
	}

	/* Get PERIPH_CLK rate */
	pclk = clk_get(NULL, "pclk_ck");
	if (IS_ERR(pclk)) {
		printk(KERN_ERR "Error: Cannot get pclk_ck clock\n");
		return PTR_ERR(pclk);
	}
	pclk_rate = clk_get_rate(pclk);

	/* Get 32KHz oscillator rate (formality, it is 32KHz...) */
	osc_32KHz = clk_get(NULL, "osc_32KHz");
	if (IS_ERR(osc_32KHz)) {
		printk(KERN_ERR "Error: Cannot get osc_32KHz clock\n");
		return PTR_ERR(osc_32KHz);
	}
	osc_32KHz_rate = clk_get_rate(osc_32KHz);

	/* Set ADC negative and positive reference voltage
	   (other settings in bits 9:6 are undefined) */
	__raw_writel(__raw_readl(LPC32XX_ADC_SELECT) | _BIT(9) | _BIT(7),
		LPC32XX_ADC_SELECT);

	return 0;
}

static void __devexit adc_exit(void) {
}

static inline void adc_enable(void) {
	/* Enable ADC clock */
	__raw_writel(_BIT(0), LPC32XX_CLKPWR_ADC_CLK_CTRL);
}

static inline void adc_start(int source) {
	u32 tmp;

	/* Select ADC source */
	tmp = __raw_readl(LPC32XX_ADC_SELECT);
	tmp &= ~LPC32XX_ADC_SELECT_AD_IN_MASK;
	__raw_writel(tmp | (source << 4), LPC32XX_ADC_SELECT);

	/* Start AD conversion */
	__raw_writel(LPC32XX_ADC_CTRL_AD_PDN_CTRL | LPC32XX_ADC_CTRL_AD_STROBE,
		LPC32XX_ADC_CTRL);
}

static inline int adc_finish(void) {
	/* Read value and mask out reserved (undefined) bits */
	return __raw_readl(LPC32XX_ADC_VALUE) & LPC32XX_ADC_VALUE_MASK;
}

static inline void adc_disable(void) {
	/* Power down ADC */
	__raw_writel(0, LPC32XX_ADC_CTRL);

	/* Disable ADC clock */
	__raw_writel(0, LPC32XX_CLKPWR_ADC_CLK_CTRL);
}

static int adc_single_shot(int source) {
	int tmp;

	spin_lock(&adc_lock);

	adc_enable();
	adc_start(source);

	/* Busyloop until the conversion is complete
	   (20..5000 iterations depending on ADC clock speed) */
	while ((__raw_readl(LPC32XX_SIC1_RSR) & LPC32XX_SIC1_RSR_ADC_INT) == 0) ;

	tmp = adc_finish();
	adc_disable();

	spin_unlock(&adc_lock);
	return tmp;
}

static int adc_get_speed(void) {
	u32 tmp;

	spin_lock(&adc_lock);
	tmp = __raw_readl(LPC32XX_CLKPWR_ADC_CLK_CTRL_1);
	spin_unlock(&adc_lock);

	if (tmp & _BIT(8))
		/* PERIPH clock */
		tmp = pclk_rate / ((tmp & 0xff) + 1);
	else
		/* 32KHz clock */
		tmp = osc_32KHz_rate;

	return tmp;
}

static void adc_set_speed(int speed) {
	u32 tmp, divisor;

	/* Prevent division by zero */
	if (speed < 0)
		speed = 1;

	/* Bound to max rated speed */
	if (speed > LPC32XX_ADC_CLK_MAX_SPEED)
		speed = LPC32XX_ADC_CLK_MAX_SPEED;

	divisor = (pclk_rate + speed / 2) / speed;

	if (divisor > 256) {
		if (abs(pclk_rate / 256 - speed) < abs(osc_32KHz_rate - speed))
			/* Use PERIPH_CLOCK with divisor of 256 */
			tmp = 0x1ff;
		else
			/* Use 32KHz clock */
			tmp = 0;
	}
	else {
		if (divisor > 0)
			--divisor;
		tmp = _BIT(8) | divisor;
	}

	spin_lock(&adc_lock);
	__raw_writel(tmp, LPC32XX_CLKPWR_ADC_CLK_CTRL_1);
	spin_unlock(&adc_lock);
}

/* Character device interface */

static int major;
static bool device_is_open = 0;
static struct kfifo fifo;
static DECLARE_WAIT_QUEUE_HEAD(wait_queue);
static bool channel_enabled[3] = {false, false, true};
static int channel;
static short buffer[BUFFER_SIZE];
static int buffer_pos;

static irqreturn_t adc_irq_handler(int irq, void *dev_id) {
	/* Read result */
	buffer[buffer_pos++] = adc_finish();

	/* Select next channel */
	do
		if (++channel == 3)
			channel = 0;
	while (!channel_enabled[channel]);

	if (unlikely(buffer_pos == BUFFER_SIZE)) {
		/* Store result */
		if (kfifo_in(&fifo, buffer, sizeof(buffer)) == sizeof(buffer)) {
			/* Wake up processes blocking on our device node */
			wake_up_interruptible(&wait_queue);

			/* Trigger new A/D conversion */
			adc_start(channel);
		}
		else
			/* Buffer overflow kills the device node until reopened,
			   because we don't start a new A/D conversion here.  This
			   slightly protects the kernel against interrupt overload. */
			printk(KERN_ERR "Error: Buffer overflow\n");

		buffer_pos = 0;
	}
	else
		/* Trigger new A/D conversion */
		adc_start(channel);

	return IRQ_HANDLED;
}

static int device_open(struct inode *inode, struct file *file) {
	int err;

	/* Device can only be open once */
	if (device_is_open)
		return -EBUSY;
	device_is_open = true;

	/* Lock module until device is closed */
	try_module_get(THIS_MODULE);

	/* Allocate FIFO */
	err = kfifo_alloc(&fifo, FIFO_SIZE, GFP_KERNEL);
	if (err != 0) {
		printk(KERN_ERR "Error: Cannot allocate kfifo\n");
		module_put(THIS_MODULE);
		device_is_open = false;
		return err;
	}
	buffer_pos = 0;

	/* Request ADC IRQ */
	err = request_irq(IRQ_LPC32XX_TS_IRQ, &adc_irq_handler, 0, "TS_IRQ", NULL);
	if (err != 0) {
		printk(KERN_ERR "Error: Cannot request IRQ\n");
		kfifo_free(&fifo);
		module_put(THIS_MODULE);
		device_is_open = false;
		return err;
	}

	/* ADC interrupt is on by default. ??? */
	disable_irq(IRQ_LPC32XX_TS_IRQ);
	enable_irq(IRQ_LPC32XX_TS_IRQ);

	/* Start! */
	channel = channel_enabled[0] ? 0 : (channel_enabled[1] ? 1 : 2);
	adc_enable();
	adc_start(channel);

	return 0;
}

static int device_release(struct inode *inode, struct file *file) {
	/* Disable ADC */
	adc_disable();

	/* Disable & free ADC IRQ */
	disable_irq(IRQ_LPC32XX_TS_IRQ);
	free_irq(IRQ_LPC32XX_TS_IRQ, NULL);

	/* Free FIFO */
	kfifo_free(&fifo);

	/* Release module and device */
	module_put(THIS_MODULE);
	device_is_open = false;

	return 0;
}

static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset) {
	int err;
	unsigned int n;

	err = wait_event_interruptible(wait_queue, !kfifo_is_empty(&fifo));
	if (err != 0)
		return err;

	err = kfifo_to_user(&fifo, buffer, length, &n);
	if (err != 0)
		return err;

	return n;
}

static struct file_operations fops = {
	.read = device_read,
	.open = device_open,
	.release = device_release
};

/* Platform device / sysfs interface */

static ssize_t show_adc_speed(struct device *dev, struct device_attribute *attr, char *buf) {
	return scnprintf(buf, PAGE_SIZE, "%d", adc_get_speed());
}

static ssize_t store_adc_speed(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	if (device_is_open)
		return -EBUSY;

	adc_set_speed(simple_strtoul(buf, NULL, 0));
	return count;
}

static ssize_t show_adc_value(struct device *dev, struct device_attribute *attr, char *buf);

static DEVICE_ATTR(adin0, S_IRUGO, show_adc_value, NULL);
static DEVICE_ATTR(adin1, S_IRUGO, show_adc_value, NULL);
static DEVICE_ATTR(adin2, S_IRUGO, show_adc_value, NULL);
static DEVICE_ATTR(speed, S_IRUGO | S_IWUSR, show_adc_speed, store_adc_speed);

static struct device_attribute *dev_attrs[] = {
	&dev_attr_adin0,
	&dev_attr_adin1,
	&dev_attr_adin2,
	&dev_attr_speed,
	NULL,
};

static ssize_t show_adc_value(struct device *dev, struct device_attribute *attr, char *buf) {
	int i;

	if (device_is_open)
		return -EBUSY;

	for (i = 0; dev_attrs[i] != NULL; ++i)
		if (attr == dev_attrs[i])
			return scnprintf(buf, PAGE_SIZE, "%d", adc_single_shot(i));

	return -EINVAL;
}

/* Platform driver */

static struct platform_device *lpc32xx_adc_device;

static int __init lpc32xx_adc_probe(struct platform_device *pdev) {
	int i, err;

	/* Initialize core */
	err = adc_init();
	if (err != 0)
		return err;

	/* Register device attributes */
	for (i = 0; dev_attrs[i] != NULL; ++i) {
		err = device_create_file(&pdev->dev, dev_attrs[i]);
		if (err != 0) {
			printk(KERN_ERR "Error: Cannot create device file\n");
			while (--i >= 0)
				device_remove_file(&pdev->dev, dev_attrs[i]);
			adc_exit();
			return err;
		}
	}

	lpc32xx_adc_device = pdev;
	return 0;
}

static int __devexit lpc32xx_adc_remove(struct platform_device *pdev) {
	adc_exit();
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

	/* Register platform driver and platform device */
	pdev = platform_create_bundle(&lpc32xx_adc_driver, lpc32xx_adc_probe, NULL, 0, NULL, 0);
	if (IS_ERR(pdev)) {
		printk(KERN_ERR "Error: Cannot create platform bundle\n");
		return PTR_ERR(pdev);
	}

	/* Register character device */
	major = register_chrdev(0, DEVICE_NAME, &fops);
	if (major < 0) {
		printk(KERN_ERR "Error: Cannot register character device\n");
		platform_device_unregister(lpc32xx_adc_device);
		platform_driver_unregister(&lpc32xx_adc_driver);
		return major;
	}

	printk(KERN_INFO "Registered character device with major number %d\n", major);
	printk(KERN_INFO "Create a device file with: mknod /dev/%s c %d 0\n", DEVICE_NAME, major);

	return 0;
}

static void __exit lpc32xx_adc_exit(void) {
	/* Unregister character device */
	unregister_chrdev(major, DEVICE_NAME);

	/* Unregister platform driver and platform device */
	platform_device_unregister(lpc32xx_adc_device);
	platform_driver_unregister(&lpc32xx_adc_driver);
}

module_init(lpc32xx_adc_init);
module_exit(lpc32xx_adc_exit);

MODULE_AUTHOR("Tobi Vollebregt <tobivollebregt@gmail.com>");
MODULE_DESCRIPTION("LPC32XX ADC Driver");
MODULE_LICENSE("GPL");
