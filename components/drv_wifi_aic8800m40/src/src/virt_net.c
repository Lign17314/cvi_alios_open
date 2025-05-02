#include <linux/module.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/ip.h>
#include "rwnx_tx.h"
#include "rwnx_defs.h"

#ifdef CONFIG_VNET_MODE
struct net_device *vnet_dev;
extern  void rwnx_netdev_setup(struct net_device *dev);
 int virt_net_init(struct rwnx_vif  *vif)
{
    #if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
    vnet_dev = alloc_netdev_mqs(0, "vnet%d", rwnx_netdev_setup, NX_NB_NDEV_TXQ, 1);
    #else
    vnet_dev = alloc_netdev_mqs(0, "vnet%d", 0, rwnx_netdev_setup, NX_NB_NDEV_TXQ, 1);
    #endif

    if (!vnet_dev)
        return -1;

    printk("virt_net_init %p\n", vnet_dev);

    //vif = netdev_priv(vnet_dev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0)
    memcpy(vnet_dev->dev_addr, vif->wdev.address, 6);
#else
    memcpy(vnet_dev->dev_addr, vif->address, 6);
#endif
    vnet_dev->mtu = 1480;
    register_netdev(vnet_dev);

    return 0;
}

void virt_net_exit(void)
{
    //printk("virt_net_exit %p\n", vnet_dev);
    if(vnet_dev && (vnet_dev->reg_state == NETREG_REGISTERED))  {
        unregister_netdev(vnet_dev);
    }
    //free_netdev(vnet_dev);
}
#endif
