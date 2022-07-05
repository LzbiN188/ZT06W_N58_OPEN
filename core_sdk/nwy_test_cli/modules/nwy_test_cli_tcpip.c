#include "osi_log.h"
#include "nwy_test_cli_utils.h"
#include "nwy_data.h"
#include "nwy_socket.h"
#include "nwy_ftp.h"
#include "nwy_file.h"
#include "nwy_http.h"
#include "nwy_md5.h"
#define NWY_TCP_SOCKET_MAX 5
#define NWY_UDP_SOCKET_MAX 5
#define NWY_BUFF_MAX 128
typedef enum
{
    NWY_CUSTOM_IP_TYPE_OR_DNS_NONE = -1,
    NWY_CUSTOM_IP_TYPE_OR_DNS_IPV4 = 0,
    NWY_CUSTOM_IP_TYPE_OR_DNS_IPV6 = 1,
    NWY_CUSTOM_IP_TYPE_OR_DNS_DNS = 2
} nwy_ip_type_or_dns_enum;
//ftp
typedef struct dataRecvContext
{
    unsigned size;
    unsigned pos;
    char data[0];
} dataRecvContext_t;
typedef struct nwy_file_ftp_info_s
{
    int is_vaild;
    char filename[256];
    int pos;
    //int length;
    //int file_size;
} nwy_file_ftp_info_s;

/***************   extern prototype   ***************/
extern int ipaddr_aton(const char *cp, ip_addr_t *addr);
extern int ip6addr_aton(const char *cp, ip6_addr_t *addr);
extern int ip4addr_aton(const char *cp, ip4_addr_t *addr);

//socket
static int s_nwy_cli_tcp_fd = 0;
static int s_nwy_cli_udp_fd = 0;
static int s_nwy_tcp_connect_flag = 0;
static int s_nwy_udp_connect_flag = 0;
nwy_osiThread_t *s_nwy_tcp_recv_thread = NULL;
nwy_osiThread_t *s_nwy_udp_recv_thread = NULL;
//ftp
static int nwy_cli_update_flag = 0;
static dataRecvContext_t *s_nwy_ftp_recv = NULL;
nwy_osiThread_t *s_nwy_ftp_current_thread = NULL;
static nwy_file_ftp_info_s s_nwy_cli_fileftp;
//http
static boolean s_nwy_http_choice = true;
static bool nwy_cli_check_str_isdigit(char *str)
{
    if (NULL == str)
        return false;
    char *p = str;
    while ('\0' != *p)
    {
        if (!('0' <= *p && '9' >= *p))
            return false;
        p++;
    }
    return true;
}
static nwy_ip_type_or_dns_enum nwy_judge_ip_or_dns(char *str)
{
    int len = 0;
    int strLen = 0;
    nwy_ip_type_or_dns_enum retValue = NWY_CUSTOM_IP_TYPE_OR_DNS_DNS;
    if (str == NULL)
    {
        return NWY_CUSTOM_IP_TYPE_OR_DNS_NONE;
    }
    else
    {
        if (strlen(str) <= 0)
        {
            return NWY_CUSTOM_IP_TYPE_OR_DNS_NONE;
        }
    }
    strLen = strlen(str);
    for (len = 0; len < strLen; len++)
    {
        if (((*(str + len) >= '0') && (*(str + len) <= '9')) || (*(str + len) == '.'))
        {
            continue;
        }
        else
        {
            break;
        }
    }
    if (len == strLen)
    {
        retValue = NWY_CUSTOM_IP_TYPE_OR_DNS_IPV4;
        return retValue;
    }
    len = 0;
    for (len = 0; len < strLen; len++)
    {
        if (((*(str + len) >= '0') && (*(str + len) <= '9')) ||
            ((*(str + len) >= 'a') && (*(str + len) <= 'f')) ||
            ((*(str + len) >= 'A') && (*(str + len) <= 'F')) ||
            (*(str + len) == ':'))
        {
            continue;
        }
        else
        {
            break;
        }
    }
    if (len == strLen)
    {
        retValue = NWY_CUSTOM_IP_TYPE_OR_DNS_IPV6;
        return retValue;
    }
    return retValue;
}
static int nwy_hostname_check(char *hostname)
{
    int a, b, c, d;
    char temp[32] = {0};
    if (strlen(hostname) > 15)
        return NWY_GEN_E_UNKNOWN;
    if ((sscanf(hostname, "%d.%d.%d.%d", &a, &b, &c, &d)) != 4)
        return NWY_GEN_E_UNKNOWN;
    if (!((a <= 255 && a >= 0) && (b <= 255 && b >= 0) && (c <= 255 && c >= 0)))
        return NWY_GEN_E_UNKNOWN;
    sprintf(temp, "%d.%d.%d.%d", a, b, c, d);

    strcpy(hostname, temp);
    return NWY_SUCESS;
}
static int nwy_get_ip_str(char *url_or_ip, char *ip_str, int *isipv6)
{
    char *str = NULL;
    nwy_ip_type_or_dns_enum ip_dns_type = NWY_CUSTOM_IP_TYPE_OR_DNS_NONE;

    ip_dns_type = nwy_judge_ip_or_dns(url_or_ip);
    if (ip_dns_type == NWY_CUSTOM_IP_TYPE_OR_DNS_DNS)
    {
        str = nwy_gethostbyname1(url_or_ip, isipv6);
        if (str == NULL || 0 == strlen(str))
        {
            nwy_test_cli_echo("\r\ninput ip or url %s invalid\r\n", url_or_ip);
            return NWY_GEN_E_UNKNOWN;
        }
        memcpy(ip_str, str, strlen(str));

        nwy_test_cli_echo("\r\n%s get ip:%s\r\n", url_or_ip, ip_str);
    }
    else
    {
        memcpy(ip_str, url_or_ip, strlen(url_or_ip));
    }
    if (strchr(ip_str, ':') != NULL)
    {
        *isipv6 = 1;
    }
    else
    {
        *isipv6 = 0;
    }
    return NWY_SUCESS;
}
static int nwy_port_get(char *port_str, int *port)
{
    if (port == NULL)
    {
        return NWY_GEN_E_INVALID_PARA;
    }
    for (int i = 0; i < strlen(port_str); i++)
    {
        if (*(port_str + i) == '\r' || *(port_str + i) == '\n')
        {
            continue;
        }
        if ((port_str[i]) < '0' || (port_str[i]) > '9')
        {
            return NWY_GEN_E_INVALID_PARA;
        }
    }

    *port = atoi(port_str);
    if (*port < 1 || *port > 65535)
    {
        *port = 0;
        return NWY_GEN_E_INVALID_PARA;
    }
    else
    {
        return NWY_SUCESS;
    }
}

