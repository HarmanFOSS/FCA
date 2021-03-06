/*
 * Copyright (c) 2016, Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/spi/spidev.h>
#include <linux/spi/spi.h>
#include <linux/spi/pxa2xx_spi.h>
#include <linux/pwm.h>
#include <linux/gpio.h>
#include <linux/i2c/r1_display.h>
#ifdef CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_CORE
#include <linux/delay.h>
#include <linux/input/synaptics_dsx.h>
#endif
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include "camera_init.h"
#define DRVNAME			"apl-bjevn60-board"

#ifdef CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_CORE
/*947*/
#define BJEV_VTP_SERIALIZER_ADDR      0x1A
#define BJEV_VTP_I2C_ADAPTER	      4
#define VTP_TOUCH_PDB_GPIO		     (434+23)
#define VTP_TOUCH_INT_GPIO           (434+9)
#define FPGA_RESET_GPIO              (434+36)
#define VTP_TOUCH_INT_GPIO_NORMAL    (310+0)

/*949*/
/*#define BJEV_VTP_SERIALIZER_ADDR      0x0C
#define BJEV_VTP_I2C_ADAPTER	      6
#define VTP_TOUCH_PDB_GPIO	      (434+0)
#define VTP_TOUCH_INT_GPIO            (434+13)*/

#define BJEV_VTP_DESERIALIZER_ADDR     0x2C
#define BJEV_VTP_SYNAPTICS_ADDR        0x20
#endif

static struct pxa2xx_spi_chip chip_data = {
	.gpio_cs = -EINVAL,
	.dma_burst_size = 1,
};

static struct spi_board_info apl_spi_slaves[] = {
	{
		.modalias = "spidev",
		.max_speed_hz = 50000000,
		.bus_num = 1,
		.chip_select = 0,
		.controller_data = &chip_data,
		.mode = SPI_MODE_0,
	},
	{
		.modalias = "spidev",
		.max_speed_hz = 50000000,
		.bus_num = 1,
		.chip_select = 1,
		.controller_data = &chip_data,
		.mode = SPI_MODE_0,
	},
	{
		.modalias = "spidev",
		.max_speed_hz = 50000000,
		.bus_num = 2,
		.chip_select = 0,
		.controller_data = &chip_data,
		.mode = SPI_MODE_0,
	},
	{
		.modalias = "spidev",
		.max_speed_hz = 50000000,
		.bus_num = 2,
		.chip_select = 1,
		.controller_data = &chip_data,
		.mode = SPI_MODE_0,
	},
	{
#ifdef CONFIG_ELINA_EMERGENCY_KERNEL
		.modalias = "spidev",
		.max_speed_hz = 50000000,
		.bus_num = 3,
		.chip_select = 0,
		.controller_data = &chip_data,
		.mode = SPI_MODE_0,
#else
		.modalias = "dabplugin",
		.max_speed_hz = 2000000,
		.bus_num = 3,
		.chip_select = 0,
		.controller_data = &chip_data,
		.mode = SPI_MODE_1,
#endif
	},
	{
		.modalias = "spidev",
		.max_speed_hz = 50000000,
		.bus_num = 3,
		.chip_select = 1,
		.controller_data = &chip_data,
		.mode = SPI_MODE_0,
	},
	{
		.modalias = "spidev",
		.max_speed_hz = 50000000,
		.bus_num = 3,
		.chip_select = 2,
		.controller_data = &chip_data,
		.mode = SPI_MODE_0,
	}
};

#ifdef CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_CORE
static struct synaptics_dsx_button_map _tag_cap_button_map ={
		.nbuttons = 0,
		.map      = NULL,
};

static struct synaptics_dsx_button_map _tag_vir_button_map ={
		.nbuttons = 0,
		.map      = NULL,
};

static struct synaptics_dsx_board_data bjev_vtp_pdata = {
	.serializer_i2c_addr   = BJEV_VTP_SERIALIZER_ADDR,
	.deserializer_i2c_addr = BJEV_VTP_DESERIALIZER_ADDR,
	.irq_gpio              = VTP_TOUCH_INT_GPIO,
	.irq_flags             = IRQF_ONESHOT | IRQF_TRIGGER_LOW,
	.irq_on_state          = 0,
	.power_gpio            = -1,
	.power_delay_ms        = 200,
	.power_on_state        = 1,
	.reset_gpio            = -1,
	.reset_delay_ms        = 200,
	.reset_on_state        = 0,
	.reset_active_ms       = 20,
	.pwr_reg_name          = NULL,
	.bus_reg_name          = NULL,
	.max_y_for_2d          = -1,
	.swap_axes             = 0,
	.x_flip                = 0,
	.y_flip                = 0,
	.ub_i2c_addr           = -1,
	.cap_button_map        = &_tag_cap_button_map,
	.vir_button_map        = &_tag_vir_button_map,
	.pwr_reg_name          = NULL,
	.panel_x               = 1920,
	.panel_y               = 1080,
};

