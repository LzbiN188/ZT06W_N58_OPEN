#ifndef __NWY_AUDIO_H__
#define __NWY_AUDIO_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
typedef int(* record_event_handler)(unsigned char *pdata, unsigned int len);

typedef enum nwy_player_status
{
  NWY_PLAYER_NONE,
}nwy_player_status;
typedef int(* player_event_handler)(nwy_player_status state);

//playback
/**
 * @brief open the player 
 *
 * @return
 *      - NWY_SUCESS    0
 */
int nwy_audio_player_open(player_event_handler handler);
/**
 * @brief play the pcm data 
 *
 * @param pdata : pcm data point
 * @param len: data len
 * @return
 *      - NWY_SUCESS    0
 *      - NWY_GEN_E_UNKNOWN      -1 (malloc failed)
 */

int nwy_audio_player_play(uint8_t *pdata, uint32_t len);



/**
 * @brief set the pcm data and tone play in nwy_audio_player_play
 *
 * @param freq1 : tone freq1
 * @param freq2 : tone freq2
 * @param time_ms: tone play time ms
 * @param tone_vol : tone vol, 1-16834
 * @param play_vol : pcm data vol, 1-100; as pcm_data/play_vol

 * @return
 *      - NWY_SUCESS    0
 *      - NWY_GEN_E_UNKNOWN      -1 (malloc failed)
 */
int nwy_audio_player_play_tone(int freq1, int freq2, int time_ms, int tone_vol, int play_vol);



/**
 * @brief stop the player 
 *
 * @return
 *      - NWY_SUCESS    0
 */
int nwy_audio_player_stop(void);
/**
 * @brief close the player 
 *
 * @return
 *      - NWY_SUCESS    0
 */

void nwy_audio_player_close(void);

//capture
/**
 * @brief open the recorder 
 *
 * @param handler : recorder callback
 * @return
 *      - NWY_SUCESS    0
 *      - NWY_ERROR      -1 
 */
int nwy_audio_recorder_open(record_event_handler handler);
/**
 * @brief start the recorder 
 *
 * @return
 *      - NWY_SUCESS    0
 *      - NWY_ERROR      -1 
 */
int nwy_audio_recorder_start(void);
/**
 * @brief stop the recorder 
 *
 * @param pdata : pcm data point
 * @param len: data len
 * @return
 *      - NWY_SUCESS    0
 */
int nwy_audio_recorder_stop(void);
/**
 * @brief close the recorder
 *
 * @return
 *      - NWY_SUCESS    0
 *      - NWY_ERROR      -1
 */
void nwy_audio_recorder_close(void);

/**
 * @brief set handset volume
 *
 * @param step : volume, 0-100
 *
 *
 */
void nwy_audio_set_handset_vol(const int step);

/**
 * @brief set mic volume
 *
 * @param step : volume, 0-9
 *
 *
 */
void nwy_audio_set_mic_vol(const int step);

/**
 * @brief set mic volume params
 *
 * @param step : anaGainParam, 0-7; adcGainParam, 0-15
 *
 *
 */
void nwy_audio_set_mic_vol_params(const uint16_t anaGainParam, const uint16_t adcGainParam);


/**
 * @brief get handset volume
 *
 *
 */
unsigned int nwy_audio_get_handset_vol(void);

/**
 * @brief get mic volume
 *
 *
 */
unsigned int nwy_audio_get_mic_vol(void);

/**
 * @brief get mic volume params
 *
 *
 */
int nwy_audio_get_mic_vol_params(uint16_t *anaGainParam, uint16_t *adcGainParam);


//TTS
typedef enum
{
    ENCODE_GBK,
    ENCODE_UTF16LE,
    ENCODE_UTF16BE,
    ENCODE_UTF8,
}nwy_tts_encode_t;

