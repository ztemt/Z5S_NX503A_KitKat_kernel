#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/sysdev.h>
#include <linux/timer.h>
#include <linux/err.h>
#include <linux/ctype.h>
#include "mdss_mdp.h"

#include "zte_disp_enhance.h"

//#define ZTE_DISP_ENHANCE_DEBUG

#define ZTE_SHARP_ENHANCE_CMD_COUNT 1

static struct zte_enhance_type zte_enhance_val = {
	.en_saturation =1,
	.saturation = INTENSITY_01,
	.colortmp =  INTENSITY_01,
#if defined (CONFIG_ZTEMT_MIPI_1080P_R63417_SHARP_IPS_5P5) || \
	defined (CONFIG_ZTEMT_MIPI_2K_R63419_SHARP_IPS_5P5) || \
	defined (CONFIG_ZTEMT_MIPI_1080P_R63311_SHARP_IPS_5P0_NX507J) || \
	defined (CONFIG_ZTEMT_MIPI_1080P_R63311_SHARP_IPS_6P4)
	.en_colortmp = 1,
#else
	.en_colortmp = 0,
#endif

};

static struct mdss_dsi_ctrl_pdata *zte_mdss_dsi_ctrl = NULL;

/*mdp adjust mayu add*/

struct mdp_pcc_cfg_data zte_pcc_cfg_warm = {
	.block = 0x10,
	.ops = 0x5,
    {
      .c = 0,
      .r = 0x8800,
      .g = 0,
      .b = 0,
      .rr = 0,
      .gg = 0,
      .bb = 0,
      .rg = 0,
      .gb = 0,
      .rb = 0,
      .rgb_0 = 0,
      .rgb_1 = 0
    },
    {
      .c = 0,
      .r = 0,
      .g = 0x8000,
      .b = 0,
      .rr = 0,
      .gg = 0,
      .bb = 0,
      .rg = 0,
      .gb = 0,
      .rb = 0,
      .rgb_0 = 0,
      .rgb_1 = 0
    },
    {
      .c = 0,
      .r = 0,
      .g = 0,
      .b = 0x8000,
      .rr = 0,
      .gg = 0,
      .bb = 0,
      .rg = 0,
      .gb = 0,
      .rb = 0,
      .rgb_0 = 0,
      .rgb_1 = 0
    },

};

struct mdp_pcc_cfg_data zte_pcc_cfg_normal = {
	.block = 0x10,
	.ops = 0x5,
    {
      .c = 0,
      .r = 0x8000,
      .g = 0,
      .b = 0,
      .rr = 0,
      .gg = 0,
      .bb = 0,
      .rg = 0,
      .gb = 0,
      .rb = 0,
      .rgb_0 = 0,
      .rgb_1 = 0
    },
    {
      .c = 0,
      .r = 0,
      .g = 0x8000,
      .b = 0,
      .rr = 0,
      .gg = 0,
      .bb = 0,
      .rg = 0,
      .gb = 0,
      .rb = 0,
      .rgb_0 = 0,
      .rgb_1 = 0
    },
    {
      .c = 0,
      .r = 0,
      .g = 0,
      .b = 0x8000,
      .rr = 0,
      .gg = 0,
      .bb = 0,
      .rg = 0,
      .gb = 0,
      .rb = 0,
      .rgb_0 = 0,
      .rgb_1 = 0
    },
};
struct mdp_pcc_cfg_data zte_pcc_cfg_cool = {
	.block = 0x10,
	.ops = 0x5,
    {
      .c = 0,
      .r = 0x7800,
      .g = 0,
      .b = 0,
    },
    {
      .c = 0,
      .r = 0,
      .g = 0x8000,
      .b = 0,
    },
    {
      .c = 0,
      .r = 0,
      .g = 0,
      .b = 0x8000,
    },
};

struct mdp_pcc_cfg_data zte_pcc_cfg_cool1 = {
	.block = 0x10,
	.ops = 0x5,
    {
      .c = 0,
      .r = 0x7000, 
      .g = 0,
      .b = 0,
    },
    {
      .c = 0,
      .r = 0,
      .g = 0x8000,
      .b = 0,
    },
    {
      .c = 0,
      .r = 0,
      .g = 0,
      .b = 0x8000,
    },
};

