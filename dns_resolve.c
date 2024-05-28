#include "ft_ping.h"

// Performs a DNS lookup
char    *dns_lookup(char *hostname, struct sockaddr_in *addr_connexion)
{
    printf("\nResolving DNS\n ");
    struct hostent  *host_entity;
    char            *ip;
    int             i;

    ip = (char*)malloc(NI_MAXHOST * sizeof(char));
    if ((host_entity = gethostbyname(hostname)) == NULL) {
        printf("No IP found for hostname\n ");
        return NULL;
    }
 
    // filling up address structure
    strcpy(ip, inet_ntoa(*(struct in_addr*)host_entity->h_addr));
    (*addr_connexion).sin_family = host_entity->h_addrtype;
    (*addr_connexion).sin_port = htons(PORT_NO);
    (*addr_connexion).sin_addr.s_addr = *(long*)host_entity->h_addr;
    return ip;
}

// Resolves the reverse lookup of the hostname
char    *reverse_dns_lookup(char *ip_addr)
{
    printf("\nReverse DNS\n ");
    struct sockaddr_in  temp_addr;
    socklen_t           len;
    char                buf[NI_MAXHOST];
    char                *ret_buf;

    temp_addr.sin_family = AF_INET;
    temp_addr.sin_addr.s_addr = inet_addr(ip_addr);
    len = sizeof(struct sockaddr_in);

    if (getnameinfo((struct sockaddr*)&temp_addr, len, buf, sizeof(buf), NULL, 0, NI_NAMEREQD))
    {
        printf("Could not resolve reverse lookup of hostname\n");
        return NULL;
    }
    ret_buf = (char*)malloc((strlen(buf) + 1) * sizeof(char));
    strcpy(ret_buf, buf);
    return ret_buf;
}