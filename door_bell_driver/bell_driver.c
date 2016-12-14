#include "modules.h"
#include "gpios.h"
#include "intrp.h"
#include "cdevice.h"


#define DEV_NAME "bell" //设备名称
#define DEV_COUNT 1	//申请连续设备号的个数
#define DEV_MINOR 0
#define IRQ_NAME "bell_irq"//中断名称
int major_dev = 0;	//主设备号
int minor_dev = 0;	//次设备号
dev_t dev_num;		//设备号

static DECLARE_WAIT_QUEUE_HEAD(bell_wq);//定义并初始化等待头队列
static volatile int have_irq = 0;

static irqreturn_t eint9_interrupt(int irq,void * dev_id){
	printk(KERN_EMERG "irq success\n");
	have_irq = 1;
	wake_up_interruptible(&bell_wq);
	printk(KERN_EMERG "have_irq:%d\n",have_irq);
	return IRQ_HANDLED;
}

static int req_gpio(){
	int ret;
	/*request home key gpio*/
	ret = gpio_request(EXYNOS4_GPX1(1),"home_key");		
	if(ret < 0){
		printk(KERN_EMERG "home_key gpio request failure:%d,%s\n",__LINE__,__FUNCTION__);
		return ret;
	}
	s3c_gpio_cfgpin(EXYNOS4_GPX1(1),S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(EXYNOS4_GPX1(1),S3C_GPIO_PULL_UP);

	/*蜂鸣器gpio申请*/
	ret = gpio_request(EXYNOS4_GPD0(0),"beep");		
	if(ret < 0){
		printk(KERN_EMERG "beep gpio request failure:%d,%s\n",__LINE__,__FUNCTION__);
		return ret;
	}
	s3c_gpio_cfgpin(EXYNOS4_GPD0(0),S3C_GPIO_OUTPUT);
	gpio_set_value(EXYNOS4_GPD0(0),0);

	return 0;	
}

static int bell_open(struct inode * inode,struct file * file){

	int ret;
	printk(KERN_EMERG "bell node open success\n");
	
	ret = req_gpio();
	if(ret < 0){

		printk(KERN_EMERG "req_gpio request failure:%d,%s\n",__LINE__,__FUNCTION__);
		return ret;
	}
	ret = request_irq(IRQ_EINT(9),eint9_interrupt,IRQ_TYPE_EDGE_FALLING,IRQ_NAME,NULL);

	if(ret < 0){

		printk(KERN_EMERG "irq request failure:%d%s\n",__LINE__,__FUNCTION__);
		return ret;
	}
	printk(KERN_EMERG "irq request success\n");

	return 0;
}

static int bell_release(struct inode * inode,struct file * file){
	printk(KERN_EMERG "bell node release success\n");
	gpio_free(EXYNOS4_GPX1(1));
	gpio_free(EXYNOS4_GPD0(0));
	free_irq(IRQ_EINT(9),NULL);
	return 0;
}

static long bell_ioctl(struct file * file,unsigned int cmd,unsigned long arg){
	printk(KERN_EMERG "bell node ioctl success:\n");
	printk(KERN_EMERG "cmd is%d,arg is %ld\n",cmd,arg);
	gpio_set_value(EXYNOS4_GPD0(0),cmd);
	return 0;
}

static ssize_t bell_read(struct file * file,char __user * buf,size_t length,loff_t * pos){
	wait_event_interruptible(bell_wq,have_irq==1);
	int dev_id = 1001;
	if(copy_to_user(buf,&dev_id,4)){
		printk(KERN_EMERG "copy_to_user is failed:%d%s\n",__LINE__,__FUNCTION__);
		return -EFAULT;
	}

	have_irq = 0;
	return length;

}


struct file_operations bell_fops={
	.owner = THIS_MODULE,
	.open = bell_open,
	.read = bell_read,
	.release = bell_release,
	.unlocked_ioctl=bell_ioctl,
};

struct  bell_cdev{
	struct cdev cdev;
};

struct bell_cdev *bell_dev;
static struct class * bell_class;

static void create_node(){
	bell_class = class_create(THIS_MODULE,DEV_NAME);
	device_create(bell_class,NULL,dev_num,NULL,DEV_NAME);
}

static int dev_reg(struct bell_cdev *dev){
	int err;
	cdev_init(&dev->cdev,&bell_fops);
	dev->cdev.owner=THIS_MODULE;
	dev->cdev.ops = &bell_fops;
	err = cdev_add(&dev->cdev,dev_num,1);
	if(err){
		printk(KERN_EMERG "cdev_add is failure:%d%s\n",__LINE__,__FUNCTION__);
		return err;
	}
	else{
		printk(KERN_EMERG "cdev_add is success\n");
		create_node();
		return 0;
	}

}

static int device_init(){
	int dev_ret;
	int ret;

	/*动态申请设备号*/
	dev_ret = alloc_chrdev_region(&dev_num,DEV_MINOR,DEV_COUNT,DEV_NAME);
	if(dev_ret != 0){
		printk(KERN_EMERG "device number failure to alloc:%d%s\n",__LINE__,__FUNCTION__);
		return dev_ret;
	}

	printk(KERN_EMERG "device number succeed to alloc:%d\n",MAJOR(dev_num));

	bell_dev = kmalloc(sizeof(struct bell_cdev),GFP_KERNEL);
	if(!bell_dev){
		printk(KERN_EMERG "cdev kmalloc failure:%d%s\n",__LINE__,__FUNCTION__);
		return 1;

	}
	memset(bell_dev,0,sizeof(struct bell_cdev));	
	ret = dev_reg(bell_dev);//register device
	/*if(ret == 0){
	  ret = req_gpio();
	  if(!ret)
	  return ret;
	  }*/
	return 0;
}

static void device_exit(){

	printk(KERN_EMERG "bell exit\n");
	//free_irq(IRQ_EINT(9),NULL);
	device_destroy(bell_class,dev_num);
	class_destroy(bell_class);
	cdev_del(&bell_dev->cdev);
	kfree(bell_dev);
	unregister_chrdev_region(dev_num,DEV_COUNT);

}


module_init(device_init);
module_exit(device_exit);
