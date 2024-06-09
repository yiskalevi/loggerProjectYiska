#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/wait.h>

#define BUFFER_SIZE 4096

DECLARE_WAIT_QUEUE_HEAD (mywq);
static int nparam = 5;
module_param(nparam, int, 0644);

dev_t dev;
int units = 2;


static struct mod_priv {
  struct cdev mod_cdev;
  struct class *mod_cl;
  struct device *mod_dev;
  int buffer_size;
  char buffer[BUFFER_SIZE];
  int p_write;
  int p_read;
  wait_queue_head_t read_queue;
  wait_queue_head_t write_queue;
} mod_priv;


static int my_open(struct inode *ino, struct file *filp);
static int my_release(struct inode *ino, struct file *filp);
static ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
};
static int __init trivmod_init (void)
{
  int err;

  printk (KERN_INFO "triv_mod loaded\n");
  err = alloc_chrdev_region (&dev, 0, units, "CourseDev");
  if (err < 0)
    goto dvr_err;
  cdev_init (&mod_priv.mod_cdev, &fops);
  err = cdev_add (&mod_priv.mod_cdev, dev, units);
  if (err < 0)
    goto cdev_err;
  //mod_priv.mod_cl = class_create (THIS_MODULE, "CourseDev");
  mod_priv.mod_cl = class_create ("CourseDev");
  if (!mod_priv.mod_cl) {
    err = -ENOMEM;
    goto clc_err;
  }
  mod_priv.mod_dev = device_create (mod_priv.mod_cl, NULL, dev, NULL, "myDev");
  if (!mod_priv.mod_dev) {
    err = -ENOMEM;
    goto dvc_err;
  }
  mod_priv.buffer_size=BUFFER_SIZE;
  mod_priv.p_write=0;
  mod_priv.p_read=0;
  init_waitqueue_head (&mod_priv.read_queue);
  init_waitqueue_head (&mod_priv.write_queue);
  return 0;
dvc_err:
  class_destroy (mod_priv.mod_cl);
clc_err:
  cdev_del (&mod_priv.mod_cdev);
cdev_err:
  unregister_chrdev_region (dev, units);
dvr_err:

  return err;
}

static void __exit trivmod_exit (void)
{
  device_destroy (mod_priv.mod_cl, dev);
  class_destroy (mod_priv.mod_cl);
  cdev_del (&mod_priv.mod_cdev);
  unregister_chrdev_region (dev, units);
  printk (KERN_INFO "triv_mod unloaded\n");
  return;
}

static int my_open(struct inode *ino, struct file *filp) {
   
    filp->private_data = container_of (ino->i_cdev,struct mod_priv, mod_cdev);
    printk (KERN_INFO "my_open\n");
    return 0;
}

static int my_release(struct inode *ino, struct file *filp) {
    printk (KERN_INFO "my_release\n");
    return 0;
}


static ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    struct mod_priv *priv = filp->private_data;
    ssize_t ret = 0; 
    size_t available_data;
     int isSignal;
start_read:
    // calculate available data
    if (priv->p_read <= priv->p_write) {
        available_data = priv->p_write - priv->p_read;
    } else {
        available_data = priv->buffer_size - priv->p_read + priv->p_write;
    }

    // if there's no data to read, return 0 
    if (available_data == 0) {
        if(filp->f_flags & O_NONBLOCK){
            printk(KERN_INFO "You have reached the end of the file\n");
            return 0;
        }
        else{
            isSignal =wait_event_interruptible(priv->read_queue, priv->p_read != priv->p_write);
            if(isSignal!=0){
                printk(KERN_INFO "A signal was received and read was not completed\n");
            return -EFAULT;
            }
            goto start_read;
        }
    }

    // limit count to available data
    if (count > available_data) {

        if(filp->f_flags & O_NONBLOCK){
            count = available_data;
            printk(KERN_INFO "Not enough data to read. Available: %zu\n", available_data);
        }
        else{
            if(count>priv->buffer_size){
                printk(KERN_INFO "The count is greater than the buffer size: %zu\n", count);
                return -EFAULT;
            }
            isSignal =wait_event_interruptible(priv->read_queue, (((priv->p_read <= priv->p_write)&&((priv->p_write - priv->p_read)>=count))||(!(priv->p_read <= priv->p_write)&&((priv->buffer_size - priv->p_read + priv->p_write)>=count))));
            if(isSignal!=0){
                printk(KERN_INFO "A signal was received and read was not completed\n");
                return -EFAULT;
            }
            goto start_read;
        }

    }

    // reading in circular
    if (priv->p_read + count <= priv->buffer_size) {
        if (copy_to_user(buf, priv->buffer + priv->p_read, count)) {
            ret = -EFAULT;
        } else {
            priv->p_read = (priv->p_read + count) % priv->buffer_size;
            ret = count;
            printk(KERN_INFO "my_read: read %zu bytes\n", count);
        }
    } else {
        size_t first_part = priv->buffer_size - priv->p_read;
        size_t second_part = count - first_part;

        if (copy_to_user(buf, priv->buffer + priv->p_read, first_part) ||
            copy_to_user(buf + first_part, priv->buffer, second_part)) {
            ret = -EFAULT;
        } else {
            priv->p_read = second_part;
            ret = count;
            printk(KERN_INFO "my_read: read %zu bytes\n", count);
        }
    }
    
    wake_up_interruptible (&priv->write_queue);
    return ret;
}


static ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    struct mod_priv *priv = filp->private_data;
    ssize_t ret = 0;
    size_t available_space;
    size_t org_count=count;
    int isSignal;
start_write:
    // calculate available space in buffer
    if (priv->p_write >= priv->p_read) {
        available_space = priv->buffer_size - priv->p_write + priv->p_read;
    } else {
        available_space = priv->p_read - priv->p_write;
    }

    // if there's no space to write, return 0 (buffer full)
    if (available_space == 0) {
        if(filp->f_flags & O_NONBLOCK){
            printk(KERN_INFO "No space left in the buffer to write\n");
            return 0;
        }
        else{
            isSignal =wait_event_interruptible(priv->write_queue, ((priv->buffer_size - priv->p_write + priv->p_read)>0)||((priv->p_read - priv->p_write)>0));
            if(isSignal!=0){
                printk(KERN_INFO "A signal was received and read was not completed\n");
                return -EFAULT;
            }
            goto start_write;
        }
    }

    // limit count to available space
    if (count > available_space) {
        
        if(filp->f_flags & O_NONBLOCK){
            count = available_space;
            printk(KERN_INFO "No space left in the buffer to write\n");
        }
        else{
            if(count>priv->buffer_size){
                printk(KERN_INFO "The count is greater than the buffer size: %zu\n", count);
                return -EFAULT;
            }
            isSignal =wait_event_interruptible(priv->write_queue,(((priv->p_write >= priv->p_read)&&((priv->buffer_size - priv->p_write + priv->p_read)>=count))||(!(priv->p_write >= priv->p_read)&&((priv->p_read - priv->p_write)>=count))));
            if(isSignal!=0){
                printk(KERN_INFO "A signal was received and read was not completed\n");
                return -EFAULT;
            }
            goto start_write;
        }
    }
    // writing in circular manner
    if (priv->p_write + count <= priv->buffer_size) {
        if (copy_from_user(priv->buffer + priv->p_write, buf, count)) {
            ret = -EFAULT;
        } else {
            priv->p_write = (priv->p_write + count) % priv->buffer_size;
            ret = count;
            printk(KERN_INFO "my_write: wrote %zu bytes\n", count);
        }
    } else {
        size_t first_part = priv->buffer_size - priv->p_write;
        size_t second_part = count - first_part;

        if (copy_from_user(priv->buffer + priv->p_write, buf, first_part) ||
            copy_from_user(priv->buffer, buf + first_part, second_part)) {
            ret = -EFAULT;
        } else {
            priv->p_write = second_part;
            ret = count;
            printk(KERN_INFO "my_write: wrote %zu bytes\n", count);
        }
    }
    // if less data was written than requested, return 0 and print the amount written
    if (ret < org_count) {
        printk(KERN_INFO "Not enough space to write. Available: %zu\n", available_space);
        wake_up_interruptible (&priv->read_queue);
        return 0;
    }
    printk(KERN_INFO "wake_up_interruptible read_queue\n");
    wake_up_interruptible (&priv->read_queue);
    return ret;
}

MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
MODULE_AUTHOR("Instructor");
MODULE_PARM_DESC(nparam, "A numeric demonstration parameter");
MODULE_DESCRIPTION("A trivial exercise module");

module_init(trivmod_init);
module_exit(trivmod_exit);
