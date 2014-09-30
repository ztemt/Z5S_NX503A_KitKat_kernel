/*
 * Copyright (C) 2009 Google, Inc.
 * Copyright (C) 2009 HTC Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/rfkill.h>
#include <asm/gpio.h>
#include <asm/mach-types.h>

//#include "board-mahimahi.h"

static struct rfkill *btrfk;
static const char btname[] = "bcm4339";

#define GPIO_BT_RESET_N 69

static int bluetooth_set_power(void *data, bool blocked)
{
	printk("%s(), line: %d , rfkill blocked %d\n",__func__, __LINE__, blocked);
	if (!blocked) {
        printk("%s(), line: %d , set GPIO_BT_RESET_N to 1\n",__func__, __LINE__);
 		gpio_direction_output(GPIO_BT_RESET_N, 1);
	} else {
	printk("%s(), line: %d , set GPIO_BT_RESET_N to 0\n",__func__, __LINE__);
		gpio_direction_output(GPIO_BT_RESET_N, 0);
	}

    printk("%s(), LINE: %d , gpio set value to : %d \n", __FUNCTION__, __LINE__,  gpio_get_value(GPIO_BT_RESET_N));
	return 0;
}

static struct rfkill_ops bluetooth_rfkill_ops = {
	.set_block = bluetooth_set_power,
};

static int mahimahi_rfkill_probe(struct platform_device *pdev)
{
	int rc = 0;
	bool default_state = true;  /* off */

	printk("%s(), LINE: %d \n", __FUNCTION__, __LINE__);
	rc = gpio_request(GPIO_BT_RESET_N, "bt_reset_4339");
	if (rc){
        printk("%s(), LINE: %d , gpio_request GPIO_BT_RESET_N failed \n", __FUNCTION__, __LINE__);
		goto err_gpio_reset;
       }

	gpio_direction_output(GPIO_BT_RESET_N, 0);
      printk("%s(), LINE: %d , gpio set value to : %d \n", __FUNCTION__, __LINE__,  gpio_get_value(GPIO_BT_RESET_N));
	bluetooth_set_power(NULL, default_state);

	btrfk = rfkill_alloc(btname, &pdev->dev, RFKILL_TYPE_BLUETOOTH,
				&bluetooth_rfkill_ops, NULL);
	if (!btrfk) {
		rc = -ENOMEM;
		goto err_rfkill_alloc;
	}

	rfkill_set_states(btrfk, default_state, false);

	/* userspace cannot take exclusive control */

	rc = rfkill_register(btrfk);
	if (rc)
		goto err_rfkill_reg;

	return 0;

err_rfkill_reg:
	rfkill_destroy(btrfk);
err_rfkill_alloc:
	//gpio_free(MAHIMAHI_GPIO_BT_SHUTDOWN_N);
//err_gpio_shutdown:
	gpio_free(GPIO_BT_RESET_N);
err_gpio_reset:
	return rc;
}

static int mahimahi_rfkill_remove(struct platform_device *dev)
{
	rfkill_unregister(btrfk);
	rfkill_destroy(btrfk);
	gpio_free(GPIO_BT_RESET_N);

	return 0;
}

#define BCM4339_PLATFORM_DEVICE_NAME "bcm4339_rfkill"

static struct platform_driver mahimahi_rfkill_driver = {
	.probe = mahimahi_rfkill_probe,
	.remove = mahimahi_rfkill_remove,
	.driver = {
		.name = BCM4339_PLATFORM_DEVICE_NAME,
		.owner = THIS_MODULE,
	},
};

struct platform_device bcm4339_rfkill_dev = {
	.name		= BCM4339_PLATFORM_DEVICE_NAME,
	.id		= -1,
};

static int __init mahimahi_rfkill_init(void)
{
	printk("%s", __FUNCTION__);
	platform_device_register(&bcm4339_rfkill_dev);
	return platform_driver_register(&mahimahi_rfkill_driver);
}

static void __exit mahimahi_rfkill_exit(void)
{
	platform_driver_unregister(&mahimahi_rfkill_driver);
}

module_init(mahimahi_rfkill_init);
module_exit(mahimahi_rfkill_exit);
MODULE_DESCRIPTION("mahimahi rfkill");
MODULE_AUTHOR("Nick Pelly <npelly@google.com>");
MODULE_LICENSE("GPL");
