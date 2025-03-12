#include "../includes/ft_ping.h"
#include <time.h>

int         pingloop = 1;
char        s_packet[DATALEN];
char        r_packet[PACKET_SIZE];
extern int  verbose;

void intHandler()
{
    pingloop = 0;
}
// void	print_icmp_header(struct icmphdr *icmp, uint32_t size) {
// 	printf("ICMP: type %u, code %u, size %u", icmp->type, icmp->code, size);
// 	if (icmp->type == ICMP_ECHO || icmp->type == ICMP_ECHOREPLY) {
// 		printf(", id 0x%04x, seq 0x%04x", icmp->un.echo.id, icmp->un.echo.sequence);
// 	}
// 	printf("\n");
// }

int send_packet(int raw_sockfd, struct sockaddr_in *ping_addr)
{
    if ((sendto(raw_sockfd, s_packet, DATALEN, 0, (struct sockaddr*)ping_addr, sizeof(struct sockaddr_in))) < 0) {
        printf("ping: Packet sent error\n");
        return 0;
    }
    usleep(PING_SLEEP_RATE);
    return 1;
}

int recieve_packet(int raw_sockfd, int *ttl_val)
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
    ip = (struct ip*)r_packet;
    iphlen = ip->ip_hl << 2; //calculate the lenght of the IP header in bytes
    *ttl_val = ip->ip_ttl;
    if (len < (ssize_t)(iphlen + sizeof(struct icmp))) {
        printf("ping: ICMP packet's length is less than expected\n");
        return 0;
    }
    hdr_r_pckt = (struct icmp*)(r_packet + iphlen);
    if (!(hdr_r_pckt->icmp_type == ICMP_ECHOREPLY && hdr_r_pckt->icmp_id == htons(getpid())))
        printf("ping: Packet sent error\n");
    else
        return 1;
    return 0;
}

void    icmp_loop(int raw_sockfd, struct sockaddr_in *ping_addr, char *argv, char *ip_addr, int ttl_val, char *ping_domain, struct icmp *icmp)
{
    struct  timeval start, end;
    double  rtt_msec;
    int     timeElapsed, msg_count, msg_received_count;

    msg_count = 0;
    msg_received_count = 0;
    rtt_msec = 0;
    double min_rtt = LONG_MAX;
    double max_rtt = 0.0;
    double sum_rtt = 0.0;
    double sum_rtt2 = 0.0;
    signal(SIGINT, intHandler);
    while (pingloop) {
        icmp->icmp_seq = htons(msg_count++);
        if (!send_packet(msg_count, raw_sockfd, ping_addr, &time_start))
            return ;
        // printf("rcv count = %d, msg count %d\n", msg_received_count, msg_count);
        while (msg_received_count < msg_count && pingloop){
            if (recieve_packet(raw_sockfd, &ttl_val)){
                gettimeofday(&start, NULL);
                rtt_msec = (time_end.tv_nsec - time_start.tv_nsec) / 1000.0;
                rtt_msec += (time_end.tv_sec - time_start.tv_sec) * 100;
                printf("%d bytes from %s (%s): icmp seq=%d ttl=%d time=%.3f ms\n", PING_PKT_S, ping_domain, ip_addr, msg_count, ttl_val, rtt_msec);
                if (rtt_msec < min_rtt) min_rtt = rtt_msec;
                if (rtt_msec > max_rtt) max_rtt = rtt_msec;
                
                sum_rtt += rtt_msec;
                sum_rtt2 += rtt_msec * rtt_msec;
                msg_received_count++;
            }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &(*tfe));
    timeElapsed = (tfe->tv_nsec - tfs->tv_nsec) / 1000.0 ;
    timeElapsed += (tfe->tv_sec - tfs->tv_sec) * 1000;
    
    printf("--- %s ping statistics ---\n", argv);
    printf("%d packets transmitted, %d received, %.f%% packet loss, time %dms\n", msg_count, msg_received_count, ((msg_count - msg_received_count) * 100.0) / msg_count, timeElapsed);
    if (msg_received_count) {
        double avg_rtt = (msg_received_count > 0) ? ((double)sum_rtt / msg_received_count) : 0;
        double mdev_rtt = 0.0;
        mdev_rtt = sqrt(((double)sum_rtt2 / msg_received_count) - (avg_rtt * avg_rtt));
        printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
            min_rtt, avg_rtt, max_rtt, mdev_rtt);    
    }
    close(raw_sockfd);
}