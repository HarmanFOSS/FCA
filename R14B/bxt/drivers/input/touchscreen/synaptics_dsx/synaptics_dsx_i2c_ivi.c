/*
 * Synaptics DSX touchscreen driver
 *
 * Copyright (C) 2012-2016 Synaptics Incorporated. All rights reserved.
 *
 * Copyright (C) 2012 Alexandra Chin <alexandra.chin@tw.synaptics.com>
 * Copyright (C) 2012 Scott Lin <scott.lin@tw.synaptics.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * INFORMATION CONTAINED IN THIS DOCUMENT IS PROVIDED "AS-IS," AND SYNAPTICS
 * EXPRESSLY DISCLAIMS ALL EXPRESS AND IMPLIED WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * AND ANY WARRANTIES OF NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS.
 * IN NO EVENT SHALL SYNAPTICS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, PUNITIVE, OR CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OF THE INFORMATION CONTAINED IN THIS DOCUMENT, HOWEVER CAUSED
 * AND BASED ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, AND EVEN IF SYNAPTICS WAS ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE. IF A TRIBUNAL OF COMPETENT JURISDICTION DOES
 * NOT PERMIT THE DISCLAIMER OF DIRECT DAMAGES OR ANY OTHER DAMAGES, SYNAPTICS'
 * TOTAL CUMULATIVE LIABILITY TO ANY PARTY SHALL NOT EXCEED ONE HUNDRED U.S.
 * DOLLARS.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/types.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/input/synaptics_dsx.h>
#include "synaptics_dsx_core.h"

#define SYN_I2C_RETRY_TIMES 10
/*
#define I2C_BURST_LIMIT 255
*/
/*
#define XFER_MSGS_LIMIT 8
*/

static unsigned char *wr_buf;

static struct synaptics_dsx_hw_interface hw_if;

static struct platform_device *synaptics_dsx_i2c_device;

