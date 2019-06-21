/*This file implements MTK boot mode.*/

#include <sys/types.h>
#include <debug.h>
#include <err.h>
#include <reg.h>
#include <video.h>
#include <platform/mt_typedefs.h>
#include <platform/boot_mode.h>
#include <platform/mt_reg_base.h>
#include <platform/mtk_key.h>
#include <platform/mt_gpt.h>
#include <target/cust_key.h>
#include <platform/mtk_wdt.h>

#define KEY_UP_   MT65XX_MENU_SELECT_KEY
#define KEY_DOWN_ MT65XX_MENU_OK_KEY
#define KEY_OK_   MTK_PMIC_PWR_KEY

extern  int unshield_recovery_detection(void);
extern BOOL recovery_check_command_trigger(void);
extern void boot_mode_menu_select();
#define MODULE_NAME "[BOOT_MENU]"
extern char g_CMDLINE [];
bool g_boot_menu = false;
/*
static void boot_mode_menus(int number)
{
    g_boot_menu = true;
    const char* title_msg = "Custom loader by Ultimate_1\n\n";

    switch(number) {
        case 1:
            video_set_cursor(video_get_rows()/2, 0);
            video_printf(title_msg);
            video_printf("[Recovery Mode]         <<==\n");
            video_printf("[Fastboot Mode]             \n");
            video_printf("[Normal   Boot]             \n");
            video_printf("[Power     OFF]             \n");
        break;

        case 2:
            video_set_cursor(video_get_rows()/2, 0);
            video_printf(title_msg);
            video_printf("[Recovery Mode]             \n");
            video_printf("[Fastboot Mode]         <<==\n");
            video_printf("[Normal   Boot]             \n");
            video_printf("[Power     OFF]             \n");
        break;

        case 3:
            video_set_cursor(video_get_rows()/2, 0);
            video_printf(title_msg);
            video_printf("[Recovery Mode]             \n");
            video_printf("[Fastboot Mode]             \n");
            video_printf("[Normal   Boot]         <<==\n");
            video_printf("[Power     OFF]             \n");
        break;

        case 4:
            video_set_cursor(video_get_rows()/2, 0);
            video_printf(title_msg);
            video_printf("[Recovery Mode]             \n");
            video_printf("[Fastboot Mode]             \n");
            video_printf("[Normal   Boot]             \n");
            video_printf("[Power     OFF]         <<==\n");
        break;
    }

    mdelay(300);
}
*/
void boot_mode_menu_select()
{
          bool pwr_key_sticking = true;
          int select = 1;
          video_clean_screen();
          mt_disp_show_menu(select);
          //mt_disp_show_boot_logo();
          //boot_mode_menus(select);
          while(1)
          {
             if(mtk_detect_key(KEY_UP_))
             {
                pwr_key_sticking = false;
                select--;
                if(select<1)
                    select=1;
                mt_disp_show_menu(select);
                mdelay(300);
                //boot_mode_menus(select);
             }
             else if(mtk_detect_key(KEY_DOWN_))
             {
                pwr_key_sticking = false;
                select++;
                if(select>4)
                    select=4;
                mt_disp_show_menu(select);
                mdelay(300);
                //boot_mode_menus(select);
             }
             else if(mtk_detect_key(KEY_OK_))
             {
                 if(!pwr_key_sticking)
                    break;
             }
             else
                pwr_key_sticking = false;

          }
          if(select == 1)
          {
                g_boot_mode = NORMAL_BOOT;
          }
          else if(select == 2)
          {
                g_boot_mode = RECOVERY_BOOT;
          }
          else if(select == 3)
          {
                g_boot_mode = FASTBOOT;
          }
          else if(select == 4)
          {
                mt6575_power_off();
          }

          video_set_cursor(video_get_rows()/2 +8, 0);
          return;
}

BOOL boot_menu_key_trigger(void)
{
#if 1
	//wait
    ulong begin = get_timer(0);
   dprintf(INFO, "\n%s Check  boot menu\n",MODULE_NAME);
   dprintf(INFO, "%s Wait 50ms for special keys\n",MODULE_NAME);

    //let some case of recovery mode pass.
    if(unshield_recovery_detection())
    {
        return TRUE;
    }

    while(get_timer(begin)<50)
    {
		if(mtk_detect_key(MT65XX_BOOT_MENU_KEY))
		{
                           mtk_wdt_disable();
                           boot_mode_menu_select();
                           mtk_wdt_init();
                           return TRUE;
		}
    }
#endif
     return FALSE;
}

BOOL boot_menu_detection(void)
{
    return boot_menu_key_trigger();
}


int unshield_recovery_detection(void)
{
    //because recovery_check_command_trigger's BOOL is different from the BOOL in this file.
    //so use code like this type.
    return recovery_check_command_trigger()? TRUE:FALSE;
}

