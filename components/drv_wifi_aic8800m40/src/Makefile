RWNX_VERS_NUM := 6.5.2.7

# MODULE
CONFIG_AIC8800_WLAN_SUPPORT = m
MODULE_NAME = aic8800_netdrv

# Driver mode support list
CONFIG_VNET_MODE    ?= y
CONFIG_RAWDATA_MODE ?= n

# Insmod mode
CONFIG_FAST_INSMOD ?= n

# Msg Callback setting
CONFIG_APP_FASYNC ?= n

# Set thread priority
CONFIG_TXRX_THREAD_PRIO = y

CONFIG_SDIO_SUPPORT  = y
CONFIG_SDIO_IPC_FC   = n

CONFIG_RX_REORDER   ?= y
CONFIG_SDIO_PWRCTRL ?= n
ifeq ($(CONFIG_VNET_MODE), y)
CONFIG_TX_NETIF_FLOWCTRL = y
endif
ifeq ($(CONFIG_RAWDATA_MODE), y)
CONFIG_TX_NETIF_FLOWCTRL = n
endif

# FW ARCH:
CONFIG_RWNX_TL4 ?= n
CONFIG_RWNX_FULLMAC ?= y

# Enable HW queue for Broadcast/Multicast traffic (need FW support)
CONFIG_RWNX_BCMC ?= y

# Enable A-MSDU support (need FW support)
## Select this if FW is compiled with AMSDU support
CONFIG_RWNX_SPLIT_TX_BUF ?= n
## Select this TO send AMSDU
CONFIG_RWNX_AMSDUS_TX ?= n

# FW VARS
ccflags-y += -DNX_VIRT_DEV_MAX=4
ccflags-y += -DNX_REMOTE_STA_MAX=10
ccflags-y += -DNX_TXDESC_CNT=64

ccflags-$(CONFIG_VNET_MODE)    += -DCONFIG_VNET_MODE
ccflags-$(CONFIG_RAWDATA_MODE) += -DCONFIG_RAWDATA_MODE
ccflags-$(CONFIG_FAST_INSMOD)  += -DCONFIG_FAST_INSMOD
ccflags-$(CONFIG_APP_FASYNC)   += -DCONFIG_APP_FASYNC
ccflags-$(CONFIG_RX_REORDER)   += -DAICWF_RX_REORDER

ccflags-y += -I$(src)/inc/
ccflags-y += -I$(src)/sdio/.
ccflags-y += -DCONFIG_RWNX_FULLMAC
ccflags-$(CONFIG_RWNX_TL4) += -DCONFIG_RWNX_TL4
ccflags-$(CONFIG_RWNX_SPLIT_TX_BUF) += -DCONFIG_RWNX_SPLIT_TX_BUF
ccflags-$(CONFIG_TX_NETIF_FLOWCTRL) += -DCONFIG_TX_NETIF_FLOWCTRL
ccflags-$(CONFIG_TXRX_THREAD_PRIO)  += -DCONFIG_TXRX_THREAD_PRIO

ifeq ($(CONFIG_RWNX_SPLIT_TX_BUF), y)
ccflags-$(CONFIG_RWNX_AMSDUS_TX) += -DCONFIG_RWNX_AMSDUS_TX
endif

ifeq ($(CONFIG_SDIO_IPC_FC), y)
ccflags-y += -DCONFIG_SDIO_IPC_FC
endif

ifeq ($(CONFIG_RWNX_BCMC), y)
ccflags-y += -DNX_TXQ_CNT=5
else
ccflags-y += -DNX_TXQ_CNT=4
endif

obj-$(CONFIG_AIC8800_WLAN_SUPPORT) := $(MODULE_NAME).o
$(MODULE_NAME)-y :=       \
	src/rwnx_main.o           \
	src/rwnx_rx.o             \
	src/rwnx_tx.o             \
	src/rwnx_platform.o       \
	src/virt_net.o            \
	src/aicwf_txrxif.o        \
	sdio/sdio_host.o          \
	sdio/aicwf_sdio.o         \
	src/aicwf_custom_utils.o

KDIR  := /lib/modules/$(shell uname -r)/build
PWD   := $(shell pwd)
KVER := $(shell uname -r)
MODDESTDIR := /lib/modules/$(KVER)/kernel/drivers/net/wireless/aic8800
ARCH ?= x86_64
CROSS_COMPILE ?=
all: modules
modules:
	make -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

install:
	mkdir -p $(MODDESTDIR)
	install -p -m 644 $(MODULE_NAME).ko  $(MODDESTDIR)
	/sbin/depmod -a ${KVER}

uninstall:
	rm -rfv $(MODDESTDIR)/$(MODULE_NAME).ko
	/sbin/depmod -a ${KVER}

clean:
	rm -rf *.o *.ko *.o.* *.mod.* modules.* Module.* .a* .o* .*.o.* *.mod .tmp* .cache.mk
	rm -rf sdio/*.o sdio/*.ko sdio/*.o.* sdio/*.mod.* sdio/modules.* sdio/Module.* sdio/.a* sdio/.o* sdio/.*.o.* sdio/*.mod sdio/.tmp* sdio/.cache.mk
	rm -rf src/*.o src/*.ko src/*.o.* src/*.mod.* src/modules.* src/Module.* src/.a* src/.o* src/.*.o.* src/*.mod src/.tmp* src/.cache.mk