#ifdef CONFIG_OF
static int parse_dt(struct device *dev, struct synaptics_dsx_board_data *bdata)
{
	int retval;
	u32 value;
	const char *name;
	struct property *prop;
	struct device_node *np = dev->of_node;

	bdata->irq_gpio = of_get_named_gpio_flags(np,
			"synaptics,irq-gpio", 0,
			(enum of_gpio_flags *)&bdata->irq_flags);

	retval = of_property_read_u32(np, "synaptics,irq-on-state",
			&value);
	if (retval < 0)
		bdata->irq_on_state = 0;
	else
		bdata->irq_on_state = value;

	retval = of_property_read_string(np, "synaptics,pwr-reg-name", &name);
	if (retval < 0)
		bdata->pwr_reg_name = NULL;
	else
		bdata->pwr_reg_name = name;

	retval = of_property_read_string(np, "synaptics,bus-reg-name", &name);
	if (retval < 0)
		bdata->bus_reg_name = NULL;
	else
		bdata->bus_reg_name = name;

	prop = of_find_property(np, "synaptics,power-gpio", NULL);
	if (prop && prop->length) {
		bdata->power_gpio = of_get_named_gpio_flags(np,
				"synaptics,power-gpio", 0, NULL);
		retval = of_property_read_u32(np, "synaptics,power-on-state",
				&value);
		if (retval < 0) {
			dev_err(dev, "%s: Unable to read synaptics,power-on-state property\n",
					__func__);
			return retval;
		} else {
			bdata->power_on_state = value;
		}
	} else {
		bdata->power_gpio = -1;
	}

	prop = of_find_property(np, "synaptics,power-delay-ms", NULL);
	if (prop && prop->length) {
		retval = of_property_read_u32(np, "synaptics,power-delay-ms",
				&value);
		if (retval < 0) {
			dev_err(dev, "%s: Unable to read synaptics,power-delay-ms property\n",
					__func__);
			return retval;
		} else {
			bdata->power_delay_ms = value;
		}
	} else {
		bdata->power_delay_ms = 0;
	}

	prop = of_find_property(np, "synaptics,reset-gpio", NULL);
	if (prop && prop->length) {
		bdata->reset_gpio = of_get_named_gpio_flags(np,
				"synaptics,reset-gpio", 0, NULL);
		retval = of_property_read_u32(np, "synaptics,reset-on-state",
				&value);
		if (retval < 0) {
			dev_err(dev, "%s: Unable to read synaptics,reset-on-state property\n",
					__func__);
			return retval;
		} else {
			bdata->reset_on_state = value;
		}
		retval = of_property_read_u32(np, "synaptics,reset-active-ms",
				&value);
		if (retval < 0) {
			dev_err(dev, "%s: Unable to read synaptics,reset-active-ms property\n",
					__func__);
			return retval;
		} else {
			bdata->reset_active_ms = value;
		}
	} else {
		bdata->reset_gpio = -1;
	}

	prop = of_find_property(np, "synaptics,reset-delay-ms", NULL);
	if (prop && prop->length) {
		retval = of_property_read_u32(np, "synaptics,reset-delay-ms",
				&value);
		if (retval < 0) {
			dev_err(dev, "%s: Unable to read synaptics,reset-delay-ms property\n",
					__func__);
			return retval;
		} else {
			bdata->reset_delay_ms = value;
		}
	} else {
		bdata->reset_delay_ms = 0;
	}

	prop = of_find_property(np, "synaptics,max-y-for-2d", NULL);
	if (prop && prop->length) {
		retval = of_property_read_u32(np, "synaptics,max-y-for-2d",
				&value);
		if (retval < 0) {
			dev_err(dev, "%s: Unable to read synaptics,max-y-for-2d property\n",
					__func__);
			return retval;
		} else {
			bdata->max_y_for_2d = value;
		}
	} else {
		bdata->max_y_for_2d = -1;
	}

	prop = of_find_property(np, "synaptics,swap-axes", NULL);
	bdata->swap_axes = prop > 0 ? true : false;

	prop = of_find_property(np, "synaptics,x-flip", NULL);
	bdata->x_flip = prop > 0 ? true : false;

	prop = of_find_property(np, "synaptics,y-flip", NULL);
	bdata->y_flip = prop > 0 ? true : false;

	prop = of_find_property(np, "synaptics,ub-i2c-addr", NULL);
	if (prop && prop->length) {
		retval = of_property_read_u32(np, "synaptics,ub-i2c-addr",
				&value);
		if (retval < 0) {
			dev_err(dev, "%s: Unable to read synaptics,ub-i2c-addr property\n",
					__func__);
			return retval;
		} else {
			bdata->ub_i2c_addr = (unsigned short)value;
		}
	} else {
		bdata->ub_i2c_addr = -1;
	}

	prop = of_find_property(np, "synaptics,cap-button-codes", NULL);
	if (prop && prop->length) {
		bdata->cap_button_map->map = devm_kzalloc(dev,
				prop->length,
				GFP_KERNEL);
		if (!bdata->cap_button_map->map)
			return -ENOMEM;
		bdata->cap_button_map->nbuttons = prop->length / sizeof(u32);
		retval = of_property_read_u32_array(np,
				"synaptics,cap-button-codes",
				bdata->cap_button_map->map,
				bdata->cap_button_map->nbuttons);
		if (retval < 0) {
			bdata->cap_button_map->nbuttons = 0;
			bdata->cap_button_map->map = NULL;
		}
	} else {
		bdata->cap_button_map->nbuttons = 0;
		bdata->cap_button_map->map = NULL;
	}

	prop = of_find_property(np, "synaptics,vir-button-codes", NULL);
	if (prop && prop->length) {
		bdata->vir_button_map->map = devm_kzalloc(dev,
				prop->length,
				GFP_KERNEL);
		if (!bdata->vir_button_map->map)
			return -ENOMEM;
		bdata->vir_button_map->nbuttons = prop->length / sizeof(u32);
		bdata->vir_button_map->nbuttons /= 5;
		retval = of_property_read_u32_array(np,
				"synaptics,vir-button-codes",
				bdata->vir_button_map->map,
				bdata->vir_button_map->nbuttons * 5);
		if (retval < 0) {
			bdata->vir_button_map->nbuttons = 0;
			bdata->vir_button_map->map = NULL;
		}
	} else {
		bdata->vir_button_map->nbuttons = 0;
		bdata->vir_button_map->map = NULL;
	}

	prop = of_find_property(np, "ti,i2c-addr", NULL);
	if (prop && prop->length) {
		retval = of_property_read_u32(np, "ti,i2c-addr",
				&value);
		if (retval < 0) {
			dev_err(dev, "%s: Unable to read ti,i2c-addr property\n",
					__func__);
			return retval;
		} else {
			bdata->serializer_i2c_addr = (unsigned short)value;
		}
	} else {
		bdata->serializer_i2c_addr = -1;
	}

	return 0;
}
#endif

