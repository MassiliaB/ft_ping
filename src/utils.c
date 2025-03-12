#include "../includes/ft_ping.h"

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