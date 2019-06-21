// working lcm driver for Tele2 Midi 1.1

#include "lcm_drv.h"

#define Lcd_Log pr_debug


// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(854)

#define PROBLEM_REVISION									// comment this line for use on non-problematic lcm

#define REGFLAG_DELAY             							0xFFE
#define REGFLAG_END_OF_TABLE      							0xFFF	//END OF REGISTERS MARKER

#define LCM_ID	0x7701

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[128];
};

static struct LCM_setting_table lcm_init_table[] = {
#ifdef PROBLEM_REVISION
	/* Sleep Mode Off */
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 200, {}},
	{0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x10}},
	{0xC0, 2, {0xE9, 0x03}},
	{0xC1, 2, {0x0D, 0x02}},
	{0xC2, 2, {0x31, 0x08}},
	{0xB0, 16, {0x40, 0x05, 0x84, 0x18, 0x1A, 0x0C, 0x04, 0x09, 0x09, 0x1A, 0x08, 0x15, 0x13, 0x09, 0x0F, 0x99}},
	{0xB1, 16, {0x40, 0x05, 0x44, 0x15, 0x19, 0x0B, 0x05, 0x09, 0x09, 0x1E, 0x08, 0x15, 0x10, 0x96, 0x19, 0xD9}},
	{0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x11}},
	{0xB0, 1, {0x50}},
	{0xB1, 1, {0x58}},
	{0xB2, 1, {0x07}},
	{0xB3, 1, {0x80}},
	{0xB5, 1, {0x47}},
	{0xB7, 1, {0x8A}},
	{0xB8, 1, {0x21}},
	{0xC1, 1, {0x78}},
	{0xC2, 1, {0x78}},
	{0xD0, 1, {0x88}},
	{REGFLAG_DELAY, 100, {}},
	{0xE0, 3, {0x00, 0x00, 0x02}},
	{0xE1, 11, {0x08, 0x00, 0x0A, 0x00, 0x07, 0x00, 0x09, 0x00, 0x00, 0x33, 0x33}},
	{0xE2, 13, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{0xE3, 4, {0x00, 0x00, 0x33, 0x33}},
	{0xE4, 2, {0x44, 0x44}},
	{0xE5, 16, {0x0E, 0x60, 0xA0, 0xA0, 0x10, 0x60, 0xA0, 0xA0, 0x0A, 0x60, 0xA0, 0xA0, 0x0C, 0x60, 0xA0, 0xA0}},
	{0xE6, 4, {0x00, 0x00, 0x33, 0x33}},
	{0xE7, 2, {0x44, 0x44}},
	{0xE8, 16, {0x0D, 0x60, 0xA0, 0xA0, 0x0F, 0x60, 0xA0, 0xA0, 0x09, 0x60, 0xA0, 0xA0, 0x0B, 0x60, 0xA0, 0xA0}},
	{0xEB, 7, {0x02, 0x01, 0xE4, 0xE4, 0x44, 0x00, 0x40}},
	{0xEC, 2, {0x02, 0x01}},
	{0xED, 16, {0xAB, 0x89, 0x76, 0x54, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x10, 0x45, 0x67, 0x98, 0xBA}},
	{REGFLAG_DELAY, 10, {}},
	{0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x00}},
	{0x36, 1, {0x02}},
