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

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/hwmon.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/spmi.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/qpnp/qpnp-adc.h>
#include <linux/platform_device.h>
#include <linux/qpnp/qpnp-device-info.h>

#define QPNP_ZTEMT_HW_VERSION_DEV_NAME	"qcom,qpnp-ztemt_hw_version"

/**
 * struct qpnp_chg_chip - device information
 * @dev:			device pointer to access the parent
 * @spmi:			spmi pointer to access spmi information
 */
struct qpnp_ztemt_hw_version_chip {
	struct device	     *dev;
	struct spmi_device	 *spmi;
	struct work_struct	 work;
	struct mutex	     lock;
};

static struct qpnp_vadc_chip* g_vadc_dev = NULL;

static int debug_device_info_mask = 1;
module_param_named(debug_device_info_mask, debug_device_info_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);
#define DBG_DEVICE_INFO(x...) do {if (debug_device_info_mask) pr_info(">>ZTEMT_DEVICE_INFO>>  " x); } while (0)

//打开调试接口
#define DEBUG 
#undef KERN_DEBUG
#define KERN_DEBUG KERN_ERR

#undef KERN_INFO
#define KERN_INFO KERN_ERR

#define ADC_PROJECT_ID  LR_MUX5_AMUX_THM2
#define ADC_HW_ID       LR_MUX4_AMUX_THM1


#define PROJECT_ID_MAX   5
#define HRADWARE_ID_MAX  5
struct hardware_id_map_st hardware_id_map[PROJECT_ID_MAX][HRADWARE_ID_MAX] = {
  {
  	{0, 	0,		Z5S_HW_INVALID,	"unknow" ,DEVICE_INDEX_INVALID,"JD"},  //id_mv=0
	},
			//0V
	{
		{0, 	0,		Z5S_HW_INVALID,	"unknow" ,DEVICE_INDEX_INVALID,"JD"},  //id_mv=0
		{0, 	150,		Z5S_HW_01AMB_B,	"QB8974_01AMB_B" ,DEVICE_01AMB_B_BCM4339,"SC"},  //id_mv=0
		{750, 	1050,	Z5S_HW_01AMB_D,	"QB8974_01AMB_D", DEVICE_01AMB_D,"JD"},  //id_mv= 0.9V
		{1650, 	1950,	Z5S_HW_01AMB_C,	"QB8974_01AMB_C", DEVICE_01AMB_C,"JD"}  //id_mv= 1.8V
  },
  			//0.3V
	{
		{0, 	0,		Z5S_HW_INVALID,	"unknow" ,DEVICE_INDEX_INVALID,"JD"},  //id_mv=0
		{0, 	150,		Z5S_HW_01AMB_B,	"QB8974_01AMB_B" ,DEVICE_01AMB_B_BCM4339,"SC"},  //id_mv=0
		{750, 	1050,	Z5S_HW_01AMB_D,	"QB8974_01AMB_D", DEVICE_01AMB_D,"JD"},  //id_mv= 0.9V
		{1650, 	1950,	Z5S_HW_01AMB_C,	"QB8974_01AMB_C", DEVICE_01AMB_C,"JD"}  //id_mv= 1.8V
  },
  //0.9V
  {
  {0, 	0,		Z5S_HW_INVALID,	"unknow" ,DEVICE_INDEX_INVALID,"JD"},  //id_mv=0
		{0, 	150,		Z5S_HW_01AMB_B_SIMPLIFY,	"QB8974_01AMB_B_SIMPLIFY",DEVICE_01AMB_B_WTR1605_L_EMMC_16_32,"JD"},  //id_mv=0
		{750, 	1050,	Z5S_HW_P3,	"QB8974_P3",DEVICE_PCB3,"JD"},  //id_mv= 
		{1650, 	1950,	Z5S_HW_P2,	"QB8974_P2",DEVICE_PCB2,"JD"}  //id_mv=
  },
  ///1.8V
	{	
	 {0, 	0,		Z5S_HW_INVALID,	"unknow" ,DEVICE_INDEX_INVALID,"JD"},  //id_mv=0
		{0, 150,	Z5S_HW_01AMBC_A,		"QB8974_01AMBC_A", DEVICE_01AMBC_A_3680,"JD"},  //id_mv=0
		{750, 1050,		Z5S_HW_01AMBC_C,		"QB8974_01AMBC_C", DEVICE_01AMBC_C,"JD"},  //id_mv= 
		{1650, 1950,	Z5S_HW_01AMBC_B,		"QB8974_01AMBC_B",DEVICE_01AMBC_B,"JD"}  //id_mv=
	}
};

