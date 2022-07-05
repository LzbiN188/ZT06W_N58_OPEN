#ifndef __NWY_LED_H__
#define __NWY_LED_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NWY_LEDK0 = 0,
    NWY_LEDK1 = 1,
    NWY_LEDK2 = 2
}nwy_led_backlight_t;

/*---------------------------Function Definition--------------------------*/
/**
 * @brief open the led backlight
 *
 * @param mode : led backlight NWY_LEDK0,NWY_LEDK1,NWY_LEDK2
 * @return
 *      - SUCCESS    1
 *      - ERROR     -1
 */
int nwy_led_back_light_open(int mode);

/**
 * @brief close the led backlight
 *
 * @param mode : led backlight NWY_LEDK0,NWY_LEDK1,NWY_LEDK2
 * @return
 *      - SUCCESS    1
 *      - ERROR     -1
 */
int nwy_led_back_light_close(int mode);

#ifdef __cplusplus
}
#endif

#endif/*__NWY_LED_H__*/

