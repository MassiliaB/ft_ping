#include "../includes/ft_ping.h"
#include <time.h>
int pingloop = 1;
char        s_packet[DATALEN + IPMAXLEN + ICMPMAXLEN];
char        r_packet[DATALEN + IPMAXLEN + ICMPMAXLEN];

void intHandler() // When CTRL + C is pressed, ping send a report and set the pingloop to false.
{
    pingloop = 0;
}

unsigned short checksum(void *addr, int len) //Used for error checking of the header
{
    unsigned short *buf = addr;
    unsigned short result;
    unsigned int sum = 0;
 
    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int send_packet(int msg_count, int raw_sockfd, struct sockaddr_in *ping_addr, struct timespec *time_start)
{
    struct icmp   *hdr_s_pckt; //ICMP packet

    bzero(&hdr_s_pckt, sizeof(hdr_s_pckt)); // filling packet
    hdr_s_pckt = (struct icmp*)s_packet;
    hdr_s_pckt->icmp_type = ICMP_ECHO; // Message Type (8 bits)
    hdr_s_pckt->icmp_code = 0; // Message Code (8 bits): echo request
    hdr_s_pckt->icmp_id = getpid(); // Identifier (16 bits): some number to trace the response
    hdr_s_pckt->icmp_seq = msg_count; // Sequence Number (16 bits): starts at 0
    hdr_s_pckt->icmp_cksum = checksum(&hdr_s_pckt, sizeof(hdr_s_pckt));
    
    clock_gettime(CLOCK_MONOTONIC, &(*time_start));
    if (sendto(raw_sockfd, s_packet, 8+DATALEN, 0, (struct sockaddr*)ping_addr, sizeof(*ping_addr)) < 0) {
        printf("Packet sent error : %s\n", strerror(errno));
        return 0;
    }
    usleep(PING_SLEEP_RATE);
    return 1;
}

void    icmp_loop(int raw_sockfd, struct sockaddr_in *ping_addr, struct timespec *tfs, struct timespec *tfe, char *argv, char *ip_addr, int ttl_val, char *ping_domain)
{
    struct icmp   *hdr_r_pckt;
    struct timespec     time_start;
    struct timespec     time_end;
    int                 pckt_sent;
    int                 msg_count;
    int                 msg_received_count;
    long double         rtt_msec;
    long double total_msec;
    double      timeElapsed;

    total_msec = 0;
    rtt_msec = 0;
    pckt_sent = 1;
    msg_count = 0;
    msg_received_count = 0;
    while (pingloop) {
        pckt_sent = 1; //was a packet sent or not
        pckt_sent = send_packet(msg_count, raw_sockfd, ping_addr, &time_start);
        msg_count++;

        printf("is that okay ? count = %d\n", msg_count);

        while (pingloop)
        {
            struct sockaddr_in  r_addr;
            memset(r_packet, 0, sizeof(&r_packet));
            hdr_r_pckt = (struct icmp*)r_packet;
            if (recvfrom(raw_sockfd, r_packet, sizeof(r_packet), 0, (struct sockaddr*)&r_addr, (socklen_t*)(sizeof(r_addr))) <= 0 && msg_count > 1)
                printf("Packet received error : %s\n", strerror(errno));
            else {
                                printf("alive ? %d\n", hdr_r_pckt->icmp_type);
                clock_gettime(CLOCK_MONOTONIC, &time_end);
                double timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec)) / 1000000.0;
                rtt_msec = (time_end.tv_sec - time_start.tv_sec) * 1000.0 + timeElapsed;
                if (pckt_sent) {
                    if (!(hdr_r_pckt->icmp_type == 0 && hdr_r_pckt->icmp_code == 0))
                        printf("Packet sent error : %s\n", strerror(errno));
                    else {
                        printf("%d bytes from %s (%s): icmp seq=%d ttl=%d time=%Lf ms\n", PING_PKT_S, ping_domain, ip_addr, msg_count, ttl_val, rtt_msec);
                        msg_received_count++;
                        break ;
                    }
                }
            }
        }
       // bzero(&hdr_r_pckt, sizeof(hdr_r_pckt));
    }
    clock_gettime(CLOCK_MONOTONIC, &(*tfe));
    timeElapsed = ((double)(tfe->tv_nsec - tfs->tv_nsec)) / 1000000.0;
    total_msec = (tfe->tv_sec - tfs->tv_sec) * 1000.0 + timeElapsed;
    printf("--- %s ping statistics ---\n", argv);
    printf("%d packets transmitted, %d received, %f%% packet loss, time %Lfms\n", msg_count, msg_received_count, ((msg_count - msg_received_count) / msg_count) * 100.0, total_msec);
    close(raw_sockfd);
}

void    send_ping(int raw_sockfd, struct sockaddr_in *ping_addr, char *ping_domain, char *ip_addr, char *argv)
{
    struct timespec     tfs;
    struct timespec     tfe;
    struct timeval      tv_out;
    int                 ttl_val;

    ttl_val = 63; //ttl that will decrease 
    tv_out.tv_sec = RECV_TIMEOUT;
    tv_out.tv_usec = 0;
    clock_gettime(CLOCK_MONOTONIC, &tfs); // get time data

    /* Now we create the packet that we send down the wire
     * Since we use IPPROTO_ICMP in socket(), we just have to create the
     * ICMP packet
     */
    if (setsockopt(raw_sockfd, IPPROTO_IP, IP_TTL, &ttl_val, sizeof(ttl_val))!= 0) {
         printf("Setting socket options to TTL failed !\n");
         return;
    }
    // setting timeout of recv setting
    setsockopt(raw_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out);

    printf("PING %s(%s): %d bytes of data.\n", argv, ip_addr, DATALEN);

    // send icmp packet in an infinite loop
    icmp_loop(raw_sockfd, ping_addr, &tfs, &tfe, argv, ip_addr, ttl_val, ping_domain);
}
 