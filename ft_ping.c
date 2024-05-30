#include "ft_ping.h"

int open_rawsock()
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    // To open a socket, you need the socket family, the socket type and protocol

    if (sockfd == -1) {
        printf("Error creating socket : %s\n", strerror(errno));
        return -1;
    }
    printf("Socket file descriptor %d received\n", sockfd);
    return sockfd;
}

int main(int ac, char **av)
{
    int     sockfd;
    struct  sockaddr_in ping_addr; // ? 
  //  char    *options;
    char    *ip_addr;
    char    *reverse_hostname; // To convert ip addr to hostname

    // Check for root access
    if (getuid() != 0){
        printf("This program requires root privileges!\n");
        return 0;
    }
    if (ac < 2 || ac > 3){
        printf("usage: ping [-v][-?] ‹Hostname or IP>\n");
        return 0;
    }
    if (!(ip_addr = dns_lookup(av[1], &ping_addr))){
        printf("DNS lookup failed: could not resolve hostname !\n");
        return 0;
    }
    reverse_hostname = reverse_dns_lookup(ip_addr);
    printf("Trying to connect to '%s' IP: %s\n", av[1], ip_addr);
    printf("Reverse Lookup domain: %s\n", reverse_hostname);
 
    if ((sockfd = open_rawsock()) < 0)
        return -1;
    signal(SIGINT, intHandler); // catching interrupt
    send_ping(sockfd, &ping_addr, reverse_hostname, ip_addr, av[1]);
    return 0;
}