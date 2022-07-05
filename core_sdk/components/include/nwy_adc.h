#ifndef __NWY_ADC_H__
#define __NWY_ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    NWY_ADC_CHANNEL2 = 2,
    NWY_ADC_CHANNEL3 = 3,
    NWY_ADC_CHANNEL4 = 4,   // vbat for N58, GPADC4 for N711
    NWY_ADC_CHANNEL5 = 5,   // vbat for N711
    NWY_ADC_CHANNEL_VBAT    // recommended to use this to obtain battery voltage
} nwy_adc_t;

typedef enum
{
    NWY_ADC_SCALE_1V250 = 0,
    NWY_ADC_SCALE_2V444 = 1,
    NWY_ADC_SCALE_3V233 = 2,
    NWY_ADC_SCALE_5V000 = 3         //MAX
} nwy_adc_aux_scale_t;
/*---------------------------Function Definition--------------------------*/
/**
 * @brief get the adc value
 *
 * @param channel:nwy_adc_t
 * @param scale:nwy_adc_aux_scale_t
 * @return
 *      - adc value
 */
int nwy_adc_get(nwy_adc_t channel,nwy_adc_aux_scale_t scale);

#ifdef __cplusplus
}
#endif

#endif/*__NWY_ADC_H__*/