struct zte_enhance_type zte_get_lcd_enhance_param(void)
{
	return zte_enhance_val;
}

#if defined(CONFIG_ZTEMT_MIPI_1080P_R63311_SHARP_IPS_5P0)
//NX503A NX507J
static char sharpca_basic_9b[] = {0xca, 0x01,0x80, 0x98,0x98,0x9b,0x40,0xbe,0xbe,0x20,0x20,
	0x80,0xfe,0x0a, 0x4a,0x37,0xa0,0x55,0xf8,0x0c,0x0c,0x20,0x10,0x3f,0x3f,0x00,
	0x00,0x10,0x10,0x3f,0x3f,0x3f,0x3f,
};
static struct dsi_cmd_desc display_glow_cmd = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(sharpca_basic_9b)}, sharpca_basic_9b

};

static char sharpca_basic_92[] = {0xca, 0x01,0x80, 0x92,0x92,0x9b,0x75,0x9b,0x9b,0x20,0x20,
	0x80,0xfe,0x0a, 0x4a,0x37,0xa0,0x55,0xf8,0x0c,0x0c,0x20,0x10,0x3f,0x3f,0x00,
	0x00,0x10,0x10,0x3f,0x3f,0x3f,0x3f,
};

static struct dsi_cmd_desc display_std_cmd = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(sharpca_basic_92)}, sharpca_basic_92

};
static char sharpca_basic_norm[] = {0xca, 0x00,0x80, 0x80,0x80,0x80,0x80,0x80,0x80,0x08,0x20,
	0x80,0x80,0x0a, 0x4a,0x37,0xa0,0x55,0xf8,0x0c,0x0c,0x20,0x10,0x3f,0x3f,0x00,
	0x00,0x10,0x10,0x3f,0x3f,0x3f,0x3f,
};

static struct dsi_cmd_desc display_soft_cmd = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(sharpca_basic_norm)}, sharpca_basic_norm

};
	
#elif defined(CONFIG_ZTEMT_MIPI_1080P_R63311_SHARP_IPS_5P5)
//NX504J
static char sharpca_basic_9b[] = {
	0xca, 0x01, 0x80, 0x9c, 0x9b, 0x9b, 0x40, 0xbe, 0xbe, 0x20,
	0x20, 0x80, 0xfe, 0x0a, 0x4a, 0x37, 0xa0, 0x55, 0xf8, 0x0c,
	0x0c, 0x20, 0x10, 0x3f, 0x3f, 0x00, 0x00, 0x10, 0x10, 0x3f,
	0x3f, 0x3f, 0x3f,
};

static struct dsi_cmd_desc display_glow_cmd = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(sharpca_basic_9b)}, sharpca_basic_9b

};

static char sharpca_basic_92[] = {
	0xca, 0x01, 0x80, 0x92, 0x92, 0x9b, 0x75, 0x9b, 0x9b, 0x20,
	0x20, 0x80, 0xfe, 0x0a, 0x4a, 0x37, 0xa0, 0x55, 0xf8, 0x0c,
	0x0c, 0x20, 0x10, 0x3f, 0x3f, 0x00, 0x00, 0x10, 0x10, 0x3f,
	0x3f, 0x3f, 0x3f,
};

static struct dsi_cmd_desc display_std_cmd = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(sharpca_basic_92)}, sharpca_basic_92

};
static char sharpca_basic_norm[] = {
	0xca, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x08,
	0x20, 0x80, 0x80, 0x0a, 0x4a, 0x37, 0xa0, 0x55, 0xf8, 0x0c,
	0x0c, 0x20, 0x10, 0x3f, 0x3f, 0x00, 0x00, 0x10, 0x10, 0x3f,
	0x3f, 0x3f, 0x3f,
};

static struct dsi_cmd_desc display_soft_cmd = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(sharpca_basic_norm)}, sharpca_basic_norm

};