static int synaptics_rmi4_i2c_alloc_buf(struct synaptics_rmi4_data *rmi4_data,
		unsigned int count)
{
	static unsigned int buf_size;

	if (count > buf_size) {
		if (buf_size)
			kfree(wr_buf);
		wr_buf = kzalloc(count, GFP_KERNEL);
		if (!wr_buf) {
			dev_err(rmi4_data->pdev->dev.parent,
					"%s: Failed to alloc mem for buffer\n",
					__func__);
			buf_size = 0;
			return -ENOMEM;
		}
		buf_size = count;
	}

	return 0;
}

static void synaptics_rmi4_i2c_check_addr(struct synaptics_rmi4_data *rmi4_data,
		struct i2c_client *i2c)
{
	if (hw_if.board_data->ub_i2c_addr == -1)
		return;

	if (hw_if.board_data->i2c_addr == i2c->addr)
		hw_if.board_data->i2c_addr = hw_if.board_data->ub_i2c_addr;
	else
		hw_if.board_data->i2c_addr = i2c->addr;

	return;
}

static int synaptics_rmi4_i2c_set_page(struct synaptics_rmi4_data *rmi4_data,
		unsigned short addr)
{
	int retval;
	unsigned char retry;
	unsigned char buf[PAGE_SELECT_LEN];
	unsigned char page;
	struct i2c_client *i2c = to_i2c_client(rmi4_data->pdev->dev.parent);
	struct i2c_msg msg[1];


	msg[0].addr = hw_if.board_data->i2c_addr;
	msg[0].flags = 0;
	msg[0].len = PAGE_SELECT_LEN;
	msg[0].buf = buf;

	page = ((addr >> 8) & MASK_8BIT);
	buf[0] = MASK_8BIT;
	buf[1] = page;

	if (page != rmi4_data->current_page) {
		for (retry = 0; retry < SYN_I2C_RETRY_TIMES; retry++) {
			if (i2c_transfer(i2c->adapter, msg, 1) == 1) {
				rmi4_data->current_page = page;
				retval = PAGE_SELECT_LEN;
				break;
			}
			dev_err(rmi4_data->pdev->dev.parent,
					"%s: I2C retry %d\n",
					__func__, retry + 1);
			msleep(20);

			if (retry == SYN_I2C_RETRY_TIMES / 2) {
				synaptics_rmi4_i2c_check_addr(rmi4_data, i2c);
				msg[0].addr = hw_if.board_data->i2c_addr;
			}
		}
	} else {
		retval = PAGE_SELECT_LEN;
	}

	return retval;
}

