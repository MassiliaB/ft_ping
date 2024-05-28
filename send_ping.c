#include "ft_ping.h"

void    icmp_loop(int p, int raw_sockfd, struct sockaddr_in *ping_addr, int *msg_count, int *msg_received_count, char *rev_host, char *ping_ip, int ttl_val, char *ping_domain)
{
    struct icmp_packet     *r_pckt;
    struct icmp_packet    *s_pckt;
    struct timespec     time_start;
    struct sockaddr_in  r_addr;
    struct timespec     time_end;
    int         i;
    int         pckt_sent;
    int         addr_len;
    char        rbuffer[128]; //receive buffer
    long double rtt_msec;

    rtt_msec = 0;
    pckt_sent = 1;
    msg_count = 0;
    while (pingloop) {
        pckt_sent = 1; //was a packet sent or not
        bzero(&s_pckt, sizeof(s_pckt)); // filling packet

        s_pckt->hdr.type = ICMP_ECHO;
        s_pckt->hdr.un.echo.id = getpid();
        for (i = 0; i < sizeof(s_pckt->msg) - 1; i++)
            s_pckt->msg[i] = i + '0';

        s_pckt->msg[i] = 0;
        s_pckt->hdr.un.echo.sequence = msg_count++;
        s_pckt->hdr.checksum = checksum(& s_pckt, sizeof(s_pckt));

        usleep(PING_SLEEP_RATE);

        // send packet
        clock_gettime(CLOCK_MONOTONIC, & time_start);
        if (sendto(raw_sockfd, &s_pckt, sizeof(s_pckt), 0, (struct sockaddr*)ping_addr, sizeof(*ping_addr)) <= 0) {
            printf("\nPacket Sending Failed !\n");
            pckt_sent = 0;
        }

        // receive packet
        addr_len = sizeof(r_addr);
        if (recvfrom(raw_sockfd, rbuffer, sizeof(rbuffer), 0, (struct sockaddr*)&r_addr, & addr_len) <= 0  &msg_count < 1) {
            printf("\nPacket receive failed !\n");
        }
        else {
            clock_gettime(CLOCK_MONOTONIC, & time_end);

            double timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec)) / 1000000.0;
            rtt_msec = (time_end.tv_sec - time_start.tv_sec) * 1000.0 + timeElapsed;

            // if packet was not sent, don't receive
            if (pckt_sent) {
                if (!(r_pckt->hdr.type == 0 || &r_pckt->hdr.code == 0)) {
                    printf(" Error..Packet receive with ICMP type % d code % d\n", r_pckt->hdr.type, r_pckt->hdr.code);
                }
                else {
                    printf(" % d bytes from % s(h: % s)(% s) msg_seq = % d ttl = % d rtt = % Lf ms.\n", PING_PKT_S, *ping_domain, rev_host, ping_ip, msg_count, ttl_val, rtt_msec);
                    msg_received_count++;
                }
            }
        }
    }
}

void    send_ping(int raw_sockfd, struct sockaddr_in *ping_addr, char *ping_domain, char *ping_ip, char *rev_host)
{
    struct timespec     tfs;
    struct timespec     tfe;
    struct timeval      tv_out;
    long double total_msec;
    int         *msg_received_count;
    int         *msg_count;
    int         ttl_val;
    double      timeElapsed;

    ttl_val = 64; //ttl that will decrease 
    total_msec = 0;
    tv_out.tv_sec = RECV_TIMEOUT;
    tv_out.tv_usec = 0;
 
    clock_gettime(CLOCK_MONOTONIC, &tfs); // get time data
 
    // set socket options ??
    if (setsockopt(raw_sockfd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val))!= 0) {
        printf("Setting socket options to TTL failed !\n");
        return;
    }
    else
        printf("Socket set to TTL\n");
    // setting timeout of recv setting
    setsockopt(raw_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out);
 
    // send icmp packet in an infinite loop
    icmp_loop(pingloop, raw_sockfd, &ping_addr, &msg_count, &msg_received_count, &rev_host, &ping_ip, ttl_val, ping_domain);

    clock_gettime(CLOCK_MONOTONIC, &tfe);
    timeElapsed = ((double)(tfe.tv_nsec - tfs.tv_nsec)) / 1000000.0;
    total_msec = (tfe.tv_sec - tfs.tv_sec) * 1000.0 + timeElapsed;
    printf("\n == = %s ping statistics ===\n", ping_ip);
    printf("\n %d packets sent, %d packets received, %f percent packet loss.Total time: %Lf ms.\n\n", *msg_count, msg_received_count, ((*msg_count - *msg_received_count) / *msg_count) * 100.0, total_msec);
}
 