#elif defined(CONFIG_ZTEMT_MIPI_1080P_R63311_SHARP_IPS_6P4)
//NX601J
static char sharpca_basic_9b[] = {
  0xca,0x01,0x80,0x9c,0x9b,0x9b,0x40,0xbe,0xbe,0x20,
  0x20,0x80,0xfe,0x0a,0x4a,0x37,0xa0,0x55,0xf8,0x0c,
  0x0c,0x20,0x10,0x3f,0x3f,0x00,0x00,0x10,0x10,0x3f,
  0x3f,0x3f,0x3f,
};

static struct dsi_cmd_desc display_glow_cmd = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(sharpca_basic_9b)}, sharpca_basic_9b

};

static char sharpca_basic_92[] = {
  0xca,0x01,0x80,0x92,0x92,0x9b,0x75,0x9b,0x9b,0x20,
  0x20,0x80,0xfe,0x0a,0x4a,0x37,0xa0,0x55,0xf8,0x0c,
  0x0c,0x20,0x10,0x3f,0x3f,0x00,0x00,0x10,0x10,0x3f,
  0x3f,0x3f,0x3f,
};

static struct dsi_cmd_desc display_std_cmd = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(sharpca_basic_92)}, sharpca_basic_92

};
static char sharpca_basic_norm[] = {
  0xca,0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x08,
  0x20,0x80,0x80,0x0a,0x4a,0x37,0xa0,0x55,0xf8,0x0c,
  0x0c,0x20,0x10,0x3f,0x3f,0x00,0x00,0x10,0x10,0x3f,
  0x3f,0x3f,0x3f,
};

static struct dsi_cmd_desc display_soft_cmd = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(sharpca_basic_norm)}, sharpca_basic_norm

};
#elif defined(CONFIG_ZTEMT_MIPI_1080P_R63417_SHARP_IPS_5P5)
//NX505J
static char sharpca_basic_9b[] = {0xca, 0x01,0x80, 0x98,0x98,0x9b,0x40,0xbe,0xbe,0x20,0x20,
	0x80,0xfe,0x0a, 0x4a,0x37,0xa0,0x55,0xf8,0x0c,0x0c,0x20,0x10,0x3f,0x3f,0x00,
	0x00,0x10,0x10,0x3f,0x3f,0x3f,0x3f,
};
static struct dsi_cmd_desc display_glow_cmd = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(sharpca_basic_9b)}, sharpca_basic_9b

};

static char sharpca_basic_92[] = {0xca, 0x01,0x80, 0x92,0x92,0x9b,0x75,0x9b,0x9b,0x20,0x20,
	0x80,0xfe,0x0a, 0x4a,0x37,0xa0,0x55,0xf8,0x0c,0x0c,0x20,0x10,0x3f,0x3f,0x00,
	0x00,0x10,0x10,0x3f,0x3f,0x3f,0x3f,
};

static struct dsi_cmd_desc display_std_cmd = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(sharpca_basic_92)}, sharpca_basic_92

};
static char sharpca_basic_norm[] = {0xca, 0x00,0x80, 0x80,0x80,0x80,0x80,0x80,0x80,0x08,0x20,
	0x80,0x80,0x0a, 0x4a,0x37,0xa0,0x55,0xf8,0x0c,0x0c,0x20,0x10,0x3f,0x3f,0x00,
	0x00,0x10,0x10,0x3f,0x3f,0x3f,0x3f,
};

static struct dsi_cmd_desc display_soft_cmd = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(sharpca_basic_norm)}, sharpca_basic_norm

};

#elif defined(CONFIG_ZTEMT_MIPI_2K_R63419_SHARP_IPS_5P5)
//NX506J
static char sharpca_basic_9b[] = {
	0xca, 0x81, 0x9f, 0x98, 0x9b, 0x40, 0xbe, 0xbe, 0x20, 0x20,
	0x80, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a,
	0x4a, 0x37, 0xa0, 0x55, 0xf8, 0x0c, 0x0c, 0x20, 0x10, 0x3f, 
	0x3f, 0x10, 0x10, 0x3f, 0x3f, 0x3f, 0x3f, 0x00, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 
};
static struct dsi_cmd_desc display_glow_cmd = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(sharpca_basic_9b)}, sharpca_basic_9b

};

