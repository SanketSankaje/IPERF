#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/ioctl.h>


#define SERVER 1
#define CLIENT 2
#define ERROR -1

#define SUCCESS 0
#define FAILURE -1

#define MAC_ADDR_LEN 18
#define MTU 1500


struct ethdr {
    unsigned short h_dest[6];
    unsigned short h_source[6];
    unsigned short h_proto;
};

struct iphdr {
    unsigned char h_vhl : 4;
    unsigned char version : 4;
    unsigned char tos;
    unsigned short total_len;
    unsigned short id;
    unsigned short frag_off;
    unsigned char ttl;
    unsigned char protocol;
    unsigned short check;
    struct in_addr saddr;
    struct in_addr daddr;
};

struct PACKET {
    struct iphdr hdr;
    char buf[MTU - sizeof(struct iphdr)];
};

#define MULTIPLE_WRITE(_fd, _buf, _cnt)\
unsigned int count = _cnt;\
do {\
    count -= write(_fd, _buf, MTU);\
    if (count <= 0) break;\
} while (1);



void print_suggestions();
void Fill_IP_PKT(struct PACKET *pkt, struct iphdr*, char *sip, char *dip, char *data);
int Configure(char *type, char *if_name, char *proto, char *dst_addr);
int GetConnection(struct sockaddr_in *dst_addr, int *sockfd);
char *get_ip(char *if_name);
char *get_mac(char *if_name, int  sockfd);
int start_tcp_server(int *fd);
int start_tcp_client(int *fd, char *dst_addr, char *src_addr);
int start_udp_server(int *fd);
int start_udp_client(int *fd, char *dst_addr, char *src_addr);
