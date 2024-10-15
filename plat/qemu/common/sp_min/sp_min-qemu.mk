#
# Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL32_SOURCES		+=	${PLAT_QEMU_COMMON_PATH}/sp_min/sp_min_setup.c		\
				${PLAT_QEMU_COMMON_PATH}/aarch32/plat_helpers.S		\
				${PLAT_QEMU_COMMON_PATH}/qemu_pm.c			\
				${PLAT_QEMU_COMMON_PATH}/topology.c			\
				drivers/gpio/gpio.c


BL32_SOURCES		+=	lib/cpus/aarch32/aem_generic.S		\
				lib/cpus/aarch32/cortex_a15.S

BL32_SOURCES		+=	plat/common/aarch32/platform_mp_stack.S \
				plat/common/plat_psci_common.c		\

include drivers/arm/gic/v2/gicv2.mk
include drivers/arm/gic/v3/gicv3.mk
BL32_SOURCES		+=	${QEMU_GIC_SOURCES}
