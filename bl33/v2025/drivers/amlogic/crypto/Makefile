# SPDX-License-Identifier: GPL-2.0+

ifeq ($(CONFIG_AML_CRYPTO_V2), y)
obj-$(CONFIG_AML_CRYPTO) += hw_dma_core_v2.o
else
obj-$(CONFIG_AML_CRYPTO) += hw_dma_core.o
endif

obj-$(CONFIG_AML_CRYPTO) += hw_aes.o

obj-$(CONFIG_AML_CRYPTO) += hw_tdes.o

obj-$(CONFIG_AML_CRYPTO) += hw_sha.o
