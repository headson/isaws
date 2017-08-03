#ifndef __KERNEL__
#  define __KERNEL__
#endif
#ifndef MODULE
#  define MODULE
#endif

#include "ihs_gpio.h"

#ifdef IHS_GPIO
//#include <linux/config.h>
#include <linux/slab.h>         /* kmalloc() */
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>       /* printk() */
#include <linux/fs.h>           /* everything... */
#include <linux/errno.h>        /* error codes */
#include <linux/types.h>        /* size_t */
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/fcntl.h>        /* O_ACCMODE */
#include <asm/system.h>         /* cli(), *_flags */
#include <linux/miscdevice.h>   /* struct miscdevice misc */
#include <mach/iomux-mx6q.h>
#include <mach/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/pwm.h>
#include <linux/signal.h>

#define dbg_print   printk

typedef struct  
{
    int eType;     // 类型
    int nState;    // 状态
    int nParam;    // 占空比
} TAG_EXT_GPIO;

//////////////////////////////////////////////////////////////////////////
// GPIO PADS
static iomux_v3_cfg_t gPads[] = {
    MX6Q_PAD_DISP0_DAT17__GPIO_5_11,    // LED1
    MX6Q_PAD_DISP0_DAT18__GPIO_5_12,    // LED2
    MX6Q_PAD_GPIO_3__GPIO_1_3,          // LED3

    //MX6Q_PAD_EIM_D23__GPIO_3_23,      // WDG EN=1
    MX6Q_PAD_NANDF_CS2__GPIO_6_15,      // WDG Keeplive

    MX6Q_PAD_SD1_DAT2__GPIO_1_19,       /* ????? */
    MX6Q_PAD_SD1_DAT1__PWM3_PWMO,       // 照明
    MX6Q_PAD_SD1_CMD__PWM4_PWMO,        // 补光灯

    MX6Q_PAD_CSI0_DATA_EN__GPIO_5_20,   // 激光笔,

//    MX6Q_PAD_SD3_DAT6__GPIO_6_18,       // 防拆开关；0=关，1=开
    MX6Q_PAD_DISP0_DAT0__GPIO_4_21,     // 震荡器

    MX6Q_PAD_DISP0_DAT5__GPIO_4_26,     // 继电器
//    MX6Q_PAD_SD3_DAT7__GPIO_6_17,       // 门禁开关

    MX6Q_PAD_KEY_ROW0__GPIO_4_7,        // 继电器2
    MX6Q_PAD_KEY_COL0__GPIO_4_6,        // 门禁开关2

    MX6Q_PAD_EIM_D31__GPIO_3_31,        /* ????? */
    MX6Q_PAD_DISP0_DAT3__GPIO_4_24,     // 门铃

    // IO SET
    MX6Q_PAD_KEY_COL2__GPIO_4_10,       /* ????? */
    MX6Q_PAD_KEY_ROW2__GPIO_4_11,       /* 科大讯飞使能 */

    MX6Q_PAD_DISP0_DAT6__GPIO_4_27,     /* NET RESET */
    MX6Q_PAD_DISP0_DAT4__GPIO_4_25,     /* ????? */
    MX6Q_PAD_DISP0_DAT1__GPIO_4_22,     /* ????? */

    MX6Q_PAD_CSI0_DAT7__GPIO_5_25,      /* 功放使能 */
};

typedef struct {
    int         type;       // type
    unsigned    gpio;       // GPIO
    char*       name;       // DESC
    int         value;      // 数据
} TAG_IHS_IO;

static TAG_IHS_IO g_io_en[] = {
    {0, IMX_GPIO_NR(4, 27),    "net_reset",     1}, /* 交换机复位  */
    {0, IMX_GPIO_NR(1, 3),     "ele_net_en",    1}, /* 电力猫 1 */
    {0, IMX_GPIO_NR(5, 25),    "gf_enable",     1}, /* 功放使能 1 */
    {0, IMX_GPIO_NR(4, 11),    "kdxf_enable",   1}, /* 科大讯飞使能 1 */
};

