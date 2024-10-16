/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv2.h>
#include <drivers/console.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include "../qemu_private.h"

#define MAP_BL32_TOTAL		MAP_REGION_FLAT(			\
					BL_CODE_BASE,			\
					BL32_END - BL_CODE_BASE,		\
					MT_MEMORY | MT_RW | EL3_PAS)
#define MAP_BL32_RO		MAP_REGION_FLAT(			\
					BL_CODE_BASE,			\
					BL_CODE_END - BL_CODE_BASE,	\
					MT_CODE | EL3_PAS),		\
				MAP_REGION_FLAT(			\
					BL_RO_DATA_BASE,		\
					BL_RO_DATA_END			\
						- BL_RO_DATA_BASE,	\
					MT_RO_DATA | EL3_PAS)

#define MAP_BL_COHERENT_RAM	MAP_REGION_FLAT(			\
					BL_COHERENT_RAM_BASE,		\
					BL_COHERENT_RAM_END		\
						- BL_COHERENT_RAM_BASE,	\
					MT_DEVICE | MT_RW | EL3_PAS)

#if RESET_TO_SP_MIN
#error qemu does not support RESET_TO_SP_MIN
#endif

static entry_point_info_t bl33_image_ep_info;

static const interrupt_prop_t stih410_interrupt_props[] = {
	PLATFORM_G1S_PROPS(GICV2_INTR_GROUP0),
	PLATFORM_G0_PROPS(GICV2_INTR_GROUP0)
};

static unsigned int target_mask_array[PLATFORM_CORE_COUNT];

static const struct gicv2_driver_data plat_gicv2_driver_data = {
	.gicd_base = GICD_BASE,
	.gicc_base = GICC_BASE,
	.interrupt_props = stih410_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(stih410_interrupt_props),
	.target_masks = target_mask_array,
	.target_masks_num = ARRAY_SIZE(target_mask_array),
};

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *sp_min_plat_get_bl33_ep_info(void)
{
	entry_point_info_t *next_image_info = &bl33_image_ep_info;

	/*
	 * None of the images on the ARM development platforms can have 0x0
	 * as the entrypoint
	 */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

void sp_min_early_platform_setup2(u_register_t arg0, u_register_t arg1,
		u_register_t arg2, u_register_t arg3)
{
	bl_params_t *params_from_bl2 = (bl_params_t *)arg0;

	/* Initialize the console to provide early debug support */
	qemu_console_init();

	ERROR("qemu sp_min, console init\n");
	/*
	 * Check params passed from BL2
	 */
	assert(params_from_bl2);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	bl_params_node_t *bl_params = params_from_bl2->head;

	/*
	 * Copy BL33 entry point information from BL2's address space.
	 */
	while (bl_params) {
		if (bl_params->image_id == BL33_IMAGE_ID)
			bl33_image_ep_info = *bl_params->ep_info;

		bl_params = bl_params->next_params_info;
	}

	if (!bl33_image_ep_info.pc)
		panic();
}

void sp_min_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_BL32_TOTAL,
		MAP_BL32_RO,
		MAP_BL_COHERENT_RAM,
		{0}
	};

	setup_page_tables(bl_regions, plat_qemu_get_mmap());

	enable_mmu_svc_mon(0);
}

void sp_min_platform_setup(void)
{
	/* Initialize the gic cpu and distributor interfaces */
	gicv2_driver_init(&plat_gicv2_driver_data);
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

unsigned int plat_get_syscnt_freq2(void)
{
	return read_cntfrq_el0();
}

void sp_min_plat_fiq_handler(uint32_t id)
{
	VERBOSE("[sp_min] interrupt #%d\n", id);
}
