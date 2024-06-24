#include "../includes/ft_ping.h"

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

int parse_args(char **av, int ac, char **addr)
{
    int opt;
    int help_flag;

    help_flag = 0;
    while ((opt = getopt(ac, av, "v?")) != -1) {
        switch (opt) {
            case 'v':
                break;
            case '?':
                help_flag = 1;
                break;
            default:
                return 0;
        }
    }
    if (help_flag) {
        printf("Usage:\n");
        printf("  ping [-v][-?] ‹Destination>\n\n");
        printf("Options:\n");
        printf("  <destination>   dns name or ip address\n");
        printf("  -v              verbose output\n");
        printf("  -?              print help and exit\n\n");
        printf("For more details see ping(8).\n");
        return -1;
    }
    if (optind == ac){
        printf("Error: Destination address required\n");
        return -1;
    }
    printf("Cc\n");
    *addr = (char*)malloc(strlen(av[optind]) + 1);
    strcpy(*addr, av[optind]);
    printf("Cc 2\n");
    return 1;
}

int main(int ac, char **av)
{
    int     sockfd;
    struct  sockaddr_in dest_addr;
    int     verbose;
    char    *addr;
    char    *ip_addr;
    char    *reverse_hostname; // To convert ip addr to hostname

    verbose = 0;
    addr = NULL;
    if (getuid() != 0){
        printf("This program requires root privileges.\n");
        return 0;
    }
    if (ac < 2 || ac > 3){
        printf("usage: ping [-v][-?] ‹Destination>\n");
        return 0;
    }
    if ((verbose = parse_args(av, ac, &addr)) < 0)
        return 0;
    if (!(ip_addr = dns_lookup(addr, &dest_addr))){
        printf("Error: %s\n", strerror(errno));
        return 0;
    }
    reverse_hostname = reverse_dns_lookup(ip_addr); 
    if ((sockfd = open_rawsock()) < 0)
        return -1;
    signal(SIGINT, intHandler); // catching interrupt
    send_ping(sockfd, &dest_addr, reverse_hostname, ip_addr, addr, verbose);
    free(addr);
    free(ip_addr);
    free(reverse_hostname);
    return 0;
}