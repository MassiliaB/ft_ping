#include "../includes/ft_ping.h"
#include <time.h>
int pingloop = 1;
char        s_packet[sizeof(struct icmp) + DATALEN];
char        r_packet[PACKET_SIZE];

void intHandler() // When CTRL + C is pressed, ping send a report and set the pingloop to false.
{
    pingloop = 0;
}

int send_packet(int msg_count, int raw_sockfd, struct sockaddr_in *ping_addr, struct timespec *time_start)
{
    struct icmp *hdr_s_pckt; //ICMP packet header
    int         packsize;

    hdr_s_pckt = (struct icmp*)s_packet;
    hdr_s_pckt->icmp_type = ICMP_ECHO; // Message Type (8 bits)
    hdr_s_pckt->icmp_code = 0; // Message Code (8 bits): echo request
    hdr_s_pckt->icmp_cksum = 0;
    hdr_s_pckt->icmp_seq = htons(msg_count); // Sequence Number (16 bits): starts at 0
    hdr_s_pckt->icmp_id = htons(getpid()); // Identifier (16 bits): some number to trace the response
    //htons est nécessaire pour assurer la compatibilité des données multi-octets entre machines avec des ordres de byte différents (endianness)

    hdr_s_pckt->icmp_cksum = checksum(s_packet, sizeof(struct icmp) + DATALEN);
    packsize = sizeof(struct icmp) + DATALEN;
    clock_gettime(CLOCK_MONOTONIC, time_start);
    if ((sendto(raw_sockfd, s_packet, packsize, 0, (struct sockaddr*)ping_addr, sizeof(*ping_addr))) < 0) {
        printf("Packet sent error : %s\n", strerror(errno));
        return 0;
    }
    usleep(PING_SLEEP_RATE);
    return 1;
}

int receive_packet(int raw_sockfd)
{
    struct sockaddr_in  r_addr;
    struct icmp   *hdr_r_pckt;
    int iphlen;
    struct ip *ip;
    ssize_t len;
    socklen_t addr_len = sizeof(r_addr);

    memset(r_packet, 0, PACKET_SIZE);
    if ((len = recvfrom(raw_sockfd, r_packet, PACKET_SIZE, 0, (struct sockaddr*)&r_addr, &addr_len)) < 0){
        printf("Packet received error : %s\n", strerror(errno));
        return 0;
    }
    ip = (struct ip*)r_packet;
    iphlen = ip->ip_hl << 2; //calculate the lenght of the IP header in bytes
    if (len < (ssize_t)(iphlen + sizeof(struct icmp))) {
        printf("ICMP packet's length is less than expected\n");
        return 0;
    }
    hdr_r_pckt = (struct icmp*)(r_packet + iphlen);
    if (!(hdr_r_pckt->icmp_type == ICMP_ECHOREPLY && hdr_r_pckt->icmp_id == htons(getpid())))
        printf("Packet sent error : %s\n", strerror(errno));
    else
        return 1;
    return 0;
}

void    icmp_loop(int raw_sockfd, struct sockaddr_in *ping_addr, struct timespec *tfs, struct timespec *tfe, char *argv, char *ip_addr, int ttl_val, char *ping_domain)
{
    struct      timespec time_start;
    struct      timespec time_end;
    long double total_msec;
    long double rtt_msec;
    double      timeElapsed;
    int         msg_count;
    int         msg_received_count;

    total_msec = 0;
    msg_count = 0;
    msg_received_count = 0;
    rtt_msec = 0;
    while (pingloop) {
        if (!send_packet(msg_count++, raw_sockfd, ping_addr, &time_start))
            return ;
        while (msg_received_count < msg_count){
            if (receive_packet(raw_sockfd)){
                clock_gettime(CLOCK_MONOTONIC, &time_end);
                double timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec)) / 1000000.0;
                rtt_msec = (time_end.tv_sec - time_start.tv_sec) * 1000.0 + timeElapsed;
                printf("%d bytes from %s (%s): icmp seq=%d ttl=%d time=%Lf ms\n", PING_PKT_S, ping_domain, ip_addr, msg_count, ttl_val, rtt_msec);
                msg_received_count++;
            }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &(*tfe));
    timeElapsed = ((double)(tfe->tv_nsec - tfs->tv_nsec)) / 1000000.0;
    total_msec = (tfe->tv_sec - tfs->tv_sec) * 1000.0 + timeElapsed;
    printf("--- %s ping statistics ---\n", argv);
    printf("%d packets transmitted, %d received, %f%% packet loss, time %Lfms\n", msg_count, msg_received_count, ((msg_count - msg_received_count) / msg_count) * 100.0, total_msec);
    close(raw_sockfd);
}

void    send_ping(int raw_sockfd, struct sockaddr_in *ping_addr, char *ping_domain, char *ip_addr, char *argv, int on)
{
    struct timespec     tfs;
    struct timespec     tfe;
    struct timeval      tv_out;
    int                 ttl_val;

    ttl_val = 64; //ttl that will decrease 
    tv_out.tv_sec = RECV_TIMEOUT;
    tv_out.tv_usec = 0;
    clock_gettime(CLOCK_MONOTONIC, &tfs); // get time data
    /* Now we create the packet that we send down the wire
     * Since we use IPPROTO_ICMP in socket(), we just have to create the
     * ICMP packet
     */
    // setting TTL to controle the range of the packets
    if (setsockopt(raw_sockfd, IPPROTO_IP, IP_TTL, &ttl_val, sizeof(ttl_val))!= 0) {
         printf("Setting socket options to TTL failed !\n");
         return;
    }
    // setting timeout delay of recv setting
    setsockopt(raw_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof(tv_out));
    if (setsockopt(raw_sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    printf("PING %s(%s): %d bytes of data.\n", argv, ip_addr, DATALEN);

    // send icmp packet in an infinite loop
    icmp_loop(raw_sockfd, ping_addr, &tfs, &tfe, argv, ip_addr, ttl_val, ping_domain);
}
 