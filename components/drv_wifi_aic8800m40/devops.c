
#include <aos/aos.h>

#include <sdio.h>
#include <soc.h>

// #include "src/sdio_func.h"

#define DEBUG_SDIO 0
#define CONFIG_SDC_ID 1

/* test wifi driver */
#define ADDR_MASK 0x10000
#define LOCAL_ADDR_MASK 0x00000

#define dbg_host printf

static sdio_card_t SDIO_Card = {0};

#define TAG "WiFiCONF"
int __sdio_bus_probe()
{
    LOGD(TAG, "%s", __FUNCTION__);

    memset(&SDIO_Card, 0, sizeof(SDIO_Card));
    SDIO_Card.usrParam.cd = NULL;
    SDIO_Card.host.base = csi_sdif_get_handle(0);

    int ret = 0;
    if (kStatus_Success != (ret= SDIO_Init(&SDIO_Card))) {
        LOGE(TAG, "SDIO Init failed ret=%d", ret);
        return -1;
    } else {
        LOGD(TAG, "SDIO Init success");
        aos_msleep(100); /** wait card reset */
    }

    //set force to 512
    if (SDIO_SetBlockSize(&SDIO_Card, kSDIO_FunctionNum1, 512) != kStatus_Success) {
        LOGE(TAG, "SDIO_SetBlockSize 512 error");
        return  1;
    } else {
        LOGD(TAG, "SDIO_SetBlockSize 512 success");
    }
    return 0;
}

int app_wifi_init(void)
{
    __sdio_bus_probe();
    return 0;
}