static struct i2c_board_info bjev_i2c_devs4[] __initdata = {
	{ I2C_BOARD_INFO("synaptics_dsx_i2c", BJEV_VTP_SYNAPTICS_ADDR),
	.platform_data = &bjev_vtp_pdata },
};

static int apl_i2c_board_setup(void)
{
	int ret;
	ret = i2c_register_board_info(BJEV_VTP_I2C_ADAPTER, bjev_i2c_devs4,
						ARRAY_SIZE(bjev_i2c_devs4));
	if (ret)
		pr_warn(DRVNAME "failed to register i2c (ret: %d)\n",ret);
	else
		pr_debug(DRVNAME "successfully registered the i2c slaves...\n");
	return ret;
}
#endif

static int apl_spi_board_setup(void)
{
	int ret = -1;

	/* Register the SPI devices */
	ret = spi_register_board_info(apl_spi_slaves,
				ARRAY_SIZE(apl_spi_slaves));
	if (ret)
		pr_warn(DRVNAME ": failed to register the SPI slaves...\n");
	else
		pr_debug(DRVNAME ": successfully registered the SPI slaves...\n");
	return ret;
}

static int __init apl_board_init(void)
{
	int ret;

	camera_init();

	pr_debug(DRVNAME ": registering APL SPI devices...\n");
	ret = apl_spi_board_setup();
	if (ret)
		goto exit;
#ifdef CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_CORE
	pr_debug(DRVNAME ": registering APL I2C devices...\n");
	ret = apl_i2c_board_setup();
#endif

exit:
	return ret;
}
arch_initcall(apl_board_init);

#define USB_CTR2	456
#define USB_PWE2	466

#define USB_CTR1	436
#define USB_PWE1	438

#define USB_OTG		437

static void usb_setup(void)
{
	gpio_request(USB_CTR2, "USB_CONFIG_CTR2");
	gpio_direction_output(USB_CTR2, 1);

	gpio_request(USB_PWE2, "USB_CONFIG_PWE2");
	gpio_direction_output(USB_PWE2, 1);
	gpio_export(USB_PWE2, 1);

	gpio_request(USB_OTG, "USB_CONFIG_OTG");
	gpio_direction_output(USB_OTG, 1);

	gpio_request(USB_CTR1, "USB_CONFIG_CTR1");
	gpio_direction_output(USB_CTR1, 1);

	gpio_request(USB_PWE1, "USB_CONFIG_PWE1");
	gpio_direction_output(USB_PWE1, 1);
	gpio_export(USB_PWE1, 1);
}

#define WLAN_PWD_GPIO		442
#define BT_RESET_GPIO		467

static void wifi_bt_setup(void)
{
	gpio_request(WLAN_PWD_GPIO, "WLAN_PD");
	gpio_direction_output(WLAN_PWD_GPIO, 1);
	gpio_export(WLAN_PWD_GPIO, 1);

	gpio_request(BT_RESET_GPIO, "BT_RESET");
	gpio_direction_output(BT_RESET_GPIO, 1);
	gpio_export(BT_RESET_GPIO, 1);
}

#ifdef CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_CORE
static void vtp_touch_setup(void)
{
	gpio_request(VTP_TOUCH_PDB_GPIO, "VTP_TOUCH_PDB");
	/*gpio_direction_output(VTP_TOUCH_PDB_GPIO, 0);
	mdelay(30);*/
	gpio_direction_output(VTP_TOUCH_PDB_GPIO, 1);
	gpio_export(VTP_TOUCH_PDB_GPIO, 1);

	gpio_request(FPGA_RESET_GPIO, "FPGA_RESET_GPIO");
	/*gpio_direction_output(FPGA_RESET_GPIO, 0);
	mdelay(30);*/
	gpio_direction_output(FPGA_RESET_GPIO, 1);
	gpio_export(FPGA_RESET_GPIO, 1);

	gpio_request(VTP_TOUCH_INT_GPIO_NORMAL, "VTP_TOUCH_INT_GPIO_NORMAL");
	gpio_direction_input(VTP_TOUCH_INT_GPIO_NORMAL);
	gpio_export(VTP_TOUCH_INT_GPIO_NORMAL, 1);
}
#endif

static int __init bjevn60_gpio_init(void)
{
	usb_setup();
	wifi_bt_setup();
#ifdef CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_CORE
	vtp_touch_setup();
#endif

	return 0;
}
subsys_initcall_sync(bjevn60_gpio_init);

MODULE_LICENSE("GPL v2");