static int synaptics_rmi4_i2c_read(struct synaptics_rmi4_data *rmi4_data,
		unsigned short addr, unsigned char *data, unsigned int length)
{
	int retval;
	unsigned char retry;
	unsigned char buf;
#ifdef I2C_BURST_LIMIT
	unsigned int ii;
	unsigned int rd_msgs = ((length - 1) / I2C_BURST_LIMIT) + 1;
#else
	unsigned int rd_msgs = 1;
#endif
	unsigned char index = 0;
	unsigned char xfer_msgs;
	unsigned char remaining_msgs;
	unsigned short i2c_addr;
	unsigned short data_offset = 0;
	unsigned int remaining_length = length;
	struct i2c_client *i2c = to_i2c_client(rmi4_data->pdev->dev.parent);
	struct i2c_adapter *adap = i2c->adapter;
	struct i2c_msg msg[rd_msgs + 1];

	mutex_lock(&rmi4_data->rmi4_io_ctrl_mutex);

	retval = synaptics_rmi4_i2c_set_page(rmi4_data, addr);
	if (retval != PAGE_SELECT_LEN) {
		retval = -EIO;
		goto exit;
	}

	msg[0].addr = hw_if.board_data->i2c_addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &buf;

#ifdef I2C_BURST_LIMIT
	for (ii = 0; ii < (rd_msgs - 1); ii++) {
		msg[ii + 1].addr = hw_if.board_data->i2c_addr;
		msg[ii + 1].flags = I2C_M_RD;
		msg[ii + 1].len = I2C_BURST_LIMIT;
		msg[ii + 1].buf = &data[data_offset];
		data_offset += I2C_BURST_LIMIT;
		remaining_length -= I2C_BURST_LIMIT;
	}
#endif

	msg[rd_msgs].addr = hw_if.board_data->i2c_addr;
	msg[rd_msgs].flags = I2C_M_RD;
	msg[rd_msgs].len = (unsigned short)remaining_length;
	msg[rd_msgs].buf = &data[data_offset];

	buf = addr & MASK_8BIT;

	remaining_msgs = rd_msgs + 1;

	while (remaining_msgs) {
#ifdef XFER_MSGS_LIMIT
		if (remaining_msgs > XFER_MSGS_LIMIT)
			xfer_msgs = XFER_MSGS_LIMIT;
		else
			xfer_msgs = remaining_msgs;
#else
		xfer_msgs = remaining_msgs;
#endif
		for (retry = 0; retry < SYN_I2C_RETRY_TIMES; retry++) {
			retval = i2c_transfer(adap, &msg[index], xfer_msgs);
			if (retval == xfer_msgs)
				break;

			dev_err(rmi4_data->pdev->dev.parent,
					"%s: I2C retry %d\n",
					__func__, retry + 1);
			msleep(20);

			if (retry == SYN_I2C_RETRY_TIMES / 2) {
				synaptics_rmi4_i2c_check_addr(rmi4_data, i2c);
				i2c_addr = hw_if.board_data->i2c_addr;
				msg[0].addr = i2c_addr;
#ifdef I2C_BURST_LIMIT
				for (ii = 0; ii < (rd_msgs - 1); ii++)
					msg[ii + 1].addr = i2c_addr;
#endif
				msg[rd_msgs].addr = i2c_addr;
			}
		}

		if (retry == SYN_I2C_RETRY_TIMES) {
			dev_err(rmi4_data->pdev->dev.parent,
					"%s: I2C read over retry limit\n",
					__func__);
			retval = -EIO;
			goto exit;
		}

		remaining_msgs -= xfer_msgs;
		index += xfer_msgs;
	}

	retval = length;

exit:
	mutex_unlock(&rmi4_data->rmi4_io_ctrl_mutex);

	return retval;
}

static int synaptics_rmi4_i2c_write(struct synaptics_rmi4_data *rmi4_data,
		unsigned short addr, unsigned char *data, unsigned int length)
{
	int retval;
	unsigned char retry;
	struct i2c_client *i2c = to_i2c_client(rmi4_data->pdev->dev.parent);
	struct i2c_msg msg[1];

	retval = synaptics_rmi4_i2c_alloc_buf(rmi4_data, length + 1);
	if (retval < 0)
		return retval;

	mutex_lock(&rmi4_data->rmi4_io_ctrl_mutex);


	retval = synaptics_rmi4_i2c_set_page(rmi4_data, addr);
	if (retval != PAGE_SELECT_LEN) {
		retval = -EIO;
		goto exit;
	}

	msg[0].addr = hw_if.board_data->i2c_addr;
	msg[0].flags = 0;
	msg[0].len = (unsigned short)(length + 1);
	msg[0].buf = wr_buf;

	wr_buf[0] = addr & MASK_8BIT;
	retval = secure_memcpy(&wr_buf[1], length, &data[0], length, length);
	if (retval < 0) {
		dev_err(rmi4_data->pdev->dev.parent,
				"%s: Failed to copy data\n",
				__func__);
		goto exit;
	}

	for (retry = 0; retry < SYN_I2C_RETRY_TIMES; retry++) {
		if (i2c_transfer(i2c->adapter, msg, 1) == 1) {
			retval = length;
			break;
		}
		dev_err(rmi4_data->pdev->dev.parent,
				"%s: I2C retry %d\n",
				__func__, retry + 1);
		msleep(20);

		if (retry == SYN_I2C_RETRY_TIMES / 2) {
			synaptics_rmi4_i2c_check_addr(rmi4_data, i2c);
			msg[0].addr = hw_if.board_data->i2c_addr;
		}
	}

	if (retry == SYN_I2C_RETRY_TIMES) {
		dev_err(rmi4_data->pdev->dev.parent,
				"%s: I2C write over retry limit\n",
				__func__);
		retval = -EIO;
	}

exit:
	mutex_unlock(&rmi4_data->rmi4_io_ctrl_mutex);

	return retval;
}

