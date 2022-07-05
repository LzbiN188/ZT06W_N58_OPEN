#include "osi_log.h"
#include "nwy_file.h"
#include "nwy_osi_api.h"
#include "nwy_security_ids.h"

#define NWY_PACKET_WARING_FILE_1        "nwy_waring_file_1"
#define NWY_PACKET_WARING_FILE_2        "nwy_waring_file_2"
#define NWY_FILE_MAX_SIZE                (20*1024)
#define NWY_EXT_SIO_RX_MAX          (4 * 1024)
#define NWY_BUFF_MAX                 1024

#define NWY_WARING_MAX                    50

static int s_waring_count1 = 0;
static int s_waring_count2 = 0;
static int s_file1_fd= 0;
static int s_file2_fd = 0;

static nwy_osiMutex_t *s_file_lock = NULL;

char nwy_ext_sio_recv_buff[NWY_EXT_SIO_RX_MAX + 1];

static int nwy_create_waring_file()
{

    s_file1_fd = nwy_sdk_fopen(NWY_PACKET_WARING_FILE_1, NWY_CREAT | NWY_RDWR |NWY_APPEND);
    s_file2_fd = nwy_sdk_fopen(NWY_PACKET_WARING_FILE_2, NWY_CREAT | NWY_RDWR |NWY_APPEND);

    if (s_file1_fd < 0 || s_file2_fd < 0) {
        return NWY_GEN_E_UNKNOWN;
    }

    nwy_sdk_fseek(s_file1_fd, 0, NWY_SEEK_END);
    nwy_sdk_fseek(s_file2_fd, 0, NWY_SEEK_END);
    return NWY_SUCESS;
}

static int nwy_waring_file_print()
{
    int size = 0;
    int fd = 0;
    int len = 0;
    int count = 0;
    int loop = 0;
    int time = 0;
    char waring_info[NWY_BUFF_MAX+1] = {0};
    char* sptr = nwy_ext_sio_recv_buff;
    struct stat st_info;
    nwy_ext_input_gets("\r\nPlease file id : ");

    nwy_lock_mutex(s_file_lock, 0);
    if (atoi(sptr) == 1) {
        fd =  nwy_sdk_fopen(NWY_PACKET_WARING_FILE_1, NWY_RDONLY);
        count = s_waring_count1;
    } else if (atoi(sptr) == 2) {
        fd =  nwy_sdk_fopen(NWY_PACKET_WARING_FILE_2, NWY_RDONLY);
        count = s_waring_count2;
    }
    nwy_sdk_fseek(fd, 0, NWY_SEEK_SET);

    memset(&st_info, 0, sizeof(st_info));
    nwy_sdk_get_stat_fd(fd, &st_info);
    size = st_info.st_size;

    time = size/NWY_BUFF_MAX;
    nwy_ext_echo("\r\nWaring count = %d", count);
    while(loop < time) {
        len = nwy_sdk_fread(fd, waring_info, NWY_BUFF_MAX);
        loop++;
        nwy_ext_echo("\r\n%s", waring_info);
        memset(waring_info, 0, sizeof(waring_info));
    }
    if (size%NWY_BUFF_MAX != 0) {
        memset(waring_info, 0, sizeof(waring_info));
        len = nwy_sdk_fread(fd, waring_info, NWY_BUFF_MAX);
        nwy_ext_echo("\r\n%s", waring_info);
    }
    nwy_sdk_fclose(fd);
    nwy_unlock_mutex(s_file_lock);
}
static int nwy_write_waring_to_file(int fd, char *waring_info, int len)
{
    int write_len = 0;
    if (fd <= 0 ) {
        return NWY_GEN_E_UNKNOWN;
    }

    write_len = nwy_sdk_fwrite(fd, waring_info, len);
    if(write_len != len) {
        return NWY_GEN_E_UNKNOWN;
    }
    return NWY_SUCESS;
}

static int nwy_clean_and_write_info(int fd, char *waring_info, int len)
{
    int ret = NWY_GEN_E_UNKNOWN;
    nwy_sdk_ftrunc_fd(fd, 0);
    nwy_sdk_fseek(fd, 0, NWY_SEEK_SET);

    ret = nwy_write_waring_to_file(fd, waring_info, len);
    return ret;
}

