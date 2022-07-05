#ifndef __NWY_SOCKET_H__
#define __NWY_SOCKET_H__
#ifdef __cplusplus
extern "C" {
#endif
#ifndef LWIP_IPV6
#define LWIP_IPV6       1
#endif

#define AF_UNSPEC       0
#define AF_INET         2
#if LWIP_IPV6
#define AF_INET6        10
#else /* LWIP_IPV6 */
#define AF_INET6        AF_UNSPEC
#endif /* LWIP_IPV6 */
#define PF_INET         AF_INET
#define PF_INET6        AF_INET6
#define PF_UNSPEC       AF_UNSPEC

/* Socket protocol types (TCP/UDP/RAW) */
#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3

#define IPPROTO_IP      0
#define IPPROTO_ICMP    1
#define IPPROTO_TCP     6
#define IPPROTO_UDP     17
#if LWIP_IPV6
#define IPPROTO_IPV6    41
#define IPPROTO_ICMPV6  58
#endif /* LWIP_IPV6 */
#define IPPROTO_UDPLITE 136
#define IPPROTO_RAW     255

#define TCP_NODELAY    0x01    /* don't delay send to coalesce packets */
#define TCP_KEEPALIVE  0x02    /* send KEEPALIVE probes when idle for pcb->keep_idle milliseconds */
#define TCP_KEEPIDLE   0x03    /* set pcb->keep_idle  - Same as TCP_KEEPALIVE, but use seconds for get/setsockopt */
#define TCP_KEEPINTVL  0x04    /* set pcb->keep_intvl - Use seconds for get/setsockopt */
#define TCP_KEEPCNT    0x05    /* set pcb->keep_cnt   - Use number of probes sent for get/setsockopt */


/*
 * Option flags per-socket. These must match the SOF_ flags in ip.h (checked in init.c)
 */
#define SO_REUSEADDR   0x0004 /* Allow local address reuse */
#define SO_KEEPALIVE   0x0008 /* keep connections alive */
#define SO_BROADCAST   0x0020 /* permit to send and to receive broadcast messages (see IP_SOF_BROADCAST option) */


/*
 * Additional options, not kept in so_options.
 */
#define SO_DEBUG        0x0001 /* Unimplemented: turn on debugging info recording */
#define SO_ACCEPTCONN   0x0002 /* socket has had listen() */
#define SO_DONTROUTE    0x0010 /* Unimplemented: just use interface addresses */
#define SO_USELOOPBACK  0x0040 /* Unimplemented: bypass hardware when possible */
#define SO_LINGER       0x0080 /* linger on close if data present */
#define SO_DONTLINGER   ((int)(~SO_LINGER))
#define SO_OOBINLINE    0x0100 /* Unimplemented: leave received OOB data in line */
#define SO_REUSEPORT    0x0200 /* Unimplemented: allow local address & port reuse */
#define SO_SNDBUF       0x1001 /* Unimplemented: send buffer size */
#define SO_RCVBUF       0x1002 /* receive buffer size */
#define SO_SNDLOWAT     0x1003 /* Unimplemented: send low-water mark */
#define SO_RCVLOWAT     0x1004 /* Unimplemented: receive low-water mark */
#define SO_SNDTIMEO     0x1005 /* send timeout */
#define SO_RCVTIMEO     0x1006 /* receive timeout */
#define SO_ERROR        0x1007 /* get error status and clear */
#define SO_TYPE         0x1008 /* get socket type */
#define SO_CONTIMEO     0x1009 /* Unimplemented: connect timeout */
#define SO_NO_CHECK     0x100a /* don't create UDP checksum */
#define SO_BINDTODEVICE 0x100b /* bind to device */

/* Flags we can use with send and recv. */
#define MSG_PEEK       0x01    /* Peeks at an incoming message */
#define MSG_WAITALL    0x02    /* Unimplemented: Requests that the function block until the full amount of data requested can be returned */
#define MSG_OOB        0x04    /* Unimplemented: Requests out-of-band data. The significance and semantics of out-of-band data are protocol-specific */
#define MSG_DONTWAIT   0x08    /* Nonblocking i/o for this operation only */
#define MSG_MORE       0x10    /* Sender will send more */
#define MSG_NOSIGNAL   0x20    /* Uninmplemented: Requests not to send the SIGPIPE signal if an attempt to send is made on a stream-oriented socket that is no longer connected. */
/* commands for fnctl */
#ifndef F_GETFL
#define F_GETFL 3
#endif
#ifndef F_SETFL
#define F_SETFL 4
#endif

/* File status flags and file access modes for fnctl,
   these are bits in an int. */
#ifndef O_NONBLOCK
#define O_NONBLOCK  1 /* nonblocking I/O */
#endif
#ifndef O_NDELAY
#define O_NDELAY    1 /* same as O_NONBLOCK, for compatibility */
#endif
#ifndef O_RDONLY
#define O_RDONLY    2
#endif
#ifndef O_WRONLY
#define O_WRONLY    4
#endif
#ifndef O_RDWR
#define O_RDWR      (O_RDONLY|O_WRONLY)
#endif

#ifndef SHUT_RD
  #define SHUT_RD   0
  #define SHUT_WR   1
  #define SHUT_RDWR 2
#endif
#define IP4ADDR_STRLEN_MAX  16
#define IP6ADDR_STRLEN_MAX    46

#ifndef INADDR_NONE
/** 255.255.255.255 */
#define INADDR_NONE         ((u32_t)0xffffffffUL)
#endif

#ifndef INADDR_LOOPBACK
/** 127.0.0.1 */
#define INADDR_LOOPBACK     ((u32_t)0x7f000001UL)
#endif

#ifndef INADDR_ANY
/** 0.0.0.0 */
#define INADDR_ANY          ((u32_t)0x00000000UL)
#endif

#ifndef INADDR_BROADCAST
/** 255.255.255.255 */
#define INADDR_BROADCAST    ((u32_t)0xffffffffUL)
#endif

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN     IP4ADDR_STRLEN_MAX
#endif
#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN    IP6ADDR_STRLEN_MAX
#endif
/*
 * Structure used for manipulating linger option.
 */
struct linger {
  int l_onoff;                /* option on/off */
  int l_linger;               /* linger time in seconds */
};

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define  SOL_SOCKET  0xfff    /* options for socket level */



#ifndef SHUT_RD
  #define SHUT_RD   0
  #define SHUT_WR   1
  #define SHUT_RDWR 2
#endif


typedef int socklen_t;
typedef uint32_t u32_t;
typedef uint8_t u8_t;
typedef uint16_t u16_t;

/* lawrence start */
#define AI_PASSIVE      0x01
struct addrinfo {
    int               ai_flags;      /* Input flags. */
    int               ai_family;     /* Address family of socket. */
    int               ai_socktype;   /* Socket type. */
    int               ai_protocol;   /* Protocol of socket. */
    socklen_t         ai_addrlen;    /* Length of socket address. */
    struct sockaddr  *ai_addr;       /* Socket address of socket. */
    char             *ai_canonname;  /* Canonical name of service location. */
    struct addrinfo  *ai_next;       /* Pointer to next in list. */
};

struct hostent {
    char  *h_name;      /* Official name of the host. */
    char **h_aliases;   /* A pointer to an array of pointers to alternative host names,
                           terminated by a null pointer. */
    int    h_addrtype;  /* Address type. */
    int    h_length;    /* The length, in bytes, of the address. */
    char **h_addr_list; /* A pointer to an array of pointers to network addresses (in
                           network byte order) for the host, terminated by a null pointer. */
#define h_addr h_addr_list[0] /* for backward compatibility */
};
/* lawrence end */

typedef unsigned char uint8_t;
#if !defined(sa_family_t) && !defined(SA_FAMILY_T_DEFINED)
typedef uint8_t sa_family_t;
#endif

#if !defined(in_port_t) && !defined(IN_PORT_T_DEFINED)
typedef unsigned short   in_port_t;
#endif

/* If your port already typedef's in_addr_t, define IN_ADDR_T_DEFINED
   to prevent this code from redefining it. */
#if !defined(in_addr_t) && !defined(IN_ADDR_T_DEFINED)
typedef u32_t in_addr_t;
#endif

struct in_addr {
  in_addr_t s_addr;
};

struct in6_addr {
  union {
    u32_t u32_addr[4];
    u8_t  u8_addr[16];
  } un;
#define s6_addr  un.u8_addr
};

enum sal_ip_addr_type {
    /** IPv4 */
    IPADDR_TYPE_V4 =   0U,
    /** IPv6 */
    IPADDR_TYPE_V6 =   6U,
    /** IPv4+IPv6 ("dual-stack") */
    IPADDR_TYPE_ANY = 46U
};

typedef struct ip4_addr {
    u32_t addr;
} ip4_addr_t;

typedef struct ip6_addr {
    u32_t addr[4];

#if 1 //LWIP_IPV6_SCOPES
    u8_t zone;
#endif /* LWIP_IPV6_SCOPES */
} ip6_addr_t;

typedef struct _ip_addr {
    union {
        ip6_addr_t ip6;
        ip4_addr_t ip4;
    } u_addr;
    /** @ref sal_ip_addr_type */
    u8_t type;
} ip_addr_t;



//#if LWIP_IPV4
/* members are in network byte order */
struct sockaddr_in {
  uint8_t            sin_len;
  sa_family_t     sin_family;
  in_port_t       sin_port;
  struct in_addr  sin_addr;
#define SIN_ZERO_LEN 8
  char            sin_zero[SIN_ZERO_LEN];
};
//#endif /* LWIP_IPV4 */

//#if LWIP_IPV6
struct sockaddr_in6 {
  uint8_t            sin6_len;      /* length of this structure    */
  sa_family_t     sin6_family;   /* AF_INET6                    */
  in_port_t       sin6_port;     /* Transport layer port #      */
  uint32_t           sin6_flowinfo; /* IPv6 flow information       */
  struct in6_addr sin6_addr;     /* IPv6 address                */
  uint32_t           sin6_scope_id; /* Set of interfaces for scope */
};
//#endif /* LWIP_IPV6 */

struct sockaddr {
  uint8_t        sa_len;
  sa_family_t sa_family;
  uint8_t        sa_data[14];
};

struct sockaddr_storage {
  uint8_t        s2_len;
  sa_family_t ss_family;
  char        s2_data1[2];
  uint8_t       s2_data2[3];
//#if LWIP_IPV6
  uint32_t       s2_data3[3];
//#endif /* LWIP_IPV6 */
};



#ifndef lwip_htons
u16_t lwip_htons(u16_t x);
#endif
#define lwip_ntohs(x) lwip_htons(x)

#ifndef lwip_htonl
u32_t lwip_htonl(u32_t x);
#endif
#define lwip_ntohl(x) lwip_htonl(x)

/* These macros should be calculated by the preprocessor and are used
   with compile-time constants only (so that there is no little-endian
   overhead at runtime). */
#define PP_HTONS(x) ((((x) & 0x00ffUL) << 8) | (((x) & 0xff00UL) >> 8))
#define PP_NTOHS(x) PP_HTONS(x)
#define PP_HTONL(x) ((((x) & 0x000000ffUL) << 24) | \
                     (((x) & 0x0000ff00UL) <<  8) | \
                     (((x) & 0x00ff0000UL) >>  8) | \
                     (((x) & 0xff000000UL) >> 24))
