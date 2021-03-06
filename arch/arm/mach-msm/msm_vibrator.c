/* include/asm/mach-msm/htc_pwrsink.h
 *
 * Copyright (C) 2008 HTC Corporation.
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2011 Code Aurora Forum. All rights reserved.
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
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/hrtimer.h>
#include <linux/sched.h>
#include "pmic.h"
#include "timed_output.h"

#include <mach/msm_rpcrouter.h>

/* <DTS2010081400556 shenjinming 20100814 begin */
/*< DTS2010071902252 shenjinming 20100719 begin */
#ifdef CONFIG_HUAWEI_EVALUATE_POWER_CONSUMPTION 
#include <mach/msm_battery.h>
#endif
/* DTS2010071902252 shenjinming 20100719 end >*/
/* DTS2010081400556 shenjinming 20100814 end> */
/* < DTS2010080500080 luojianhong 201000817 begin*/
#include <linux/delay.h>
#define VIBRATOR_DELAY 20
#define VIBRATOR_MIN 50
/*  DTS2010080500080 luojianhong 201000817 end > */
/*<BU5D07918, sibingsong 20100415 begin*/
#define PM_LIBPROG      0x30000061
#ifndef CONFIG_HUAWEI_FEATURE_VIBRATOR
#if (CONFIG_MSM_AMSS_VERSION == 6220) || (CONFIG_MSM_AMSS_VERSION == 6225)
#define PM_LIBVERS      0xfb837d0b
#else
#define PM_LIBVERS      0x10001
#endif
#else
/*< DTS2012020306500 lijianzhao 20120204 begin */
#ifdef CONFIG_ARCH_MSM7X27
#define PM_LIBVERS  0x60001
#else
#define PM_LIBVERS	0x00030005
#endif
/* DTS2012020306500 lijianzhao 20120204 end >*/
#endif
/*<BU5D08979 sibingsong 20100429 begin*/
#ifndef CONFIG_HUAWEI_FEATURE_VIBRATOR
#define HTC_PROCEDURE_SET_VIB_ON_OFF	21
#define PMIC_VIBRATOR_LEVEL	(3000)
#else
#define HW_PROCEDURE_SET_VIB_ON_OFF	22
#define PMIC_VIBRATOR_LEVEL	(3000)
#endif
/*BU5D07918, sibingsong 20100415 end>*/
/*BU5D08979 sibingsong 20100429 end>*/
static struct work_struct work_vibrator_on;
static struct work_struct work_vibrator_off;
static struct hrtimer vibe_timer;
/* < DTS2010080500080 luojianhong 201000817 begin*/
#ifdef CONFIG_HUAWEI_SETTING_TIMER_FOR_VIBRATOR_OFF
static int time_value = 0;
#endif
/*  DTS2010080500080 luojianhong 201000817 end > */

static void set_pmic_vibrator(int on)
{
	static struct msm_rpc_endpoint *vib_endpoint;
    /* < DTS2012041806002 houming 20120504 begin*/	
	int ret=0;
	/*  DTS2012041806002 houming 20120504 end > */
	struct set_vib_on_off_req {
		struct rpc_request_hdr hdr;
/* < DTS2010080500080 luojianhong 201000817 begin*/
		#ifndef CONFIG_HUAWEI_SETTING_TIMER_FOR_VIBRATOR_OFF
		uint32_t data;
		#else
		uint32_t vib_volt;
		uint32_t vib_time;//vibratting time pass to modem .
		#endif
/*  DTS2010080500080 luojianhong 201000817 end > */
	} req;

	if (!vib_endpoint) {
		vib_endpoint = msm_rpc_connect(PM_LIBPROG, PM_LIBVERS, 0);
		if (IS_ERR(vib_endpoint)) {
			printk(KERN_ERR "init vib rpc failed!\n");
			vib_endpoint = 0;
			return;
		}
	}

/* < DTS2010080500080 luojianhong 201000817 begin*/
	if (on)
	{
		#ifndef CONFIG_HUAWEI_SETTING_TIMER_FOR_VIBRATOR_OFF
		req.data = cpu_to_be32(PMIC_VIBRATOR_LEVEL);
		#else
		req.vib_volt = cpu_to_be32(PMIC_VIBRATOR_LEVEL); 
		req.vib_time = cpu_to_be32(time_value); 
		#endif
	}
	else
	{
		#ifndef CONFIG_HUAWEI_SETTING_TIMER_FOR_VIBRATOR_OFF
		req.data = cpu_to_be32(0);
		#else
		req.vib_volt = cpu_to_be32(0); 
		req.vib_time = cpu_to_be32(0); 
		#endif
	}
/*  DTS2010080500080 luojianhong 201000817 end > */
/*<BU5D07918, sibingsong 20100416 begin*/
#ifndef CONFIG_HUAWEI_FEATURE_VIBRATOR
/*BU5D07918, sibingsong 20100416 end>*/
	msm_rpc_call(vib_endpoint, HTC_PROCEDURE_SET_VIB_ON_OFF, &req,
		sizeof(req), 5 * HZ);
/*<BU5D07918, sibingsong 20100416 begin*/
#else
    /* < DTS2012041806002 houming 20120504 begin*/
	/* Add return value to determine */
	ret=msm_rpc_call(vib_endpoint, HW_PROCEDURE_SET_VIB_ON_OFF, &req,
		sizeof(req), 5 * HZ);
	if(ret)
	{
		printk("%s:msm_rpc_call fail,ret=%d\n",__func__,ret);
	}
	/*  DTS2012041806002 houming 20120504 end > */
#endif
/*BU5D07918, sibingsong 20100416 end>*/
}

