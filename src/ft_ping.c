#include "../includes/ft_ping.h"

int verbose = 0;

void    init_ping(int raw_sockfd, struct sockaddr_in *ping_addr, char *ping_domain, char *ip_addr, char *argv)
{
    struct timeval      tv_out;
    int                 ttl_val;
    struct icmp *icmp = (struct icmp *) s_packet;

    ttl_val = 255;
    memset(s_packet, 0, DATALEN);
    if (setsockopt(raw_sockfd, IPPROTO_IP, IP_TTL, &ttl_val, sizeof(ttl_val))!= 0) {
        printf("ping: Setting socket options to TTL failed !\n");
        return;
    }
    setsockopt(raw_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof(tv_out));
    icmp.icmp_type = ICMP_ECHO; // Message Type (8 bits)
    icmp.icmp_code = 0; // Message Code (8 bits): echo request
    icmp.icmp_cksum = 0;
    icmp.icmp_seq = 0; // Sequence Number (16 bits): starts at 0
    icmp.icmp_id = htons(getpid()); // Identifier (16 bits): some number to trace the response
    memcpy(s_packet + sizeof(struct icmp), "AAAA", 4);
    icmp.icmp_cksum = checksum(s_packet, DATALEN);
    tv_out.tv_sec = 1;

    printf("PING %s(%s): %d bytes of data", argv, ip_addr, DATALEN);
    if (verbose){
        printf (", id 0x%04x = %u", icmp.icmp_id, icmp.icmp_id);
    }
    printf("\n");
    icmp_loop(raw_sockfd, ping_addr, argv, ip_addr, ttl_val, ping_domain, &icmp);
}
 
int main(int ac, char **av)
{
    int     sockfd;
    struct  sockaddr_in dest_addr;
    char    *addr;
    char    *ip_addr;
    char    *reverse_hostname;

    addr = NULL;
    if (parse_args(av, ac, &addr) < 0)
        return 0;
    if (!(ip_addr = dns_lookup(addr, &dest_addr))){
        return 0;
    }
    reverse_hostname = reverse_dns_lookup(ip_addr); 
    if ((sockfd = open_rawsock()) < 0)
        return -1;
    init_ping(sockfd, &dest_addr, reverse_hostname, ip_addr, addr);
    free(addr);
    free(ip_addr);
    free(reverse_hostname);
    return 0;
}