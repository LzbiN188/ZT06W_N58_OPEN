# Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
# All rights reserved.
#
# This software is supplied "AS IS" without any warranties.
# RDA assumes no responsibility or liability for the use of the software,
# conveys no license or title under any patent, copyright, or mask work
# right to the product. RDA reserves the right to make changes in the
# software without notification.  RDA also make no representation or
# warranty that such application will be suitable for the specified use
# without further testing or modification.

cmake_minimum_required(VERSION 3.13)

set(SOURCE_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BINARY_TOP_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_C_COMPILER_FORCED   1)
set(CMAKE_CXX_COMPILER_FORCED 1)

set(out_hex_dir ${CMAKE_CURRENT_BINARY_DIR}/hex)
set(out_lib_dir ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(out_inc_dir ${CMAKE_CURRENT_BINARY_DIR}/include)
set(tools_dir ${CMAKE_CURRENT_SOURCE_DIR}/tools)
if(DEFINED BUILD_CUST_PROJ)
	set(preset_dir ${CMAKE_CURRENT_SOURCE_DIR}/${BUILD_CUST_PROJ}/preset)
else()
	set(preset_dir ${CMAKE_CURRENT_SOURCE_DIR}/prebuilts/preset)
endif()
set(custom_nv_dir ${CMAKE_CURRENT_SOURCE_DIR}/prebuilts/custom_nv)

set(core_stub_o ${CMAKE_CURRENT_SOURCE_DIR}/components/lib/core_stub.o)
set(flash_ldscript ${SOURCE_TOP_DIR}/ldscripts/app_flashimg.ld)
set(file_ldscript ${SOURCE_TOP_DIR}/ldscripts/app_fileimg.ld)
set(cmd_mkappimg dtools mkappimg)
set(pacgen_py ${tools_dir}/pacgen.py)

include(cmake/core_config.cmake)
include(cmake/toolchain-gcc.cmake)
set(BUILD_SHARED_LIBS OFF)

project(core C CXX ASM)

include_directories(components/include components/newlib/include components/mbedtls/include)

function(cpp_only target file)
    add_library(${target} OBJECT ${file})
    set_source_files_properties(${file} PROPERTIES LANGUAGE C)
    target_compile_options(${target} PRIVATE -E -P -x c)
endfunction()

function(add_appimg target ldscript)
    set(gen_ldscript ${target}_ldscript)
    set(target_map_file ${out_hex_dir}/${target}.map)
    set(target_img_file ${out_hex_dir}/${target}.img)
    cpp_only(${gen_ldscript} ${ldscript})
    add_executable(${target} ${ARGN} ${core_stub_o})
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${out_hex_dir})
    target_link_libraries(${target} PRIVATE -T $<TARGET_OBJECTS:${gen_ldscript}>)
    target_link_libraries(${target} PRIVATE -Wl,-Map=${target_map_file} -nostdlib -Wl,--gc-sections)

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${cmd_mkappimg} $<TARGET_FILE:${target}> ${target_img_file}
        BYPRODUCTS ${target_img_file} ${target_map_file}
    )
endfunction()

configure_file(${SOURCE_TOP_DIR}/components/lib/fdl1.img ${out_hex_dir}/fdl1.img COPYONLY)
configure_file(${SOURCE_TOP_DIR}/components/lib/fdl2.img ${out_hex_dir}/fdl2.img COPYONLY)
macro(pac_init_fdl cmd)
    set(${cmd}
        cfg-init --pname UIX8910_MODEM --palias "APPIMG"
            --pversion "8910 MODULE" --version "BP_R1.0.0"
            --flashtype 1
        cfg-host-fdl -a ${CONFIG_FDL1_IMAGE_START} -s ${CONFIG_FDL1_IMAGE_SIZE}
            -p ${out_hex_dir}/fdl1.img
        cfg-fdl2 -a ${CONFIG_FDL2_IMAGE_START} -s ${CONFIG_FDL2_IMAGE_SIZE}
            -p ${out_hex_dir}/fdl2.img
    )
endmacro()

if(CONFIG_APPIMG_LOAD_FLASH)
    set(target appimg_flash_delete)
    set(pac_file ${out_hex_dir}/${target}.pac)
    pac_init_fdl(init_fdl)
    add_custom_command(OUTPUT ${pac_file}
        COMMAND python3 ${pacgen_py} ${init_fdl}
            cfg-erase-flash -i ERASE_APPIMG
                -a ${CONFIG_APPIMG_FLASH_ADDRESS}
                -s ${CONFIG_APPIMG_FLASH_SIZE}
            pac-gen ${pac_file}
        DEPENDS ${pacgen_py}
        WORKING_DIRECTORY ${SOURCE_TOP_DIR}
    )
    add_custom_target(${target}_pacgen ALL DEPENDS ${pac_file})