static void pmic_vibrator_on(struct work_struct *work)
{
	set_pmic_vibrator(1);
}

static void pmic_vibrator_off(struct work_struct *work)
{
	set_pmic_vibrator(0);
}

/* < DTS2010080500080 luojianhong 201000817 begin*/
#ifndef CONFIG_HUAWEI_SETTING_TIMER_FOR_VIBRATOR_OFF
static void timed_vibrator_on(struct timed_output_dev *sdev)
{
	schedule_work(&work_vibrator_on);
}

#endif
/*  DTS2010080500080 luojianhong 201000817 end > */
static void timed_vibrator_off(struct timed_output_dev *sdev)
{
	schedule_work(&work_vibrator_off);
}
/* < DTS2010080500080 luojianhong 201000817 begin*/
#ifndef CONFIG_HUAWEI_SETTING_TIMER_FOR_VIBRATOR_OFF
static void vibrator_enable(struct timed_output_dev *dev, int value)
{
	hrtimer_cancel(&vibe_timer);

	if (value == 0)
		timed_vibrator_off(dev);
	else {
		value = (value > 15000 ? 15000 : value);

		timed_vibrator_on(dev);

		hrtimer_start(&vibe_timer,
			      ktime_set(value / 1000, (value % 1000) * 1000000),
			      HRTIMER_MODE_REL);
	}
}
#else
static void vibrator_enable(struct timed_output_dev *dev, int value)
{
	time_value = value;//save this value as vibratting time
    /* < DTS2012041806002 houming 20120504 begin*/
    /* detele */
    /*  DTS2012041806002 houming 20120504 end > */
	if (value == 0)
	{
		mdelay(VIBRATOR_DELAY);
		//timed_vibrator_off(dev);
		pmic_vibrator_off(NULL);
	}
	else {
		value = (value > 15000 ? 15000 : value);
		value = (value < VIBRATOR_MIN ? VIBRATOR_MIN : value);

		//timed_vibrator_on(dev);
		pmic_vibrator_on(NULL);//use this function instead of timed_vibrator_on.
    /* < DTS2012041806002 houming 20120504 begin*/
    /* detele */
    /*  DTS2012041806002 houming 20120504 end > */
	}
}
#endif
/*  DTS2010080500080 luojianhong 201000817 end > */

void msm_timed_vibrate(int value) {
        vibrator_enable(0,value);
}


static int vibrator_get_time(struct timed_output_dev *dev)
{
	if (hrtimer_active(&vibe_timer)) {
		ktime_t r = hrtimer_get_remaining(&vibe_timer);
		struct timeval t = ktime_to_timeval(r);
		return t.tv_sec * 1000 + t.tv_usec / 1000;
	}
	return 0;
}

static enum hrtimer_restart vibrator_timer_func(struct hrtimer *timer)
{
	timed_vibrator_off(NULL);
	return HRTIMER_NORESTART;
}

static struct timed_output_dev pmic_vibrator = {
	.name = "vibrator",
	.get_time = vibrator_get_time,
	.enable = vibrator_enable,
};

void __init msm_init_pmic_vibrator(void)
{
	INIT_WORK(&work_vibrator_on, pmic_vibrator_on);
	INIT_WORK(&work_vibrator_off, pmic_vibrator_off);

	hrtimer_init(&vibe_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	vibe_timer.function = vibrator_timer_func;

	timed_output_dev_register(&pmic_vibrator);
}

MODULE_DESCRIPTION("timed output pmic vibrator device");
MODULE_LICENSE("GPL");