static int i2c_read(struct i2c_client *client, u16 slave_addr,
		u8 *reg, unsigned int reg_len, u8 *val)
{
	int ret = 0;
	u8 *data = NULL;
	struct i2c_msg msg[2];
	unsigned int size = reg_len + 1u;

	if ((reg == NULL) || (val == NULL) || (reg_len <= 0u)) {
		printk("reg/val/reg_len is %02x/%02x/%d\n",
			*reg, *val, reg_len);
		return -EINVAL;
	}

	data = kzalloc(size, GFP_KERNEL);
	if (data == NULL)
		return -ENOSPC;

	(void)memcpy(data, reg, reg_len);
	(void)memset(msg, 0, sizeof(msg));

	msg[0].addr = (slave_addr);
	msg[0].flags = 0;
	msg[0].len = (__u16)reg_len;
	msg[0].buf = data;

	msg[1].addr = (slave_addr);
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = data + reg_len;

	//client->addr = (slave_addr);

	ret = i2c_transfer(client->adapter, msg, 2);

	*val = *(data + reg_len);
	kfree(data);
	data = NULL;
	printk("read dev/reg/val/ret is %02x/%02x/%02x/%d\n",
		slave_addr, *reg, *val, ret);

	return ret;
}

static int i2c_write(struct i2c_client *client, u16 slave_addr,
		u8 *reg, unsigned int reg_len, u8 *val)
{
	u8 *data = NULL;
	int ret = 0;
	struct i2c_msg msg;
	unsigned int size = reg_len + 1u;

	if ((reg == NULL) || (val == NULL) || (reg_len <= 0u)) {
		printk("reg/val/reg_len is %02x/%02x/%d\n",
			*reg, *val, reg_len);
		return -EINVAL;
	}

	data = kzalloc(size, GFP_KERNEL);
	if (data == NULL)
		return -ENOSPC;

	(void)memcpy(data, reg, reg_len);
	*(data + reg_len) = *val;

	(void)memset(&msg, 0, sizeof(msg));
	msg.addr = (slave_addr);
	msg.flags = 0;
	msg.len = (u16)size;
	msg.buf = data;

	//client->addr = (slave_addr);
	ret = i2c_transfer(client->adapter, &msg, 1);

	kfree(data);
	data = NULL;
	printk("write dev/reg/val/ret is %02x/%02x/%02x/%d\n",
		slave_addr, *reg, *val, ret);
	return ret;
}

static int serilizer_read_isr(struct synaptics_rmi4_data *rmi4_data)
{
	int retval;
	u8 serilizer_reg,reg_val;
	struct i2c_client *i2c = to_i2c_client(rmi4_data->pdev->dev.parent);

	//read ti947 ISR to clean interrupt
	serilizer_reg = 0xC7;
	retval = i2c_read(i2c, hw_if.board_data->serializer_i2c_addr, &serilizer_reg, 1, &reg_val);

	return retval;
}