#define PP_NTOHL(x) PP_HTONL(x)


/* Provide usual function names as macros for users, but this can be turned off */
#define htons(x) lwip_htons(x)
#define ntohs(x) lwip_ntohs(x)
#define htonl(x) lwip_htonl(x)
#define ntohl(x) lwip_ntohl(x)

u16_t lwip_htons(u16_t n);
//{
//  return (u16_t)PP_HTONS(n);
//}

u32_t lwip_htonl(u32_t n);
//{
//  return (u32_t)PP_HTONL(n);
//}




#define inet_addr_from_ip4addr(target_inaddr, source_ipaddr) ((target_inaddr)->s_addr = ip4_addr_get_u32(source_ipaddr))
#define inet_addr_to_ip4addr(target_ipaddr, source_inaddr)   (ip4_addr_set_u32(target_ipaddr, (source_inaddr)->s_addr))

/* directly map this to the lwip internal functions */
#define inet_addr(cp)                   ipaddr_addr(cp)
#define inet_aton(cp, addr)             ip4addr_aton(cp, (ip4_addr_t*)addr)
#define inet_ntoa(addr)                 nwy_ip4addr_ntoa((const ip4_addr_t*)&(addr))
#define inet_ntoa_r(addr, buf, buflen)  ip4addr_ntoa_r((const ip4_addr_t*)&(addr), buf, buflen)