endif()

if(CONFIG_APPIMG_LOAD_FILE)
    set(target appimg_file_delete)
    set(pac_file ${out_hex_dir}/${target}.pac)
    pac_init_fdl(init_fdl)
    add_custom_command(OUTPUT ${pac_file}
        COMMAND python3 ${pacgen_py} ${init_fdl}
            cfg-del-appimg -i DEL_APPIMG
            pac-gen ${pac_file}
        DEPENDS ${pacgen_py}
        WORKING_DIRECTORY ${SOURCE_TOP_DIR}
    )
    add_custom_target(${target}_pacgen ALL DEPENDS ${pac_file})
endif()

macro(pac_nvitem_8910 cmd)
    set(${cmd}
        cfg-nvitem -n "Calibration" -i 0xFFFFFFFF --use 1 --replace 0 --continue 0 --backup 1
        cfg-nvitem -n "GSM Calibration" -i 0x26d --use 1 --replace 0 --continue 1 --backup 1
        cfg-nvitem -n "LTE Calibration" -i 0x26e --use 1 --replace 0 --continue 0 --backup 1
        cfg-nvitem -n "IMEI" -i 0xFFFFFFFF --use 1 --replace 0 --continue 0 --backup 1
    )
endmacro()

function(add_subdirectory_if_exist dir)
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/CMakeLists.txt)
            add_subdirectory(${dir})
        endif()
    endif()
endfunction()