static char sharpca_basic_92[] = {
	0xca, 0x81, 0x96, 0x92, 0x9b, 0x75, 0x9b, 0x9b, 0x20, 0x20,
	0x80, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a,
	0x4a, 0x37, 0xa0, 0x55, 0xf8, 0x0c, 0x0c, 0x20, 0x10, 0x3f, 
	0x3f, 0x10, 0x10, 0x3f, 0x3f, 0x3f, 0x3f, 0x00, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 
};

static struct dsi_cmd_desc display_std_cmd = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(sharpca_basic_92)}, sharpca_basic_92

};
static char sharpca_basic_norm[] = {
	0xca, 0x81, 0x8a, 0x80, 0x80, 0x80, 0x80, 0x80, 0x10, 0x20,
	0x80, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a,
	0x4a, 0x37, 0xa0, 0x55, 0xf8, 0x0c, 0x0c, 0x20, 0x10, 0x3f, 
	0x3f, 0x10, 0x10, 0x3f, 0x3f, 0x3f, 0x3f, 0x00, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 
};

static struct dsi_cmd_desc display_soft_cmd = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(sharpca_basic_norm)}, sharpca_basic_norm

};

#else
static struct dsi_cmd_desc display_glow_cmd;
static struct dsi_cmd_desc display_std_cmd;
static struct dsi_cmd_desc display_soft_cmd;
#endif

void zte_send_cmd(struct dsi_cmd_desc *cmds,int cmdcount)
{
	struct dcs_cmd_req cmdreq;

	if((!zte_mdss_dsi_ctrl) || (cmdcount < 1))	{
		pr_err("lcd:faild:%s zte_mdss_dsi_ctrl is null\n",__func__);
		return;
	}

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = cmds;
	cmdreq.cmds_cnt = cmdcount;
	cmdreq.flags = CMD_REQ_COMMIT;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_dsi_cmdlist_put(zte_mdss_dsi_ctrl, &cmdreq);
}

void zte_mipi_saturation(void)
{
	unsigned int value;
	value =zte_enhance_val.saturation;

	if(!zte_enhance_val.en_saturation || ((NULL == display_std_cmd.payload) || \
		(NULL == display_soft_cmd.payload) ||(NULL == display_glow_cmd.payload)) ||\
		(NULL == zte_mdss_dsi_ctrl))
		return;

#ifdef ZTE_DISP_ENHANCE_DEBUG
	printk("lcd:%s value=%d\n", __func__, value);
#endif

	switch (value) {
	case INTENSITY_00:
		zte_send_cmd(&display_soft_cmd,ZTE_SHARP_ENHANCE_CMD_COUNT);
		break;
	case INTENSITY_01:
		zte_send_cmd(&display_std_cmd,ZTE_SHARP_ENHANCE_CMD_COUNT);
		break;
	case INTENSITY_02:
		zte_send_cmd(&display_glow_cmd,ZTE_SHARP_ENHANCE_CMD_COUNT);
		break;
	default:
		zte_send_cmd(&display_std_cmd,ZTE_SHARP_ENHANCE_CMD_COUNT);
		break;
	}
}

static ssize_t saturation_show(struct kobject *kobj, 
	struct kobj_attribute *attr, char *buf)
{	
	return snprintf(buf, PAGE_SIZE, "%d\n",	zte_enhance_val.en_saturation);
}

static ssize_t saturation_store(struct kobject *kobj, struct kobj_attribute *attr,
							const char *buf, size_t size)
{
	int val;

	if(!zte_enhance_val.en_saturation)
		return size;

	sscanf(buf, "%d", &val);

#ifdef ZTE_DISP_ENHANCE_DEBUG
	printk("lcd:%s state=%d size=%d\n", __func__, (int)val, (int)size);
#endif

	zte_enhance_val.saturation =val;

	zte_mipi_saturation();
	return size;
}