//#endif /* LWIP_IPV4 */

//#if LWIP_IPV6
#define inet6_addr_from_ip6addr(target_in6addr, source_ip6addr) {(target_in6addr)->un.u32_addr[0] = (source_ip6addr)->addr[0]; \
                                                                 (target_in6addr)->un.u32_addr[1] = (source_ip6addr)->addr[1]; \
                                                                 (target_in6addr)->un.u32_addr[2] = (source_ip6addr)->addr[2]; \
                                                                 (target_in6addr)->un.u32_addr[3] = (source_ip6addr)->addr[3];}
#define inet6_addr_to_ip6addr(target_ip6addr, source_in6addr)   {(target_ip6addr)->addr[0] = (source_in6addr)->un.u32_addr[0]; \
                                                                 (target_ip6addr)->addr[1] = (source_in6addr)->un.u32_addr[1]; \
                                                                 (target_ip6addr)->addr[2] = (source_in6addr)->un.u32_addr[2]; \
                                                                 (target_ip6addr)->addr[3] = (source_in6addr)->un.u32_addr[3]; \
                                                                 ip6_addr_clear_zone(target_ip6addr);}

/* directly map this to the lwip internal functions */
#define inet6_aton(cp, addr)            ip6addr_aton(cp, (ip6_addr_t*)addr)
#define inet6_ntoa(addr)                ip6addr_ntoa((const ip6_addr_t*)&(addr))
#define inet6_ntoa_r(addr, buf, buflen) ip6addr_ntoa_r((const ip6_addr_t*)&(addr), buf, buflen)

