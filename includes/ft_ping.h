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
#include <time.h>

#define PING_PKT_S 64 // ping packet size
#define PORT_NO 0 // Automatic port number
#define PING_SLEEP_RATE 1000000 // Automatic port number
#define RECV_TIMEOUT 1 // Timeout delay for receiving packets in seconds
#define NI_MAXHOST      1025
#define NI_MAXSERV      32

#define DATALEN         20
#define IPMAXLEN        60
#define ICMPMAXLEN      76
#define PACKET_SIZE 1024

char    *reverse_dns_lookup(char *ip_addr);
char    *dns_lookup(char *hostname, struct sockaddr_in *addr_connexion);
void    send_ping(int raw_sockfd, struct sockaddr_in *ping_addr, char *ping_domain, char *ip_addr, char *argv, int on);
void    intHandler();
unsigned short checksum(void *addr, int len);