/*
 * FPDLink Serializer driver
 *
 */
#ifndef __LINUX_r1_display_SER_H
#define __LINUX_r1_display_SER_H
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/platform_data/atmel_mxt_ts.h>

#if defined(CONFIG_GWM_BOARD) || defined(CONFIG_GWMV3_BOARD)
extern void ser_read_isr(void);
#endif

/* The platform data for the FPDLink Serializer driver */
struct r1_display_platform_data {
	struct mxt_platform_data mxt_platform_data;
};
#endif /* __LINUX_r1_display_SER_H */
