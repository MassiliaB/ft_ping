#include <libc.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/socket.h>
//#include <netpacket/packet.h>

#define PING_PKT_S 64 // ping packet size
#define PORT_NO 0 // Automatic port number
#define PING_SLEEP_RATE 1000000 // Automatic port number
#define RECV_TIMEOUT 1 // Timeout delay for receiving packets in seconds

int pingloop = 1;

// The ICMP Packet Structure
struct icmp_packet {
    struct ethhdr eth; //Ethernet header
    struct ip ip;	//IP header 
    struct icmphdr hdr; //ICMP header
    char msg[PING_PKT_S-sizeof(struct icmphdr)]; //Junk payload 
};

char    *reverse_dns_lookup(char *ip_addr);
char    *dns_lookup(char *hostname, struct sockaddr_in *addr_connexion);
void    send_ping(int raw_sockfd, struct sockaddr_in *addr_con, char *ping_domain, char *ping_ip, char *rev_host);
