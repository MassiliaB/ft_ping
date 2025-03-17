#include "../includes/ft_ping.h"
#include <time.h>
#include <sys/time.h>
int         pingloop = 1;
char        r_packet[PACKET_SIZE];
char        s_packet[DATALEN];
extern int  verbose;

void intHandler()
{
    pingloop = 0;
}

int send_packet(int msg_count, int raw_sockfd, struct sockaddr_in *ping_addr, t_global *time)
{
    struct icmp *icmp = (struct icmp *) s_packet;
    memset(s_packet, 0, DATALEN);

    icmp->icmp_type = ICMP_ECHO; // Message Type (8 bits)
    icmp->icmp_code = 0; // Message Code (8 bits): echo request
    icmp->icmp_cksum = 0;
    icmp->icmp_seq = htons(msg_count); // Sequence Number (16 bits): starts at 0
    icmp->icmp_id = htons(getpid()); // Identifier (16 bits): some number to trace the response

    memcpy(s_packet + sizeof(struct icmp), "AAAA", 4);
    icmp->icmp_cksum = checksum(s_packet, DATALEN);
    if ((sendto(raw_sockfd, s_packet, DATALEN, 0, (struct sockaddr*)ping_addr, sizeof(struct sockaddr_in))) < 0) {
        printf("ping: ");
        icmp_error(icmp->icmp_type, icmp->icmp_code);
        return 0;
    }
    gettimeofday(&time->start, NULL);
   // usleep(PING_SLEEP_RATE);
    return 1;
}

int recieve_packet(int raw_sockfd, int *ttl_val, t_global *global)
{
    struct sockaddr_in  r_addr;
    struct icmp         *hdr_r_pckt;
    int                 iphlen;
    struct ip           *ip;
    ssize_t             len;
    socklen_t addr_len = sizeof(r_addr);

    memset(r_packet, 0, PACKET_SIZE);
    if ((len = recvfrom(raw_sockfd, r_packet, PACKET_SIZE, 0, (struct sockaddr*)&r_addr, &addr_len)) < 0){
        printf("ping: Packet recieved error\n");
        return 0;
    }
    gettimeofday(&global->end, NULL);
    ip = (struct ip*)r_packet;
    iphlen = ip->ip_hl << 2; 
    *ttl_val = ip->ip_ttl;
    if (len < (ssize_t)(iphlen + sizeof(struct icmp))) {
        printf("ping: ICMP packet's length is less than expected\n");
        return 0;
    }
    hdr_r_pckt = (struct icmp*)(r_packet + iphlen);
    global->icmp_type = hdr_r_pckt->icmp_type;
    global->icmp_code = hdr_r_pckt->icmp_code;
    if (!(hdr_r_pckt->icmp_type == ICMP_ECHOREPLY && hdr_r_pckt->icmp_id == htons(getpid())))
        return 2;
    return 1;
}

void    cal_rtt(t_global *time){
    
    double res = (time->end.tv_usec - time->start.tv_usec) / 1000000.0 + (time->end.tv_sec - time->start.tv_sec);
    res *= 1000.0;

    time->rtt = res;
    if (res < time->min_rtt || time->min_rtt == 0.0) time->min_rtt = res;
    if (res > time->max_rtt) time->max_rtt = res;
    time->sum_rtt += res;
    time->sum_rtt2 += res * res;
}

void    icmp_loop(int raw_sockfd, struct sockaddr_in *ping_addr, char *argv, char *ip_addr, int ttl_val)
{
    int     msg_count, msg_received_count;
    int     recv;
    t_global   global;

    msg_count = 0;
    msg_received_count = 0;
    global.min_rtt = LONG_MAX;
    global.max_rtt = 0.0;
    global.sum_rtt = 0.0;
    global.sum_rtt2 = 0.0;
    signal(SIGINT, intHandler);
    gettimeofday(&global.tfs, NULL);
    while (pingloop) {
        if (!send_packet(++msg_count, raw_sockfd, ping_addr, &global))
            return ;
        if ((recv = recieve_packet(raw_sockfd, &ttl_val, &global))){
            msg_received_count++;
            cal_rtt(&global);
            printf("%d bytes from %s: icmp seq=%d ttl=%u time=%.2f ms",
                PING_PKT_S, ip_addr, msg_count, ttl_val, global.rtt);
            if (recv == 2)
                printf(" type=%d code=%d", global.icmp_type, global.icmp_code);
            printf("\n");
            usleep(PING_SLEEP_RATE);
        }
    }
    double total_time = (global.end.tv_usec - global.tfs.tv_usec) / 1000000.0 + (global.end.tv_sec - global.tfs.tv_sec);
    total_time *= 1000.0;

    printf("--- %s ping statistics ---\n", argv);
    printf("%d packets transmitted, %d received, %.0f%% packet loss, time %.fms\n", msg_count, msg_received_count, ((msg_count - msg_received_count) * 100.0) / msg_count, total_time);
    if (msg_received_count) {
        global.avg_rtt = global.sum_rtt / msg_received_count;
        double mdev = sqrt((global.sum_rtt2 / msg_received_count) - (global.avg_rtt * global.avg_rtt));
        printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
            global.min_rtt, global.avg_rtt, global.max_rtt, mdev);    
    }
    close(raw_sockfd);
}

void    init_ping(int raw_sockfd, struct sockaddr_in *ping_addr, char *ip_addr, char *argv)
{
    struct timeval      tv_out;
    int                 ttl_val;

    ttl_val = 255;
    tv_out.tv_sec = 1;
    tv_out.tv_usec = 0;
    if (verbose)
        printf("ping: sock4.fd: %d (socktype: SOCK_RAW), hints.ai_family: AF_INET\n", raw_sockfd);
    if (setsockopt(raw_sockfd, IPPROTO_IP, IP_TTL, &ttl_val, sizeof(ttl_val))!= 0) {
        printf("ping: Setting socket options to TTL failed !\n");
        return;
    }
    setsockopt(raw_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof(tv_out));
    printf("PING %s: %d bytes of data.", ip_addr, DATALEN);
    if (verbose){
        printf (", id 0x%04x = %u", htons(getpid()), htons(getpid()));
    }
    printf("\n");
    icmp_loop(raw_sockfd, ping_addr, argv, ip_addr, ttl_val);
}