static int synaptics_serilizer_init(struct i2c_client *i2c)
{
	int retval;
	u8 serilizer_reg,reg_val;


	//read serilizer device id
	serilizer_reg = 0x00;
	retval = i2c_read(i2c, hw_if.board_data->serializer_i2c_addr, &serilizer_reg, 1, &reg_val);
	printk("ti947 device id:0x%x\n",reg_val);

	//Set I2C pass-through mode enable.
	serilizer_reg = 0x03;
	reg_val = 0xDA;
	retval = i2c_write(i2c, hw_if.board_data->serializer_i2c_addr, &serilizer_reg, 1, &reg_val);

	//Set Slave ID.
	serilizer_reg = 0x07;
	reg_val = 0x40;
//	reg_val = 0x48;
	retval = i2c_write(i2c, hw_if.board_data->serializer_i2c_addr, &serilizer_reg, 1, &reg_val);

	//Set Slave Alias.
	serilizer_reg = 0x08;
	reg_val = 0x40;
	retval = i2c_write(i2c, hw_if.board_data->serializer_i2c_addr, &serilizer_reg, 1, &reg_val);


	//set dual pixel mode
	serilizer_reg = 0x4F;
	reg_val = 0x00;
	retval = i2c_write(i2c, hw_if.board_data->serializer_i2c_addr, &serilizer_reg, 1, &reg_val);

	//Pass through remote INTB_IN to INTB
	serilizer_reg = 0xC6;
	reg_val = 0x21;
	retval = i2c_write(i2c, hw_if.board_data->serializer_i2c_addr, &serilizer_reg, 1, &reg_val);

	/* Enable the communication over I2C Control (to deserializer and touch controller) */
	serilizer_reg = 0x17;
	reg_val = 0x9e;
	retval = i2c_write(i2c, hw_if.board_data->serializer_i2c_addr, &serilizer_reg, 1, &reg_val);

	/*Read from HDCP_ISR register to arm the interrupt for the first time*/
	serilizer_reg = 0xc7;
	retval = i2c_read(i2c, hw_if.board_data->serializer_i2c_addr, &serilizer_reg, 1, &reg_val);

	//Set 947 gpio1/2 remote hold mode
	serilizer_reg = 0x0E;
	reg_val = 0x55;
	retval = i2c_write(i2c, hw_if.board_data->serializer_i2c_addr, &serilizer_reg, 1, &reg_val);

	//read 948 de-serilizer device id
	serilizer_reg = 0x00;
	retval = i2c_read(i2c, hw_if.board_data->deserializer_i2c_addr, &serilizer_reg, 1, &reg_val);
	printk(KERN_WARNING"ti948 device id:0x%x, readbackid:0x%x\n",hw_if.board_data->deserializer_i2c_addr,reg_val);

	//Set 948 gpio1/2 input
	serilizer_reg = 0x1E;
	reg_val = 0x33;
	retval = i2c_write(i2c, hw_if.board_data->deserializer_i2c_addr, &serilizer_reg, 1, &reg_val);

	 /*Set 948, Change the color packing order - Fixes psychodelic colors on 1080p display (set this 949 is work --hubert,)*/
	/*serilizer_reg = 0x49;
	reg_val = 0x60;
	retval = i2c_write(i2c, hw_if.board_data->deserializer_i2c_addr, &serilizer_reg, 1, &reg_val);*/

	return retval;
}


static struct synaptics_dsx_bus_access bus_access = {
	.type = BUS_I2C,
	.read = synaptics_rmi4_i2c_read,
	.write = synaptics_rmi4_i2c_write,
	.read_isr = serilizer_read_isr,
};

static void synaptics_rmi4_i2c_dev_release(struct device *dev)
{
	kfree(synaptics_dsx_i2c_device);

	return;
}

static int synaptics_rmi4_i2c_probe(struct i2c_client *client,
		const struct i2c_device_id *dev_id)
{
	int retval;

	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&client->dev,
				"%s: SMBus byte data commands not supported by host\n",
				__func__);
		return -EIO;
	}

	synaptics_dsx_i2c_device = kzalloc(
			sizeof(struct platform_device),
			GFP_KERNEL);
	if (!synaptics_dsx_i2c_device) {
		dev_err(&client->dev,
				"%s: Failed to allocate memory for synaptics_dsx_i2c_device\n",
				__func__);
		return -ENOMEM;
	}