struct project_id_map_st project_id_map[] = {
	{0, 0,Z5S_PROJECT_INVALID,"unknow"},  //id_mv=0
	{0, 150,Z5S_PROJECT_AMB,"QB8974_01AMB"},  //id_mv=0
	{150, 450,Z5S_PROJECT_NX503J_V4,"NX503J_V4"},  //id_mv=0.3V
	{750, 1050,Z5S_PROJECT_AMB_SIMPLIFY,"QB8974_01AMB_SIMPLIFY"},  //id_mv= 0.9V	
	{1650, 1950,Z5S_PROJECT_AMBC,"QB8974_01AMBC"},  //id_mv= 1.8V
};

project_version_type ztemt_project_id = Z5S_PROJECT_INVALID;
hw_version_type ztemt_hw_id = Z5S_HW_INVALID;

static int32_t zte_qpnp_vadc_read(enum qpnp_vadc_channels channel,
				struct qpnp_vadc_result *result)
{
	if(!g_vadc_dev)
		goto fail_adc_enable;

	return qpnp_vadc_read(g_vadc_dev,channel,result);

	fail_adc_enable:
	return -EINVAL;
}

static int 
ztemt_get_project_type( struct project_id_map_st *pts,
		int tablesize, int input)
{
	int i = 0;
	if ( pts == NULL )
		return Z5S_PROJECT_INVALID;

	while (i < tablesize) {
		if ( (pts[i].low_mv*1000 <= input) &&( input <= pts[i].high_mv*1000) ) {
			DBG_DEVICE_INFO(" project_index = %d \n" , i);
			break;
			}
		else 
			i++;
	}

	if ( i < tablesize ) 
		return pts[i].project_type;
	else 
		return Z5S_PROJECT_INVALID;
	
}

int 
ztemt_get_project_id( void )
{
	int rc;
	int64_t project_id_uv = 0;
	static int get_project = 0;
	struct qpnp_vadc_result  adc_result;


	if(get_project && (ztemt_project_id != Z5S_PROJECT_INVALID))
		return ztemt_project_id;

	rc = zte_qpnp_vadc_read(ADC_PROJECT_ID, &adc_result);
	if (rc) {
		DBG_DEVICE_INFO("error reading channel = %d, rc = %d\n",ADC_PROJECT_ID, rc);
		return rc;
	}
	project_id_uv = adc_result.physical;

DBG_DEVICE_INFO("project_id_uv.physical = %lld \n", adc_result.physical);

	ztemt_project_id = ztemt_get_project_type(project_id_map,
			ARRAY_SIZE(project_id_map),	(project_id_uv));

	DBG_DEVICE_INFO("project_id_uv = %lld uv project_id=%d project_ver=%s\n",
		project_id_uv,
		ztemt_project_id ,
		project_id_map[ztemt_project_id].project_ver);

	get_project = 1;
	return ztemt_project_id;
}

static int 
ztemt_get_hardware_type( struct hardware_id_map_st pts[][HRADWARE_ID_MAX] , int project_id,
		int tablesize, int input)
{
	uint32_t i = 0;
	if ( pts == NULL )
		return Z5S_HW_INVALID ;

	while (i < tablesize) {
		if ( (pts[project_id][i].low_mv*1000 <= input) && (input <= pts[project_id][i].high_mv*1000) ) {
     	DBG_DEVICE_INFO(" pcb_index = %d \n" , i);
			break;
			}
		else 
			i++;
	}

	if ( i < tablesize ) 
		return pts[project_id][i].hw_type;
	else 
		return Z5S_HW_INVALID;
}

