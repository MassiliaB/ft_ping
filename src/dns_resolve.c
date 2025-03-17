#include "../includes/ft_ping.h"

char    *dns_lookup(char *hostname, struct sockaddr_in *ping_addr)
{
    struct hostent  *host_entity;
    char            *ip;

    if (!(host_entity = gethostbyname(hostname))) {
        printf("ping: Temporary failure in name resolution\n");
        return 0;
    }
    ip = (char*)malloc(NI_MAXHOST * sizeof(char));
    strcpy(ip, inet_ntoa(*(struct in_addr*)host_entity->h_addr));
    (*ping_addr).sin_family = host_entity->h_addrtype;
    (*ping_addr).sin_port = htons(PORT_NO);
    (*ping_addr).sin_addr.s_addr = *(long*)host_entity->h_addr;
    return ip;
}

char    *reverse_dns_lookup(char *ip_addr)
{
    struct sockaddr_in  temp_addr;
    socklen_t           len;
    char                buf[NI_MAXHOST];
    char                *ret_buf;

    temp_addr.sin_family = AF_INET;
    temp_addr.sin_addr.s_addr = inet_addr(ip_addr);
    len = sizeof(struct sockaddr_in);

    if (getnameinfo((struct sockaddr*)&temp_addr, len, buf, sizeof(buf), NULL, 0, NI_NAMEREQD))
    {
        printf("ping: Temporary failure in name resolution\n");
        return NULL;
    }
    ret_buf = (char*)malloc((strlen(buf) + 1) * sizeof(char));
    strcpy(ret_buf, buf);
    return ret_buf;
}
