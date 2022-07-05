#ifndef __NWY_PWM_H__
#define __NWY_PWM_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NAME_PWM_1  "PWM1"

typedef struct nwyPwm nwy_pwm_t;

nwy_pwm_t* nwy_pwm_init(char *name,uint32_t freq,uint32_t duty);
bool nwy_pwm_start(nwy_pwm_t *d);
bool nwy_pwm_stop(nwy_pwm_t *d);
int nwy_pwm_deinit(nwy_pwm_t *d);

/**
 * @brief pwm init,user can set clk div 
 *
 * @param name : NAME_PWM_1
 * @param div  : clk_div(0-255)
 * @param freq : freq(HZ)
 * @param duty : duty(0-100)
 * @return
 *      - SPI_EC_SUCESS success
 *      - otherwise     fail
 */
nwy_pwm_t* nwy_pwm_clk_div_init(char *name,uint8_t div,uint32_t freq,uint32_t duty);

#ifdef __cplusplus
   }
#endif

#endif // __NWY_PWM_H__