typedef enum neoway_result
{
    PLAY_ERROR,
    INIT_ERROR,
    PARAM_ERROR,
    SET_PARAM_ERROR,
    PLAY_END,
    SYNTHESIZETEXT_ERROR,
    TTS_TERMINATED,
    TTS_TIMEOUT,
}nwy_neoway_result_t;

typedef void (*tts_cb)(void* cb_para, nwy_neoway_result_t result);
/**
 * @brief play tts buf
 *
 * @param data : tts data point
 * @param size: data size
 * @param type: data type:nwy_tts_encode_t
 * @param cb : play callback
 * @param cb_param: callback param
 * @return
 *      - NWY_SUCESS    0
 *      - NWY_ERROR      -1 
 */


int nwy_tts_playbuf(char * data, int size, nwy_tts_encode_t type, tts_cb cb, void* cb_param);
/**
 * @brief stop tts 
 *
 * @return
 *      - NWY_SUCESS    0
 *      - NWY_ERROR      -1 
 */

void nwy_tts_stop_play(void);


/**
 * @brief play tone 
 *
 * @param tone : DTMF
 * @param time: duration time
 * @return
 *      - NWY_SUCESS    0
 *      - NWY_ERROR      -1 
 */

void nwy_audio_tone_play(const char * tone, unsigned time, int vol);
void nwy_audio_tone_play_freq(unsigned int freq1, unsigned int freq2, unsigned time, int vol);
void nwy_audio_tone_play_freq_async(unsigned int freq1, unsigned int freq2, unsigned time, int vol);
void nwy_audio_tone_stop_async(void);



/* constants for values of field nParamID */
#define NWY_ivTTS_PARAM_PARAMCH_CALLBACK    0x00000000  /* parameter change callback entry */
#define NWY_ivTTS_PARAM_LANGUAGE            0x00000100  /* language, e.g. Chinese */
#define NWY_ivTTS_PARAM_INPUT_CODEPAGE      0x00000101  /* input code page, e.g. GBK */
#define NWY_ivTTS_PARAM_TEXT_MARK           0x00000102  /* text mark, e.g. CSSML */
#define NWY_ivTTS_PARAM_USE_PROMPTS         0x00000104  /* whether use prompts */
#define NWY_ivTTS_PARAM_RECOGNIZE_PHONEME   0x00000105  /* how to recognize phoneme input */
#define NWY_ivTTS_PARAM_PROGRESS_BEGIN      0x00000204  /* current processing position */
#define NWY_ivTTS_PARAM_PROGRESS_LENGTH     0x00000205  /* current processing length */
#define NWY_ivTTS_PARAM_PROGRESS_CALLBACK   0x00000206  /* progress callback entry */
#define NWY_ivTTS_PARAM_READ_PUNCTUATIONS   0x00000300  /* whether read punctuations */
#define NWY_ivTTS_PARAM_READ_AS_NAME        0x00000301  /* whether read as name */
#define NWY_ivTTS_PARAM_READ_DIGIT          0x00000302  /* how to read digit, e.g. read as number, read as value  */
#define NWY_ivTTS_PARAM_CHINESE_NUMBER_1    0x00000303  /* how to read number "1" in Chinese */
#define NWY_ivTTS_PARAM_MANUAL_PROSODY      0x00000304  /* whether use manual prosody */
#define NWY_ivTTS_PARAM_PHONEME_WATCH       0x00000402  /* how to watch phoneme, e.g. Chinese Pin-Yin */
#define NWY_ivTTS_PARAM_WATCH_CALLBACK      0x00000403  /* watch callback entry */
#define NWY_ivTTS_PARAM_EVENT_CALLBACK      0x00000404  /* sleep callback entry */

