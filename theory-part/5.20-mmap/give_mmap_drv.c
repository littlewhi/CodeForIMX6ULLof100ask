
#include "asm/memory.h"
#include "asm/page.h"

#include <linux/module.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <asm/pgtable.h>
#include <linux/mm.h>
#include <linux/slab.h>

static struct class *cl;
static unsigned int major;

static char *kernel_buf = NULL;
#ifndef KERNEL_BUF_SIZE 
#define KERNEL_BUF_SIZE ( 1024u << 2 )
#define MIN( x, y ) ( (x) < (y) ? (x) : (y) )
#endif

static ssize_t read( struct file * filp, char __user *buf, size_t size, loff_t *lp )
{
    size_t requestSize = MIN( size, KERNEL_BUF_SIZE);
    
    if( copy_to_user( buf, kernel_buf, requestSize ) )
    {
        printk( "ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__ );
        return -1;
    }

    return requestSize;
}


static int my_mmap ( struct file *file, struct vm_area_struct *vma )
{
    char *phy_addr;
    unsigned long size = vma->vm_end - vma->vm_start;
    if(size > KERNEL_BUF_SIZE)
        return -1;

    /* 获得物理地址 */
    phy_addr = (char *) virt_to_phys( kernel_buf );

    /* 设置属性 */
    vma->vm_page_prot = pgprot_writecombine( vma->vm_page_prot ); /* 不适用cache，使用buffer */

    /* 映射 */
    if( remap_pfn_range( vma, vma->vm_start, 
        (unsigned long) phy_addr >> PAGE_SHIFT, 
        size, vma->vm_page_prot ) )
    {
        printk( "ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__ );
        return -1;
    }

    return 0;
}

struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = read,
    .mmap = my_mmap,
};

static int drv_init( void )
{
    kernel_buf = kmalloc( KERNEL_BUF_SIZE, GFP_KERNEL );
    if( kernel_buf == NULL )
    {
        printk( "ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__ );
        return -1;
    }

    major = register_chrdev( 0, "yhb_mem", &fops );
    if( major < 0 )
    {
        printk( "ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__ );
        return -1;
    }

    cl = class_create( THIS_MODULE, "yhb_mem" );
    if( IS_ERR( cl ) )
    {
        unregister_chrdev( major, "yhb_mem" );
        printk( "ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__ );
        return -1;
    }

    if( IS_ERR( device_create( cl, NULL, MKDEV( major, 0 ), NULL, "yhb_mem" ) ) )
    {
        unregister_chrdev( major, "yhb_mem" );
        class_destroy( cl );
        printk( "ERROR : %s %s %d\n", __FILE__, __FUNCTION__, __LINE__ );
        return -1;
    }

    return 0;
}

static void drv_exit( void )
{
    kfree( kernel_buf );
    device_destroy( cl, MKDEV( major, 0 ) );
    class_destroy( cl );
    unregister_chrdev( major, "yhb_mem" );
}

module_init( drv_init );
module_exit( drv_exit );
MODULE_LICENSE( "GPL" );