//#endif /* LWIP_IPV6 */
#if LWIP_IPV6
/** @ingroup ip4addr */
#define IP_IS_V4_VAL(ipaddr)          (IP_GET_TYPE(&ipaddr) == IPADDR_TYPE_V4)
/** @ingroup ip6addr */
#define IP_IS_V6_VAL(ipaddr)          (IP_GET_TYPE(&ipaddr) == IPADDR_TYPE_V6)
/** @ingroup ip4addr */
#define IP_IS_V4(ipaddr)              (((ipaddr) == NULL) || IP_IS_V4_VAL(*(ipaddr)))
/** @ingroup ip6addr */
#define IP_IS_V6(ipaddr)              (((ipaddr) != NULL) && IP_IS_V6_VAL(*(ipaddr)))

#define IP_SET_TYPE_VAL(ipaddr, iptype) do { (ipaddr).type = (iptype); }while(0)
#define IP_SET_TYPE(ipaddr, iptype)     do { if((ipaddr) != NULL) { IP_SET_TYPE_VAL(*(ipaddr), iptype); }}while(0)
#define IP_GET_TYPE(ipaddr)           ((ipaddr)->type)
#endif

/** @ingroup ip6addr
 * Convert generic ip address to specific protocol version
 */
#define ip_2_ip6(ipaddr)   (&((ipaddr)->u_addr.ip6))
/** @ingroup ip4addr
 * Convert generic ip address to specific protocol version
 */
#define ip_2_ip4(ipaddr)   (&((ipaddr)->u_addr.ip4))


/** IPv4 only: set the IP address given as an u32_t */
#define ip4_addr_set_u32(dest_ipaddr, src_u32) ((dest_ipaddr)->addr = (src_u32))
/** IPv4 only: get the IP address as an u32_t */
#define ip4_addr_get_u32(src_ipaddr) ((src_ipaddr)->addr)

#ifdef FD_SETSIZE
#undef FD_SETSIZE
#endif

