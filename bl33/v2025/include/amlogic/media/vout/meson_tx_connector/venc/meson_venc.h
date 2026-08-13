/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __MESON_TX_VENC_H
#define __MESON_TX_VENC_H

#include <linux/types.h>
#include <amlogic/media/vout/meson_tx_connector/meson_tx_format_para.h>

enum venc_type {
	VENC_ENCP,
	VENC_ENCL,
};

/* Original API */
int meson_venc_mode_set(u32 enc_index, u32 enc_type,
	enum venc_bist_type bist_type, void *para);
int meson_venc_bist_mode_set(u32 enc_index, enum venc_type enc_type,
	enum venc_bist_type bist_type);

int meson_venc_mode_check(u32 enc_index, void *para);
int meson_venc_mode_disable(u32 enc_index, u32 enc_type);

int meson_venc_init(void);

#endif