void zte_mipi_colortmp(void)
{
	unsigned int value;
	value =zte_enhance_val.colortmp;

	if(!zte_enhance_val.en_colortmp || (NULL == zte_mdss_dsi_ctrl))
		return ;
	
#ifdef ZTE_DISP_ENHANCE_DEBUG
	printk("lcd:%s value=%d\n", __func__, value);
#endif

//#if defined(CONFIG_ZTEMT_MIPI_1080P_R63311_SHARP_IPS_6P4)
#if 1
	switch (value) {
	case INTENSITY_00:
		zte_mdss_pcc_config(&zte_pcc_cfg_normal);
		break;
	case INTENSITY_01:
		zte_mdss_pcc_config(&zte_pcc_cfg_cool);
		break;
	case INTENSITY_02:
		zte_mdss_pcc_config(&zte_pcc_cfg_cool1);
		break;
	default:
		zte_mdss_pcc_config(&zte_pcc_cfg_normal);
		break;
	}
#else
	switch (value) {
	case INTENSITY_00:
		zte_mdss_pcc_config(&zte_pcc_cfg_warm);
		break;
	case INTENSITY_01:
		zte_mdss_pcc_config(&zte_pcc_cfg_normal);
		break;
	case INTENSITY_02:
		zte_mdss_pcc_config(&zte_pcc_cfg_cool);
		break;
	default:
		zte_mdss_pcc_config(&zte_pcc_cfg_normal);
		break;
	}
#endif
}

static ssize_t colortmp_show(struct kobject *kobj, 
	struct kobj_attribute *attr, char *buf)
{	
	return snprintf(buf, PAGE_SIZE, "%d\n",	zte_enhance_val.colortmp);
}

static ssize_t colortmp_store(struct kobject *kobj, struct kobj_attribute *attr,
    const char *buf, size_t size)
{
	int val;

	if(!zte_enhance_val.en_colortmp)
	     return size;

	sscanf(buf, "%d", &val);

#ifdef ZTE_DISP_ENHANCE_DEBUG
	printk("lcd:%s state=%d size=%d\n", __func__, (int)val, (int)size);
#endif

	zte_enhance_val.colortmp = val;
	
	zte_mipi_colortmp();
	return size;
}


void zte_set_ctrl_point(struct mdss_dsi_ctrl_pdata * ctrl)
{
#ifdef ZTE_DISP_ENHANCE_DEBUG
	printk("lcd:%s \n", __func__);
#endif

	zte_mdss_dsi_ctrl = ctrl;
}

void zte_boot_begin_enhance(struct mdss_dsi_ctrl_pdata *ctrl)
{
#ifdef ZTE_DISP_ENHANCE_DEBUG
	printk("lcd:%s \n", __func__);
#endif
	zte_set_ctrl_point(ctrl);

	zte_mipi_saturation();
	zte_mipi_colortmp();
}

static struct kobj_attribute attrs[] = {
	__ATTR(saturation, 0664, saturation_show, saturation_store),
	__ATTR(colortmp, 0664, colortmp_show, colortmp_store),
};

struct kobject *enhance__kobj;

static int __init enhance_init(void)
{
	int retval;
	int attr_count = 0;

	enhance__kobj = kobject_create_and_add("lcd_enhance", kernel_kobj);

	if (!enhance__kobj)
		return -ENOMEM;

	/* Create the files associated with this kobject */
	for (attr_count = 0; attr_count < ARRAY_SIZE(attrs); attr_count++) {
		
		if(!zte_enhance_val.en_colortmp && (attr_count == 1))
			continue;
	
		retval = sysfs_create_file(enhance__kobj, &attrs[attr_count].attr);
		if (retval < 0) {
			pr_err("%s: Failed to create sysfs attributes\n", __func__);
			goto err_sys;
		}
	}
	
	pr_info("lcd: %s Done.\n",__func__);

	return retval;
	
err_sys:
	for (attr_count--; attr_count >= 0; attr_count--) {
		sysfs_remove_file(enhance__kobj, &attrs[attr_count].attr);
	}
	
	kobject_put(enhance__kobj);
	
	pr_info("lcd: %s init ERR.\n",__func__);

	return retval;
}

static void __exit enhance_exit(void)
{
	int attr_count = 0;
	
	for (attr_count = 0; attr_count < ARRAY_SIZE(attrs); attr_count++) {
		sysfs_remove_file(enhance__kobj, &attrs[attr_count].attr);
	}
	
	kobject_put(enhance__kobj);
	zte_mdss_dsi_ctrl = NULL;
}

module_init(enhance_init);
module_exit(enhance_exit);

