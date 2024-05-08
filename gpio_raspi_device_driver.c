#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>  //copy_to/from_user()
#include <linux/gpio.h>     //GPIO
#include <linux/err.h>
#include <linux/ioctl.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Omar Adel Shalaan");
MODULE_DESCRIPTION("GPIO Raspi Driver");
MODULE_VERSION("1.00");

/*******************************************************************************************/
#define GPIO_PIN_1 4
#define GPIO_PIN_2 17
#define GPIO_PIN_3 27
#define GPIO_PIN_4 22
/******************************************Global Variable**********************************/
dev_t dev = 0;
unsigned int countDev = 1;
unsigned int baseminorNum = 0;
#define SIZE 255
unsigned char buf[SIZE]; 

struct cdev gpio_cdev;
/*******************************************************************************************/
int driver_open(struct inode *device_file, struct file *inst)
{
    printk("%s GPIO Driver Opened \n",__FUNCTION__);
    return 0;
}
ssize_t driver_read(struct file *file, char __user *buf_user, size_t count, loff_t *off)
{
    printk("%s GPIO Driver Read \n",__FUNCTION__);
    int not_copied;
    if(count + *off > SIZE)
    {
        count = SIZE - *off;
    }
    not_copied = copy_to_user(buf_user, &buf[*off], count );
    if(not_copied)
    {
        return -1;
    }
    *off = count;
    printk("%s not copied %d\n",__FUNCTION__, not_copied);
    printk("%s message %s\n",__FUNCTION__, buf_user);
    return 0;
}
ssize_t driver_write(struct file *file, const char __user *buf_user, size_t count, loff_t *off)
{
    printk("%s GPIO Driver write \n",__FUNCTION__);
    int not_copied;
    if(count + *off > SIZE)
    {
        count = SIZE - *off;
    }
    if(!count)
    {
        printk("no space left\n");
        return -1;
    }
    not_copied = copy_from_user(&buf[*off], buf_user, count);

    if(not_copied)
    {
        return -1;
    }
    *off = count;
    printk("%s already done %ld\n", __func__,count);
    
    return count;
    
}
	

const struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = driver_write,
    .read = driver_read,
    .open = driver_open,
};
struct class *dev_class;
/*******************************************************************************************/
static int __init gpio_driver_init(void)
{

    /*Allocating Major number*/
    //int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count,const char *name)
    if((alloc_chrdev_region(&dev, baseminorNum, countDev, "gpio_dev")) < 0)
    {
        printk("Cannot allocate major number\n");
        goto r_unreg;
    }
    printk("alloc gpio_dev\n");
    printk("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
    /*Creating cdev structure*/
    //void cdev_init(struct cdev *cdev, const struct file_operations *fops)
    cdev_init(&gpio_cdev,&fops);
    
    /*Adding character device to the system*/
    //int cdev_add(struct cdev *p, dev_t dev, unsigned count)
    if((cdev_add(&gpio_cdev,dev,countDev)) < 0){
        printk("Cannot add the device to the system\n");
        goto r_del;
    }
    /*Creating struct class*/
    if( (dev_class = class_create("gpio_class")) == NULL)
    {
        printk("Cannot create the struct class\n");
        goto r_class;
    }
    
    /*Creating device*/
    if((device_create(dev_class,NULL,dev,NULL,"gpio_device")) == NULL)
    {
        printk( "Cannot create the Device \n");
        goto r_device;
    }

    //Checking the GPIO_PIN_1 is valid or not
    if(gpio_is_valid(GPIO_PIN_1) == false){
        printk("GPIO_PIN_1 %d is not valid\n", GPIO_PIN_1);
        goto r_device;
    }
    //Requesting the GPIO_PIN_1
    if(gpio_request(GPIO_PIN_1,"GPIO_PIN_1") < 0){
        printk("ERROR: GPIO_PIN_1 %d request\n", GPIO_PIN_1);
        goto r_gpio1;
    }


    //Checking the GPIO_PIN_2 is valid or not
    if(gpio_is_valid(GPIO_PIN_2) == false){
        printk("GPIO_PIN_2 %d is not valid\n", GPIO_PIN_2);
        goto r_device;
    }
    //Requesting the GPIO_PIN_2
    if(gpio_request(GPIO_PIN_2,"GPIO_PIN_2") < 0){
        printk("ERROR: GPIO_PIN_2 %d request\n", GPIO_PIN_2);
        goto r_gpio2;
    }


    //Checking the GPIO_PIN_3 is valid or not
    if(gpio_is_valid(GPIO_PIN_3) == false){
        printk("GPIO_PIN_3 %d is not valid\n", GPIO_PIN_3);
        goto r_device;
    }
    //Requesting the GPIO_PIN_3
    if(gpio_request(GPIO_PIN_3,"GPIO_PIN_3") < 0){
        printk("ERROR: GPIO_PIN_3 %d request\n", GPIO_PIN_3);
        goto r_gpio3;
    }


    //Checking the GPIO_PIN_4 is valid or not
    if(gpio_is_valid(GPIO_PIN_4) == false){
        printk("GPIO_PIN_4 %d is not valid\n", GPIO_PIN_4);
        goto r_device;
    }
    //Requesting the GPIO_PIN_4
    if(gpio_request(GPIO_PIN_4,"GPIO_PIN_4") < 0){
        printk("ERROR: GPIO_PIN_4 %d request\n", GPIO_PIN_4);
        goto r_gpio4;
    }

    
    gpio_direction_input(GPIO_PIN_1);
    gpio_direction_input(GPIO_PIN_2);
    gpio_direction_input(GPIO_PIN_3);
    gpio_direction_input(GPIO_PIN_4);

    printk(KERN_INFO "GPIO input pins configured\n"); 
    printk("Device Driver Insert...Done!!!\n");   
    return 0;

r_gpio4:
    gpio_free(GPIO_PIN_4);
r_gpio3:
    gpio_free(GPIO_PIN_3);
r_gpio2:
    gpio_free(GPIO_PIN_2);
r_gpio1:
    gpio_free(GPIO_PIN_1);
r_device:
    device_destroy(dev_class,dev);
r_class:
    class_destroy(dev_class);
r_del:
    cdev_del(&gpio_cdev);
r_unreg:
    unregister_chrdev_region(dev,1);
    return -1;
}


static void __exit gpio_driver_exit(void)
{
    gpio_free(GPIO_PIN_1);
    gpio_free(GPIO_PIN_2);
    gpio_free(GPIO_PIN_3);
    gpio_free(GPIO_PIN_4);
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&gpio_cdev);
    unregister_chrdev_region(dev, countDev);
    printk("GPIO driver exited\n");
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);
