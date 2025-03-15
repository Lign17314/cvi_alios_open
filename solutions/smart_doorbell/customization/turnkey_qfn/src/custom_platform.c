#include "platform.h"
#include <drv/pin.h>
#include <pinctrl-mars.h>
#include "cvi_type.h"
#include <mmio.h>
#include <stdint.h>

#define GPIO_SPKEN_GRP 4
#define GPIO_SPKEN_NUM 2

#define GPIO_PIN_MASK(_gpio_num) (1 << _gpio_num)

static void _GPIOSetValue(u8 gpio_grp, u8 gpio_num, u8 level)
{
	csi_error_t ret;
	csi_gpio_t gpio = {0};

	ret = csi_gpio_init(&gpio, gpio_grp);
	if(ret != CSI_OK) {
		printf("csi_gpio_init failed\r\n");
		return;
	}
	// gpio write
	ret = csi_gpio_dir(&gpio , GPIO_PIN_MASK(gpio_num), GPIO_DIRECTION_OUTPUT);

	if(ret != CSI_OK) {
		printf("csi_gpio_dir failed\r\n");
		return;
	}
	csi_gpio_write(&gpio , GPIO_PIN_MASK(gpio_num), level);
	//printf("test pin end and success.\r\n");
	csi_gpio_uninit(&gpio);
}

static void _SensorPinmux()
{
	PINMUX_CONFIG(PWR_WAKEUP0, IIC4_SCL);
	PINMUX_CONFIG(PWR_BUTTON1, IIC4_SDA);
	PINMUX_CONFIG(PAD_MIPIRX4N, CAM_MCLK0); // IR DATA LANE
	PINMUX_CONFIG(PWR_GPIO1, PWR_GPIO_1);
}

static void _MipiRxPinmux(void)
{
//mipi rx pinmux
    // PINMUX_CONFIG(PAD_MIPIRX4P, XGPIOC_3);
    // PINMUX_CONFIG(PAD_MIPIRX4N, XGPIOC_2);
}

static void _MipiTxPinmux(void)
{
//mipi tx pinmux
#if CONFIG_PANEL_ILI9488
	PINMUX_CONFIG(PAD_MIPI_TXM1, XGPIOC_14);
	PINMUX_CONFIG(PAD_MIPI_TXP1, XGPIOC_15);
	PINMUX_CONFIG(PAD_MIPI_TXM2, XGPIOC_16);
	PINMUX_CONFIG(PAD_MIPI_TXP2, XGPIOC_17);
	PINMUX_CONFIG(IIC0_SCL, XGPIOA_28);
#elif (CONFIG_PANEL_HX8394)
	PINMUX_CONFIG(PAD_MIPI_TXM0, XGPIOC_12);
	PINMUX_CONFIG(PAD_MIPI_TXP0, XGPIOC_13);
	PINMUX_CONFIG(PAD_MIPI_TXM1, XGPIOC_14);
	PINMUX_CONFIG(PAD_MIPI_TXP1, XGPIOC_15);
	PINMUX_CONFIG(PAD_MIPI_TXM2, XGPIOC_16);
	PINMUX_CONFIG(PAD_MIPI_TXP2, XGPIOC_17);
	PINMUX_CONFIG(JTAG_CPU_TCK, XGPIOA_18);
	PINMUX_CONFIG(JTAG_CPU_TMS, XGPIOA_19);
	PINMUX_CONFIG(SPK_EN, XGPIOA_15);
#elif (CONFIG_PANEL_GC9503CV)
	PINMUX_CONFIG(PAD_MIPI_TXM0, XGPIOC_12);
	PINMUX_CONFIG(PAD_MIPI_TXP0, XGPIOC_13);
	PINMUX_CONFIG(PAD_MIPI_TXM1, XGPIOC_14);
	PINMUX_CONFIG(PAD_MIPI_TXP1, XGPIOC_15);
	PINMUX_CONFIG(PAD_MIPI_TXM2, XGPIOC_16);
	PINMUX_CONFIG(PAD_MIPI_TXP2, XGPIOC_17);
	PINMUX_CONFIG(JTAG_CPU_TCK, XGPIOA_18);
	PINMUX_CONFIG(JTAG_CPU_TMS, XGPIOA_19);
	PINMUX_CONFIG(SPK_EN, XGPIOA_15);
#endif

}

#if (CONFIG_SUPPORT_USB_HC || CONFIG_SUPPORT_USB_DC)

static void _UsbPinmux(void)
{
	// SOC_PORT_SEL
	PINMUX_CONFIG(PWR_GPIO0, PWR_GPIO_0);
	PINMUX_CONFIG(PWR_GPIO1, PWR_GPIO_1);
}

static void _UsbIoInit(void)
{
#if CONFIG_SUPPORT_USB_HC
	_GPIOSetValue(4, 0, 0);
	_GPIOSetValue(4, 1, 0);
#elif CONFIG_SUPPORT_USB_DC
	_GPIOSetValue(4, 0, 1);
	_GPIOSetValue(4, 1, 1);
#endif
}

#endif


void PLATFORM_IoInit(void)
{
//pinmux 切换接口
#if (CONFIG_SUPPORT_USB_HC || CONFIG_SUPPORT_USB_DC)
	_UsbPinmux();
	_UsbIoInit();
#endif
    _MipiRxPinmux();
    _MipiTxPinmux();
    _SensorPinmux();
	PLATFORM_PanelBacklightCtl(30);
	PINMUX_CONFIG(PWR_GPIO0, PWR_GPIO_0);
	_GPIOSetValue(4,0,0);
	udelay(1000);
	_GPIOSetValue(4,0,1);
	PINMUX_CONFIG(JTAG_CPU_TMS, XGPIOA_19);
	_GPIOSetValue(0,19,0);
}

void PLATFORM_PowerOff(void)
{
//下电休眠前调用接口
}

int PLATFORM_PanelInit(void)
{
    return CVI_SUCCESS;
}
#if 0
void PLATFORM_PanelBacklightCtl(int level)
{

}
#else
#include <drv/pwm.h>
void PLATFORM_PanelBacklightCtl(int duty)
{
	PINMUX_CONFIG(PWM0_BUCK, PWM_0);
    if(duty < 0) {
        return;
    }
    csi_pwm_t pwm;
    if(csi_pwm_init(&pwm, 0) == 0 ){
        csi_pwm_out_stop(&pwm, 0);
        if(duty != 0) {
            //printf("PLATFORM_IrCutCtl duty %d \n",duty);
            csi_pwm_out_config(&pwm, 0, 10000, duty / 100 * 10000, PWM_POLARITY_HIGH);
            csi_pwm_out_start(&pwm, 0);
			return;
        } else {
            return;
        }
    } else {
        printf("csi_pwm_init err \n");
        return;
    }
    return;
}
#endif

void PLATFORM_SpkMute(int value)
{
//0静音 ，1非静音
    if(value == 0) {
        _GPIOSetValue(GPIO_SPKEN_GRP, GPIO_SPKEN_NUM, 0);
    } else {
        _GPIOSetValue(GPIO_SPKEN_GRP, GPIO_SPKEN_NUM, 1);
    }
}

int PLATFORM_IrCutCtl(int duty)
{
    return 0;
}

#if CONFIG_QUICK_STARTUP_SUPPORT
void PLATFORM_CLK_AXI4_Restore()
{
	// restore axi4 to 300M
	if (mmio_read_32(0x030020B8) != 0x00050009) {
		mmio_write_32(0x030020B8, 0x00050009);
	}
}
#endif