static int nwy_cli_socket_destory(int *socketid)
{
    int ret = 0;
    ret = nwy_socket_close(*socketid);
    if (ret != NWY_SUCESS)
    {
        nwy_test_cli_echo("\r\nSocket close fail");
        return ret;
    }
    *socketid = 0;
    nwy_test_cli_echo("\r\nSocket close sucess");
    return ret;
}
static void nwy_cli_tcp_recv_func(void *param)
{
    char recv_buff[NWY_UART_RECV_SINGLE_MAX + 1] = {0};
    int recv_len = 0, result = 0;
    fd_set rd_fd;
    fd_set ex_fd;
    FD_ZERO(&rd_fd);
    FD_ZERO(&ex_fd);
    FD_SET(s_nwy_cli_tcp_fd, &rd_fd);
    FD_SET(s_nwy_cli_tcp_fd, &ex_fd);
    struct timeval tv = {0};
    tv.tv_sec = 20;
    tv.tv_usec = 0;
    while (1)
    {
        FD_ZERO(&rd_fd);
        FD_ZERO(&ex_fd);
        FD_SET(s_nwy_cli_tcp_fd, &rd_fd);
        FD_SET(s_nwy_cli_tcp_fd, &ex_fd);
        result = nwy_socket_select(s_nwy_cli_tcp_fd + 1, &rd_fd, NULL, &ex_fd, &tv);
        if (result < 0)
        {
            nwy_test_cli_echo("\r\ntcp select error:\r\n");
            nwy_cli_socket_destory(&s_nwy_cli_tcp_fd);
            s_nwy_tcp_connect_flag = 0;
            nwy_exit_thread();
        }
        else if (result > 0)
        {
            if (FD_ISSET(s_nwy_cli_tcp_fd, &rd_fd))
            {
                memset(recv_buff, 0, NWY_UART_RECV_SINGLE_MAX + 1);
                recv_len = nwy_socket_recv(s_nwy_cli_tcp_fd, recv_buff, NWY_UART_RECV_SINGLE_MAX, 0);
                if (recv_len > 0)
                    nwy_test_cli_echo("\r\ntcp read:%d:%s\r\n", recv_len, recv_buff);
                else if (recv_len == 0)
                {
                    nwy_test_cli_echo("\r\ntcp close by server\r\n");
                    nwy_cli_socket_destory(&s_nwy_cli_tcp_fd);
                    s_nwy_tcp_connect_flag = 0;
                    nwy_exit_thread();
                }
                else
                {
                    nwy_test_cli_echo("\r\ntcp connection error\r\n");
                    nwy_cli_socket_destory(&s_nwy_cli_tcp_fd);
                    s_nwy_tcp_connect_flag = 0;
                    nwy_exit_thread();
                }
            }
            if (FD_ISSET(s_nwy_cli_tcp_fd, &ex_fd))
            {
                nwy_test_cli_echo("\r\ntcp select ex_fd:\r\n");
                nwy_cli_socket_destory(&s_nwy_cli_tcp_fd);
                s_nwy_tcp_connect_flag = 0;
                nwy_exit_thread();
            }
        }
        else
            OSI_LOGI(0, "\r\ntcp select timeout:\r\n");
        nwy_sleep(1000);
    }
}
static void nwy_cli_udp_recv_func(void *param)
{
    char recv_buff[512] = {0};
    int recv_len = 0, result = 0;
    fd_set rd_fd;
    fd_set ex_fd;
    FD_ZERO(&rd_fd);
    FD_ZERO(&ex_fd);
    FD_SET(s_nwy_cli_udp_fd, &rd_fd);
    FD_SET(s_nwy_cli_udp_fd, &ex_fd);
    struct timeval tv = {0};
    tv.tv_sec = 20;
    tv.tv_usec = 0;
    while (1)
    {
        FD_ZERO(&rd_fd);
        FD_ZERO(&ex_fd);
        FD_SET(s_nwy_cli_udp_fd, &rd_fd);
        FD_SET(s_nwy_cli_udp_fd, &ex_fd);
        result = nwy_socket_select(s_nwy_cli_udp_fd + 1, &rd_fd, NULL, &ex_fd, &tv);
        if (result < 0)
        {
            nwy_test_cli_echo("\r\nudp select error:\r\n");
            nwy_cli_socket_destory(&s_nwy_cli_udp_fd);
            s_nwy_udp_connect_flag = 0;
            nwy_exit_thread();
        }
        else if (result > 0)
        {
            if (FD_ISSET(s_nwy_cli_udp_fd, &rd_fd))
            {
                memset(recv_buff, 0, 512);
                recv_len = nwy_socket_recv(s_nwy_cli_udp_fd, recv_buff, sizeof(recv_buff), 0);
                if (recv_len > 0)
                    nwy_test_cli_echo("\r\nudp read:%d:%s\r\n", recv_len, recv_buff);
                else if (recv_len == 0)
                    nwy_test_cli_echo("\r\nudp close by server\r\n");
                else
                {
                    nwy_test_cli_echo("\r\nudp connection error\r\n");
                    nwy_cli_socket_destory(&s_nwy_cli_udp_fd);
                    s_nwy_udp_connect_flag = 0;
                    nwy_exit_thread();
                }
            }
            if (FD_ISSET(s_nwy_cli_udp_fd, &ex_fd))
            {
                nwy_test_cli_echo("\r\nudp select ex_fd:\r\n");
                nwy_cli_socket_destory(&s_nwy_cli_udp_fd);
                s_nwy_udp_connect_flag = 0;
                nwy_exit_thread();
            }
        }
        else
        {
        } //nwy_test_cli_echo("\r\nudp select timeout:\r\n");
    }
}
static int nwy_cli_tcp_connect(struct sockaddr_in *sa_v4, struct sockaddr_in6 *sa_v6, int af_inet_flag)
{
    uint64_t start = 0;
    int ret = NWY_GEN_E_INVALID_PARA;
    start = nwy_get_ms();
    OSI_LOGI(0, "nwy_cli_tcp_connect enter");
    do
    {
        if ((nwy_get_ms() - start) >= 10000)
        {
            nwy_test_cli_echo("\r\nsocket connect timeout\r\n");
            return NWY_GEN_E_UNKNOWN;
        }
        if (af_inet_flag == AF_INET6)
        {
            ret = nwy_socket_connect(s_nwy_cli_tcp_fd, (struct sockaddr *)sa_v6, sizeof(struct sockaddr));
        }
        else
        {
            ret = nwy_socket_connect(s_nwy_cli_tcp_fd, (struct sockaddr *)sa_v4, sizeof(struct sockaddr));
        }
        if (ret != NWY_SUCESS)
        {
            if (EISCONN == nwy_socket_errno())
            {
                nwy_test_cli_echo("\r\nnwy_net_connect_tcp connect ok..");
                s_nwy_tcp_connect_flag = 1;
                break;
            }
            if (EINPROGRESS != nwy_socket_errno() && EALREADY != nwy_socket_errno())
            {
                nwy_test_cli_echo("\r\nconnect errno = %d", nwy_socket_errno());
                nwy_cli_socket_destory(&s_nwy_cli_tcp_fd);
                break;
            }
        }
        nwy_sleep(1000);
    } while (1);
    OSI_LOGI(0, "nwy_cli_tcp_connect s_nwy_tcp_connect_flag = %d", s_nwy_tcp_connect_flag);
    if (s_nwy_tcp_connect_flag)
    {
        s_nwy_tcp_recv_thread = nwy_create_thread("s_nwy_cli_recv_thread", nwy_cli_tcp_recv_func, (void *)&s_nwy_cli_tcp_fd, NWY_OSI_PRIORITY_NORMAL, 2048, 4);
        if (s_nwy_tcp_recv_thread == NULL)
        {
            nwy_test_cli_echo("\r\ncreate tcp recv thread failed, close connect");
            nwy_cli_socket_destory(&s_nwy_cli_tcp_fd);
            s_nwy_tcp_connect_flag = 0;
        }
        return NWY_SUCESS;
    }
    return NWY_GEN_E_UNKNOWN;
}
static int nwy_cli_socket_setup_pro(char *url_or_ip, int port, int socekt_type)
{
    int isipv6 = 0;
    int fd = 0;
    int on = 1;
    int opt = 1;
    int ret = NWY_GEN_E_UNKNOWN;
    int af_inet_flag = AF_INET;
    char ip_buf[NWY_BUFF_MAX + 1] = {0};
    ip_addr_t addr = {0};
    struct sockaddr_in sa_v4;
    struct sockaddr_in6 sa_v6;

    if (url_or_ip == NULL)
    {
        return NWY_GEN_E_INVALID_PARA;
    }
    nwy_test_cli_echo("\r\nurl test start:%s:%d\r\n", url_or_ip, port);
    ret = nwy_get_ip_str(url_or_ip, ip_buf, &isipv6);
    if (ret != NWY_SUCESS)
    {
        return NWY_GEN_E_INVALID_PARA;
    }
    if (isipv6)
    {
        if (ip6addr_aton(ip_buf, &addr.u_addr.ip6) == 0)
        {
            nwy_test_cli_echo("\r\ninput ip or url is invalid");
            return NWY_GEN_E_INVALID_PARA;
        }
        else
        {

            inet6_addr_from_ip6addr(&sa_v6.sin6_addr, ip_2_ip6(&addr));
            sa_v6.sin6_len = sizeof(struct sockaddr_in);
            sa_v6.sin6_family = AF_INET6;
            sa_v6.sin6_port = htons(port);
            af_inet_flag = AF_INET6;
        }
    }
    else
    {
        ret = nwy_hostname_check(ip_buf);
        if (ret != NWY_SUCESS)
        {
            nwy_test_cli_echo("\r\ninput ip or url is invalid");
            return NWY_GEN_E_INVALID_PARA;
        }

        if (ip4addr_aton(ip_buf, &addr.u_addr.ip4) == 0)
        {
            nwy_test_cli_echo("\r\ninput ip error:\r\n");
            return NWY_GEN_E_INVALID_PARA;
        }
        inet_addr_from_ip4addr(&sa_v4.sin_addr, ip_2_ip4(&addr));
        sa_v4.sin_len = sizeof(struct sockaddr_in);
        sa_v4.sin_family = AF_INET;
        sa_v4.sin_port = htons(port);
        af_inet_flag = AF_INET;
    }

    if (socekt_type == IPPROTO_TCP)
    {
        if (s_nwy_cli_tcp_fd <= 0)
        {
            s_nwy_cli_tcp_fd = nwy_socket_open(af_inet_flag, SOCK_STREAM, socekt_type);
            if (fd < 0)
            {
                nwy_test_cli_echo("\r\n socket open fail:\r\n");
                return NWY_GEN_E_INVALID_PARA;
            }
        }
        OSI_LOGI(0, "nwy s_nwy_cli_tcp_fd = %d", s_nwy_cli_tcp_fd);
        nwy_socket_setsockopt(s_nwy_cli_tcp_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));
        nwy_socket_setsockopt(s_nwy_cli_tcp_fd, IPPROTO_TCP, TCP_NODELAY, (void *)&opt, sizeof(opt));
        if (0 != nwy_socket_set_nonblock(s_nwy_cli_tcp_fd))
        {
            nwy_test_cli_echo("\r\nsocket set err\r\n");
            return NWY_GEN_E_INVALID_PARA;
        }
        ret = nwy_cli_tcp_connect(&sa_v4, &sa_v6, af_inet_flag);
        if (ret == NWY_SUCESS)
        {
            nwy_test_cli_echo("\r\nsocket set up sucess\r\n");
        }
    }
    else if (socekt_type == IPPROTO_UDP)
    {
        if (s_nwy_cli_udp_fd <= 0)
        {
            s_nwy_cli_udp_fd = nwy_socket_open(af_inet_flag, SOCK_DGRAM, socekt_type);
            if (s_nwy_cli_udp_fd <= 0)
            {
                nwy_test_cli_echo("\r\nsocket open fail:\r\n");
                return NWY_GEN_E_INVALID_PARA;
            }
        }
        s_nwy_udp_recv_thread = nwy_create_thread("nwy_udp_recv_thread", nwy_cli_udp_recv_func, (void *)&s_nwy_cli_udp_fd, NWY_OSI_PRIORITY_NORMAL, 2048, 4);
        if (s_nwy_udp_recv_thread == NULL)
        {
            nwy_test_cli_echo("\r\ncreate udp recv thread failed");
            return NWY_GEN_E_INVALID_PARA;
        }
        else
        {
            s_nwy_udp_connect_flag = 1;
            nwy_test_cli_echo("\r\nUDP setup successful");
        }
    }
    return ret;
}
/**************************TCP*********************************/
void nwy_test_cli_tcp_setup()
{
    char *tmp = NULL;
    char url_or_ip[128 + 1] = {0};
    int port = 0;
    int ret = 0;
    if (s_nwy_cli_tcp_fd > 0)
    {
        nwy_test_cli_echo("\r\nSocket has been setup");
        return;
    }
    tmp = nwy_test_cli_input_gets("\r\nPlease input url: ");
    memcpy(url_or_ip, tmp, strlen(tmp));
    tmp = nwy_test_cli_input_gets("\r\nPlease input port: ");
    if (nwy_port_get(tmp, &port) != NWY_SUCESS)
    {
        nwy_test_cli_echo("\r\nInput port error");
        return;
    }

    ret = nwy_cli_socket_setup_pro(url_or_ip, port, IPPROTO_TCP);
    if (ret != NWY_SUCESS)
    {
        nwy_cli_socket_destory(&s_nwy_cli_tcp_fd);
        nwy_test_cli_echo("\r\nTCP setup error");
    }
}
void nwy_test_cli_tcp_send()
{
    int send_len = 0;
    int len = 0;
    char *buff = NULL;
    if (!s_nwy_tcp_connect_flag)
    {
        nwy_test_cli_echo("\r\ntcp not setup");
        return;
    }
    buff = nwy_test_cli_input_gets("\r\nPlease input send data(<=512): ");
    len = nwy_test_cli_input_len_gets();
    if (len > NWY_UART_RECV_SINGLE_MAX)
    {
        nwy_test_cli_echo("\r\nNo more than 512 bytes at a time to send ");
        return;
    }
    send_len = nwy_socket_send(s_nwy_cli_tcp_fd, buff, len, 0);
    if (send_len != len)
    {
        nwy_test_cli_echo("\r\nsend len=%d, return len=%d", len, send_len);
    }
    else
        nwy_test_cli_echo("\r\nsend ok");
}
void nwy_test_cli_tcp_close()
{
    nwy_cli_socket_destory(&s_nwy_cli_tcp_fd);
}
/**************************UDP*********************************/
void nwy_test_cli_udp_setup()
{
    char *tmp = NULL;
    char url_or_ip[128 + 1] = {0};
    int port = 0;
    if (s_nwy_cli_udp_fd > 0)
    {
        nwy_test_cli_echo("\r\nSocket has been setup");
        return;
    }
    tmp = nwy_test_cli_input_gets("\r\nPlease input url: ");
    memcpy(url_or_ip, tmp, strlen(tmp));
    tmp = nwy_test_cli_input_gets("\r\nPlease input port: ");
    if (nwy_port_get(tmp, &port) != NWY_SUCESS)
    {
        nwy_test_cli_echo("\r\nInput port error");
        return;
    }
    nwy_cli_socket_setup_pro(url_or_ip, port, IPPROTO_UDP);
}
void nwy_test_cli_udp_send()
{
    int send_len = 0;
    int len = 0;
    char *buff = NULL;
    buff = nwy_test_cli_input_gets("\r\nPlease input send data(<=512): ");
    len = nwy_test_cli_input_len_gets();
    if (len > NWY_UART_RECV_SINGLE_MAX)
    {
        nwy_test_cli_echo("\r\nNo more than 512 bytes at a time to send ");
        return;
    }
    send_len = nwy_socket_send(s_nwy_cli_udp_fd, buff, len, 0);
    if (send_len != strlen(buff))
    {
        nwy_test_cli_echo("\r\nsend len=%d, return len=%d", len, send_len);
    }
    else
        nwy_test_cli_echo("\r\nsend ok");
}
void nwy_test_cli_udp_close()
{
    nwy_cli_socket_destory(&s_nwy_cli_udp_fd);
}
/**************************FTP*********************************/
static void nwy_cli_app_info_proc(const char *data, int length)
{
    if (s_nwy_ftp_current_thread == NULL)
    {
        nwy_test_cli_echo("\r\n FTP error :nwy_cli_app_info_proc");
    }

    if (length >= s_nwy_ftp_recv->size - s_nwy_ftp_recv->pos)
    {
        length = s_nwy_ftp_recv->size - s_nwy_ftp_recv->pos;
        OSI_LOGI(0, "\r\ninput data len > mem len so cut off %d", length);
    }

    memcpy((s_nwy_ftp_recv->data + s_nwy_ftp_recv->pos), data, length);
    s_nwy_ftp_recv->pos = s_nwy_ftp_recv->pos + length;
    nwy_test_cli_echo("\r\n nwy_app_info.rev_len = %d length = %d\r\n", s_nwy_ftp_recv->pos, length);
    if (s_nwy_ftp_recv->pos == s_nwy_ftp_recv->size)
    {
        nwy_test_cli_send_sig(s_nwy_ftp_current_thread, NWY_EXT_FOTA_DATA_REC_END);
        nwy_cli_update_flag = 0;
    }
    if (s_nwy_ftp_recv->pos > s_nwy_ftp_recv->size)
    {
        nwy_test_cli_send_sig(s_nwy_ftp_current_thread, NWY_EXT_FOTA_DATA_REC_END);
        nwy_cli_update_flag = 0;
        //nwy_test_cli_echo("\r\nAPP info is error,upgrade fail");
    }
}
static void nwy_cli_appimg_info_proc(const char *data, int length)
{
    if (s_nwy_ftp_current_thread == NULL)
    {
        nwy_test_cli_echo("\r\n FTP error:nwy_cli_appimg_info_proc");
    }
    memcpy((s_nwy_ftp_recv->data + s_nwy_ftp_recv->pos), data, length);
    s_nwy_ftp_recv->pos = s_nwy_ftp_recv->pos + length;
    nwy_test_cli_echo("\r\n nwy_app_info.rev_len = %d length = %d\r\n", s_nwy_ftp_recv->pos, length);
    if (s_nwy_ftp_recv->pos == s_nwy_ftp_recv->size)
    {
        nwy_test_cli_send_sig(s_nwy_ftp_current_thread, NWY_EXT_APPIMG_FOTA_DATA_REC_END);
        nwy_cli_update_flag = 0;
    }
    if (s_nwy_ftp_recv->pos > s_nwy_ftp_recv->size)
    {
        nwy_test_cli_send_sig(s_nwy_ftp_current_thread, NWY_EXT_APPIMG_FOTA_DATA_REC_END);
        nwy_cli_update_flag = 0;
        nwy_test_cli_echo("\r\nAPP info is error,upgrade fail");
    }
}
static void nwy_cli_ftp_put_proc(const char *data, int length)
{
    if (nwy_cli_update_flag == 1)
    {
        nwy_cli_app_info_proc(data, length);
    }
    else if (nwy_cli_update_flag == 2)
    {
        nwy_cli_appimg_info_proc(data, length);
    }
}
static int nwy_cli_ftp_write_file(nwy_file_ftp_info_s *pFileFtp, unsigned char *data, unsigned int len)
{
    char file_name[128 + 1] = "/nwy/";
    strcat(file_name, pFileFtp->filename);
    int fs = nwy_sdk_fopen(file_name, NWY_CREAT | NWY_RDWR);
    if (fs < 0)
    {
        OSI_LOGE(0, "nwy file open fail");
        return -1;
    }
    nwy_sdk_fseek(fs, pFileFtp->pos, 0);
    nwy_sdk_fwrite(fs, data, len);
    nwy_sdk_fclose(fs);
    pFileFtp->pos += len;
    OSI_LOGE(0, "nwy file write %d size success.", len);
    return 0;
}
static void nwy_cli_ftp_result_cb(nwy_ftp_result_t *param)
{
    int *size;
    int ret;
    if (NULL == param)
    {
        nwy_test_cli_echo("event is NULL\r\n");
    }
    OSI_LOGI(0, "event is %d", param->event);
    if (NWY_FTP_EVENT_LOGIN == param->event)
    {
        nwy_test_cli_echo("\r\nFtp login success");
    }
    else if (NWY_FTP_EVENT_PASS_ERROR == param->event)
    {
        nwy_test_cli_echo("\r\nFtp passwd error");
    }
    else if (NWY_FTP_EVENT_FILE_NOT_FOUND == param->event)
    {
        nwy_test_cli_echo("\r\nFtp file not found");
    }
    else if (NWY_FTP_EVENT_FILE_SIZE_ERROR == param->event)
    {
        nwy_test_cli_echo("\r\nFtp file size error");
    }
    else if (NWY_FTP_EVENT_SIZE == param->event)
    {
        size = (int *)param->data;
        nwy_test_cli_echo("\r\nFtp size is %d", *size);
    }
    else if (NWY_FTP_EVENT_LOGOUT == param->event)
    {
        nwy_test_cli_echo("\r\nFtp logout");
    }
    else if (NWY_FTP_EVENT_CLOSED == param->event)
    {
        nwy_test_cli_echo("\r\nFtp connection closed");
    }
    else if (NWY_FTP_EVENT_SIZE_ZERO == param->event)
    {
        nwy_test_cli_echo("\r\nFtp size is zero");
    }
    else if (NWY_FTP_EVENT_FILE_DELE_SUCCESS == param->event)
    {
        nwy_test_cli_echo("\r\nFtp file del success");
    }
    else if (NWY_FTP_EVENT_DATA_PUT_FINISHED == param->event)
    {
        nwy_test_cli_echo("\r\nFtp put file success");
    }
    else if (NWY_FTP_EVENT_DNS_ERR == param->event || NWY_FTP_EVENT_OPEN_FAIL == param->event)
    {
        nwy_test_cli_echo("\r\nFtp login fail");
    }
    else if (NWY_FTP_EVENT_DATA_GET == param->event)
    {
        //nwy_test_cli_echo("\r\n recv data");
        if (NULL == param->data)
        {
            nwy_test_cli_echo("\r\nrecved data is NULL");
            return;
        }
        if (1 == s_nwy_cli_fileftp.is_vaild)
        {
            if (param->data_len != 0)
            {
                ret = nwy_cli_ftp_write_file(&s_nwy_cli_fileftp, (unsigned char *)param->data, param->data_len);
                if (0 != ret)
                {
                    nwy_test_cli_echo("\r\nwrite failed");
                }
                else
                {
                    //nwy_test_cli_echo("\r\n write success");
                }
            }
        }
        else
        {
            //nwy_test_cli_echo("\r\ndata_size is %d", param->data_len);
            //nwy_test_cli_echo("\r\nparam->data is %s", param->data);
        }
    }
    else if (NWY_FTP_EVENT_DATA_CLOSED == param->event)
    {
        s_nwy_cli_fileftp.is_vaild = 0;
        nwy_test_cli_echo("\r\nfile %s download success.", s_nwy_cli_fileftp.filename);
    }
    else if(NWY_FTP_EVENT_DATA_OPEN_FAIL == param->event)
    {
        nwy_test_cli_echo("\r\nFtp data open fail");
    }
    else if(NWY_FTP_EVENT_DATA_SETUP_ERR == param->event)
    {
        nwy_test_cli_echo("\r\nFtp data setup error");
    }
    else
    {
        //nwy_test_cli_echo("\r\n data_size is %d", param->data_len);
        //nwy_test_cli_echo("\r\n param->data is %s", param->data);
    }
    return;
}
void nwy_test_cli_ftp_login()
{
    int result = NWY_GEN_E_UNKNOWN;
    char *sptr = NULL;
    nwy_ftp_login_t ftp_param;
    memset(&ftp_param, 0x00, sizeof(ftp_param));
    sptr = nwy_test_cli_input_gets("\r\n input channel(1-7):");
    if (!nwy_cli_check_str_isdigit(sptr))
    {
        nwy_test_cli_echo("\r\n input error");
        return;
    }
    ftp_param.channel = atoi(sptr);
    if (ftp_param.channel < 1 || ftp_param.channel > 7)
    {
        nwy_test_cli_echo("\r\n input error");
        return;
    }

    sptr = nwy_test_cli_input_gets("\r\n input ip:");
    strcpy((char *)ftp_param.host, sptr);
    sptr = nwy_test_cli_input_gets("\r\n input port:");
    if (!nwy_cli_check_str_isdigit(sptr))
    {
        nwy_test_cli_echo("\r\n input error");
        return;
    }
    ftp_param.port = atoi(sptr);
    sptr = nwy_test_cli_input_gets("\r\n input FTPtype (active mode = 1 passive mode = 0):");
    if (!nwy_cli_check_str_isdigit(sptr))
    {
        nwy_test_cli_echo("\r\n input error");
        return;
    }
    ftp_param.mode = atoi(sptr);
    if (ftp_param.mode < 0 || ftp_param.mode > 1)
    {
        nwy_test_cli_echo("\r\n input error");
        return;
    }
    sptr = nwy_test_cli_input_gets("\r\n input username:");
    strcpy((char *)ftp_param.username, sptr);
    sptr = nwy_test_cli_input_gets("\r\n input passwd:");
    strcpy((char *)ftp_param.passwd, sptr);
    ftp_param.timeout = 180;
    result = nwy_ftp_login(&ftp_param, nwy_cli_ftp_result_cb);
    if (0 == result)
    {
        nwy_test_cli_echo("\r\n nwy_ftp_login success");
    }
    else
    {
        nwy_test_cli_echo("\r\n nwy_ftp_login fail");
    }
}
void nwy_test_cli_ftp_get()
{
    int result = NWY_GEN_E_UNKNOWN;
    int type = 0;
    char buff[128] = {0};
    char *sptr = NULL;
    sptr = nwy_test_cli_input_gets("\r\n input type:(1 ASCII 2 Binary)");
    if (!nwy_cli_check_str_isdigit(sptr))
    {
        nwy_test_cli_echo("\r\n input error");
        return;
    }
    type = atoi(sptr);
    if (type < 1 || type > 2)
    {
        nwy_test_cli_echo("\r\n input error");
        return;
    }
    sptr = nwy_test_cli_input_gets("\r\n input filename:");
    strcpy(buff, sptr);
    s_nwy_cli_fileftp.is_vaild = 1;
    strcpy(s_nwy_cli_fileftp.filename, sptr);
    result = nwy_ftp_get(buff, type, 0, 0);
    if (0 == result)
    {
        nwy_test_cli_echo("\r\n nwy_ftp_get success");
    }
    else
    {
        nwy_test_cli_echo("\r\n nwy_ftp_get fail");
    }
}
void nwy_test_cli_ftp_put()
{
    int type = 0;
    int mode = 0;
    int len = 0;
    int result = NWY_GEN_E_UNKNOWN;
    char filename[128] = {0};
    char *sptr = NULL;
    nwy_osiEvent_t event;
    memset(&event, 0, sizeof(event));
    sptr = nwy_test_cli_input_gets("\r\n input filename:");
    strcpy(filename, sptr);
    sptr = nwy_test_cli_input_gets("\r\n input type:(1 ASCII 2 Binary)");
    if (!nwy_cli_check_str_isdigit(sptr))
    {
        nwy_test_cli_echo("\r\n input error");
        return;
    }
    type = atoi(sptr);
    if (type < 1 || type > 2)
    {
        nwy_test_cli_echo("\r\n input error");
        return;
    }
    sptr = nwy_test_cli_input_gets("\r\n input mode:(1 STOR 2 APPE 3 DELE)");
    if (!nwy_cli_check_str_isdigit(sptr))
    {
        nwy_test_cli_echo("\r\n input error");
        return;
    }
    mode = atoi(sptr);
    if (mode < 1 || mode > 3)
    {
        nwy_test_cli_echo("\r\n input error");
        return;
    }
    sptr = nwy_test_cli_input_gets("\r\n input put data len:");
    if (!nwy_cli_check_str_isdigit(sptr))
    {
        nwy_test_cli_echo("\r\n input error");
        return;
    }
    len = atoi(sptr);
    ;
    s_nwy_ftp_recv = malloc(sizeof(dataRecvContext_t) + len);
    if (s_nwy_ftp_recv == NULL)
    {
        nwy_test_cli_echo("\r\n malloc buffer fail");
        return;
    }
    memset(s_nwy_ftp_recv, 0, sizeof(dataRecvContext_t) + len);
    s_nwy_ftp_recv->size = len;
    s_nwy_ftp_recv->pos = 0;
    s_nwy_ftp_current_thread = nwy_get_current_thread();
    nwy_test_cli_sio_enter_trans_mode(nwy_cli_ftp_put_proc);
    nwy_cli_update_flag = 1;
    nwy_test_cli_echo("\r\nplease input data");
    nwy_wait_thead_event(s_nwy_ftp_current_thread, &event, 0);
    if (event.id == NWY_EXT_FOTA_DATA_REC_END)
    {
        result = nwy_ftp_put(filename, type, mode, s_nwy_ftp_recv->data, s_nwy_ftp_recv->size);
        if (0 == result)
        {
            nwy_test_cli_echo("\r\n nwy_ftp_put success");
        }
        else
        {
            nwy_test_cli_echo("\r\n nwy_ftp_put fail");
        }
        free(s_nwy_ftp_recv);
        s_nwy_ftp_recv = NULL;
    }
}
void nwy_test_cli_ftp_fsize()
{
    char *sptr = NULL;
    char buff[128] = {0};
    int result = NWY_GEN_E_UNKNOWN;
    sptr = nwy_test_cli_input_gets("\r\n input filename:");
    strcpy(buff, sptr);
    result = nwy_ftp_filesize(buff, 30);
    if (0 == result)
    {
        nwy_test_cli_echo("\r\n nwy_ftp_filesize success");
    }
    else
    {
        nwy_test_cli_echo("\r\n nwy_ftp_filesize fail");
    }
}
void nwy_test_cli_ftp_list()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}
void nwy_test_cli_ftp_delet()
{
    int ret = 0;
    char *sptr = NULL;
    char file_name[128 + 1] = "/nwy/";
    nwy_test_cli_echo("\r\n delete get file");
    sptr = nwy_test_cli_input_gets("\r\n input filename");
    strcat(file_name, sptr);
    int testfd = nwy_sdk_fopen(file_name, NWY_RDONLY);
    if (testfd < 0)
    {
        nwy_test_cli_echo("\r\nfile %s don't exist\r\n", file_name);
        return;
    }
    nwy_sdk_fclose(testfd);
    ret = nwy_sdk_file_unlink(file_name);
    if (ret == 0)
    {
        nwy_test_cli_echo("\r\n delete %s file success", file_name);
    }
    else
    {
        nwy_test_cli_echo("\r\n delete %s file fail", file_name);
    }
}
void nwy_test_cli_ftp_logout()
{
    int result = NWY_GEN_E_UNKNOWN;
    nwy_test_cli_echo("\r\n ftp will logout");
    result = nwy_ftp_logout(30);
    if (0 == result)
    {
        nwy_test_cli_echo("\r\n nwy_ftp_logout success");
    }
    else
    {
        nwy_test_cli_echo("\r\n nwy_ftp_logout fail");
    }
}
/**************************HTTP*********************************/
static void nwy_cli_https_choice()
{
    char *sptr = NULL;
    sptr = nwy_test_cli_input_gets("\r\n input 0 for http, 1 for https:");
    s_nwy_http_choice = atoi(sptr) ? true : false;
}

