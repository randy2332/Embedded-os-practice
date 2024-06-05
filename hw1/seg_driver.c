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
 
#define GPIO_9 (9) // E
#define GPIO_10 (10)//D
#define GPIO_22 (22)//C
#define GPIO_27 (27)//B
#define GPIO_17 (17)//A
#define GPIO_4 (4)//F
#define GPIO_3 (3)//G

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

  //write to user
  len = 1;
  if( copy_to_user(buf, &gpio_state, len) > 0) {
    pr_err("ERROR: Not all the bytes have been copied to user\n");
  }
  pr_info("Read function : GPIO_21 = %d \n", gpio_state);
  return 0;
}

static int num[10][7]={
        {1,1,1,1,1,1,0}, // 0
        {0,1,1,0,0,0,0}, // 1
        {1,1,0,1,1,0,1}, // 2
        {1,1,1,1,0,0,1}, // 3
        {0,1,1,0,0,1,1}, // 4
        {1,0,1,1,0,1,1}, // 5
        {1,0,1,1,1,1,1}, // 6
        {1,1,1,0,0,0,0}, // 7
        {1,1,1,1,1,1,1}, // 8
        {1,1,1,1,0,1,1}, // 9
    };

static void set_gpio_segments(int *segment_mask) {
    gpio_set_value(GPIO_17, segment_mask[0]);//A
    gpio_set_value(GPIO_27, segment_mask[1]);//B
    gpio_set_value(GPIO_22, segment_mask[2]);//C
    gpio_set_value(GPIO_10, segment_mask[3]);//D
    gpio_set_value(GPIO_9, segment_mask[4]);//E
    gpio_set_value(GPIO_4, segment_mask[5]);//F
    gpio_set_value(GPIO_3, segment_mask[6]);//G
}


/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off){
    int price =0;
    int digits[10]; // Assuming price can have up to 10 digits
    int count = 0;
    
    if( copy_from_user( &price, buf, len) > 0) {
      pr_err("ERROR: Not all the bytes have been copied from user\n");
    }
    else{
      pr_info("Copied data from user price: %d\n",price);
    }
    
    // Extract digits and store them in the array
    while (price > 0) {
        digits[count] = price % 10; // Extract the last digit
        price /= 10; // Remove the last digit
        count++; // Move to the next array index
    }
    // show on led
    for (int i = count - 1; i >= 0; i--) {
        fsleep(500000);
        pr_info("%d\n", digits[i]); // Print each digit
        
        set_gpio_segments(num[digits[i]]);
        
    }
    
    
   
    return len;
}	



/*
** Module Init function
*/
static int __init etx_driver_init(void)
{
  /*Allocating Major number*/
  if((alloc_chrdev_region(&dev, 0, 1, "mydev")) <0){
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
  if((dev_class = class_create(THIS_MODULE,"seg_class")) == NULL){
    pr_err("Cannot create the struct class\n");
    goto r_class;
   }
/*Creating device*/
  if((device_create(dev_class,NULL,dev,NULL,"mydev")) == NULL){
    pr_err( "Cannot create the Device \n");
    goto r_device;
  }
  
  //Checking the GPIO is valid or not
  if(gpio_is_valid(GPIO_9) == false){
    pr_err("GPIO %d is not valid\n", GPIO_9);
    goto r_device;
  }
  if(gpio_is_valid(GPIO_10) == false){
    pr_err("GPIO %d is not valid\n", GPIO_10);
    goto r_device;
  }
  if(gpio_is_valid(GPIO_22) == false){
    pr_err("GPIO %d is not valid\n", GPIO_22);
    goto r_device;
  }
  if(gpio_is_valid(GPIO_27) == false){
    pr_err("GPIO %d is not valid\n", GPIO_27);
    goto r_device;
  }
    if(gpio_is_valid(GPIO_17) == false){
    pr_err("GPIO %d is not valid\n", GPIO_17);
    goto r_device;
  }
  if(gpio_is_valid(GPIO_4) == false){
    pr_err("GPIO %d is not valid\n", GPIO_4);
    goto r_device;
  }
  if(gpio_is_valid(GPIO_3) == false){
    pr_err("GPIO %d is not valid\n", GPIO_3);
    goto r_device;
  }
 
  //Requesting the GPIO
  if(gpio_request(GPIO_9,"GPIO_9") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_9);
    goto r_gpio;
  }
  if(gpio_request(GPIO_10,"GPIO_10") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_10);
    goto r_gpio;
  }
  if(gpio_request(GPIO_22,"GPIO_22") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_22);
    goto r_gpio;
  }
  if(gpio_request(GPIO_27,"GPIO_27") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_27);
    goto r_gpio;
  }
    if(gpio_request(GPIO_17,"GPIO_17") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_17);
    goto r_gpio;
  }
  if(gpio_request(GPIO_4,"GPIO_4") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_4);
    goto r_gpio;
  }
  if(gpio_request(GPIO_3,"GPIO_3") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_3);
    goto r_gpio;
  }
 
//configure the GPIO as output
  gpio_direction_output(GPIO_9, 0);
  gpio_direction_output(GPIO_10, 0);
  gpio_direction_output(GPIO_22, 0);
  gpio_direction_output(GPIO_27, 0);
  gpio_direction_output(GPIO_17, 0);
  gpio_direction_output(GPIO_4, 0);
  gpio_direction_output(GPIO_3, 0);
/* Using this call the GPIO 21 will be visible in /sys/class/gpio/
** Now you can change the gpio values by using below commands also.
** echo 1 > /sys/class/gpio/gpio21/value (turn ON the LED)
** echo 0 > /sys/class/gpio/gpio21/value (turn OFF the LED)
** cat /sys/class/gpio/gpio21/value (read the value LED)
**
** the second argument prevents the direction from being changed.
*/
  gpio_export(GPIO_9, false);
  gpio_export(GPIO_10, false);
  gpio_export(GPIO_22, false);
  gpio_export(GPIO_27, false);
  gpio_export(GPIO_17, false);
  gpio_export(GPIO_4, false);
  gpio_export(GPIO_3, false);

  pr_info("Device Driver Insert...Done(hw1 -seg)!!!\n");
  return 0;
r_gpio:
  gpio_free(GPIO_9);
  gpio_free(GPIO_10);
  gpio_free(GPIO_22);
  gpio_free(GPIO_27);
  gpio_free(GPIO_17);
  gpio_free(GPIO_4);
  gpio_free(GPIO_3);
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
  gpio_unexport(GPIO_9);
  gpio_free(GPIO_9);
  gpio_unexport(GPIO_10);
  gpio_free(GPIO_10);
  gpio_unexport(GPIO_22);
  gpio_free(GPIO_22);
  gpio_unexport(GPIO_27);
  gpio_free(GPIO_27);
  gpio_unexport(GPIO_17);
  gpio_free(GPIO_17);
  gpio_unexport(GPIO_4);
  gpio_free(GPIO_4);
  gpio_unexport(GPIO_3);
  gpio_free(GPIO_3);

  device_destroy(dev_class,dev);
  class_destroy(dev_class);
  cdev_del(&etx_cdev);
  unregister_chrdev_region(dev, 1);
  pr_info("Device Driver Remove(7 seg )...Done!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);
MODULE_LICENSE("GPL");

MODULE_AUTHOR("randy");
MODULE_DESCRIPTION("eos hw1 seg Driver");
MODULE_VERSION("1.32");
