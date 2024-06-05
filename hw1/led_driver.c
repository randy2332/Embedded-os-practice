#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h> //copy_to/from_user()
#include <linux/gpio.h>   //GPIO

#define GPIO_21 (21)
#define GPIO_20 (20)
#define GPIO_16 (16)
#define GPIO_12 (12)
#define GPIO_1 (1)
#define GPIO_7 (7)
#define GPIO_8 (8)
#define GPIO_25 (25)
dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);

/*************** Driver functions **********************/
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp,
	char __user *buf, size_t len,loff_t * off);
static ssize_t etx_write(struct file *filp,
	const char *buf, size_t len, loff_t * off);
/**********************************/

//File operation structure
static struct file_operations fops =
{
  .owner	= THIS_MODULE,
  .read		= etx_read,
  .write 	= etx_write,
  .open 	= etx_open,
  .release 	= etx_release,
};

/*
** This function will be called when we open the Device file
*/
static int etx_open(struct inode *inode, struct file *file)
{
  pr_info("Device File Opened...!!!\n");
  return 0;
}

/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file)
{
  pr_info("Device File Closed...!!!\n");
  return 0;
}


/*
** This function will be called when we read the Device file
*/
static ssize_t etx_read(struct file *filp,
	char __user *buf, size_t len, loff_t *off)
{
  uint8_t gpio_state = 0;
  //reading GPIO value
  gpio_state = gpio_get_value(GPIO_21);
  //write to user
  len = 1;
  if( copy_to_user(buf, &gpio_state, len) > 0) {
    pr_err("ERROR: Not all the bytes have been copied to user\n");
  }
  pr_info("Read function : GPIO_21 = %d \n", gpio_state);
  return 0;
}

static int get_gpio_by_index(int index) {
    switch (index) {
        case 1:
            return GPIO_21;
        case 2:
            return GPIO_20;
        case 3:
            return GPIO_16;
        case 4:
            return GPIO_12;
        case 5:
            return GPIO_1;
        case 6:
            return GPIO_7;
        case 7:
            return GPIO_8;
        case 8:
            return GPIO_25;
        default:
            pr_err("Invalid LED index\n");
            return -EINVAL;
    }
}

/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off){
    int num_leds=0;

    
    if( copy_from_user( &num_leds, buf, len) > 0) {
      pr_err("ERROR: Not all the bytes have been copied from user\n");
    }
    else{
      pr_info("Copied data from user: %d\n",num_leds);
    }

    int i;
    for (i = 1; i <= 8; ++i) {
        if (i <= num_leds) {
            gpio_set_value(get_gpio_by_index(i), 1);
            
        } else {
            gpio_set_value(get_gpio_by_index(i), 0);
        }
    }
    int j;

    for (j=8;j>=1;j--){
    	if (j>num_leds){
    	    continue;
    	}
    	else{
    	    fsleep(1000000);
    	    gpio_set_value(get_gpio_by_index(j), 0);
    	    pr_info("led:%d off ",j);
    	    
    	}
    }
   
  
    return len;
}	



