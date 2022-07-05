#ifndef _NWY_KEYPAD_H_
#define _NWY_KEYPAD_H_

#include <stdint.h>
#include "drv_keypad.h"
#ifdef __cplusplus
extern "C" {
#endif

#define NWY_KEY_MAP(out, in) ((out) * 8 + (in))
typedef enum
{
#ifdef FEATURE_NWY_PROJ_N711
    KEY_I1_O0 = NWY_KEY_MAP(0, 1),
    KEY_I2_O0 = NWY_KEY_MAP(0, 2),
    KEY_I3_O0 = NWY_KEY_MAP(0, 3),
    KEY_I4_O0 = NWY_KEY_MAP(0, 4),
    KEY_I5_O0 = NWY_KEY_MAP(0, 5),
    
    KEY_I1_O1 = NWY_KEY_MAP(1, 1),
    KEY_I2_O1 = NWY_KEY_MAP(1, 2),
    KEY_I3_O1 = NWY_KEY_MAP(1, 3),
    KEY_I4_O1 = NWY_KEY_MAP(1, 4),
    KEY_I5_O1 = NWY_KEY_MAP(1, 5),
    
    KEY_I1_O2 = NWY_KEY_MAP(2, 1),
    KEY_I2_O2 = NWY_KEY_MAP(2, 2),
    KEY_I3_O2 = NWY_KEY_MAP(2, 3),
    KEY_I4_O2 = NWY_KEY_MAP(2, 4),
    KEY_I5_O2 = NWY_KEY_MAP(2, 5),
    
    KEY_I1_O3 = NWY_KEY_MAP(3, 1),
    KEY_I2_O3 = NWY_KEY_MAP(3, 2),
    KEY_I3_O3 = NWY_KEY_MAP(3, 3),
    KEY_I4_O3 = NWY_KEY_MAP(3, 4),
    KEY_I5_O3 = NWY_KEY_MAP(3, 5),

    KEY_POWER = NWY_KEY_MAP(6, 6),
#else   // for N58 & N716
    KEY_I1_O1 = 0,
    KEY_I2_O1 = 1,
    KEY_I3_O1 = 2,

    KEY_I1_O2 = 3,
    KEY_I2_O2 = 4,
    KEY_I3_O2 = 5,
    
    KEY_I1_O3 = 6,
    KEY_I2_O3 = 7,
    KEY_I3_O3 = 8,

    KEY_I1_O4 = 9,
    KEY_I2_O4 = 10,
    KEY_I3_O4 = 11,

    KEY_POWER = 12,
#endif
    KEY_MAX_ID,
} nwy_key_t;

typedef enum
{
    key_state_press = (1 << 0),
    key_state_release = (1 << 1),
} nwy_keyState_t;

typedef void (*nwy_key_event_cb_t)(nwy_key_t key, nwy_keyState_t evt);

/*---------------------------Function Definition--------------------------*/
/**
 * @brief registered the key event callback
 *
 * @param cb: key event callback
 * @return
 */
extern void reg_nwy_key_cb(nwy_key_event_cb_t cb);

/**
 * @brief config the debunce time and ivt time
 *
 * @param debunce:debunce time = (debunc + 1)*0.3125*4                debounce time < 480ms
 * @param itv:interval of IRQ generation = (itv + 1 )*debounce time
 * @return
 */
void nwy_keypad_debounce_time(uint32_t debunc,uint32_t itv);


extern void nwy_key_cb(keyMap_t id, keyState_t evt);
#ifdef __cplusplus
}
#endif

#endif