void nwy_store_waring_proc(unsigned char *data, int len)
{
    int ret = NWY_GEN_E_UNKNOWN;
    int fd_1 = 0;
    static int file_flag = 0;   //indicate the last store file
    char *buf = NULL;


    nwy_lock_mutex(s_file_lock, 0);

    nwy_create_waring_file();

    OSI_LOGXI(OSI_LOGPAR_SII, 0, "nwy rhino:%s  count1 = %d count2= %d", data, s_waring_count1, s_waring_count2);
    if (s_waring_count1 < NWY_WARING_MAX) {
        ret = nwy_write_waring_to_file(s_file1_fd, data, len);
        if (ret != NWY_SUCESS) {
            OSI_LOGI(0, "nwy write waring in file error");
            goto END;
        }
        file_flag = 1;
        s_waring_count1++;
    } else {
        /*file1 is full and file2 is not full */
        if (s_waring_count2 < NWY_WARING_MAX) {
            ret = nwy_write_waring_to_file(s_file2_fd, data, len);
            if (ret != NWY_SUCESS) {
                OSI_LOGI(0, "nwy write waring in file error");
                goto END;
            }
            file_flag = 2;
            s_waring_count2++;
        } else {
            if (file_flag == 1) {
                s_waring_count2 = 0;
                ret = nwy_clean_and_write_info(s_file2_fd, data, len);
                if (ret != NWY_SUCESS) {
                    OSI_LOGI(0, "nwy write waring in file error");
                    goto END;
                }
                file_flag = 2;
                s_waring_count2++;
            } else {
                s_waring_count1 = 0;
                ret = nwy_clean_and_write_info(s_file1_fd, data, len);
                if (ret != NWY_SUCESS) {
                    OSI_LOGI(0, "nwy write waring in file error");
                    goto END;
                }
                file_flag = 1;
                s_waring_count1++;
            }
        }
    }

END:
    nwy_sdk_fclose(s_file1_fd);
    nwy_sdk_fclose(s_file2_fd);

    nwy_unlock_mutex(s_file_lock);
}

void nwy_waring_packet_proc(unsigned char *data, int len)
{
    nwy_store_waring_proc(data, len);
}

void nwy_set_rhino_debug_switch()
{
    int level = 0;

    char* sptr = nwy_ext_sio_recv_buff;
    nwy_ext_input_gets("\r\nPlease debug switch(0-1) : ");

    level = atoi(sptr);
    nwy_ids_config_debuglvl(level);
}

void nwy_rhino_test_menu()
{
    nwy_ext_echo("\r\n1. get waring file");
    nwy_ext_echo("\r\n2. set debug level (0-1)");
    nwy_ext_echo("\r\n0. exit");
}
void nwy_rhino_ids_test()
{
    char* sptr = nwy_ext_sio_recv_buff;
    static int enter_flag = 0;
    int ret = NWY_GEN_E_UNKNOWN;

    if (enter_flag == 0) {
        s_file_lock = nwy_create_mutex();
        if (s_file_lock == NULL) {
            nwy_ext_echo("\r\ncreate s_file_lock fail");
            return;
        }

        /*when first enter, delete the original file*/
        nwy_sdk_file_unlink(NWY_PACKET_WARING_FILE_1);
        nwy_sdk_file_unlink(NWY_PACKET_WARING_FILE_2);

        ret = nwy_ids_init();
        if (ret != NWY_SUCESS) {
            nwy_ext_echo("\r\nnwy_ids_init init fail");
            return;
        }
        nwy_ids_config_debuglvl(0);
        nwy_get_warning_packet_reg(nwy_waring_packet_proc);
        enter_flag = 1;
    }

    while(1) {
        nwy_rhino_test_menu();
        nwy_ext_input_gets("\r\nPlease input : ");

        switch(atoi(sptr)) {
            case 0:
                return;
            case 1:
                nwy_waring_file_print();
                break;
            case 2:
                nwy_set_rhino_debug_switch();
                break;
            default:
                break;
        }
    }

}


