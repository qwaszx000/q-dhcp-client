#include<sys/ioctl.h>
#include<sys/time.h>
#include<net/if.h>
#include<linux/if_arp.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<arpa/inet.h>

#include"./structures.h"
#include"./defines.h"

#define PTYPE_IPV4 0x800

extern ARGS_OPTIONS options;
extern char ip[IP_ADDR_LEN];

static RAW_ARP arp;
static int tmp;
static int s;

static int create_arp_socket(char *hwaddr){
	s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if(-1 == s){
		perror("socket error");
		return -1;
	}

	struct ifreq req;
	memcpy(req.ifr_name, options.interface_name, HWADDR_LEN);
	tmp = ioctl(s, SIOCGIFINDEX, &req);
	if(-1 == tmp){
		perror("ioctl get ifindex error");
		return -1;
	}
	
	struct sockaddr_ll bind_ll;
	bind_ll.sll_family = AF_PACKET;
	bind_ll.sll_protocol = htons(ETH_P_ARP);
	bind_ll.sll_ifindex = req.ifr_ifindex;
	bind_ll.sll_halen = HWADDR_LEN;
	bind_ll.sll_hatype = ARPHRD_ETHER;
	bind_ll.sll_pkttype = 0;
	memcpy(bind_ll.sll_addr, hwaddr, HWADDR_LEN);
	
	tmp = bind(s, (struct sockaddr*)&bind_ll, sizeof(bind_ll));
	if(-1 == tmp){
		perror("bind error");
		return -1;
	}

	struct timeval recv_timeout;
	recv_timeout.tv_sec = 1;
	recv_timeout.tv_usec = 0;
	tmp = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,
		&recv_timeout, sizeof(recv_timeout));
	if(-1 == tmp){
		perror("setsockopt recv timeout setting error");
		return -1;
	}
}

static void prep_arp_req(char *src_hwaddr){
	memset(&arp, 0, sizeof(arp));
	memset(arp.dst_hwaddr, 0xff, HWADDR_LEN);
	memcpy(arp.src_hwaddr, src_hwaddr, HWADDR_LEN);
	arp.ether_type = htons(ETH_P_ARP);
	arp.htype = htons(ARPHRD_ETHER);
	arp.ptype = htons(PTYPE_IPV4);
	arp.hlen = htons(HWADDR_LEN);
	arp.plen = htons(IP_ADDR_LEN);
	arp.op = htons(ARPOP_REQUEST);
	memcpy(arp.sha, src_hwaddr, HWADDR_LEN);
	memcpy(arp.tpa, ip, IP_ADDR_LEN);
	//*(uint32_t*)&arp.tpa = inet_addr("192.168.0.1"); /*TEST PURPOSE*/
}

int check_arp(char *src_hwaddr){
	if(-1 == create_arp_socket(src_hwaddr)){
		printf("ARP check: socket creation error\n");
		return -1;
	}

	prep_arp_req(src_hwaddr);

	tmp = send(s, &arp, sizeof(arp), 0);
	if(-1 == tmp){
		perror("ARP request sending error");
		close(s);
		return -1;
	}

	tmp = recv(s, &arp, sizeof(arp), 0);
	if(-1 == tmp){
		if(EAGAIN == errno || EWOULDBLOCK == errno){
			close(s);
			return ARP_FREE_IP;
		} else {
			perror("ARP request recv error");
			close(s);
			return -1;
		}
	}

	if(ARPOP_REPLY == ntohs(arp.op)){
		close(s);
		return ARP_OCCUPIED_IP;
	}
}