#define NWY_ivTTS_PARAM_ROLE                0x00000500  /* speaker role */
#define NWY_ivTTS_PARAM_SPEAK_STYLE         0x00000501  /* speak style */
#define NWY_ivTTS_PARAM_VOICE_SPEED         0x00000502  /* voice speed */
#define NWY_ivTTS_PARAM_VOICE_PITCH         0x00000503  /* voice tone */
#define NWY_ivTTS_PARAM_VOLUME              0x00000504  /* volume value */
#define NWY_ivTTS_PARAM_DELAYTIME           0x00000505  /* delay time */
#define NWY_ivTTS_PARAM_VOICE_TIMBRE        0x00000506  /* voice timbre */
#define NWY_ivTTS_PARAM_VOLUME_INCREASE     0X00000507  /* volume value increase */


#define NWY_ivTTS_PARAM_VEMODE              0x00000600  /* voice effect - predefined mode */

#define NWY_ivTTS_PARAM_OUTPUT_BUF          0x00000700  /* output buffer */
#define NWY_ivTTS_PARAM_OUTPUT_BUFSIZE      0x00000701  /* output buffer size */


/* constants for values of parameter ivTTS_PARAM_LANGUAGE */
#define NWY_ivTTS_LANGUAGE_CHINESE          1           /* Chinese (with English) */
#define NWY_ivTTS_LANGUAGE_ENGLISH          2           /* English */

/* constants for values of parameter ivTTS_PARAM_INPUT_CODEPAGE */
#define NWY_ivTTS_CODEPAGE_ASCII            437         /* ASCII */
#define NWY_ivTTS_CODEPAGE_GBK              936         /* GBK (default) */
#define NWY_ivTTS_CODEPAGE_BIG5             950         /* Big5 */
#define NWY_ivTTS_CODEPAGE_UTF16LE          1200        /* UTF-16 little-endian */
#define NWY_ivTTS_CODEPAGE_UTF16BE          1201        /* UTF-16 big-endian */
#define NWY_ivTTS_CODEPAGE_UTF7             65000       /* UTF-7 */
#define NWY_ivTTS_CODEPAGE_UTF8             65001       /* UTF-8 */
#define NWY_ivTTS_CODEPAGE_GB2312           NWY_ivTTS_CODEPAGE_GBK
#define NWY_ivTTS_CODEPAGE_GB18030          NWY_ivTTS_CODEPAGE_GBK
#define NWY_ivTTS_CODEPAGE_UTF16            NWY_ivTTS_CODEPAGE_UTF16LE
#define NWY_ivTTS_CODEPAGE_UNICODE          NWY_ivTTS_CODEPAGE_UTF16
#define NWY_ivTTS_CODEPAGE_PHONETIC_PLAIN   23456       /* Kingsoft Phonetic Plain */

/* constants for values of parameter ivTTS_PARAM_TEXT_MARK */
#define NWY_ivTTS_TEXTMARK_NONE             0           /* none */
#define NWY_ivTTS_TEXTMARK_SIMPLE_TAGS      1           /* simple tags (default) */

/* constants for values of parameter ivTTS_PARAM_INPUT_MODE */
#define NWY_ivTTS_INPUT_FIXED_BUFFER        0           /* from fixed buffer */
#define NWY_ivTTS_INPUT_CALLBACK            1           /* from callback */

/* constants for values of parameter ivTTS_PARAM_READ_DIGIT */
#define NWY_ivTTS_READDIGIT_AUTO            0           /* decide automatically (default) */
#define NWY_ivTTS_READDIGIT_AS_NUMBER       1           /* say digit as number */
#define NWY_ivTTS_READDIGIT_AS_VALUE        2           /* say digit as value */

/* constants for values of parameter ivTTS_PARAM_CHINESE_NUMBER_1 */
#define NWY_ivTTS_CHNUM1_READ_YAO           0           /* read number "1" [yao1] in chinese (default) */
#define NWY_ivTTS_CHNUM1_READ_YI            1           /* read number "1" [yi1] in chinese */

/* constants for values of parameter ivTTS_PARAM_PHONEME_WATCH */
#define NWY_ivTTS_PHONEME_NONE              0           /* none (default) */
#define NWY_ivTTS_PHONEME_PINYIN            1           /* Chinese Pin-Yin */