static void nwy_cli_http_result_cb(nwy_http_result_t *param)
{
    if (NULL == param)
    {
        nwy_test_cli_echo("event is NULL\r\n");
    }
    OSI_LOGI(0, "event is %d", param->event);
    if (NWY_HTTP_DNS_ERR == param->event)
    {
        nwy_test_cli_echo("\r\nHTTP dns err");
    }
    else if (NWY_HTTP_OPEN_FAIL == param->event)
    {
        nwy_test_cli_echo("\r\nHTTP open fail");
    }
    else if (NWY_HTTP_OPENED == param->event)
    {
        nwy_test_cli_echo("\r\nHTTP setup success");
    }
    else if (NWY_HTTP_CLOSED_PASV == param->event || NWY_HTTP_CLOSED == param->event)
    {
        nwy_test_cli_echo("\r\nHTTP closed");
    }
    else if (NWY_HTTP_DATA_RECVED == param->event)
    {
        nwy_test_cli_echo("\r\nHTTP recv data len %d.\r\n", param->data_len);
        char *buff = (char *)malloc(NWY_EXT_SIO_RX_MAX);
        if (NULL == buff)
        {
            nwy_test_cli_echo("%s\r\n", param->data);
            return;
        }
        int offset = 0;
        int tmeplen = 0;
        while (param->data_len)
        {
            memset(buff, 0x00, NWY_EXT_SIO_RX_MAX);
            tmeplen = param->data_len > NWY_EXT_SIO_RX_MAX ? NWY_EXT_SIO_RX_MAX : param->data_len;
            memcpy(buff, param->data + offset, tmeplen);
            nwy_test_cli_echo("%s", buff);
            offset += tmeplen;
            param->data_len -= tmeplen;
            OSI_LOGI(0, "param->data_len %d", param->data_len);
        }
        free(buff);
        nwy_test_cli_echo("\r\n");
    }
    else if (NWY_HTTP_DATA_SEND_ERR == param->event)
    {
        nwy_test_cli_echo("\r\nHTTP request send error");
    }
    else if (NWY_HTTP_DATA_SEND_FINISHED == param->event)
    {
        nwy_test_cli_echo("\r\nHTTP request send OK");
    }
    else
    {
        nwy_test_cli_echo("\r\nHTTP unkown error");
    }
    return;
}