#ifdef CONFIG_OF
	if (client->dev.of_node) {
		hw_if.board_data = devm_kzalloc(&client->dev,
				sizeof(struct synaptics_dsx_board_data),
				GFP_KERNEL);
		if (!hw_if.board_data) {
			dev_err(&client->dev,
					"%s: Failed to allocate memory for board data\n",
					__func__);
			return -ENOMEM;
		}
		hw_if.board_data->cap_button_map = devm_kzalloc(&client->dev,
				sizeof(struct synaptics_dsx_button_map),
				GFP_KERNEL);
		if (!hw_if.board_data->cap_button_map) {
			dev_err(&client->dev,
					"%s: Failed to allocate memory for 0D button map\n",
					__func__);
			return -ENOMEM;
		}
		hw_if.board_data->vir_button_map = devm_kzalloc(&client->dev,
				sizeof(struct synaptics_dsx_button_map),
				GFP_KERNEL);
		if (!hw_if.board_data->vir_button_map) {
			dev_err(&client->dev,
					"%s: Failed to allocate memory for virtual button map\n",
					__func__);
			return -ENOMEM;
		}
		parse_dt(&client->dev, hw_if.board_data);
	}
#else
	hw_if.board_data = client->dev.platform_data;
#endif

	hw_if.bus_access = &bus_access;
	hw_if.board_data->i2c_addr = client->addr;

#if 0
	client->irq = gpiod_to_irq(gpio_to_desc(hw_if.board_data->irq_gpio));
	hw_if.board_data->irq = client->irq;
#endif


	synaptics_dsx_i2c_device->name = PLATFORM_DRIVER_NAME_IVI;
	synaptics_dsx_i2c_device->id = 0;
	synaptics_dsx_i2c_device->num_resources = 0;
	synaptics_dsx_i2c_device->dev.parent = &client->dev;
	synaptics_dsx_i2c_device->dev.platform_data = &hw_if;
	synaptics_dsx_i2c_device->dev.release = synaptics_rmi4_i2c_dev_release;

	retval = platform_device_register(synaptics_dsx_i2c_device);
	if (retval) {
		dev_err(&client->dev,
				"%s: Failed to register platform device\n",
				__func__);
		return -ENODEV;
	}

    #ifndef CONFIG_BJEVN60_VIRTUAL
	synaptics_serilizer_init(client);
    #endif

	return 0;
}

static int synaptics_rmi4_i2c_remove(struct i2c_client *client)
{
	platform_device_unregister(synaptics_dsx_i2c_device);

	return 0;
}

static const struct i2c_device_id synaptics_rmi4_id_table[] = {
	{I2C_DRIVER_NAME_IVI, 0},
	{},
};
MODULE_DEVICE_TABLE(i2c, synaptics_rmi4_id_table);

#ifdef CONFIG_OF
static struct of_device_id synaptics_rmi4_of_match_table[] = {
	{
		.compatible = "synaptics,dsx-i2c",
	},
	{},
};
MODULE_DEVICE_TABLE(of, synaptics_rmi4_of_match_table);
#else
#define synaptics_rmi4_of_match_table NULL
#endif

static struct i2c_driver synaptics_rmi4_i2c_driver = {
	.driver = {
		.name = I2C_DRIVER_NAME_IVI,
		.owner = THIS_MODULE,
		.of_match_table = synaptics_rmi4_of_match_table,
	},
	.probe = synaptics_rmi4_i2c_probe,
	.remove = synaptics_rmi4_i2c_remove,
	.id_table = synaptics_rmi4_id_table,
};

int synaptics_rmi4_bus_ivi_init(void)
{
	return i2c_add_driver(&synaptics_rmi4_i2c_driver);
}
EXPORT_SYMBOL(synaptics_rmi4_bus_init);

void synaptics_rmi4_bus_ivi_exit(void)
{
	kfree(wr_buf);

	i2c_del_driver(&synaptics_rmi4_i2c_driver);

	return;
}
EXPORT_SYMBOL(synaptics_rmi4_bus_exit);

MODULE_AUTHOR("Synaptics, Inc.");
MODULE_DESCRIPTION("Synaptics DSX I2C Bus Support Module");
MODULE_LICENSE("GPL v2");
