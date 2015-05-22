/*
 * irq-prober
 *
 * Copyright (C) 2015 Savoir-faire Linux, Inc.
 *
 * Authors:
 *     Sebastien Bourdelin <sebastien.bourdelin@gmail.com>
 *
 */
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>

#define MODULE_NAME "irq-prober"

static int delay = 50;
module_param(delay, int, 0);

static int irq = -1;
module_param(irq, int, 0);

static int gpio = -1;
module_param(gpio, int, 0);

static int share = 0;
module_param(share, int, 0);

static int dev_id;
static struct work_struct irq_wq;

void irq_autoprobe(void)
{
	unsigned long mask;
	int irq_detect = -1;

	mask = probe_irq_on();
	udelay(delay);
	irq_detect = probe_irq_off(mask);

	if (irq_detect == 0) {
		pr_info("%s: no irq detected using kernel autoprobe\n", MODULE_NAME);
	}
	/* if more than one line has been activated, the result is negative */
	else if (irq_detect < 0) {
		pr_info("%s: more than one irq detected using kernel autorpobe\n", MODULE_NAME);
	}

}

static void irq_workqueue(struct work_struct *work)
{
	pr_info("%s: IRQ %i handle successfully\n", MODULE_NAME, irq);
}

static irqreturn_t irq_handler(int irq, void *dev_id)
{
	schedule_work(&irq_wq);

	return IRQ_HANDLED;
}

static int __init irq_prober_init(void)
{
	int err;
	unsigned long mask;

	INIT_WORK(&irq_wq, irq_workqueue);

	/* if no irq has been chosen, we used the kernel irq
	 * autoprobe feature */
	if (irq < 0 && gpio < 0) {
		irq_autoprobe();
	}
	/* irq has been chosen */
	else {
		if (gpio >= 0) {
			err = gpio_request(gpio, MODULE_NAME);
			if (err) {
				pr_err("%s: failed to request GPIO %i\n", MODULE_NAME,
						gpio);
				return err;
			}

			err = gpio_direction_input(gpio);
			if (err) {
				pr_err("%s: failed to set GPIO %i direction\n",
						MODULE_NAME, gpio);
				goto err_gpio_request;
			}

			irq = gpio_to_irq(gpio);
			if (irq < 0) {
				pr_err("%s: failed to set GPIO %i as an IRQ\n", MODULE_NAME,
						gpio);
				err = 1;
				goto err_gpio_request;
			}
		}

		mask = IRQF_TRIGGER_PROBE;
		if (share) {
			mask |= IRQF_SHARED;
		}

		err = request_irq(irq, irq_handler, mask, MODULE_NAME, &dev_id);
		if (err) {
			pr_err("%s: failed to assigned IRQ %i (%i)\n", MODULE_NAME, irq,
					err);
			irq = -1;
		}

	}

	return 0;

err_gpio_request:
	gpio_free(gpio);
	return err;
}

static void __exit irq_prober_exit(void)
{
	if (irq >= 0) {
		free_irq(irq, &dev_id);
	}

	if (gpio >= 0) {
		gpio_free(gpio);
	}

	flush_scheduled_work();
}

module_init(irq_prober_init);
module_exit(irq_prober_exit);

MODULE_AUTHOR("Sebastien Bourdelin");
MODULE_DESCRIPTION("IRQ prober");
MODULE_LICENSE("GPL v2");