static void nwy_cli_https_result_cb(nwy_http_result_t *param)
{
    if (NULL == param)
    {
        nwy_test_cli_echo("event is NULL\r\n");
    }
    OSI_LOGI(0, "event is %d", param->event);
    if (NWY_HTTP_DNS_ERR == param->event)
    {
        nwy_test_cli_echo("\r\nHTTPS dns err");
    }
    else if (NWY_HTTP_OPEN_FAIL == param->event)
    {
        nwy_test_cli_echo("\r\nHTTPS open fail");
    }
    else if (NWY_HTTPS_SSL_CONNECTED == param->event)
    {
        nwy_test_cli_echo("\r\nHTTPS setup success");
    }
    else if (NWY_HTTP_CLOSED_PASV == param->event || NWY_HTTP_CLOSED == param->event)
    {
        nwy_test_cli_echo("\r\nHTTPS closed");
    }
    else if (NWY_HTTP_DATA_RECVED == param->event)
    {
        nwy_test_cli_echo("\r\nHTTPS recv data len %d.\r\n", param->data_len);
        char *buff = (char *)malloc(NWY_EXT_SIO_RX_MAX);
        if (NULL == buff)
        {
            nwy_test_cli_echo("%s\r\n", param->data);
            return;
        }
        int offset = 0;
        int tmeplen = 0;
        while (param->data_len)
        {
            memset(buff, 0x00, NWY_EXT_SIO_RX_MAX);
            tmeplen = param->data_len > NWY_EXT_SIO_RX_MAX ? NWY_EXT_SIO_RX_MAX : param->data_len;
            memcpy(buff, param->data + offset, tmeplen);
            nwy_test_cli_echo("%s", buff);
            offset += tmeplen;
            param->data_len -= tmeplen;
            OSI_LOGI(0, "param->data_len %d", param->data_len);
        }
        free(buff);
        nwy_test_cli_echo("\r\n");
    }
    else if (NWY_HTTP_DATA_SEND_ERR == param->event)
    {
        nwy_test_cli_echo("\r\nHTTPS request send error");
    }
    else if (NWY_HTTP_DATA_SEND_FINISHED == param->event)
    {
        nwy_test_cli_echo("\r\nHTTP request send ok");
    }
    else if (NWY_HTTPS_SSL_INIT_ERROR == param->event)
    {
        nwy_test_cli_echo("\r\nHTTPS SSL init fail");
    }
    else if (NWY_HTTPS_SSL_HANDSHAKE_ERROR == param->event)
    {
        nwy_test_cli_echo("\r\nHTTPS SSL handshare fail");
    }
    else if (NWY_HTTPS_SSL_AUTH_FAIL == param->event)
    {
        nwy_test_cli_echo("\r\nHTTPS SSL Authentication fail");
    }
    else
    {
        nwy_test_cli_echo("\r\nHTTPS unkown error");
    }
    return;
}