#define FD_SETSIZE 8

#ifdef FD_SET
#undef FD_SET
#undef FD_CLR
#undef FD_ISSET
#undef FD_ZERO
#undef fd_set
#endif

#define LWIP_SOCKET_OFFSET              1
#define MEMP_NUM_NETCONN              8

/* FD_SET used for lwip_select */
#ifndef FD_SET
#undef  FD_SETSIZE
/* Make FD_SETSIZE match NUM_SOCKETS in socket.c */
#define FD_SETSIZE    MEMP_NUM_NETCONN
#define FDSETSAFESET(n, code) do { \
  if (((n) - LWIP_SOCKET_OFFSET < MEMP_NUM_NETCONN) && (((int)(n) - LWIP_SOCKET_OFFSET) >= 0)) { \
  code; }} while(0)
#define FDSETSAFEGET(n, code) (((n) - LWIP_SOCKET_OFFSET < MEMP_NUM_NETCONN) && (((int)(n) - LWIP_SOCKET_OFFSET) >= 0) ?\
  (code) : 0)
#define FD_SET(n, p)  FDSETSAFESET(n, (p)->fd_bits[((n)-LWIP_SOCKET_OFFSET)/8] |=  (1 << (((n)-LWIP_SOCKET_OFFSET) & 7)))
#define FD_CLR(n, p)  FDSETSAFESET(n, (p)->fd_bits[((n)-LWIP_SOCKET_OFFSET)/8] &= ~(1 << (((n)-LWIP_SOCKET_OFFSET) & 7)))
#define FD_ISSET(n,p) FDSETSAFEGET(n, (p)->fd_bits[((n)-LWIP_SOCKET_OFFSET)/8] &   (1 << (((n)-LWIP_SOCKET_OFFSET) & 7)))
#define FD_ZERO(p)    memset((void*)(p), 0, sizeof(*(p)))

typedef struct fd_set
{
  unsigned char fd_bits [(FD_SETSIZE+7)/8];
} fd_set;

#elif LWIP_SOCKET_OFFSET
#error LWIP_SOCKET_OFFSET does not work with external FD_SET!
#elif FD_SETSIZE < MEMP_NUM_NETCONN
#error "external FD_SETSIZE too small for number of sockets"
#endif /* FD_SET */



