#include "support.h"
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>

void add_time_stamp(struct PACKET *pkt) {
    time_t t;
    struct tm* tm1;

    t = time(NULL);
    tm1 = localtime(&t);
    strftime(pkt->time_stamp, 26, "%Y-%m-%d %H:%M:%S", tm1);
}

char *get_ip(char *if_name) {
    char *ip_address = NULL;
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;

            char ipaddr[INET_ADDRSTRLEN];

            inet_ntop(AF_INET, &sa->sin_addr, ipaddr, INET_ADDRSTRLEN);

            if (strcmp(ifa->ifa_name, if_name) == 0) {
                ip_address = strdup(&ipaddr[0]);
                break;
            }
        }
    }
    return ip_address;
}


char *get_mac(char *if_name, int socketfd) {
    char *mac_address = NULL;
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr; 

            char ipaddr[INET_ADDRSTRLEN];

            inet_ntop(AF_INET, &sa->sin_addr, ipaddr, INET_ADDRSTRLEN);

            if (strcmp(ifa->ifa_name, if_name) == 0) {
                struct ifreq ifr;
                memset(&ifr, 0, sizeof(ifr));
                strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ-1);

                if (ioctl(socketfd, SIOCGIFHWADDR, &ifr) == 0) {
                    char macaddr[MAC_ADDR_LEN];
                    sprintf(macaddr, "%02x:%02x:%02x:%02x:%02x:%02x",
                            (unsigned char)ifr.ifr_hwaddr.sa_data[0],
                            (unsigned char)ifr.ifr_hwaddr.sa_data[1],
                            (unsigned char)ifr.ifr_hwaddr.sa_data[2],
                            (unsigned char)ifr.ifr_hwaddr.sa_data[3],
                            (unsigned char)ifr.ifr_hwaddr.sa_data[4],
                            (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
                    mac_address = strdup(macaddr);
                    break;
                } else {
                    perror("ioctl");
                }
            }
        }
    }
    return mac_address;
}

void Fill_IP_PKT(struct PACKET *pkt, struct iphdr *ip, char *sip, char *dip, char *data) {
    int buf_len = sizeof(data);
    ip->h_vhl = 5;                // Header Length (5 words)
    ip->version = 4;               // IP Version (4)
    ip->tos = 0;                  // Type of Service (default)
    ip->total_len = sizeof(struct iphdr) + sizeof(data); // Total Length
    ip->id = htons(54321);        // Identification (arbitrary)
    ip->frag_off = 0;               // Fragment Offset (no fragmentation)
    ip->ttl = 64;                 // Time to Live (64 seconds)
    ip->protocol = IPPROTO_IP;  // Protocol (ICMP)
    ip->saddr.s_addr = inet_addr(sip); // Source IP Address
    ip->daddr.s_addr = inet_addr(dip);  // Destination IP Address

    // pkt->hdr = (struct iphdr*)strndup((char *)ip, sizeof(*ip));
    // pkt->buf = strdup(data);
    memcpy(&pkt->hdr, ip, sizeof(struct iphdr));
    memcpy(&pkt->buf, data, buf_len);
}

int GetConnection(struct sockaddr_in *dst_addr, int *sockfd) {
    int status;
    status = connect(*sockfd, (struct sockaddr *)dst_addr, sizeof(*dst_addr));
    return status;
}

void print_suggestions(void) {
    printf("Options:\nArg[0]  executable\nArg[1]  -s -> server (OR) -c -> client\nArg[2]  interface_name\nArg[3]  -tcp -> TCP (OR) -udp -> UDP\nArg[4]  Dest IP incase of CLIENT application else NULL\n");
}

int start_tcp_server(int *fd) {
    struct sockaddr_in saddr, daddr;
    int new_fd;
    int opt = 1;
    int addr_len = sizeof(struct sockaddr);
    if (setsockopt(*fd, SOL_SOCKET,
                SO_REUSEADDR | SO_REUSEPORT, &opt,
                sizeof(opt))) {
        perror("setsockopt");
        close(*fd);
        return FAILURE;
    }
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_family = AF_INET;
    saddr.sin_port = 5201;
    bind(*fd, (struct sockaddr *)&saddr, sizeof(saddr));
    listen(*fd, 3);
    printf("Server running .....\n");
    char *buf = malloc(sizeof(char) *255);
    memset(buf, 0, 255);
    while (1) {
        if ((new_fd = accept(*fd, (struct sockaddr *)&daddr, (socklen_t*)&addr_len)) < 0) {
            perror("accept");
            close(*fd);
            return FAILURE;
        }
        while(read(new_fd, buf, MTU) > 0) {
            struct PACKET *pkt = (struct PACKET *)buf;
            printf("%s, from %s to %s at %s\n", pkt->buf, inet_ntoa(pkt->hdr.daddr), inet_ntoa(pkt->hdr.saddr), pkt->time_stamp);
            memset(buf, 0, 255);
        }
        close(new_fd);
    }
    free(buf);

    return SUCCESS;
}

