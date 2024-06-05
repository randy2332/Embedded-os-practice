#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h> //copy_to/from_user()



static const char* seg_for_c[27] = {
        "1111001100010001", // A
        "0000011100000101", // b
        "1100111100000000", // C
        "0000011001000101", // d
        "1000011100000001", // E
        "1000001100000001", // F
        "1001111100010000", // G
        "0011001100010001", // H
        "1100110001000100", // I
        "1100010001000100", // J
        "0000000001101100", // K
        "0000111100000000", // L
        "0011001110100000", // M
        "0011001110001000", // N
        "1111111100000000", // O
        "1000001101000001", // P
        "0111000001010000", //q
        "1110001100011001", //R
        "1101110100010001", //S
        "1100000001000100", //T
        "0011111100000000", //U
        "0000001100100010", //V
        "0011001100001010", //W
        "0000000010101010", //X
        "0000000010100100", //Y
        "1100110000100010", //Z
        "0000000000000000"
    };

static  char led[17]; // 16 characters for LED pattern + 1 for null terminator



/*
** This function will be called when we open the Device file
*/
static int my_open(struct inode *inode, struct file *file)
{
  pr_info("Device File Opened...!!!\n");
  return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t  my_read(struct file *filp, char __user *buf, size_t len, loff_t *fops)
{
  // let alpha to string
  if (copy_to_user(buf, led, sizeof(led)) != 0) {
      pr_err("Failed to copy LED pattern to user\n");
      return -EFAULT;
  }
  return sizeof(led);
  

  
}

/*
** This function will be called when we write the Device file
*/
static ssize_t  my_write(struct file *filp, const char __user *buf, size_t len, loff_t *fops)
{
    char rec_buf; // only one char at one time

    if (copy_from_user(&rec_buf, buf, len) != 0) {
        pr_err("ERROR: Not all characters have been copied from    user\n");
        return -EFAULT; // Error code for bad address
    }
    
    // Print the received character
    //pr_info("Received character: %c\n", name);
    
    int index;
    if (rec_buf >= 'A' && rec_buf <= 'Z')
        index = rec_buf - 'A'; //char to int
    else if (rec_buf >= 'a' && rec_buf <= 'z')
        index = rec_buf - 'a';
    else
        index = 26; // If character is not a letter, use default pattern (all LEDs off)

    //pr_info("Received character to seg_for_c index: %d\n", index);
    
    const char *pattern = seg_for_c[index];
    
    strcpy(led, pattern);

    // Print the LED pattern for verification
    pr_info("LED pattern for character %c: %s\n", rec_buf, led);
    
    return len;
}


/*
** Module Init function
*/



struct file_operations my_fops =
{
   read:my_read,
   write:my_write,
   open:my_open,
};

# define MAJOR_NUM 255
# define DEVICE_NAME "mydev"


static int __init my_init(void){
    printk("call init\n");
    if( register_chrdev(MAJOR_NUM, DEVICE_NAME,&my_fops) < 0 ){
        printk("Can not get major number %d\n",MAJOR_NUM);
        return(-EBUSY); // EBUSY define in errno.h
    }
    
    printk("My device is started and the major is %d\n",MAJOR_NUM);
    return 0;
    

}

static void __exit my_exit(void){
    unregister_chrdev(MAJOR_NUM,DEVICE_NAME);
    printk("Call exit\n");
}



module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("randy ");
MODULE_DESCRIPTION("A simple device driver ");
MODULE_VERSION("1.32");