#else
	{0x01, 0, {}},
	{REGFLAG_DELAY, 120, {}},
	{0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x11}},
	{0xD1, 1, {0x11}},
	/* Sleep Mode Off */
	{0x11, 0, {}},
	{REGFLAG_DELAY, 120, {}},
	{0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x10}},
	{0xC0, 2, {0xE9, 0x03}},
	{0xC1, 2, {0x0C, 0x07}},
	{0xC2, 2, {0x31, 0x08}},
	{0xB0, 16, {0x40, 0x02, 0x08, 0x11, 0x17, 0x0B, 0x07, 0x09, 0x09, 0x1B, 0x07, 0x16, 0x12, 0x0E, 0x13, 0x0F}},
	{0xB1, 16, {0x40, 0x01, 0x08, 0x0F, 0x15, 0x09, 0x04, 0x09, 0x09, 0x1C, 0x09, 0x15, 0x12, 0x0E, 0x12, 0x0F}},
	{0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x11}},
	{0xB0, 1, {0x4D}},
	{0xB1, 1, {0x5E}},
	{0xB2, 1, {0x07}},
	{0xB3, 1, {0x80}},
	{0xB5, 1, {0x47}},
	{0xB7, 1, {0x8A}},
	{0xB8, 1, {0x20}},
	{0xBB, 1, {0x03}},
	{0xC1, 1, {0x78}},
	{0xC2, 1, {0x78}},
	{0xD0, 1, {0x88}},
	{REGFLAG_DELAY, 100, {}},
	{0xE0, 3, {0x00, 0x00, 0x02}},
	{0xE1, 11, {0x06, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20}},
	{0xE2, 13, {0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{0xE3, 4, {0x00, 0x00, 0x33, 0x33}},
	{0xE4, 2, {0x44, 0x44}},
	{0xE5, 16, {0x09, 0x67, 0xBE, 0xA0, 0x0B, 0x67, 0xBE, 0xA0, 0x05, 0x67, 0xBE, 0xA0, 0x07, 0x67, 0xBE, 0xA0}},
	{0xE6, 4, {0x00, 0x00, 0x33, 0x33}},
	{0xE7, 2, {0x44, 0x44}},
	{0xE8, 16, {0x08, 0x67, 0xBE, 0xA0, 0x0A, 0x67, 0xBE, 0xA0, 0x04, 0x67, 0xBE, 0xA0, 0x06, 0x67, 0xBE, 0xA0}},
	{0xEA, 16, {0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00}},
	{0xEB, 7, {0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{0xEC, 2, {0x02, 0x02}},
	{0xED, 16, {0xF5, 0x47, 0x6F, 0x0B, 0x8F, 0x9F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF9, 0xF8, 0xB0, 0xF6, 0x74, 0x5F}},
	{0xEF, 12, {0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}},
	{0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x00}},
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 150, {}},
#endif
	/* Display ON */
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0, {}}
};

static struct LCM_setting_table lcm_suspend_table[] = {
	/* Display off sequence */
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

	/* Sleep Mode On */
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for (i = 0; i < count; i++) {

        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd) {

        case REGFLAG_DELAY :
            MDELAY(table[i].count);
            break;

        case REGFLAG_END_OF_TABLE :
            break;

        default:
            dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
        }
    }
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params, 0, sizeof(LCM_PARAMS));

    params->type   = LCM_TYPE_DSI;

    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;

    // enable tearing-free
    params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
    params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

    params->dsi.mode   = SYNC_PULSE_VDO_MODE;

    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM				= LCM_TWO_LANE;
    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

    // Highly depends on LCD driver capability.
    // Not support in MT6573
    params->dsi.packet_size=256;

    // Video settings
    params->dsi.intermediat_buffer_num = 0;

    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	#ifdef PROBLEM_REVISION
    params->dsi.vertical_sync_active				= 2;
    params->dsi.vertical_backporch					= 24;
    params->dsi.vertical_frontporch					= 18;
    params->dsi.horizontal_sync_active				= 12;
    params->dsi.horizontal_backporch				= 70;
	#else
    params->dsi.vertical_sync_active				= 10;
    params->dsi.vertical_backporch					= 20;
    params->dsi.vertical_frontporch					= 30;
    params->dsi.horizontal_sync_active				= 10;
    params->dsi.horizontal_backporch				= 80;
	#endif
    params->dsi.horizontal_frontporch				= 80;
	
    params->dsi.vertical_active_line				= FRAME_HEIGHT;
    params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.word_count = 1440;
	params->dsi.PLL_CLOCK = 220;
}


static void lcm_init(void)
{
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(100);

    push_table(lcm_init_table, sizeof(lcm_init_table) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
	push_table(lcm_suspend_table, sizeof(lcm_suspend_table) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_resume(void)
{
	lcm_init();
}

static unsigned int lcm_compare_id(void) // this function is working
{
	int   array[4];
	char  buffer[6];
	unsigned int id=0;

	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(50);
	SET_RESET_PIN(1);
	MDELAY(120);
	
	array[0] = 0x00053700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xA1,buffer,5); // buffer = 0x77 0x01 0xFF 0xFF 0xFF

	// buffer[0] = 0x77,
	// buffer[0]<<8 = 0x7700,
	// buffer[1] = 0x01,
	// id = 0x7700 + 0x01,
	// id = 0x7701
	id=(buffer[0]<<8)+buffer[1];

	return (LCM_ID == id)?1:0;
}

LCM_DRIVER st7701_fwvga_dsi_vdo_lcm_drv =
    {
        .name			= "st7701_fwvga_dsi_vdo",
        .set_util_funcs = lcm_set_util_funcs,
        .get_params     = lcm_get_params,
        .init           = lcm_init,
        .suspend        = lcm_suspend,
        .resume         = lcm_resume,
        .compare_id     = lcm_compare_id
    };

