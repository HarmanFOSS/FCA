/*
 * Autogenerated file by GPU Top : https://github.com/rib/gputop
 * DO NOT EDIT manually!
 *
 *
 * Copyright (c) 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#include <linux/sysfs.h>

#include "i915_drv.h"
#include "i915_oa_hsw.h"

static const struct i915_oa_reg b_counter_config_render_basic[] = {
	{ _MMIO(0x2724), 0x00800000 },
	{ _MMIO(0x2720), 0x00000000 },
	{ _MMIO(0x2714), 0x00800000 },
	{ _MMIO(0x2710), 0x00000000 },
};

static const struct i915_oa_reg flex_eu_config_render_basic[] = {
};

static const struct i915_oa_reg mux_config_render_basic[] = {
	{ _MMIO(0x9840), 0x00000080 },
	{ _MMIO(0x253a4), 0x01600000 },
	{ _MMIO(0x25440), 0x00100000 },
	{ _MMIO(0x25128), 0x00000000 },
	{ _MMIO(0x2691c), 0x00000800 },
	{ _MMIO(0x26aa0), 0x01500000 },
	{ _MMIO(0x26b9c), 0x00006000 },
	{ _MMIO(0x2791c), 0x00000800 },
	{ _MMIO(0x27aa0), 0x01500000 },
	{ _MMIO(0x27b9c), 0x00006000 },
	{ _MMIO(0x2641c), 0x00000400 },
	{ _MMIO(0x25380), 0x00000010 },
	{ _MMIO(0x2538c), 0x00000000 },
	{ _MMIO(0x25384), 0x0800aaaa },
	{ _MMIO(0x25400), 0x00000004 },
	{ _MMIO(0x2540c), 0x06029000 },
	{ _MMIO(0x25410), 0x00000002 },
	{ _MMIO(0x25404), 0x5c30ffff },
	{ _MMIO(0x25100), 0x00000016 },
	{ _MMIO(0x25110), 0x00000400 },
	{ _MMIO(0x25104), 0x00000000 },
	{ _MMIO(0x26804), 0x00001211 },
	{ _MMIO(0x26884), 0x00000100 },
	{ _MMIO(0x26900), 0x00000002 },
	{ _MMIO(0x26908), 0x00700000 },
	{ _MMIO(0x26904), 0x00000000 },
	{ _MMIO(0x26984), 0x00001022 },
	{ _MMIO(0x26a04), 0x00000011 },
	{ _MMIO(0x26a80), 0x00000006 },
	{ _MMIO(0x26a88), 0x00000c02 },
	{ _MMIO(0x26a84), 0x00000000 },
	{ _MMIO(0x26b04), 0x00001000 },
	{ _MMIO(0x26b80), 0x00000002 },
	{ _MMIO(0x26b8c), 0x00000007 },
	{ _MMIO(0x26b84), 0x00000000 },
	{ _MMIO(0x27804), 0x00004844 },
	{ _MMIO(0x27884), 0x00000400 },
	{ _MMIO(0x27900), 0x00000002 },
	{ _MMIO(0x27908), 0x0e000000 },
	{ _MMIO(0x27904), 0x00000000 },
	{ _MMIO(0x27984), 0x00004088 },
	{ _MMIO(0x27a04), 0x00000044 },
	{ _MMIO(0x27a80), 0x00000006 },
	{ _MMIO(0x27a88), 0x00018040 },
	{ _MMIO(0x27a84), 0x00000000 },
	{ _MMIO(0x27b04), 0x00004000 },
	{ _MMIO(0x27b80), 0x00000002 },
	{ _MMIO(0x27b8c), 0x000000e0 },
	{ _MMIO(0x27b84), 0x00000000 },
	{ _MMIO(0x26104), 0x00002222 },
	{ _MMIO(0x26184), 0x0c006666 },
	{ _MMIO(0x26284), 0x04000000 },
	{ _MMIO(0x26304), 0x04000000 },
	{ _MMIO(0x26400), 0x00000002 },
	{ _MMIO(0x26410), 0x000000a0 },
	{ _MMIO(0x26404), 0x00000000 },
	{ _MMIO(0x25420), 0x04108020 },
	{ _MMIO(0x25424), 0x1284a420 },
	{ _MMIO(0x2541c), 0x00000000 },
	{ _MMIO(0x25428), 0x00042049 },
};

static ssize_t
show_render_basic_id(struct device *kdev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "1\n");
}

void
i915_perf_load_test_config_hsw(struct drm_i915_private *dev_priv)
{
	strncpy(dev_priv->perf.oa.test_config.uuid,
		"403d8832-1a27-4aa6-a64e-f5389ce7b212",
		UUID_STRING_LEN);
	dev_priv->perf.oa.test_config.id = 1;

	dev_priv->perf.oa.test_config.mux_regs = mux_config_render_basic;
	dev_priv->perf.oa.test_config.mux_regs_len = ARRAY_SIZE(mux_config_render_basic);

	dev_priv->perf.oa.test_config.b_counter_regs = b_counter_config_render_basic;
	dev_priv->perf.oa.test_config.b_counter_regs_len = ARRAY_SIZE(b_counter_config_render_basic);

	dev_priv->perf.oa.test_config.flex_regs = flex_eu_config_render_basic;
	dev_priv->perf.oa.test_config.flex_regs_len = ARRAY_SIZE(flex_eu_config_render_basic);

	dev_priv->perf.oa.test_config.sysfs_metric.name = "403d8832-1a27-4aa6-a64e-f5389ce7b212";
	dev_priv->perf.oa.test_config.sysfs_metric.attrs = dev_priv->perf.oa.test_config.attrs;

	dev_priv->perf.oa.test_config.attrs[0] = &dev_priv->perf.oa.test_config.sysfs_metric_id.attr;

	dev_priv->perf.oa.test_config.sysfs_metric_id.attr.name = "id";
	dev_priv->perf.oa.test_config.sysfs_metric_id.attr.mode = 0444;
	dev_priv->perf.oa.test_config.sysfs_metric_id.show = show_render_basic_id;
}
