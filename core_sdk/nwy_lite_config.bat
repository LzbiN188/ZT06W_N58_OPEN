@echo off
setlocal enabledelayedexpansion
set h_file="components\include\nwy_config.h"
set lite_str="FEATURE_NWY_OPEN_LITE"

set cmake_path=cmake\
set cmake_file=core_config.cmake
set mqtt_conf=CONFIG_NWY_OPEN_LITE_PAHOMQTT
set ali_conf=CONFIG_NWY_OPEN_LITE_ALIMQTT
set tts_conf=CONFIG_NWY_OPEN_LITE_TTS
set eng_tts_conf=CONFIG_NWY_OPEN_LITE_ENG_TTS
set rws_conf=CONFIG_NWY_OPEN_LITE_RWS
set azure_conf=CONFIG_NWY_OPEN_LITE_AZURE
set lwm2m_conf=CONFIG_NWY_OPEN_LITE_LWM2M
set rtmp_conf=CONFIG_NWY_OPEN_LITE_RTMP
set aws_mqtt_conf=CONFIG_NWY_OPEN_LITE_AWS_MQTT

findstr /e !lite_str! !h_file! >nul
if !errorlevel! equ 0 (
call:get_config
set /p go_config=Set the configurations 0-exit 1-set:
if !go_config! equ 0 goto end
set /p mqtt_on=Please config MQTT 0-OFF 1-ON:
set /p ali_on=Please config Ali MQTT 0-OFF 1-ON:
set /p tts_on=Please config TTS 0-OFF 1-ON:
set /p eng_tts_on=Please config ENG_TTS 0-OFF 1-ON:
set /p rws_on=Please config RWS 0-OFF 1-ON:
set /p azure_on=Please config Azure 0-OFF 1-ON:
set /p lwm2m_on=Please config lwm2m 0-OFF 1-ON:
set /p rtmp_on=Please config RTMP 0-OFF 1-ON:
set /p aws_mqtt_on=Please config AWS_MQTT 0-OFF 1-ON:

call:cmake_file_config
goto end
) else (
echo Error:not NWY_OPEN_LITE and can not config!!
goto end
)

:get_config
echo ******************************Current Configurations******************************
findstr !mqtt_conf! !cmake_path!!cmake_file! >nul
if !errorlevel! equ 0 (echo MQTT : ON) else (echo MQTT : OFF)
findstr !ali_conf! !cmake_path!!cmake_file! >nul
if !errorlevel! equ 0 (echo Ali MQTT : ON) else (echo Ali MQTT : OFF)
findstr !tts_conf! !cmake_path!!cmake_file! >nul
if !errorlevel! equ 0 (echo TTS : ON) else (echo TTS : OFF)
findstr !eng_tts_conf! !cmake_path!!cmake_file! >nul
if !errorlevel! equ 0 (echo ENG_TTS : ON) else (echo ENG_TTS : OFF)
findstr !rws_conf! !cmake_path!!cmake_file! >nul
if !errorlevel! equ 0 (echo RWS : ON) else (echo RWS : OFF)
findstr !azure_conf! !cmake_path!!cmake_file! >nul
if !errorlevel! equ 0 (echo AZURE : ON) else (echo AZURE : OFF)
findstr !lwm2m_conf! !cmake_path!!cmake_file! >nul
if !errorlevel! equ 0 (echo LWM2M : ON) else (echo LWM2M : OFF)
findstr !rtmp_conf! !cmake_path!!cmake_file! >nul
if !errorlevel! equ 0 (echo RTMP : ON) else (echo RTMP : OFF)
findstr !aws_mqtt_conf! !cmake_path!!cmake_file! >nul
if !errorlevel! equ 0 (echo AWS_MQTT : ON) else (echo AWS_MQTT : OFF)


echo ******************************Current Configurations******************************
goto:eof

:cmake_file_config
findstr /v "!mqtt_conf! !ali_conf! !tts_conf! !eng_tts_conf! !rws_conf! !azure_conf! !lwm2m_conf! !rtmp_conf! !aws_mqtt_conf!" %cmake_path%%cmake_file% > %cmake_path%%cmake_file%.tmp
if !mqtt_on! neq 0 echo set^(!mqtt_conf! on^) >> %cmake_path%%cmake_file%.tmp
if !ali_on! neq 0 echo set^(!ali_conf! on^) >> %cmake_path%%cmake_file%.tmp
if !tts_on! neq 0 echo set^(!tts_conf! on^) >> %cmake_path%%cmake_file%.tmp
if !eng_tts_on! neq 0 echo set^(!eng_tts_conf! on^) >> %cmake_path%%cmake_file%.tmp
if !rws_on! neq 0 echo set^(!rws_conf! on^) >> %cmake_path%%cmake_file%.tmp
if !azure_on! neq 0 echo set^(!azure_conf! on^) >> %cmake_path%%cmake_file%.tmp
if !lwm2m_on! neq 0 echo set^(!lwm2m_conf! on^) >> %cmake_path%%cmake_file%.tmp
if !rtmp_on! neq 0 echo set^(!rtmp_conf! on^) >> %cmake_path%%cmake_file%.tmp
if !aws_mqtt_on! neq 0 echo set^(!aws_mqtt_conf! on^) >> %cmake_path%%cmake_file%.tmp
cd !cmake_path!
del !cmake_file!
ren !cmake_file!.tmp !cmake_file!
cd ..
goto:eof

:end