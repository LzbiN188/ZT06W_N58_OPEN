#ifndef __NWY_GPIO_H__
#define __NWY_GPIO_H__

#include "nwy_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NWY_GPIO_MAX   10

#if defined(FEATURE_NWY_PROJ_N715)
// N715
#ifndef FEATURE_NWY_QSCLK_DTR
#define NWY_DTR        10 //wakeup_in pin49
#else
#define NWY_DTR        22 //uart1_dtr pin39
#endif
#elif defined(FEATURE_NWY_PROJ_N711)
#define NWY_DTR        21 //N711 pin1
#else
#define NWY_DTR        10 //N58 & N716  pin79
#endif

#ifdef FEATURE_NWY_PCIE_ENABLE
#define NWY_HDFLIGHT    4    //pin96
#define NWY_GPIO1       1   //
#endif
typedef enum nwy_num_t{
  NWY_GPIO_24 = 24,
  NWY_GPIO_25 = 25,
}nwy_num_t;



typedef enum nwy_dir_t{
  NWY_INPUT =0,
  NWY_OUTPUT,
  NWY_IRQ_RISING,
  NWY_IRQ_FALLING,
  NWY_IRQ_RISING_FALLING,
  NWY_IRQ_HIGH,
  NWY_IRQ_LOW,
  NWY_SLEEP_WAKE,
  
  NWY_DIR_NULL
}nwy_dir_t;

typedef enum nwy_val_t{
  NWY_LOW=0,
  NWY_HIGH=1,
  
  NWY_VAL_NULL
}nwy_val_t;

typedef enum nwy_slp_trig_mode_t{
  NWY_FALLING_SLEEP_WAKEUP =0,
  NWY_RISING_SLEEP_WAKEUP
}nwy_slp_trig_mode_t;

typedef enum nwy_state_t{
  NWY_WAKE_STATE =0,
  NWY_SLEEP_STATE  
}nwy_state_t;

typedef void(*irq_callback)(int param);


extern int nwy_gpio_set_val(int gpio_n, int value);
extern int nwy_gpio_get_val(int gpio_n);
extern int nwy_gpio_set_dir(int gpio_n, int dir);
extern int nwy_gpio_get_dir(int gpio_n);
extern int nwy_gpio_irq_config(int gpio_n, int irq_mode, irq_callback cb);
extern int nwy_gpio_pm_config(int gpio_n, int irq_mode, int sleep_mode, int sleep_en, irq_callback cb);
extern int nwy_gpio_irq_enable(int gpio_n);
extern int nwy_gpio_irq_disable(int gpio_n);
extern int nwy_gpio_close(int gpio_n);


#ifdef __cplusplus
}
#endif


#endif/*__NWY_GPIO_H__*/