static TAG_IHS_IO g_io_out[] = {
    {E_WriteLED1,       IMX_GPIO_NR(5, 11),     "E_WriteLED1",      1}, /* LED1 */
    {E_WriteLED2,       IMX_GPIO_NR(5, 12),     "E_WriteLED2",      1}, /* LED2 */
    {E_WriteLED3,       IMX_GPIO_NR(1, 3),      "E_WriteLED3",      1}, /* LED3 */

    {E_WWatchdog,       IMX_GPIO_NR(6, 15),     "E_WWatchdog",      0}, /* 喂狗 */

    {E_WPilotLamp,      IMX_GPIO_NR(5, 20),     "E_WPilotLamp",     0}, /* 激光笔；1=关，0=开 */
    {E_WriteSwitch1,    IMX_GPIO_NR(4, 26),     "E_WriteSwitch1",   0}, /* 门禁（继电器）。1=开，0=关(常态) */
    {E_WriteSwitch2,    IMX_GPIO_NR(4, 7),      "E_WriteSwitch2",   0}, /* 门禁（继电器）。1=开，0=关(常态) */
};

static TAG_IHS_IO g_io_in[] = {
    {E_RDoorbell,       IMX_GPIO_NR(4, 24),      "E_RDoorbell",      0}, /* 门铃开关 */

    {E_ReadCover,       IMX_GPIO_NR(6, 18),      "E_ReadCover",      0}, /* 防拆开关 */
    {E_ReadShake,       IMX_GPIO_NR(4, 21),      "E_ReadShake",      0}, /* 震动报警 */

    {E_ReadSwitch1,     IMX_GPIO_NR(6, 17),      "E_ReadSwitch1",    1}, /* 门禁开关1 */
    {E_ReadSwitch2,     IMX_GPIO_NR(4, 6),       "E_ReadSwitch2",    1}, /* 门禁开关2 */
};

typedef struct
{
    int                type;
    struct pwm_device *dev;
    char*              name;
    int                num;
    int                duty;
} TAG_IHS_PWM;

static TAG_IHS_PWM g_pwm_lgt[] = {
    {E_ZhaoMing,  NULL,    "zhao_ming", 2, 1}, /* 照明灯 */
    {E_Infrared,  NULL,    "ir_light",  3, 1}, /* 红外补光灯 */
};

//////////////////////////////////////////////////////////////////////////
static int ihs_init_pads(void)
{
    int i = 0;
    int ret = 0;
    
    printk("ihs_init_pads start.\n");
    ret = mxc_iomux_v3_setup_multiple_pads(gPads, ARRAY_SIZE(gPads));
    if (ret < 0)
    {
        dbg_print("mxc_iomux_v3_setup_multiple_pads failed.\n");
        return -1;
    }

    // ENABLE
    for (i = 0; i < sizeof(g_io_en)/sizeof(TAG_IHS_IO); i++)
    {
        gpio_request(g_io_en[i].gpio, g_io_en[i].name);
        gpio_direction_output(g_io_en[i].gpio, g_io_en[i].value);
    }

    // OUT
    for (i = 0; i < sizeof(g_io_out)/sizeof(TAG_IHS_IO); i++)
    {
        gpio_request(g_io_out[i].gpio, g_io_out[i].name);
        gpio_direction_output(g_io_out[i].gpio, g_io_out[i].value);
    }

    for (i = 0; i < sizeof(g_io_in)/sizeof(TAG_IHS_IO); i++)
    {
        gpio_request(g_io_in[i].gpio, g_io_in[i].name);
        gpio_direction_input(g_io_in[i].gpio);
    }

    // PWM
    for (i = 0; i < sizeof(g_pwm_lgt)/sizeof(TAG_IHS_PWM); i++)
    {
        g_pwm_lgt[i].dev = pwm_request(g_pwm_lgt[i].num, g_pwm_lgt[i].name);
        if (IS_ERR(g_pwm_lgt[i].dev)) 
        {
            g_pwm_lgt[i].dev = NULL;
            continue;
        }
        pwm_config(g_pwm_lgt[i].dev, g_pwm_lgt[i].duty, 50000);
        pwm_disable(g_pwm_lgt[i].dev);
        dbg_print("pwm %d-%s request success.\n", g_pwm_lgt[i].num, g_pwm_lgt[i].name);
    }

    return 0;
}

static void ihs_exit_pads(void)
{
    int i = 0;
    for (i = 0; i < sizeof(g_io_en)/sizeof(TAG_IHS_IO); i++)
        gpio_free(g_io_en[i].gpio);

    //for (i = 0; i < sizeof(g_io_out)/sizeof(TAG_IHS_IO); i++)
    //    gpio_free(g_io_en[i].gpio);

    for (i = 0; i < sizeof(g_pwm_lgt)/sizeof(TAG_IHS_PWM); i++)
        pwm_free(g_pwm_lgt[i].dev);
}