/* constants for values of parameter ivTTS_PARAM_ROLE */
#define NWY_ivTTS_ROLE_YANPING              3           /* Yanping (female, Chinese) */
#define NWY_ivTTS_ROLE_YUFENG               4           /* Yufeng (male, Chinese) */
#define NWY_ivTTS_ROLE_NANNAN               7           /* Nannan (child, Chinese) */
#define NWY_ivTTS_ROLE_XIAOQIAN             11          /* Xiaoqian (female, Chinese Northeast) */
#define NWY_ivTTS_ROLE_XIAORONG             14          /* Xiaorong (female, Chinese Szechwan) */
#define NWY_ivTTS_ROLE_XIAOMEI              15          /* Xiaomei (female, Cantonese) */
#define NWY_ivTTS_ROLE_JOHN                 17          /* John (male, US English) */
#define NWY_ivTTS_ROLE_TERRY                19          /* Terry (female, US English) */
#define NWY_ivTTS_ROLE_JIUXU                51          /* Jiu Xu (male, Chinese) */
#define NWY_ivTTS_ROLE_DUOXU                52          /* Duo Xu (male, Chinese) */
#define NWY_ivTTS_ROLE_XIAOPING             53          /* Xiaoping (female, Chinese) */
#define NWY_ivTTS_ROLE_DONALDDUCK           54          /* Donald Duck (male, Chinese) */
#define NWY_ivTTS_ROLE_BABYXU               55          /* Baby Xu (child, Chinese) */
#define NWY_ivTTS_ROLE_USER                 99          /* user defined */

/* constants for values of parameter ivTTS_PARAM_SPEAK_STYLE */
#define NWY_ivTTS_STYLE_PLAIN               0           /* plain speak style */
#define NWY_ivTTS_STYLE_NORMAL              1           /* normal speak style (default) */

/* constants for values of parameter ivTTS_PARAM_VOICE_SPEED */
/* the range of voice speed value is from -32768 to +32767 */
#define NWY_ivTTS_SPEED_MIN                 -32768      /* slowest voice speed */
#define NWY_ivTTS_SPEED_NORMAL              0           /* normal voice speed (default) */
#define NWY_ivTTS_SPEED_MAX                 +32767      /* fastest voice speed */

/* constants for values of parameter ivTTS_PARAM_VOICE_PITCH */
/* the range of voice tone value is from -32768 to +32767 */
#define NWY_ivTTS_PITCH_MIN                 -32768      /* lowest voice tone */
#define NWY_ivTTS_PITCH_NORMAL              0           /* normal voice tone (default) */
#define NWY_ivTTS_PITCH_MAX                 +32767      /* highest voice tone */

/* constants for values of parameter ivTTS_PARAM_VOLUME */
/* the range of volume value is from -32768 to +32767 */
#define NWY_ivTTS_VOLUME_MIN                -32768      /* minimized volume */
#define NWY_ivTTS_VOLUME_NORMAL             0           /* normal volume */
#define NWY_ivTTS_VOLUME_MAX                +32767      /* maximized volume (default) */

/* constants for values of parameter ivTTS_PARAM_VOLUME_INCREASE */
/* the range of volume value is from 0 to 10 */
#define NWY_ivTTS_VOLUME_INCREASE_MIN               0       /* minimized volume (default) */
#define NWY_ivTTS_VOLUME_INCREASE_MAX               10      /* maximized volume */

/* constants for values of parameter ivTTS_PARAM_VEMODE */
#define NWY_ivTTS_VEMODE_NONE               0           /* none */
#define NWY_ivTTS_VEMODE_WANDER             1           /* wander */
#define NWY_ivTTS_VEMODE_ECHO               2           /* echo */
#define NWY_ivTTS_VEMODE_ROBERT             3           /* robert */
#define NWY_ivTTS_VEMODE_CHROUS             4           /* chorus */
#define NWY_ivTTS_VEMODE_UNDERWATER         5           /* underwater */
#define NWY_ivTTS_VEMODE_REVERB             6           /* reverb */
#define NWY_ivTTS_VEMODE_ECCENTRIC          7           /* eccentric */

