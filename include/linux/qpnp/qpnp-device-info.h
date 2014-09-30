/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef QPNP_DEVICE_INFO_H
#define QPNP_DEVICE_INFO_H

#include <linux/errno.h>
#include <linux/qpnp/qpnp-adc.h>

typedef enum
{
  Z5S_HW_INVALID=0,
  	
	Z5S_HW_01AMB_B=1,
	Z5S_HW_01AMB_D,
	Z5S_HW_01AMB_C,

	Z5S_HW_01AMB_B_SIMPLIFY=1,
	Z5S_HW_P3,
	Z5S_HW_P2,

	Z5S_HW_01AMBC_A=1,
	Z5S_HW_01AMBC_C,
	Z5S_HW_01AMBC_B,

	Z5S_HW_MAX ,      // unknow, fail read
}hw_version_type;

typedef enum
{
	Z5S_PROJECT_INVALID=0,     
	Z5S_PROJECT_AMB,
	Z5S_PROJECT_NX503J_V4,
	Z5S_PROJECT_AMB_SIMPLIFY,	 //0.9V
	Z5S_PROJECT_AMBC,
	Z5S_PROJECT_MAX,
}project_version_type;

/*
 Notice:
   Added From The Last Index
*/
typedef enum
{
	DEVICE_INDEX_INVALID=0,
	DEVICE_01AMB_B_BCM4339, // Complete Version
  DEVICE_01AMB_C,
  DEVICE_01AMB_D,
  DEVICE_01AMB_B_WTR1605_L_EMMC_16_32, //Simple Version
  DEVICE_PCB2,
  DEVICE_PCB3,
  DEVICE_01AMBC_A_3680,
  DEVICE_01AMBC_B,
  DEVICE_01AMBC_C,   
  DEVICE_INDEX_MAX,
}device_index_type;

struct hardware_id_map_st {
	int low_mv;
	int high_mv;
	hw_version_type hw_type;
	char hw_ver[50];
	device_index_type device_index;
	char names[10];
};

struct project_id_map_st {
	int low_mv;
	int high_mv;
	project_version_type project_type;
	char project_ver[50];
};

void 
ztemt_get_hw_pcb_version(char* result);
void 
ztemt_get_project_version(char* result);
int  
ztemt_get_device_index(char* result);
int   
device_info_init(struct qpnp_vadc_chip* chip);
#endif
