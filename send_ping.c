#include "ft_ping.h"
#include <time.h>
int pingloop = 1;

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

int receive_packet(int msg_received_count)
{

    return msg_received_count;
}

void    icmp_loop(int raw_sockfd, struct sockaddr_in *ping_addr, struct timespec *tfs, struct timespec *tfe, char *argv, char *ip_addr, int ttl_val, char *ping_domain)
{
    struct icmp_packet  *r_pckt;
    struct icmp_packet  *s_pckt;
    struct timespec     time_start;
    struct sockaddr_in  r_addr;
    struct timespec     time_end;
    long unsigned int   i;
    int                 pckt_sent;
    int                 msg_count;
    int                 msg_received_count;
    char        r_buffer[400]; //receive buffer
    long double rtt_msec;
    long double total_msec;
    double      timeElapsed;

    total_msec = 0;
    rtt_msec = 0;
    pckt_sent = 1;
    msg_count = 0;
    msg_received_count = 0;

    memset(r_buffer, 0, sizeof(r_buffer));

    while (pingloop) {
        pckt_sent = 1; //was a packet sent or not
        bzero(&s_pckt, sizeof(s_pckt)); // filling packet
    printf("here 2\n");

        s_pckt->hdr.icmp_type = ICMP_ECHO; // Message Type (8 bits)
        s_pckt->hdr.icmp_code = 0; // Message Code (8 bits): echo request
        s_pckt->hdr.icmp_id = getpid(); // Identifier (16 bits): some number to trace the response
        s_pckt->hdr.icmp_seq = msg_count++; // Sequence Number (16 bits): starts at 0
        s_pckt->hdr.icmp_cksum = checksum(&s_pckt, sizeof(s_pckt));
    printf("here 3\n");
        usleep(PING_SLEEP_RATE);
    printf("here 4\n");

        // send packet
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        if (sendto(raw_sockfd, &s_pckt, sizeof(s_pckt), 0, (struct sockaddr*)ping_addr, sizeof(*ping_addr)) < 0) {
            printf("\nPacket Sending Failed !\n");
            pckt_sent = 0;
        }
    printf("here 5\n");

        // receive packet
        if (recvfrom(raw_sockfd, r_buffer, sizeof(r_buffer), 0, (struct sockaddr*)&r_addr, (socklen_t*)(sizeof(r_addr))) < 0 && msg_count > 1) {
            printf("\nPacket receive failed !\n");
        }
        else {
            clock_gettime(CLOCK_MONOTONIC, &time_end);

            double timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec)) / 1000000.0;
            rtt_msec = (time_end.tv_sec - time_start.tv_sec) * 1000.0 + timeElapsed;

            // if packet was not sent, don't receive
            if (pckt_sent) {
                if (!(r_pckt->hdr.icmp_type == 0 && r_pckt->hdr.icmp_code == 0)) {
                    printf(" Error..Packet receive with ICMP type % d code % d\n", r_pckt->hdr.type, r_pckt->hdr.code);
                }
                else {
                    printf(" %d bytes from %s(h: %s)(%s) msg_seq = %d ttl = %d rtt = %Lf ms.\n", PING_PKT_S, ping_domain, argv, ip_addr, msg_count, ttl_val, rtt_msec);
                    msg_received_count++;
                }
            }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &(*tfe));
    timeElapsed = ((double)(tfe->tv_nsec - tfs->tv_nsec)) / 1000000.0;
    total_msec = (tfe->tv_sec - tfs->tv_sec) * 1000.0 + timeElapsed;
    printf("\n == = %s ping statistics ===\n", ip_addr);
    printf("\n %d packets sent, %d packets received, %f percent packet loss.Total time: %Lf ms.\n\n", msg_count, msg_received_count, ((msg_count - msg_received_count) / msg_count) * 100.0, total_msec);
}

void    send_ping(int raw_sockfd, struct sockaddr_in *ping_addr, char *ping_domain, char *ip_addr, char *argv)
{
    struct timespec     tfs;
    struct timespec     tfe;
    struct timeval      tv_out;
    int                 ttl_val;

    ttl_val = 64; //ttl that will decrease 
    tv_out.tv_sec = RECV_TIMEOUT;
    tv_out.tv_usec = 0;

    clock_gettime(CLOCK_MONOTONIC, &tfs); // get time data

    // set socket options ??
    if (setsockopt(raw_sockfd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val))!= 0) {
        printf("Setting socket options to TTL failed !\n");
        return;
    }
    printf("Socket set to TTL\n");
    // setting timeout of recv setting
    setsockopt(raw_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out);
    printf("here 1\n");

    // send icmp packet in an infinite loop
    icmp_loop(raw_sockfd, ping_addr, &tfs, &tfe, argv, ip_addr, ttl_val, ping_domain);
}
 