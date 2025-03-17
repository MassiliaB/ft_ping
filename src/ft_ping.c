#include "../includes/ft_ping.h"

int parse_args(char **av, int ac, char **addr)
{
    int opt;
    int help_flag;
    int verbose;

    verbose = 0;
    help_flag = 0;
    if (getuid() != 0){
        printf("ping: This program requires root privileges.\n");
        return -1;
    }
    if (ac < 2 || ac > 3){
        printf("ping: usage: ping [-v][-?] ‹Destination>\n");
        return -1;
    }
    while ((opt = getopt(ac, av, "v?")) != -1) {
        switch (opt) {
            case 'v':
                verbose = 1;
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
        printf("Error: Destination address required.\n");
        return -1;
    }
    *addr = (char*)malloc(strlen(av[optind]) + 1);
    strcpy(*addr, av[optind]);
    return verbose;
}

int main(int ac, char **av)
{
    int     sockfd;
    struct  sockaddr_in dest_addr;
    char    *ip_addr;
    int     verbose = 0;
    char    *addr = NULL;

    if ((verbose = parse_args(av, ac, &addr)) < 0)
        return 0;
    if ((sockfd = open_rawsock()) < 0)
        return 0;
    if (verbose)
        printf("ping: sock4.fd: %d (socktype: SOCK_RAW), hints.ai_family: AF_INET\n\n", sockfd);
    if (!(ip_addr = dns_lookup(addr, &dest_addr))){
        free(addr);
        return 0; 
    }  
    init_ping(sockfd, &dest_addr, ip_addr, addr, verbose);
    free(addr);
    free(ip_addr);
    return 0;
}