int 
ztemt_get_hw_id(int project_id)
{
	int rc;
	int64_t hw_id_uv = 0;
	static int get_hw = 0;
	struct qpnp_vadc_result  adc_result;

	if(get_hw &&( ztemt_hw_id !=Z5S_HW_INVALID))
		return ztemt_hw_id;

	rc = zte_qpnp_vadc_read(ADC_HW_ID, &adc_result);
	if (rc) {
		DBG_DEVICE_INFO("error reading channel  = %d, rc = %d\n",ADC_HW_ID, rc);
		return rc;
	}
	hw_id_uv = adc_result.physical;

DBG_DEVICE_INFO("hw_id_uv.physical = %lld \n", adc_result.physical);

	DBG_DEVICE_INFO(">> ZTEMT >> size = %d \n" , 
		ARRAY_SIZE(hardware_id_map[project_id] ));
	
	ztemt_hw_id = 
		ztemt_get_hardware_type( hardware_id_map,
			project_id,
			ARRAY_SIZE(hardware_id_map[project_id]),
			hw_id_uv );

	DBG_DEVICE_INFO("hw_id_uv=%lld uv hw_id=%d hw_ver=%s\n",
		hw_id_uv,
		ztemt_hw_id,
		hardware_id_map[project_id][ztemt_hw_id].hw_ver);

	get_hw = 1;
	return ztemt_hw_id;
}

/*
* Public Interface
*/

int ztemt_get_device_index(char* result)
{
	int hw_id, project_id;
	/* 默认为降成本版本*/
	device_index_type device_index = 
							DEVICE_01AMB_B_WTR1605_L_EMMC_16_32;
	
	project_id = ztemt_get_project_id();
	if(project_id != Z5S_PROJECT_INVALID)
	{
		hw_id = ztemt_get_hw_id( project_id );
		if(hw_id != Z5S_HW_INVALID){
			device_index = hardware_id_map[project_id][hw_id].device_index;
			}	
	}

	if(result){
			sprintf(result, "%d",device_index);
	}
	
	return device_index;
}

/**
Return Device Name
*/
void ztemt_get_device_name(char* result)
{
	int hw_id, project_id;
	if(!result)
		return;

	project_id = ztemt_get_project_id();
	if(project_id != Z5S_PROJECT_INVALID)
	{
		hw_id = ztemt_get_hw_id( project_id );
		if(hw_id != Z5S_HW_INVALID){
			sprintf(result, "%s",hardware_id_map[project_id][hw_id].names);
	}else
		sprintf(result, "%s","unknow");
	}
	else
		sprintf(result, "%s","unknow");

	 return ;
}

void 
ztemt_get_hw_pcb_version(char* result)
{
	int hw_id, project_id;
	if(!result)
		return;

	project_id = ztemt_get_project_id();
	if(project_id != Z5S_PROJECT_INVALID)
	{
		hw_id = ztemt_get_hw_id( project_id );
		if(hw_id != Z5S_HW_INVALID){
			sprintf(result, "%s",hardware_id_map[project_id][hw_id].hw_ver);
	}else
		sprintf(result, "%s","unknow");
	}
	else
		sprintf(result, "%s","unknow");
}
EXPORT_SYMBOL(ztemt_get_hw_pcb_version);

void 
ztemt_get_project_version(char* result)
{
	int project_id;
	if(!result)
		return;

	project_id = ztemt_get_project_id();
	if(project_id != Z5S_PROJECT_INVALID){
		sprintf(result, "%s",project_id_map[project_id].project_ver);
	}else
		sprintf(result, "%s","unknow");
}
EXPORT_SYMBOL(ztemt_get_project_version);

static ssize_t  projectid_info_show(struct device *dev, 
		struct device_attribute *attr, char *buf)
{
	char *tmp = buf;
	ztemt_get_project_version(buf);
	while ((*tmp++ ) != '\0');
	return (tmp - buf);
}
static ssize_t projectid_info_store(struct device *dev, 
		struct device_attribute *attr, const char *buf, size_t count)
{
     return count;
}
static DEVICE_ATTR(project_info, 0444, projectid_info_show, projectid_info_store);

static ssize_t  hardwareid_pcb_info_show(struct device *dev, 
		struct device_attribute *attr, char *buf)
{
	char *tmp = buf;
	ztemt_get_hw_pcb_version(buf);
	while ((*tmp++ ) != '\0');
	return (tmp - buf);
}
static ssize_t hardwareid_pcb_store(struct device *dev, 
		struct device_attribute *attr, const char *buf, size_t count)
{
     return count;
}
static DEVICE_ATTR(pcb_info, 0444, hardwareid_pcb_info_show, hardwareid_pcb_store);