/**
 * @brief set tts param
 *
 * @param nParamID : param ID
 * @param nParamValue: param value
 * @return
 *      - void    
 */
void nwy_tts_param_set(long nParamID, long nParamValue);

typedef enum nwy_tts_param_command_type
{
    NWY_TTS_PARAM_CODEPAGE_CMD,     // codepage of input text,nwy_tts_codepage_type
    NWY_TTS_PARAM_VOLUME_CMD,       // volume value, 0-99
    NWY_TTS_PARAM_SPEED_CMD,        // voice speed,0-99
    NWY_TTS_PARAM_PITCH_CMD,        // voice tone,0-99
    NWY_TTS_PARAM_STYL_CMD,         // 0-99
}nwy_tts_param_command_type;
void nwy_tts_set_percent_of_param(nwy_tts_param_command_type command, int val);
void nwy_tts_get_percent_of_param(nwy_tts_param_command_type command, int *val);

//pa
typedef enum{
    NWY_PA_NORMAL_MODE = 0,
    NWY_PA_AW8733 = 1,
}nwy_pa_type;

typedef enum{
    NWY_PA_MODE0 = 0, //shutdown
    NWY_PA_MODE1, //12dB
    NWY_PA_MODE2, //16dB
    NWY_PA_MODE3, //24dB
    NWY_PA_MODE4, //27.5dB
    NWY_PA_MODE5,
    NWY_PA_MODE6,
    NWY_PA_MODE7,
    NWY_PA_MODEMAX = 20,
}nwy_pa_mode;

/**
 * @brief set pa control status
 *
 * @param pa_type : pa type: aw8733/normal pa
 * @param gpio_num: pa's gpio
 * @param mode: pa's mode
 * @return
 *		- NWY_SUCESS	0
 *		- NWY_ERROR 	 -1
 */

int nwy_audio_pa_control(nwy_pa_type pa_type, int gpio_num, nwy_pa_mode mode);

/**
  * @brief set audio pa enable level
  * @param level: 1: low enable,high disable; 0: high enable, low disable
*/
void nwy_audio_pa_enable_level(int level);


typedef enum{
    NWY_AUSTREAM_FORMAT_UNKNOWN, ///< placeholder for unknown format
    NWY_AUSTREAM_FORMAT_PCM,     ///< raw PCM data
    NWY_AUSTREAM_FORMAT_WAVPCM,  ///< WAV, PCM inside
    NWY_AUSTREAM_FORMAT_MP3,     ///< MP3
    NWY_AUSTREAM_FORMAT_AMRNB,   ///< AMR-NB
    NWY_AUSTREAM_FORMAT_AMRWB,   ///< AMR_WB
}nwy_stream_format;

typedef enum{
    NWY_AUENCODER_QUALITY_LOW,    ///< quality low
    NWY_AUENCODER_QUALITY_MEDIUM, ///< quality medium
    NWY_AUENCODER_QUALITY_HIGH,   ///< quality high
    NWY_AUENCODER_QUALITY_BEST,   ///< quality best
}nwy_encode_quality;

/**
 * @brief set record format
 *
 * @param nwy_stream_format : record type
 * @param nwy_encode_quality: record quality
 * @return void
 */
 void nwy_audio_set_record_format(nwy_stream_format type, nwy_encode_quality quality);

 /**
  * @brief play audio file
  * @ you need to put audio_file in the /core_sdk/prebuilts/preset in advance
  *
  * @param filename : file name
  * @return void
  */
  void nwy_audio_file_play(char *filename);

/**
 * @brief record audio file
 *
 * @param filename : file name
 * @param quality : only for amr quality (0-3)
 * @return
 *	  - NWY_SUCESS	  0
 *	  - NWY_ERROR	  -1
 */
 int nwy_audio_file_record(char *filename, int quality);