/*
** Module Init function
*/
static int __init etx_driver_init(void)
{





  /*Allocating Major number*/
    if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
        pr_err("Cannot allocate major number\n");
        goto r_unreg;
    }
    pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

    /*Creating cdev structure*/
    cdev_init(&etx_cdev,&fops);

    /*Adding character device to the system*/
    if((cdev_add(&etx_cdev,dev,1)) < 0){
        pr_err("Cannot add the device to the system\n");
        goto r_del;
    }
    /*Creating struct class*/
    if((dev_class = class_create(THIS_MODULE,"etx_class")) == NULL){
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }
    /*Creating device*/
    if((device_create(dev_class,NULL,dev,NULL,"etx_device")) == NULL){
        pr_err( "Cannot create the Device \n");
        goto r_device;
    }

  
  //Checking the GPIO is valid or not
  if(gpio_is_valid(GPIO_21) == false){
    pr_err("GPIO %d is not valid\n", GPIO_21);
    goto r_device;
  }
  if(gpio_is_valid(GPIO_20) == false){
    pr_err("GPIO %d is not valid\n", GPIO_20);
    goto r_device;
  }
  if(gpio_is_valid(GPIO_16) == false){
    pr_err("GPIO %d is not valid\n", GPIO_16);
    goto r_device;
  }
  if(gpio_is_valid(GPIO_12) == false){
    pr_err("GPIO %d is not valid\n", GPIO_12);
    goto r_device;
  }
    if(gpio_is_valid(GPIO_1) == false){
    pr_err("GPIO %d is not valid\n", GPIO_1);
    goto r_device;
  }
  if(gpio_is_valid(GPIO_7) == false){
    pr_err("GPIO %d is not valid\n", GPIO_7);
    goto r_device;
  }
  if(gpio_is_valid(GPIO_8) == false){
    pr_err("GPIO %d is not valid\n", GPIO_8);
    goto r_device;
  }
  if(gpio_is_valid(GPIO_25) == false){
    pr_err("GPIO %d is not valid\n", GPIO_25);
    goto r_device;
  }
  //Requesting the GPIO
  if(gpio_request(GPIO_21,"GPIO_21") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_21);
    goto r_gpio;
  }
  if(gpio_request(GPIO_20,"GPIO_20") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_20);
    goto r_gpio;
  }
  if(gpio_request(GPIO_16,"GPIO_16") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_16);
    goto r_gpio;
  }
  if(gpio_request(GPIO_12,"GPIO_12") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_12);
    goto r_gpio;
  }
    if(gpio_request(GPIO_1,"GPIO_1") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_1);
    goto r_gpio;
  }
  if(gpio_request(GPIO_7,"GPIO_7") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_7);
    goto r_gpio;
  }
  if(gpio_request(GPIO_8,"GPIO_8") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_8);
    goto r_gpio;
  }
  if(gpio_request(GPIO_25,"GPIO_25") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_25);
    goto r_gpio;
  }
//configure the GPIO as output
  gpio_direction_output(GPIO_21, 0);
  gpio_direction_output(GPIO_20, 0);
  gpio_direction_output(GPIO_16, 0);
  gpio_direction_output(GPIO_12, 0);
  gpio_direction_output(GPIO_1, 0);
  gpio_direction_output(GPIO_7, 0);
  gpio_direction_output(GPIO_8, 0);
  gpio_direction_output(GPIO_25, 0);
/* Using this call the GPIO 21 will be visible in /sys/class/gpio/
** Now you can change the gpio values by using below commands also.
** echo 1 > /sys/class/gpio/gpio21/value (turn ON the LED)
** echo 0 > /sys/class/gpio/gpio21/value (turn OFF the LED)
** cat /sys/class/gpio/gpio21/value (read the value LED)
**
** the second argument prevents the direction from being changed.
*/
  gpio_export(GPIO_21, false);
  gpio_export(GPIO_20, false);
  gpio_export(GPIO_16, false);
  gpio_export(GPIO_12, false);
  gpio_export(GPIO_1, false);
  gpio_export(GPIO_7, false);
  gpio_export(GPIO_8, false);
  gpio_export(GPIO_25, false);
  pr_info("Device Driver Insert...Done(hw1)!!!\n");
  return 0;
r_gpio:
  gpio_free(GPIO_21);
  gpio_free(GPIO_20);
  gpio_free(GPIO_16);
  gpio_free(GPIO_12);
  gpio_free(GPIO_1);
  gpio_free(GPIO_7);
  gpio_free(GPIO_8);
  gpio_free(GPIO_25);
r_device:
  device_destroy(dev_class,dev);
r_class:
  class_destroy(dev_class);
r_del:
  cdev_del(&etx_cdev);
r_unreg:
  unregister_chrdev_region(dev,1);
return -1;
}

/*
** Module exit function
*/
static void __exit etx_driver_exit(void)
{
  gpio_unexport(GPIO_21);
  gpio_free(GPIO_21);
  gpio_unexport(GPIO_20);
  gpio_free(GPIO_20);
  gpio_unexport(GPIO_16);
  gpio_free(GPIO_16);
  gpio_unexport(GPIO_12);
  gpio_free(GPIO_12);
  gpio_unexport(GPIO_1);
  gpio_free(GPIO_1);
  gpio_unexport(GPIO_7);
  gpio_free(GPIO_7);
  gpio_unexport(GPIO_8);
  gpio_free(GPIO_8);
  gpio_unexport(GPIO_25);
  gpio_free(GPIO_25);
  device_destroy(dev_class,dev);
  class_destroy(dev_class);
  cdev_del(&etx_cdev);
  unregister_chrdev_region(dev, 1);
  pr_info("Device Driver Remove...Done!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);
MODULE_LICENSE("GPL");

MODULE_AUTHOR("randy");
MODULE_DESCRIPTION("eos hw1 led Driver");
MODULE_VERSION("1.32");
