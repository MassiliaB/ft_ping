#include "ft_ping.h"

void intHandler(int handle) // When CTRL + C is pressed, ping send a report and set the pingloop to false.
{
    pingloop = 0;
}

int open_rawsock()
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    // To open a socket, you need the socket family, the socket type and protocol

    if (sockfd < 0) {
        printf("Error: Socket file descriptor not received\n");
        return 0;
    }
    printf("\nSocket file descriptor % d received\n", sockfd);
    return sockfd;
}

int main(int ac, char **av)
{
    int     sockfd;
    struct  sockaddr_in addr_con; // ? 
    int     addrlen;
    char    net_buf[NI_MAXHOST];
    char    *options;
    char    *ip_addr;
    char    *reverse_hostname; // To convert ip addr to hostname

    if (ac < 2 && ac > 3){
        printf("usage: ping [-v][-?] ‹Hostname or IP>\n");
        return 0;
    }
    addrlen = sizeof(addr_con);
    if (!(ip_addr = dns_lookup(av[1], &addr_con))){
        printf("\nDNS lookup failed !Could not resolve hostname !\n");
        return 0;
    }
    reverse_hostname = reverse_dns_lookup(ip_addr);
    printf("\nTrying to connect to '%s' IP: % s\n", av[1], ip_addr);
    printf("\nReverse Lookup domain: % s", reverse_hostname);
 
    if ((sockfd = open_rawsock()) < 0)
        return -1;
    signal(SIGINT, intHandler); // catching interrupt
    send_ping(sockfd, &addr_con, reverse_hostname, ip_addr, av[1]);
    return 0;
}