/**
 * @brief stop record audio file
 *
 * @return
 *	  - NWY_SUCESS	  0
 *	  - NWY_ERROR	  -1
 */
int nwy_audio_file_record_stop();

 /**
 * \brief audio output device
 */
typedef enum
{
    NWY_AUDEV_OUTPUT_RECEIVER = 0,   ///< receiver
    NWY_AUDEV_OUTPUT_HEADPHONE = 1,  ///< headphone
    NWY_AUDEV_OUTPUT_SPEAKER = 2,    ///< speaker
} nwy_audevOutput_t;

 /**
  * \brief audio input device
  */
typedef enum
{
    NWY_AUDEV_INPUT_MAINMIC = 0, ///< main mic
    NWY_AUDEV_INPUT_AUXMIC = 1,  ///< auxilary mic
    NWY_AUDEV_INPUT_DUALMIC = 2, ///< dual mic
    NWY_AUDEV_INPUT_HPMIC_L = 3, ///< headphone mic left
    NWY_AUDEV_INPUT_HPMIC_R = 4, ///< headphone mic right
} nwy_audevInput_t;

 /**
  * @brief set output channel
  *
  * @param device : output device
  * @return void
  */
 void nwy_change_output_channel(int device);

 /**
  * @brief set input channel
  *
  * @param device : input device
  * @return void
  */
 void nwy_change_input_channel(int device);


#ifdef FEATURE_NWY_AUDIO_SET_HEADSETCB
 typedef enum
 {
     NWY_MSG_HEADSET_PLUGIN = 1,
     NWY_MSG_HEADSET_PLUGOUT = 2,
     NWY_MSG_HEADSET_BTN_DOWN = 3,
     NWY_MSG_HEADSET_BTN_UP = 4,
 } nwy_drvHeadsetNotifyMsg_t;

 typedef void (*headset_msg_cb)(nwy_drvHeadsetNotifyMsg_t headset_msg, uint32_t param);
 /**
  * @brief set headsetcb func
  *
  * @param headset_cb : headset call_back
  * @return void
  */
 void nwy_audio_set_headsetcb(headset_msg_cb headset_cb);
#endif

typedef enum
{
    NWY_AUDIO_PARAMS_CAWTF = 0,// Write calibration param to flash
    NWY_AUDIO_PARAMS_CACCP,// Audio Codec Calibration Param
    NWY_AUDIO_PARAMS_CAVQE,// Audio ZSP VQE Calibration Param
    NWY_AUDIO_PARAMS_CAPRE,// Audio ZSP Pre -Processing Calibration Param
    NWY_AUDIO_PARAMS_CAPOST,// Audio ZSP Post -Processing Calibration Param
    NWY_AUDIO_PARAMS_CANXP,
} nwy_audio_params_t;

/**
 * @brief set audio params
 *
 * @param type : set params type
 * @param mode : APMode Sel
 * @param patch : ITF Sel
 * @param ctrl : Specific parameter setting options
 * @param param : audio EQ/GAIN param
 *
 * @return 0 : success
 *      else : err code
 * when write to flash(NWY_AUDIO_PARAMS_CAWTF) mode is invalid
 */
int nwy_set_audio_params(nwy_audio_params_t type, uint8_t mode, uint8_t path, uint8_t ctrl, const char *param);
/**
 * @brief get headset init status
 *
 * @return 0 : initing
 * @return 1 : init done
 */

bool nwy_get_hp_init_status(void);

typedef enum
{
    NWY_AUDIO_HP_NO = 0,
    NWY_AUDIO_HP_NC,
} nwy_hp_type_t;
/**
 * @brief set headset type
 *
 * @return void
 */
void nwy_audio_set_hp_type(nwy_hp_type_t hp_type);

/**
 * @brief set tone switch
 * 
 * @return void
 */
