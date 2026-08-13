/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __MESON_TX_TASK_H
#define __MESON_TX_TASK_H

//#include <linux/workqueue.h>

#include "meson_tx_hw_task.h"

struct tx_task_manager;

#define TASK_FLAG_INIT   BIT(0)
#define TASK_FLAG_QUEUE  BIT(1)
#define TASK_FLAG_DELAY_WORK BIT(2)
#define TASK_FLAG_WORK   BIT(3)

#define QUEUE_NAME_MAX_LEN 64

typedef void (*tx_task_callback)(void *);

/*
 * Workqueue flags and constants.  For details, please refer to
 * Documentation/core-api/workqueue.rst.
 */
enum wq_flags {
	WQ_BH			= 1 << 0, /* execute in bottom half (softirq) context */
	WQ_UNBOUND		= 1 << 1, /* not bound to any cpu */
	WQ_FREEZABLE		= 1 << 2, /* freeze during suspend */
	WQ_MEM_RECLAIM		= 1 << 3, /* may be used for memory reclaim */
	WQ_HIGHPRI		= 1 << 4, /* high priority */
	WQ_CPU_INTENSIVE	= 1 << 5, /* cpu intensive workqueue */
	WQ_SYSFS		= 1 << 6, /* visible in sysfs, see workqueue_sysfs_register() */

	/*
	 * Per-cpu workqueues are generally preferred because they tend to
	 * show better performance thanks to cache locality.  Per-cpu
	 * workqueues exclude the scheduler from choosing the CPU to
	 * execute the worker threads, which has an unfortunate side effect
	 * of increasing power consumption.
	 *
	 * The scheduler considers a CPU idle if it doesn't have any task
	 * to execute and tries to keep idle cores idle to conserve power;
	 * however, for example, a per-cpu work item scheduled from an
	 * interrupt handler on an idle CPU will force the scheduler to
	 * execute the work item on that CPU breaking the idleness, which in
	 * turn may lead to more scheduling choices which are sub-optimal
	 * in terms of power consumption.
	 *
	 * Workqueues marked with WQ_POWER_EFFICIENT are per-cpu by default
	 * but become unbound if workqueue.power_efficient kernel param is
	 * specified.  Per-cpu workqueues which are identified to
	 * contribute significantly to power-consumption are identified and
	 * marked with this flag and enabling the power_efficient mode
	 * leads to noticeable power saving at the cost of small
	 * performance disadvantage.
	 *
	 * http://thread.gmane.org/gmane.linux.kernel/1480396
	 */
	WQ_POWER_EFFICIENT	= 1 << 7,

	__WQ_DESTROYING		= 1 << 15, /* internal: workqueue is destroying */
	__WQ_DRAINING		= 1 << 16, /* internal: workqueue is draining */
	__WQ_ORDERED		= 1 << 17, /* internal: workqueue is ordered */
	__WQ_LEGACY		= 1 << 18, /* internal: create*_workqueue() */

	/* BH wq only allows the following flags */
	__WQ_BH_ALLOWS		= WQ_BH | WQ_HIGHPRI,
};

enum task_queue_type {
	TASK_QUEUE_HPD,
	TASK_QUEUE_CORE,
	TASK_QUEUE_HPD_IRQ,
	TASK_QUEUE_TIMER,
	TASK_QUEUE_LOW,
	TASK_QUEUE_HIGH,
	TASK_QUEUE_SYSTEM,
	MAX_TASK_QUEUE,
};

struct tx_task_info {
	char *name;
	void *para;
	tx_task_callback fn;
	enum task_type type;
	enum task_queue_type queue_type;
	u32 flag;
	char *init_queue_name;
	char queue_name[QUEUE_NAME_MAX_LEN];
	u32 queue_flag;
};

struct tx_task_manager *tx_task_mgr_init(void);
void tx_task_mgr_release(struct tx_task_manager *mgr);
int tx_task_mgr_setup_task(struct tx_task_manager *mgr, struct tx_task_info *info, void *para);
int tx_task_mgr_queue_task(struct tx_task_manager *mgr, u32 type, u32 delay);
int tx_task_mgr_cancel_task(struct tx_task_manager *mgr, u32 type, bool sync);

#endif
