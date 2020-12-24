// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2020 Ryuichi Ueda.  All rights reserved.
 */


#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>    //追加
#include <linux/uaccess.h>    //ヘッダに追加
#include <linux/io.h>   //ヘッダファイルを追加
#include <linux/delay.h>

MODULE_AUTHOR("Ryuichi Ueda and Hidetoshi Kawano");
MODULE_DESCRIPTION("task1 about controlling  driver for LED control");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.3");

static dev_t dev;
static struct cdev cdv;
static struct class *cls = NULL;  //追加
static volatile u32 *gpio_base = NULL;  //アドレスをマッピングするための配列 をグローバルで定義

static ssize_t led_write(struct file* filp, const char* buf, size_t count, loff_t* pos)
{
	        char c;//読み込んだ字を入れる変数
		 if(copy_from_user(&c,buf,sizeof(char)))
		   return -EFAULT;
		printk(KERN_INFO "receive %c\n",c);
		
	        if(c == '0') {
		   gpio_base[10] = 1 << 25;
		   gpio_base[10] = 1 << 24;
		   gpio_base[10] = 1 << 18;
		}
		 else if(c == '1') {
		   gpio_base[7] = 1 << 25;
		   gpio_base[10] = 1 << 24;
		   gpio_base[10] = 1 << 18;
		}
		 else if(c == '2') {
		   gpio_base[10] = 1 <<25;
		   gpio_base[7] = 1 << 24;
		   gpio_base[10] = 1 << 18;
		 }
				                                                                                                                                         else if(c == '3') {
		gpio_base[10] = 1 <<25;
		gpio_base[10] = 1 << 24;
		gpio_base[7] = 1 << 18;
	}
		else if(c == 's') { //echo ''/dev/myled0の箇所 s:startの略
	//赤・黄・青の順でledが点灯し、10秒たったら全てが点滅するよ うにする
	//base[7]:点灯　・　base[10]:消灯
		gpio_base[7] = 1 << 25; //赤
		gpio_base[10] = 1 << 24;
		gpio_base[10] = 1 << 18;
	
		mdelay(1000);
				
		gpio_base[10] = 1 << 25;
		gpio_base[7] = 1 << 24; //黄
		gpio_base[10] = 1 << 18;
	
		mdelay(1000);
		
		gpio_base[10] = 1 << 25;
		gpio_base[10] = 1 << 24;
		gpio_base[7] = 1 << 18; //青
				
		mdelay(1000);				                                                                                                                                                                                                                                                                                                                                                                                                                                         //全灯
		gpio_base[7] = 1 << 25;
		gpio_base[7] = 1 << 24;
		gpio_base[7] = 1 << 18;
	
		mdelay(100);

		//全消灯
		
		gpio_base[10] = 1 << 25;
		gpio_base[10] = 1 << 24;
		gpio_base[10] = 1 << 18;
		
		mdelay(1000);

		int i = 0;
	
		for (; i < 5; i++) {
		 gpio_base[7] = 1 << 25;
		 gpio_base[7] = 1 << 24;
		 gpio_base[7] = 1 << 18;

		 mdelay(100);
	
		 gpio_base[10] = 1 << 25;
		 gpio_base[10] = 1 << 24;
		 gpio_base[10] = 1 << 18;
		}
	
		return 1; //読み込んだ文字数を返す（この場合はダミーの1）
	}

static ssize_t sushi_read(struct file* filp, char* buf, size_t count, loff_t* pos)
{
	int size = 0;
	char sushi[] = {'s','u','s','h','i',0x0A}; //寿司の絵文字のバイナリ
	if(copy_to_user(buf+size,(const char *)sushi, sizeof(sushi))){
	 printk( KERN_INFO "sushi : copy_to_user failed\n" );
	 return -EFAULT;
	 }
	 
	size += sizeof(sushi);
	return size;
	}

static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.write = led_write,
	.read = sushi_read
};

static int __init init_mod(void) //カーネルモジュールの初期化
{
	int retval;
	retval =  alloc_chrdev_region(&dev, 0, 1, "myled");
	if(retval < 0){
	printk(KERN_ERR "alloc_chrdev_region failed.\n");
	return retval;
}
	printk(KERN_INFO "%s is loaded. major:%d\n",__FILE__,MAJOR(dev));

	cdev_init(&cdv, &led_fops);
	retval = cdev_add(&cdv, dev, 1);
	if(retval < 0){
	printk(KERN_ERR "cdev_add failed. major:%d, minor:%d",MAJOR(dev),MINOR(dev));
	return retval;
	}
	cls = class_create(THIS_MODULE,"myled");   //ここから追加
	
	if(IS_ERR(cls)){
	 printk(KERN_ERR "class_create failed.");
	return PTR_ERR(cls);
	}
	
	device_create(cls, NULL, dev, NULL, "myled%d",MINOR(dev));

	gpio_base = ioremap_nocache(0xfe200000,0xA0);

	const u32 led = 25;
	const u32 index = led/10;//GPFSEL2
	const u32 shift = (led%10)*3;//15bit
	const u32 mask = ~(0x7 << shift);//11111111111111000111111111111111
	gpio_base[index] = (gpio_base[index] & mask) | (0x1 << shift);//001: output flag
//11111111111111001111111111111111


	const u32 led2 = 24;
	const u32 index2 = led2/10;
	const u32 shift2 = (led2%10)*3;
	const u32 mask2 = ~(0x7 << shift2);
	gpio_base[index2] = (gpio_base[index2] & mask2) | (0x1 << shift2);

	const u32 led3 = 18;
	const u32 index3 = led3/10;
	const u32 shift3 = (led3%10)*3;
	const u32 mask3 = ~(0x7 << shift3);
	gpio_base[index3] = (gpio_base[index3] & mask3) | (0x1 << shift3);


	return 0;
}

static void __exit cleanup_mod(void) //後始末
{
	cdev_del(&cdv);
	device_destroy(cls, dev);
	class_destroy(cls);  //追加
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "%s is unloaded.\n",__FILE__,MAJOR(dev));
}
module_init(init_mod);     // マクロで関数を登録
module_exit(cleanup_mod);  // 同上