#define	EPERM 1		/* Not owner */
#define	ENOENT 2	/* No such file or directory */
#define	ESRCH 3		/* No such process */
#define	EINTR 4		/* Interrupted system call */
#define	EIO 5		/* I/O error */
#define	ENXIO 6		/* No such device or address */
#define	E2BIG 7		/* Arg list too long */
#define	ENOEXEC 8	/* Exec format error */
#define	EBADF 9		/* Bad file number */
#define	ECHILD 10	/* No children */
#define	EAGAIN 11	/* No more processes */
#define	ENOMEM 12	/* Not enough space */
#define	EACCES 13	/* Permission denied */
#define	EFAULT 14	/* Bad address */
#define	ENOTBLK 15	/* Block device required */
#define	EBUSY 16	/* Device or resource busy */
#define	EEXIST 17	/* File exists */
#define	EXDEV 18	/* Cross-device link */
#define	ENODEV 19	/* No such device */
#define	ENOTDIR 20	/* Not a directory */
#define	EISDIR 21	/* Is a directory */
#define	EINVAL 22	/* Invalid argument */
#define	ENFILE 23	/* Too many open files in system */
#define	EMFILE 24	/* File descriptor value too large */
#define	ENOTTY 25	/* Not a character device */
#define	ETXTBSY 26	/* Text file busy */
#define	EFBIG 27	/* File too large */
#define	ENOSPC 28	/* No space left on device */
#define	ESPIPE 29	/* Illegal seek */
#define	EROFS 30	/* Read-only file system */
#define	EMLINK 31	/* Too many links */
#define	EPIPE 32	/* Broken pipe */
#define	EDOM 33		/* Mathematics argument out of domain of function */
#define	ERANGE 34	/* Result too large */
#define	ENOMSG 35	/* No message of desired type */
#define	EIDRM 36	/* Identifier removed */
#define	ECHRNG 37	/* Channel number out of range */
#define	EL2NSYNC 38	/* Level 2 not synchronized */
#define	EL3HLT 39	/* Level 3 halted */
#define	EL3RST 40	/* Level 3 reset */
#define	ELNRNG 41	/* Link number out of range */
#define	EUNATCH 42	/* Protocol driver not attached */
#define	ENOCSI 43	/* No CSI structure available */
#define	EL2HLT 44	/* Level 2 halted */
#define	EDEADLK 45	/* Deadlock */
#define	ENOLCK 46	/* No lock */
#define EBADE 50	/* Invalid exchange */
#define EBADR 51	/* Invalid request descriptor */
#define EXFULL 52	/* Exchange full */
#define ENOANO 53	/* No anode */
#define EBADRQC 54	/* Invalid request code */
#define EBADSLT 55	/* Invalid slot */
#define EDEADLOCK 56	/* File locking deadlock error */
#define EBFONT 57	/* Bad font file fmt */
#define ENOSTR 60	/* Not a stream */
#define ENODATA 61	/* No data (for no delay io) */
#define ETIME 62	/* Stream ioctl timeout */
#define ENOSR 63	/* No stream resources */
#define ENONET 64	/* Machine is not on the network */
#define ENOPKG 65	/* Package not installed */
#define EREMOTE 66	/* The object is remote */
#define ENOLINK 67	/* Virtual circuit is gone */
#define EADV 68		/* Advertise error */
#define ESRMNT 69	/* Srmount error */
#define	ECOMM 70	/* Communication error on send */
#define EPROTO 71	/* Protocol error */
#define	EMULTIHOP 74	/* Multihop attempted */
#define	ELBIN 75	/* Inode is remote (not really error) */
#define	EDOTDOT 76	/* Cross mount point (not really error) */
#define EBADMSG 77	/* Bad message */
#define EFTYPE 79	/* Inappropriate file type or format */
#define ENOTUNIQ 80	/* Given log. name not unique */
#define EBADFD 81	/* f.d. invalid for this operation */
#define EREMCHG 82	/* Remote address changed */
#define ELIBACC 83	/* Can't access a needed shared lib */
#define ELIBBAD 84	/* Accessing a corrupted shared lib */
#define ELIBSCN 85	/* .lib section in a.out corrupted */
#define ELIBMAX 86	/* Attempting to link in too many libs */
#define ELIBEXEC 87	/* Attempting to exec a shared library */
#define ENOSYS 88	/* Function not implemented */
#define ENMFILE 89      /* No more files */
#define ENOTEMPTY 90	/* Directory not empty */
#define ENAMETOOLONG 91	/* File or path name too long */
#define ELOOP 92	/* Too many symbolic links */
#define EOPNOTSUPP 95	/* Operation not supported on socket */
#define EPFNOSUPPORT 96 /* Protocol family not supported */
#define ECONNRESET 104  /* Connection reset by peer */
#define ENOBUFS 105	/* No buffer space available */
#define EAFNOSUPPORT 106 /* Address family not supported by protocol family */
#define EPROTOTYPE 107	/* Protocol wrong type for socket */
#define ENOTSOCK 108	/* Socket operation on non-socket */
#define ENOPROTOOPT 109	/* Protocol not available */
#define ESHUTDOWN 110	/* Can't send after socket shutdown */
#define ECONNREFUSED 111	/* Connection refused */
#define EADDRINUSE 112		/* Address already in use */
#define ECONNABORTED 113	/* Software caused connection abort */
#define ENETUNREACH 114		/* Network is unreachable */
#define ENETDOWN 115		/* Network interface is not configured */
#define ETIMEDOUT 116		/* Connection timed out */
#define EHOSTDOWN 117		/* Host is down */
#define EHOSTUNREACH 118	/* Host is unreachable */
#define EINPROGRESS 119		/* Connection already in progress */
#define EALREADY 120		/* Socket already connected */
#define EDESTADDRREQ 121	/* Destination address required */
#define EMSGSIZE 122		/* Message too long */
#define EPROTONOSUPPORT 123	/* Unknown protocol */
#define ESOCKTNOSUPPORT 124	/* Socket type not supported */
#define EADDRNOTAVAIL 125	/* Address not available */
#define ENETRESET 126		/* Connection aborted by network */
#define EISCONN 127		/* Socket is already connected */
#define ENOTCONN 128		/* Socket is not connected */
#define ETOOMANYREFS 129
#define EPROCLIM 130
#define EUSERS 131
#define EDQUOT 132
#define ESTALE 133
#define ENOTSUP 134		/* Not supported */
#define ENOMEDIUM 135   /* No medium (in tape drive) */
#define ENOSHARE 136    /* No such host or network path */
#define ECASECLASH 137  /* Filename exists with different case */
#define EILSEQ 138		/* Illegal byte sequence */
#define EOVERFLOW 139	/* Value too large for defined data type */
#define ECANCELED 140	/* Operation canceled */
#define ENOTRECOVERABLE 141	/* State not recoverable */
#define EOWNERDEAD 142	/* Previous owner died */
#define ESTRPIPE 143	/* Streams pipe error */

