#include "../includes/ft_ping.h"

// Performs a DNS lookup to get IP adresse
char    *dns_lookup(char *hostname, struct sockaddr_in *ping_addr)
{
    struct hostent  *host_entity;
    char            *ip;

    if (!(host_entity = gethostbyname(hostname))) {
        printf("Error Dns lookup : %s\n", strerror(errno));
        return 0;
    }
    // filling up address structure
    ip = (char*)malloc(NI_MAXHOST * sizeof(char));
    strcpy(ip, inet_ntoa(*(struct in_addr*)host_entity->h_addr)); // convertit l'addr en une string format ipv4
    (*ping_addr).sin_family = host_entity->h_addrtype;
    (*ping_addr).sin_port = htons(PORT_NO);
    (*ping_addr).sin_addr.s_addr = *(long*)host_entity->h_addr;
    return ip;
}

// Resolves the reverse lookup of the hostname
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
        printf("Error reverse Dns lookup : %s\n", strerror(errno));
        return NULL;
    }
    ret_buf = (char*)malloc((strlen(buf) + 1) * sizeof(char));
    strcpy(ret_buf, buf);
    return ret_buf;
}
//ttl 64
//affichage time
