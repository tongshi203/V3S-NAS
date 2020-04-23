#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

/*时间和定时器相关的文件，如果不使用sleep和mdelay等，可以去掉delay函数，如果不*使用定时器，可以去掉定时器相关函数
*/
#include <linux/delay.h>
#include<linux/timer.h>
#include<linux/jiffies.h>
/*驱动注册的头文件，包含驱动的结构体和注册和卸载的函数*/
#include <linux/platform_device.h>
/*注册杂项设备头文件*/
#include <linux/miscdevice.h>
#include <linux/device.h>
/*注册设备节点的文件结构体*/
#include <linux/fs.h>
/*Linux中申请GPIO的头文件*/
#include <linux/gpio.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>

#define LED_MAGIC 'k'
#define IOCTL_LED_ON  _IOW (LED_MAGIC, 1, int)
#define IOCTL_LED_OFF _IOW (LED_MAGIC, 2, int)
#define IOCTL_LED_RUN _IOW (LED_MAGIC, 3, int)
#define IOCTL_RST_H  _IOW (LED_MAGIC, 4, int)
#define IOCTL_RST_L  _IOW (LED_MAGIC, 5, int)
#define IOCTL_READ_KEY _IOW (LED_MAGIC, 6, int)

/*
由宏定义组成的一组32位数据
bit31~bit30 2位为 “区别读写” 区，作用是区分是读取命令还是写入命令。
bit29~bit15 14位为 "数据大小" 区，表示 ioctl() 中的 arg 变量传送的内存大小。
bit20~bit08  8位为 “魔数"(也称为"幻数")区，这个值用以与其它设备驱动程序的 ioctl 命令进行区别。
bit07~bit00   8位为 "区别序号" 区，是区分命令的命令顺序序号。
_IO (魔数， 基数);
_IOR (魔数， 基数， 变量型)
_IOW  (魔数， 基数， 变量型)
一般情况下用于作为CMD指令的参量，第一个魔数，可以设定一个字母，第二个为指令的值，第三个直接写int数据类型即可，只要在主程序将上述代码添加，然后使用宏定义，即可操作字符设备的函数
*/
#define TIMER_SET (200*HZ)/1000 /*设定定时器的定时值，亮灭都为0.2HZ*/

#define DEMOD_RST_GPIO  151
#define STR_DEMOD_RST_GPIO  "Demod_RST_PE23"
#define LED_GPIO        192
#define STR_LED_GPIO  "GLED_PG0"
#define KEY_GPIO       152
#define STR_KEY_GPIO  "KEY_PE24"

static int major;	/*定义一个用于保存major的值，可以指定值，也可以自由分配*/
static struct class *ts_gpio_class;	/*定义一个class，用于注册设备*/
static struct device *ts_gpio_device; /*将device自动加入到设备列表中，方便操作*/
static struct timer_list test_timer;	/*定时器，用于定时*/
unsigned char ledflag = 1;		/*是否闪烁的flag，可以使用其他名称*/
MODULE_LICENSE("Dual BSD/GPL");	/*常规描述*/
MODULE_AUTHOR("Ryan");/*常规描述*/



/*操作函数*/
static long TS_GPIO_IOCTL( struct file *files, unsigned int cmd, unsigned long arg){

	printk("cmd is %d,arg is %d \n",cmd,(unsigned int)(arg));

	switch(cmd){
		case IOCTL_LED_ON:
		{
			gpio_set_value(LED_GPIO, 0);
			break;
		}
		case IOCTL_LED_OFF:
		{
			gpio_set_value(LED_GPIO, 1);
			break;
		}
		case IOCTL_RST_H:
		{
            gpio_set_value(DEMOD_RST_GPIO, 1);
            break;
		}
		case IOCTL_RST_L:
		{
            gpio_set_value(DEMOD_RST_GPIO, 0);
            break;
		}
		case IOCTL_READ_KEY:
		{
            break;
		}
		default:
			break;
	}
    return 0;
}
/*释放函数*/
static int TS_GPIO_release(struct inode *inode, struct file *file){
    printk(KERN_EMERG "TS_GPIO release\n");
    return 0;
}
/*打开函数*/
static int TS_GPIO_open(struct inode *inode, struct file *file){
    printk(KERN_EMERG "TS_GPIO open\n");
    return 0;
}
/*ops结构体，存储相关的操作函数*/
static struct file_operations TS_GPIO_ops = {
    .owner					= THIS_MODULE,
    .open					= TS_GPIO_open,
    .release				= TS_GPIO_release,
    .unlocked_ioctl = TS_GPIO_IOCTL,
};

static int TS_GPIO_init(void)
{
    //int DriverState;
    int ret;

    printk(KERN_EMERG "module TS GPIO init...!\n");
    ret = gpio_request(LED_GPIO,STR_LED_GPIO);
    if(ret < 0){
        printk(KERN_EMERG "gpio_request LED_GPIO failed!\n");
        return ret;
    }
    gpio_direction_output(LED_GPIO,1);
    gpio_set_value(LED_GPIO, 0);

    ret = gpio_request(DEMOD_RST_GPIO,STR_DEMOD_RST_GPIO);
    if(ret < 0){
        printk(KERN_EMERG "gpio_request DEMOD_RST_GPIO failed!\n");
        return ret;
    }
    gpio_direction_output(DEMOD_RST_GPIO,1);
    gpio_set_value(DEMOD_RST_GPIO, 1);

    ret = gpio_request(KEY_GPIO,STR_KEY_GPIO);
    if(ret < 0){
        printk(KERN_EMERG "gpio_request KEY_GPIO failed!\n");
        return ret;
    }
    gpio_direction_input(KEY_GPIO);
//    gpio_set_value(KEY_GPIO, 1);

    major = register_chrdev(0, "TS_GPIO", &TS_GPIO_ops);

	ts_gpio_class = class_create(THIS_MODULE, "ts_gpio_class");
    if (!ts_gpio_class) {
        printk(KERN_INFO "ts_gpio_class create fail\n");
        return -1;
    }
    ts_gpio_device = device_create(ts_gpio_class, NULL, MKDEV(major, 0), NULL, "TS_GPIO");
    if (!ts_gpio_device) {
        printk(KERN_INFO "TS_gpio device_create fail\n");
        return -1;
    }

   // printk(KERN_EMERG "DriverState is %d\n",DriverState);
    return 0;
}

static void TS_GPIO_exit(void)
{
    printk(KERN_EMERG "module TS_GPIO exit!\n");

    unregister_chrdev(major,"TS_GPIO");
    gpio_free(LED_GPIO);
    gpio_free(DEMOD_RST_GPIO);
    device_unregister(ts_gpio_device);
    class_destroy(ts_gpio_class);
	del_timer(&test_timer);
}

module_init(TS_GPIO_init);
module_exit(TS_GPIO_exit);
