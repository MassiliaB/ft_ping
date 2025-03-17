#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <netpacket/packet.h>
#include <errno.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>

#define PING_PKT_S      64 // ping packet size
#define PORT_NO         0 // Automatic port number
#define PING_SLEEP_RATE 1000000 
#define RECV_TIMEOUT    1 // Timeout delay for receiving packets in seconds
#define NI_MAXHOST      1025
#define NI_MAXSERV      32

#define DATALEN         64
#define IPMAXLEN        60
#define ICMPMAXLEN      76
#define PACKET_SIZE     1024

typedef struct	s_global
{
	double	min_rtt;
	double	max_rtt;
    double  sum_rtt;
    double  sum_rtt2;
    double  avg_rtt;
    double  rtt;
    struct  timeval start;
    struct  timeval end;
    struct  timeval tfs;
    int     icmp_code;
    int     icmp_type;
}				t_global;

char    *reverse_dns_lookup(char *ip_addr);
char    *dns_lookup(char *hostname, struct sockaddr_in *addr_connexion);
void    init_ping(int raw_sockfd, struct sockaddr_in *ping_addr, char *ip_addr, char *argv);
void    intHandler();
int     open_rawsock();
int     parse_args(char **av, int ac, char **addr);
unsigned short  checksum(void *addr, int len);
void    icmp_error(int type, int code);