typedef enum {
    NWY_UNKNOWN      = -1,
    NWY_CLOSED      = 0,
    NWY_LISTEN      = 1,
    NWY_SYN_SENT    = 2,
    NWY_SYN_RCVD    = 3,
    NWY_ESTABLISHED = 4,
    NWY_FIN_WAIT_1  = 5,
    NWY_FIN_WAIT_2  = 6,
    NWY_CLOSE_WAIT  = 7,
    NWY_CLOSING     = 8,
    NWY_LAST_ACK    = 9,
    NWY_TIME_WAIT   = 10
}nwy_tcp_state;

typedef enum {
    NWY_LWIP_EVENT_ACCEPT,
    NWY_LWIP_EVENT_SENT,
    NWY_LWIP_EVENT_RECV,
    NWY_LWIP_EVENT_CONNECTED,
    NWY_LWIP_EVENT_POLL,
    NWY_LWIP_EVENT_ERR
}nwy_socket_event;

typedef void (*nwy_socket_event_cb)(int socketid, nwy_socket_event event);


/*---------------------------function define---------------------*/
/**
 * @.get socket error no
 *
 * @param 
 * @return
        return error code
 
 */
int nwy_socket_errno(void);

/**
 * @.open and create socket
 *
 * @param 
       domain: ip type,AF_INET for ipv4,AF_INET6 for ipv6
       type: socket type, SOCK_STREAM =1, SOCK_DGRAM=2,SOCK_RAW=3
       protocol: protocol type
 * @return
        success:return socket id
        fail: return -1
 
 */
    int nwy_socket_open(int domain, int type, int protocol);

/**
 * @.socket send info
 *
 * @param 
       s: the socket fd
       data: send info buffer
       size: send info length
       flags:0
 * @return
        success:return send info len
        fail: return -1
 */
int nwy_socket_send(int s, const void *data, size_t size, int flags);

/**
 * @.socket send hex data (data whill be converted to hex and sent)
 *
 * @param
       s: the socket fd
       data: send info buffer
       size: send info length
       flags:0
 * @return
        success:return send info len
        fail: return -1
 */
int nwy_socket_send_hex(int s, const void *data, size_t size, int flags);

/**
 * @.socket receive info
 *
 * @param 
       s: the socket fd
       data: receive buffer
       size: buffer max len
       flags:0
 * @return
        success:return receive info len
        fail: return -1
 */
int nwy_socket_recv(int s, void *mem, size_t len, int flags);

/**
 * @.socket send info to host
 *
 * @param 
       s: the socket fd
       data: send info
       size: send info len
       flags:0
       to:the host addr
       tolen:sockaddr len
 * @return
        success:return receive info len
        fail: return -1
 */
int nwy_socket_sendto(int s, const void *data, size_t size, int flags,
                      const struct sockaddr *to, socklen_t tolen);

/**
 * @.socket receive info from host
 *
 * @param 
       s: the socket fd
       data: receive info buffer
       size: receive info buffer max len
       flags:0
       to:the host addr
       tolen:sockaddr len
 * @return
        success:return receive info len
        fail: return -1
 */