static ssize_t  device_index_show(struct device *dev, 
		struct device_attribute *attr, char *buf)
{
		ztemt_get_device_index(buf);
		return 1;
}
static ssize_t device_index_store(struct device *dev, 
		struct device_attribute *attr, const char *buf, size_t count)
{
     return count;
}
static DEVICE_ATTR(index, 0444, device_index_show, device_index_store);

/*
* /sys/ztemt_device_info
*/
static struct kobject * device_info_kobject = NULL;
int device_info_init(struct qpnp_vadc_chip * vadc_dev )
{
	int ret;

	g_vadc_dev = vadc_dev;
	
	device_info_kobject = kobject_create_and_add("ztemt_device_info", NULL);
	if(device_info_kobject == NULL) {
		ret = -ENOMEM;
		goto err1;
	}

	ret = sysfs_create_file(device_info_kobject, &dev_attr_project_info.attr);
	if(ret){
		goto err;
	}
	
	ret = sysfs_create_file(device_info_kobject, &dev_attr_pcb_info.attr);
	if(ret){
		goto err;
	}

	ret = sysfs_create_file(device_info_kobject, &dev_attr_index.attr);
	if(ret){
		goto err;
	}
	return 0;

err:
	kobject_del(device_info_kobject);
err1:
	DBG_DEVICE_INFO("DeviceInfo: Failed To Create Device Info Sys File \n");
	return ret;
}
EXPORT_SYMBOL(device_info_init);

/**
  For Compatile Other Project
*/
static ssize_t ztemt_device_name_store(struct device *dev, 
		struct device_attribute *attr, const char *buf, size_t count)
{
     return count;
}

static ssize_t ztemt_device_name_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
//	char *tmp = buf;
	ztemt_get_device_name(buf);
	//while ((*tmp++ ) != '\0');
	//return (tmp - buf);
	  return sprintf(buf,"%s\n",buf);
}

static DEVICE_ATTR(ztemt_hw_sc, 0444, ztemt_device_name_show, ztemt_device_name_store);

static int __devinit
qpnp_ztemt_hw_version_probe(struct spmi_device *spmi)
{
	
	struct qpnp_ztemt_hw_version_chip	*chip;
	int rc = 0;

   printk("Enter HW Device Module\n");
   
	chip = devm_kzalloc(&spmi->dev,
			sizeof(struct qpnp_ztemt_hw_version_chip), GFP_KERNEL);
	if (chip == NULL) {
		pr_err("qpnp_ztemt_hw_version_probe : kzalloc() failed.\n");
		return -ENOMEM;
	}
    dev_set_drvdata(&spmi->dev, chip);
	chip->dev = &(spmi->dev);
	chip->spmi = spmi;
	
    rc=sysfs_create_file(&chip->dev->kobj,&dev_attr_ztemt_hw_sc.attr);
	return rc;
}

static int __devexit
qpnp_ztemt_hw_version_remove(struct spmi_device *spmi)
{
	struct qpnp_ztemt_hw_version_chip *chip = dev_get_drvdata(&spmi->dev);
    devm_kfree(&spmi->dev,chip);
	return 0;
}

static struct of_device_id qpnp_ztemt_hw_version_match_table[] = {
	{ .compatible = QPNP_ZTEMT_HW_VERSION_DEV_NAME, },
	{}
};

static struct spmi_driver qpnp_ztemt_hw_version_driver = {
	.probe		= qpnp_ztemt_hw_version_probe,
	.remove		= __devexit_p(qpnp_ztemt_hw_version_remove),
	.driver		= {
		.name		    = QPNP_ZTEMT_HW_VERSION_DEV_NAME,
		.owner		    = THIS_MODULE,
		.of_match_table	= qpnp_ztemt_hw_version_match_table,
	},
};

int __init
qpnp_ztemt_hw_version_init(void)
{
	return spmi_driver_register(&qpnp_ztemt_hw_version_driver);
}
module_init(qpnp_ztemt_hw_version_init);

static void __exit
qpnp_ztemt_hw_version_exit(void)
{
	spmi_driver_unregister(&qpnp_ztemt_hw_version_driver);
}
module_exit(qpnp_ztemt_hw_version_exit);

MODULE_DESCRIPTION("qpnp ztemt_hw_version driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" QPNP_ZTEMT_HW_VERSION_DEV_NAME);