static void ihs_io_out(int type, int value)
{
    int i = 0;
    for (i = 0; i < sizeof(g_io_out)/sizeof(TAG_IHS_IO); i++)
    {
        if (g_io_out[i].type == type)
        {
            gpio_set_value(g_io_out[i].gpio, value);

            //printk("set gpio %d-%s value %d.\n", 
            //    g_io_out[i].type, g_io_out[i].name, value);
            return;
        }
    }
}

static void ihs_pwm_duty(int type, int sw, int duty)
{
    int i = 0;
    for (i = 0; i < sizeof(g_pwm_lgt)/sizeof(TAG_IHS_PWM); i++)
    {
        if (g_pwm_lgt[i].type == type)
        {
            if (sw)
            {
                pwm_config(g_pwm_lgt[i].dev, duty, 50000);
                pwm_enable(g_pwm_lgt[i].dev);
            }
            else
            {
                pwm_config(g_pwm_lgt[i].dev, duty, 50000);
                pwm_disable(g_pwm_lgt[i].dev);
            }
            //printk("set pwm %d-%d-%s value %d.\n", 
            //    sw, type, g_pwm_lgt[i].name, duty);
            return;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
/* Open and close */
int ihs_open(struct inode *inode, struct file *filp)
{
    /*int i = 0;
    for (i = 0; i < sizeof(g_io_en)/sizeof(TAG_IHS_IO); i++)
    {
        gpio_request(g_io_en[i].gpio, g_io_en[i].name);
        gpio_direction_output(g_io_en[i].gpio, g_io_en[i].value);
    }*/

    return 0; /* success */
}

int ihs_release(struct inode *inode, struct file *filp)
{
    /*int i = 0;
    for (i = 0; i < sizeof(g_pwm_lgt)/sizeof(TAG_IHS_PWM); i++)
    {
        pwm_config(g_pwm_lgt[i].dev, 0, 50000);
        pwm_disable(g_pwm_lgt[i].dev);
    }
    
    for (i = 0; i < sizeof(g_io_en)/sizeof(TAG_IHS_IO); i++)
    {
        gpio_request(g_io_en[i].gpio, g_io_en[i].name);
        gpio_direction_output(g_io_en[i].gpio, (g_io_en[i].value?0:1));
        
        gpio_direction_output(g_io_en[i].gpio, g_io_en[i].value);
        gpio_direction_output(g_io_en[i].gpio, g_io_en[i].value);
    }*/

    return 0;
}

/* Data management: read and write*/
ssize_t ihs_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
    int i = 0;
    TAG_EXT_GPIO cGpio;

    if (copy_from_user(&cGpio, buf, sizeof(cGpio)))
        goto err;

    for (i = 0; i < sizeof(g_io_in)/sizeof(TAG_IHS_IO); i++)
    {
        if (g_io_in[i].type == cGpio.eType)
        {
            cGpio.nState = gpio_get_value(g_io_in[i].gpio);
            break;
        }
    }

    if (copy_to_user(buf, &cGpio, sizeof(cGpio)))
        return 0;

    return 1;
err:
    return 0;
}

ssize_t ihs_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
    int  ret = 0;
    TAG_EXT_GPIO cGpio;

    //mutex_lock(&gSem);
    if (copy_from_user(&cGpio, buf, sizeof(cGpio)))
        goto err;

    ihs_io_out(cGpio.eType, cGpio.nState);
    ihs_pwm_duty(cGpio.eType, cGpio.nState, cGpio.nParam);

    //dbg_print("type %d, %d, %d.\n", cGpio.eType, cGpio.nState, cGpio.nParam);
err:
    //mutex_unlock(&gSem);
    return ret;
}

long ihs_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    return 0;
}

/*  */
static struct file_operations ihs_fops = {
    .owner          = THIS_MODULE,
    .open           = ihs_open,
    .release        = ihs_release,
    .read           = ihs_read,
    .write          = ihs_write,
    .unlocked_ioctl = ihs_ioctl,
};

static struct miscdevice misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = DEVICE_NAME,
    .fops  = &ihs_fops,
};

/*  */
static void ihs_cleanup_module(void)
{
    misc_deregister(&misc);

    ihs_exit_pads();
}

static int ihs_init_module(void)
{
    int ret = -1;

    ret = ihs_init_pads();
    if (ret < 0)
        return -1;

    ret = misc_register(&misc);
    printk (DEVICE_NAME" initialized.\n");
    return ret;
}

module_init( ihs_init_module);
module_exit( ihs_cleanup_module);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sober.Peng");
#endif // IHS_GPIO