// 0 open platform tone, use platform tone
// 1 close platform tone, use Custom tone
void nwy_audio_set_tone_switch(uint8_t Callmode, uint8_t Smsmode);

/**
 * @brief set poc sampleRate
 * 
 * @return void
 */
void nwy_set_poc_sampleRate(uint32_t samplerate);

/**
 * @brief get playing share memory time
 * 
 * @return ms(playing share memory time)
 */
int nwy_audio_get_playShmemTime(void);

/**
 * @brief read pipe space
 * 
 * @return pipe space bytes
 */
int nwy_read_pipe_space(void);



/**
 * @brief set headset status to modem, used for pa control by player.
 * 
 * @ is_plugin: 1:headset plugin; 0:headset plugout
 * @return void

 * @notice: 
 *   This function can not be called by irq ( maybe need 200 us )
 */
void nwy_headset_status(int is_plugin);
/**
 * @brief generate audio tone buf
 *
 * @param tone : tone
 * @param time : duration time (ms)
 * @param vol : vol
 * @param buf : tone buf pointer
 *
 * @return  : tone_buf_size
 */
int nwy_audio_tone_play_v2(const char * tone, unsigned time, int vol, uintptr_t * buf);//vol: 0-15
/**
 * @brief generate audio tone buf
 *
 * @param freq1 : freq 1
 * @param freq2 : freq 2
 * @param time : duration time (ms)
 * @param vol : vol
 * @param buf : tone buf pointer
 *
 * @return  : tone_buf_size
 */
int nwy_audio_tone_play_freq_v2(unsigned int freq1, unsigned int freq2, unsigned time, int vol,  uintptr_t * buf);//vol: 0-15

/**
 * @brief get headset status
 *
 * @param isplugin : 0: HEADSET_DISCONNECT;1: HEADSET_CONNECT
 * @param mictype : 0:DRV_HEADSET_TYPE_UNKNOWN_MIC_TYPE;1:DRV_HEADSET_TYPE_NO_MIC;2:DRV_HEADSET_TYPE_4POLE_NORMAL;
                    3:DRV_HEADSET_TYPE_4POLE_NOT_NORMAL; 4:DRV_HEADSET_TYPE_APPLE;5:DRV_HEADSET_TYPE_ERR;
                    -1: HEADSET_TYPE_ERR
 */
void nwy_get_headset_status(int *isplugin, int *mictype);

typedef enum
{
    NWY_PLAY_FINISH = 1,
} nwy_PlayEvent_t;

typedef void (*nwyPlayerEventCallback_t)(void *param, nwy_PlayEvent_t event);
/**
 * @brief play file async
 *
 * @param filename : filename
 * @param cb : fplay finish cb
 *
 * @return 0 : success
 *      else : err code
 */

int nwy_audio_file_play_async(char *filename, nwyPlayerEventCallback_t cb);
/**
 * @brief stop file play
 *
 * @return 0 : success
 *      else : err code
 */
int nwy_audio_file_play_stop(void);

//playback
/**
 * @brief open the player
 *
 * @return
 *      - NWY_SUCESS    0
 */

int nwy_audio_player_open_v2(player_event_handler handler);

/**
 * @brief play the audio data
 *
 * @param pdata : audio data point
 * @param len: data len
 * @return
 *      - NWY_SUCESS    0
 *      - NWY_GEN_E_UNKNOWN      -1 (malloc failed)
 */

int nwy_audio_player_play_v2(uint8_t *pdata, uint32_t len);
/**
 * @brief stop the player
 *
 * @return
 *      - NWY_SUCESS    0
 */
int nwy_audio_player_stop_v2(void);

/**
* @brief close the player
*
* @return
*       - NWY_SUCESS    0
*/
void nwy_audio_player_close_v2(void);

/**
 * @brief set poc mode
 *
 * @param poc_mode :0 : half duplex; 1 : full duplex
 *
 */
void nwy_set_poc_mode(int poc_mode);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /*__NWY_AUDIO_H__*/