int start_tcp_client(int *fd, char *dst_addr, char *src_addr) {
    struct sockaddr_in daddr;
    struct PACKET *pkt;
    struct iphdr *ip_hdr;
    if (dst_addr != NULL) {
        daddr.sin_family = AF_INET;
        daddr.sin_port = 5201;
        if (inet_pton(AF_INET, dst_addr, &daddr.sin_addr) <= 0) {
            perror("Invalid address or address not supported\n");
            close(*fd);
            return FAILURE;
        }
    }
    if (GetConnection(&daddr,fd) < 0) {
        perror("Connection Failed");
        close(*fd);
        return FAILURE;
    }
    printf("Client running ......\n");
    char buf[255] = "hello";
    ip_hdr = calloc(1, sizeof(*ip_hdr));
    pkt = calloc(1, sizeof(*pkt));
    Fill_IP_PKT(pkt, ip_hdr, src_addr, dst_addr, buf);
    add_time_stamp(pkt);
    write(*fd, pkt, MTU);
    printf("%s, from %s to %s at %s\n", pkt->buf, inet_ntoa(pkt->hdr.daddr), inet_ntoa(pkt->hdr.saddr), pkt->time_stamp);
    free(pkt);
    free(ip_hdr);
    return SUCCESS;
}

int start_udp_server(int *fd) {
    struct sockaddr_in saddr;
    int opt = 1;
    if (setsockopt(*fd, SOL_SOCKET,
                SO_REUSEADDR | SO_REUSEPORT, &opt,
                sizeof(opt))) {
        perror("setsockopt");
        close(*fd);
        return FAILURE;
    }
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_family = AF_INET;
    saddr.sin_port = 5201;
    bind(*fd, (struct sockaddr *)&saddr, sizeof(saddr));
    printf("Server running .....\n");
    char *buf = malloc(MTU + 1);
    memset(buf, 0, 255);
    while(read(*fd, buf, MTU) > 0) {
        struct PACKET *pkt = (struct PACKET *)buf;
        printf("%s, from %s to %s at %s\n", pkt->buf, inet_ntoa(pkt->hdr.daddr), inet_ntoa(pkt->hdr.saddr), pkt->time_stamp);
        memset(buf, 0, 255);
    }
    free(buf);

    return SUCCESS;
}

int start_udp_client(int *fd, char *dst_addr, char *src_addr) {
    struct sockaddr_in daddr;
    struct PACKET *pkt;
    struct iphdr *ip_hdr;
    if (dst_addr != NULL) {
        daddr.sin_family = AF_INET;
        daddr.sin_port = 5201;
        if (inet_pton(AF_INET, dst_addr, &daddr.sin_addr) <= 0) {
            perror("Invalid address or address not supported\n");
            close(*fd);
            return FAILURE;
        }
    }
    if (GetConnection(&daddr,fd) < 0) {
        perror("Connection Failed");
        close(*fd);
        return FAILURE;
    }
    printf("Client running ......\n");
    char buf[255] = "hello";
    pkt = calloc(1, sizeof(*pkt));
    ip_hdr = calloc(1, sizeof(*ip_hdr));
    Fill_IP_PKT(pkt, ip_hdr, src_addr, dst_addr, buf);
    add_time_stamp(pkt);
    write(*fd, pkt, MTU);
    printf("%s, from %s to %s at %s\n", pkt->buf, inet_ntoa(pkt->hdr.daddr), inet_ntoa(pkt->hdr.saddr), pkt->time_stamp);
    free(pkt);
    free(ip_hdr);
    return SUCCESS;
}

int Configure(char *APPtype, char *if_name, char *proto, char *dst_addr) {
    int fd;
    char *ip_a, *mac_a;
    if (strcmp(proto, "-udp") == 0)
        fd = socket(AF_INET, SOCK_DGRAM, 0);
    else if (strcmp(proto, "-tcp") == 0)
        fd = socket(AF_INET, SOCK_STREAM, 0);
    else {
        perror("ERROR in opening socket!!!!");
        return FAILURE;
    }
    ip_a = get_ip(if_name);
    if (ip_a == NULL) {printf("Invalid interface\n"); return FAILURE;}
    mac_a = get_mac(if_name, fd);
    if (mac_a == NULL) {printf("Invalid interface\n"); return FAILURE;}

    printf("IP address of %s: %s\n", if_name, ip_a);
    printf("MAC address of %s: %s\n", if_name, mac_a);

    if (strcmp(APPtype, "-s") == 0) {
        if (strcmp(proto, "-udp") == 0) {
            if (start_udp_server(&fd) < 0) {
                printf("Cannot start server\n");
                return FAILURE;
            }
        } else if (start_tcp_server(&fd) < 0) {
            printf("Cannot start server\n");
            return FAILURE;
        }
    } else if (strcmp(APPtype, "-c") == 0) {
        if (strcmp(proto, "-udp") == 0) {
            if (start_udp_client(&fd, dst_addr, ip_a) < 0) {
                printf("Cannot start client\n");
                return FAILURE;
            }
        } else if (start_tcp_client(&fd, dst_addr, ip_a) < 0) {
            printf("Cannot start client\n");
            return FAILURE;
        }
    } else {
        print_suggestions();
        close(fd);
        return FAILURE;
    }

    close(fd);
    return SUCCESS;
}