set(CONFIG_NVBIN_FIXED_SIZE 0x20000)
if(CONFIG_APPIMG_LOAD_FLASH)
	message("proj is ${BUILD_CUST_PROJ}")
	
	if(CONFIG_NWY_OPEN_LITE_PAHOMQTT)
	add_definitions(-D FEATURE_NWY_PAHO_MQTT)
	else()
	add_definitions(-D FEATURE_NWY_OPEN_LITE_PAHOMQTT_OFF)
	endif()
	
    if(CONFIG_NWY_OPEN_LITE_ALIMQTT)
	add_definitions(-D FEATURE_NWY_ALI_MQTT)
	else()
	add_definitions(-D FEATURE_NWY_OPEN_LITE_ALIMQTT_OFF)
	endif()
	
    if(CONFIG_NWY_OPEN_LITE_TTS)
	else()
	add_definitions(-D FEATURE_NWY_OPEN_LITE_TTS_OFF)
	endif()
	
	if(CONFIG_NWY_OPEN_LITE_RWS)
	add_definitions(-D RWS_PLAT_RDA)
	add_definitions(-D NWY_RWS_RTOS)
	endif()
	
	if(DEFINED BUILD_CUST_PROJ)
		set(target ${BUILD_CUST_PROJ})
		add_subdirectory_if_exist(${BUILD_CUST_PROJ})
	else()
		set(target hello_flash)
		file(GLOB_RECURSE APP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/app/*.c)
		add_appimg(${target} ${flash_ldscript}  ${APP_SRC})
		#add_appimg(${target} ${flash_ldscript}  hello_world.c nwy_drv_lcd_st7735.c nwy_multi_socket_test.c)
	endif()
	
	if(CONFIG_NWY_OPEN_LITE_PAHOMQTT)
	target_link_libraries(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/components/lib/libnwyphomqtt.a)
	endif()
	
    if(CONFIG_NWY_OPEN_LITE_ALIMQTT)
	target_link_libraries(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/components/lib/libnwyalimqtt.a)
	endif()
	
    if(CONFIG_NWY_OPEN_LITE_TTS)
	target_link_libraries(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/components/lib/libnwyivtts.a)
	target_link_libraries(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/components/lib/libAiSound5.a)
	endif()
	
	if(CONFIG_NWY_OPEN_LITE_RWS)
	target_link_libraries(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/components/lib/libnwyrws.a)
	endif()
	
    target_link_libraries(${target} PRIVATE ${libc_file_name} ${libm_file_name} ${libgcc_file_name})

	target_link_libraries(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/components/lib/libcarMotion.a)
	target_link_libraries(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/components/lib/LIBCH58xBLE.a)
    set(pac_file ${out_hex_dir}/${target}.pac)
    set(target_img_file ${out_hex_dir}/${target}.img)
    pac_init_fdl(init_fdl)
    if(custom_nv_dir)
    file(GLOB_RECURSE custom_nv_files ${custom_nv_dir}/*nvitem.bin)
    foreach(custom_nv_bin ${custom_nv_files})
    set(pkt_phase_check
    cfg-phase-check
    )
    pac_nvitem_8910(nvitem_8910)
    set(pkt_custom_nv 
    cfg-clear-nv
    cfg-nv -s ${CONFIG_NVBIN_FIXED_SIZE} -p ${custom_nv_bin}
    )
    endforeach(custom_nv_bin)
    endif(custom_nv_dir)
    
    if(preset_dir)
    file(REMOVE ${preset_dir}/.gitkeep)
    file(GLOB_RECURSE preset_files ${preset_dir}/*)
    foreach(preset ${preset_files})
    string(REGEX REPLACE ${preset_dir}/ "" preset_file ${preset})
    message("find preset file ${preset_file} in dir ${preset_dir}")
    set(pkt_preset_flies 
    ${pkt_preset_flies} 
    cfg-pack-file -i ${preset_file} -p ${preset_dir}/${preset_file} -n /${preset_file}
    )
    message("cmd ${pkt_preset_flies}")
    endforeach(preset)
    endif(preset_dir)
    
    add_custom_command(OUTPUT ${pac_file}
        COMMAND python3 ${pacgen_py} ${init_fdl} ${nvitem_8910} ${pkt_phase_check}
            cfg-image -i APPIMG -a ${CONFIG_APPIMG_FLASH_ADDRESS}
                -s ${CONFIG_APPIMG_FLASH_SIZE}
                -p ${target_img_file}
                ${pkt_preset_flies}
                ${pkt_custom_nv}
            pac-gen ${pac_file}
        DEPENDS ${pacgen_py} ${target_img_file}
        WORKING_DIRECTORY ${SOURCE_TOP_DIR}
    )
    add_custom_target(${target}_pacgen ALL DEPENDS ${pac_file})
endif()

if(CONFIG_APPIMG_LOAD_FILE)
    set(target hello_file)
	file(GLOB_RECURSE APP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/app/*.c)
	add_appimg(${target} ${flash_ldscript}  ${APP_SRC})
    #add_appimg(${target} ${file_ldscript} hello_world.c nwy_drv_lcd_st7735.c nwy_multi_socket_test.c)
    target_link_libraries(${target} PRIVATE ${libc_file_name} ${libm_file_name} ${libgcc_file_name})

    set(pac_file ${out_hex_dir}/${target}.pac)
    set(target_img_file ${out_hex_dir}/${target}.img)
    pac_init_fdl(init_fdl)
    if(custom_nv_dir)
    file(GLOB_RECURSE custom_nv_files ${custom_nv_dir}/*nvitem.bin)
    foreach(custom_nv_bin ${custom_nv_files})
    set(pkt_phase_check
    cfg-phase-check
    )
    pac_nvitem_8910(nvitem_8910)
    set(pkt_custom_nv 
    cfg-clear-nv
    cfg-nv -s ${CONFIG_NVBIN_FIXED_SIZE} -p ${custom_nv_bin}
    )
    endforeach(custom_nv_bin)
    endif(custom_nv_dir)
    
    if(preset_dir)
    file(REMOVE ${preset_dir}/.gitkeep)
    file(GLOB_RECURSE preset_files ${preset_dir}/*)
    foreach(preset ${preset_files})
    string(REGEX REPLACE ${preset_dir}/ "" preset_file ${preset})
    message("find preset file ${preset_file} in dir ${preset_dir}")
    set(pkt_preset_flies 
    ${pkt_preset_flies} 
    cfg-pack-file -i ${preset_file} -p ${preset_dir}/${preset_file} -n /${preset_file}
    )
    message("cmd ${pkt_preset_flies}")
    endforeach(preset)
    endif(preset_dir)
    
    add_custom_command(OUTPUT ${pac_file}
        COMMAND python3 ${pacgen_py} ${init_fdl} ${nvitem_8910} ${pkt_phase_check}
            cfg-pack-file -i APPIMG -p ${target_img_file}
                -n ${CONFIG_APPIMG_LOAD_FILE_NAME}
            ${pkt_preset_flies}
            ${pkt_custom_nv}
            pac-gen ${pac_file}
        DEPENDS ${pacgen_py} ${target_img_file}
        WORKING_DIRECTORY ${SOURCE_TOP_DIR}
    )
    add_custom_target(${target}_pacgen ALL DEPENDS ${pac_file})
endif()
