#include "../includes/ft_ping.h"

void    icmp_error(int type, int code) {
    switch (type) {
        case ICMP_DEST_UNREACH:
            switch (code) {
                case ICMP_NET_UNREACH: printf("Network Unreachable\n"); break;
                case ICMP_HOST_UNREACH: printf("Host Unreachable\n"); break;
                case ICMP_PROT_UNREACH: printf("Protocol Unreachable\n"); break;
                case ICMP_PORT_UNREACH: printf("Port Unreachable\n"); break;
                case ICMP_FRAG_NEEDED: printf("Fragmentation Needed\n"); break;
                case ICMP_SR_FAILED: printf("Source Route Failed\n"); break;
                case ICMP_NET_UNKNOWN: printf("Network Unknown\n"); break;
                case ICMP_HOST_UNKNOWN: printf("Host Unknown\n"); break;
                default: printf("Destination Unreachable\n"); break;
            }
        break;
        case ICMP_TIME_EXCEEDED:
            if (code == ICMP_EXC_TTL) 
                printf("TTL Expired in Transit\n");
            else
                printf("Fragment Reassembly Time Exceeded\n");
            break;
        default:
            printf("Unknown ICMP Type\n");
    }
}

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
    int sockfd;
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd == -1) {
        printf("ping: Error creating socket\n");
        return -1;
    }
    return sockfd;
}