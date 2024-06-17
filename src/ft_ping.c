#include "../includes/ft_ping.h"

#include <stdint.h>

// Fonction pour calculer le checksum
uint16_t checksum(void *addr, int len) {
    uint16_t *buf = addr;
    uint32_t sum = 0;
    uint16_t result;

    for (; len > 1; len -= 2) {
        sum += *buf++;
    }
    if (len == 1) {
        sum += *(uint8_t *)buf;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}


int open_rawsock()
{
    int sockfd; //Raw socket - if you use IPPROTO_ICMP, then kernel will fill in the correct ICMP header checksum, if IPPROTO_RAW, then it wont
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if (sockfd == -1) {
        printf("Error creating socket : %s\n", strerror(errno));
        return -1;
    }
    return sockfd;
}

int main(int ac, char **av)
{
    int     sockfd;
    struct  sockaddr_in dest_addr; // ? 
  //  char    *options;
    char    *ip_addr;
    char    *reverse_hostname; // To convert ip addr to hostname

    if (getuid() != 0){
        printf("This program requires root privileges!\n");
        return 0;
    }
    if (ac < 2 || ac > 3){
        printf("usage: ping [-v][-?] ‹Hostname or IP>\n");
        return 0;
    }
    if (!(ip_addr = dns_lookup(av[1], &dest_addr))){
        printf("DNS lookup failed: could not resolve hostname !\n");
        return 0;
    }
    reverse_hostname = reverse_dns_lookup(ip_addr); 
    if ((sockfd = open_rawsock()) < 0)
        return -1;
    signal(SIGINT, intHandler); // catching interrupt
   // signal(SIGALRM, intAlarm);
    send_ping(sockfd, &dest_addr, reverse_hostname, ip_addr, av[1]);
    free(ip_addr);
    free(reverse_hostname);
    return 0;
}