int nwy_socket_recvfrom(int s, void *mem, size_t len, int flags,
                      struct sockaddr *from, socklen_t *fromlen);

/**
* @.set socket option
*
* @param 
    s: the socket fd
    level: The protocol layer of the option
    optname: option name
    optval:the option name value
    optlen:option value len
* @return
    success:return 0
    fail: return -1
*/
int nwy_socket_setsockopt(int s, int level, int optname, 
                      const void * optval, socklen_t optlen);

/**
* @.get socket option
*
* @param 
    s: the socket fd
    level: The protocol layer of the option
    optname: option name
    optval:point the option name value
    optlen:option value len
* @return
    success:return 0
    fail: return -1
*/

int nwy_socket_getsockopt(int s, int level, int optname, 
                      void * optval, socklen_t * optlen);

/**
* @.get host addr by domain name 
*
* @param 
    name:domain name
* @return
    return the host addr
*/
char* nwy_gethostbyname(const char *name);

/**
* @.get host addr by domain name and check ipv6
*
* @param 
    name:domain name
    isipv6: check ipv6
* @return
    return the host addr
*/
char* nwy_gethostbyname1(const char *name, int *isipv6);

/**
* @.close socket
*
* @param 
    socket:socket fd
* @return
    success:return 0
    fail: return -1
*/
int nwy_socket_close(int socket);


/**
* @.shutdown socket
*
* @param 
    socket:socket fd
    how: SHUT_RD SHUT_WR SHUT_RDWR 
* @return
    success:return 0
    fail: return -1
*/
int nwy_socket_shutdown(int socket, int how);


/**
* @.socket connect
*
* @param 
    socket:socket fd
    name:sever addr
    namelen:the struct sockaddr len
* @return
    success:return 0
    fail: return -1
*/
int nwy_socket_connect(int socket, const struct sockaddr_in *name, socklen_t namelen);

/**
* @.socket bind by port
*
* @param 
    socket:socket fd
    lport:bind port
* @return
    success:return 0
    fail: return not 0
*/
int nwy_socket_bind_lport(int socket, uint16_t lport);

/**
* @.socket bind
*
* @param 
    socket:socket fd
    name:sever addr
    namelen:the struct sockaddr len
* @return
    success:return 0
    fail: return -1
*/
int nwy_socket_bind(int socket, const struct sockaddr *addr, socklen_t addrlen);

/**
* @.socket listen
*
* @param 
    socket:socket fd
    backlog:listen client count
* @return
    success:return 0
    fail: return -1
*/
int nwy_socket_listen(int socket, int backlog);

/**
* @.socket accept
*
* @param 
    socket:socket fd
    addr:client addr
    namelen:the struct sockaddr len
* @return
    success:return 0
    fail: return -1
*/
int nwy_socket_accept(int socket, struct sockaddr *addr, socklen_t *addrlen);
//#if LWIP_SOCKET_SELECT

/**
* @.socket accept
*
* @param 
    maxfdp1:This parameter is ignored for compatibility only
    readset:Points to a set of interfaces waiting for read(Optional)
    writeset:Points to a set of interfaces waiting for read(Optional)
    exceptset:Points to a set of interfaces waiting for exception(Optional)
    timeout:timeout value,if null block mode
* @return
    success:return 0
    fail: return -1
*/
//int nwy_socket_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
//            struct timeval *timeout);
//#endif


int nwy_socket_set_nonblock(int socket);

int nwy_socket_get_state(int socket);

/**
* @.nwy_socket_get_ack
*
* @param 
    socket:socket fd
* @return
    the content to the peer length
*/
int nwy_socket_get_ack(int socket);

/**
* @.nwy_socket_get_ack
*
* @param 
    socket:socket fd
* @return
    the socket send content length
*/
int nwy_socket_get_sent(int socket);


int nwy_socket_bind_netif(int sockid, int simid, int cid);
int nwy_get_default_netif_v4_addr(ip4_addr_t *addr);
int nwy_socket_event_report_reg(nwy_socket_event_cb cb);

#ifdef __cplusplus
   }
#endif

#endif