void nwy_test_cli_http_setup()
{
    int result = NWY_GEN_E_UNKNOWN;
    int port = 0;
    uint8_t channel = 0;
    char url[NWY_HTTP_URL_MAX_LEN] = {0};
    char *sptr = NULL;
    nwy_app_ssl_conf_t ssl_cfg;
    sptr = nwy_test_cli_input_gets("\r\n input channel:(1-7)");
    channel = atoi(sptr);
    if ((channel <= 0) || (channel > 7))
    {
        nwy_test_cli_echo("\r\nInvaild channel: %d ", channel);
        return;
    }
    sptr = nwy_test_cli_input_gets("\r\n input url:");
    strcpy(url, sptr);
    sptr = nwy_test_cli_input_gets("\r\n input port:");
    if (!nwy_cli_check_str_isdigit(sptr))
    {
        nwy_test_cli_echo("input error");
        return;
    }
    port = atoi(sptr);
    nwy_cli_https_choice();
    if (s_nwy_http_choice)
    {
        memset(&ssl_cfg, 0x00, sizeof(ssl_cfg));
        sptr = nwy_test_cli_input_gets("\r\n input ssl version: 0:SSL3.0 1:TLS1.0 2:TLS1.1 3:TLS1.2");
        ssl_cfg.ssl_version = atoi(sptr);
        sptr = nwy_test_cli_input_gets("\r\n input authmode: 0:No authentication 1:Manage server authentication 2:Manage server and client authentication");
        ssl_cfg.authmode = atoi(sptr);
        if (0 != ssl_cfg.authmode)
        {
            sptr = nwy_test_cli_input_gets("\r\n input cacert name:");
            strcpy(ssl_cfg.cacert.cert_name, sptr);
            sptr = nwy_test_cli_input_gets("\r\n input clientcert name:");
            strcpy(ssl_cfg.clientcert.cert_name, sptr);
            sptr = nwy_test_cli_input_gets("\r\n input clientkey name:");
            strcpy(ssl_cfg.clientkey.cert_name, sptr);
        }
        result = nwy_https_setup(channel, url, port, nwy_cli_https_result_cb, &ssl_cfg);
    }
    else
    {
        result = nwy_http_setup(channel, url, port, nwy_cli_http_result_cb);
    }
    if (0 == result)
    {
        nwy_test_cli_echo("\r\n %s success", s_nwy_http_choice ? "nwy_https_setup" : "nwy_http_setup");
    }
    else
    {
        nwy_test_cli_echo("\r\n %s fail", s_nwy_http_choice ? "nwy_https_setup" : "nwy_http_setup");
    }
}
void nwy_test_cli_http_get()
{
    char *sptr = NULL;
    int keepalive = 0;
    int offset = 0;
    int size = 0;
    int result = NWY_GEN_E_UNKNOWN;
    sptr = nwy_test_cli_input_gets("\r\n input whether keepalive:(0 1)");
    keepalive = atoi(sptr);
    sptr = nwy_test_cli_input_gets("\r\n input offset:");
    offset = atoi(sptr);
    sptr = nwy_test_cli_input_gets("\r\n input size:");
    size = atoi(sptr);
    nwy_cli_https_choice();
    result = nwy_http_get(keepalive, offset, size, s_nwy_http_choice);
    if (0 == result)
    {
        nwy_test_cli_echo("\r\n nwy_http_get success");
    }
    else
    {
        nwy_test_cli_echo("\r\n nwy_http_get fail");
    }
}
void nwy_test_cli_http_head()
{
    char *sptr = NULL;
    int keepalive = 0;
    int result = NWY_GEN_E_UNKNOWN;
    sptr = nwy_test_cli_input_gets("\r\n input whether keepalive:(0 1)");
    keepalive = atoi(sptr);
    nwy_cli_https_choice();
    result = nwy_http_head(keepalive, s_nwy_http_choice);
    if (0 == result)
    {
        nwy_test_cli_echo("\r\n nwy_http_head success");
    }
    else
    {
        nwy_test_cli_echo("\r\n nwy_http_head fail");
    }
}
void nwy_test_cli_http_post()
{
    char *sptr = NULL;
    int keepalive = 0;
    int type = 0;
    int result = NWY_GEN_E_UNKNOWN;
    sptr = nwy_test_cli_input_gets("\r\n input whether keepalive:(0 1)");
    keepalive = atoi(sptr);
    sptr = nwy_test_cli_input_gets("\r\n input type: 0:x-www-form-urlencoded 1:text 2:json 3:xml 4:html 99:User-defined mode");
    type = atoi(sptr);
    sptr = nwy_test_cli_input_gets("\r\n input data:");
    nwy_cli_https_choice();
    result = nwy_http_post(keepalive, type, sptr, strlen(sptr), s_nwy_http_choice);
    if (0 == result)
    {
        nwy_test_cli_echo("\r\n nwy_http_post success");
    }
    else
    {
        nwy_test_cli_echo("\r\n nwy_http_post fail");
    }
}
void nwy_test_cli_http_close()
{
    int result = NWY_GEN_E_UNKNOWN;
    nwy_cli_https_choice();
    result = nwy_http_close(s_nwy_http_choice);
    if (0 == result)
    {
        nwy_test_cli_echo("\r\n %s success", s_nwy_http_choice ? "nwy_https_close" : "nwy_http_close");
    }
    else
    {
        nwy_test_cli_echo("\r\n %s fail", s_nwy_http_choice ? "nwy_https_close" : "nwy_http_close");
    }
}
void nwy_test_cli_https_add_cert()
{
    char *sptr = NULL;
    char filename[128] = {0};
    nwy_osiEvent_t event;
    sptr = nwy_test_cli_input_gets("\r\n begin input cacert name:");
    strcpy(filename, sptr);
    sptr = nwy_test_cli_input_gets("\r\n input cacert length:");
    uint32_t length = atoi(sptr);
    if (length <= 0)
    {
        nwy_test_cli_echo("\r\n invalid cacert size:%d", length);
        return;
    }
    s_nwy_ftp_recv = malloc(sizeof(dataRecvContext_t) + length + CERT_EFS_CHECKSUM_STR_SIZE + 1);
    if (s_nwy_ftp_recv == NULL)
    {
        nwy_test_cli_echo("\r\n malloc buffer fail");
        return;
    }
    memset(s_nwy_ftp_recv, 0, sizeof(dataRecvContext_t) + length + CERT_EFS_CHECKSUM_STR_SIZE + 1);
    s_nwy_ftp_recv->size = length;
    s_nwy_ftp_recv->pos = 0;
    nwy_test_cli_sio_enter_trans_mode(nwy_cli_ftp_put_proc);
    nwy_cli_update_flag = 1;
    nwy_test_cli_echo("\r\nplease input cert");
    s_nwy_ftp_current_thread = nwy_get_current_thread();
    memset(&event, 0, sizeof(event));
    nwy_wait_thead_event(s_nwy_ftp_current_thread, &event, 0);
    if (event.id == NWY_EXT_FOTA_DATA_REC_END)
    {
        //nwy_test_cli_echo("\r\n recv %d bytes data success", g_recv->size);
        unsigned wsize = nwy_cert_add(filename, s_nwy_ftp_recv->data, s_nwy_ftp_recv->size);
        if (wsize != s_nwy_ftp_recv->size)
        {
            nwy_test_cli_echo("\r\nnwy_cert_add fail");
        }
        else
        {
            nwy_test_cli_echo("\r\nnwy_cert_add success");
        }
        free(s_nwy_ftp_recv);
        s_nwy_ftp_recv = NULL;
    }
    nwy_test_cli_echo("\r\nadd end");
}
void nwy_test_cli_https_check_cert()
{
    char filename[128] = {0};
    char *sptr = NULL;
    int result = NWY_GEN_E_UNKNOWN;
    sptr = nwy_test_cli_input_gets("\r\n input cacert name:");
    memset(filename, 0x00, sizeof(filename));
    strcpy(filename, sptr);
    result = nwy_cert_check(filename);
    if (0 == result)
    {
        nwy_test_cli_echo("\r\n nwy_cert_check success");
    }
    else
    {
        nwy_test_cli_echo("\r\n nwy_cert_check fail");
    }
}
void nwy_test_cli_https_delet_cert()
{
    char filename[128] = {0};
    char *sptr = NULL;
    int result = NWY_GEN_E_UNKNOWN;
    sptr = nwy_test_cli_input_gets("\r\n input cacert name:");
    memset(filename, 0x00, sizeof(filename));
    strcpy(filename, sptr);
    result = nwy_cert_del(filename);
    if (0 == result)
    {
        nwy_test_cli_echo("\r\n nwy_cert_del success");
    }
    else
    {
        nwy_test_cli_echo("\r\n nwy_cert_del fail");
    }
}
void nwy_test_cli_http_compute_md5()
{
    char filename[128] = {0};
    char *sptr = NULL;
    int result = NWY_GEN_E_UNKNOWN;
    char compute_mtd5[64] = {0};
    sptr = nwy_test_cli_input_gets("\r\n input file name:");
    memset(filename, 0x00, sizeof(filename));
    strcpy(filename, sptr);
    result = compute_file_md5(filename, compute_mtd5);
    if (0 == result)
        nwy_test_cli_echo("\r\n compute md5 success:%s\r\n", compute_mtd5);
    else
        nwy_test_cli_echo("\r\n compute md5 fail");
}
