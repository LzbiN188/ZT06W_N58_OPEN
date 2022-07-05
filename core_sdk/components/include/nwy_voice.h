#ifndef __NWY_VOICE_H__
#define __NWY_VOICE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define true 1
#define false 0
#define FALSE 0

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t ;

typedef enum
{
  NWY_VOICE_CALL_STATE_INCOMING =0x0000,
  NWY_VOICE_CALL_STATE_DIALING  =0x0001,
  NWY_VOICE_CALL_STATE_ALERTNG  =0x0002,
  NWY_VOICE_CALL_STATE_ACTIVE  =0x0003,
  NWY_VOICE_CALL_STATE_HOLDING  =0x0004,
  NWY_VOICE_CALL_STATE_END =0x0005,
  NWY_VOICE_CALL_STATE_WAITING  =0x0006,
} nwy_vc_call_state_t;

typedef struct  
{   
    uint8_t sim_id;
	char phone_num[20 + 1];
	nwy_vc_call_state_t  state;
    uint16_t uti;
} nwy_voice_call_info_t;

typedef void (*nwy_voice_statehandlerfunc)
(
    int                     sim_id,
    char*                   phone_num,
    nwy_vc_call_state_t     state,
    void                    *contextPtr
);

int nwy_voice_call_start(uint8_t sim_id,char *phone_num);
int nwy_voice_call_end(uint8_t sim_id);
int nwy_voice_call_hold(uint8_t sim_id);
int nwy_voice_call_unhold(uint8_t sim_id);
int nwy_voice_call_autoanswver();
int nwy_voice_setvolte(uint8_t sim_id, uint8_t setvolte);
void nwy_get_voice_state(char *nwy_call_state);
void nwy_get_voice_callerid(char* nwy_call_rsp);

#ifdef __cplusplus
   }
#endif

#endif
