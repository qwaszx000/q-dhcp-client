#include<sys/ioctl.h>
#include<net/route.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"./structures.h"
#include"./defines.h"

extern ARGS_OPTIONS options;
extern char ip[IP_ADDR_LEN];
extern char net_mask[IP_ADDR_LEN];
extern char rip[IP_ADDR_LEN];
extern int lease_secs;

static struct ifreq req;

#define PRINT_IP(_var) "%hhu.%hhu.%hhu.%hhu\n", _var[0], _var[1],\
				_var[2], _var[3]

void get_interface_hwaddr(const int s, char *dst){
	strncpy(req.ifr_name, options.interface_name, IFNAMSIZ);
	if(-1 == ioctl(s, SIOCGIFHWADDR, &req)){
		perror("ioctl error in get_interface_hwaddr");
		exit(-1);
	}	

	memcpy(dst, req.ifr_hwaddr.sa_data, HWADDR_LEN);
	if(options.debug){
		printf("Hwaddr of interface %s = "
			"%.2hhX:%.2hhX:%.2hhX:%.2hhX:%.2hhX:%.2hhX\n",
			options.interface_name,
			dst[0], dst[1], dst[2], dst[3], dst[4], dst[5]);
	}
}

void update_interface(const int s){
	struct sockaddr_in *tmp_in;
	tmp_in = (struct sockaddr_in*)&req.ifr_addr;
	tmp_in->sin_family = AF_INET;

	tmp_in->sin_addr.s_addr = options.release ? 0 : *(uint32_t*)ip;
	if(-1 == ioctl(s, SIOCSIFADDR, &req)){
		perror("ioctl set ip error");
		exit(-1);
	}
	if(options.release) return;

	tmp_in->sin_addr.s_addr = *(uint32_t*)net_mask;
	if(-1 == ioctl(s, SIOCSIFNETMASK, &req)){
		perror("ioctl set netmask error");
		exit(-1);
	}


	struct rtentry rt;
	memset(&rt, 0, sizeof(rt));

	tmp_in = (struct sockaddr_in*)&rt.rt_dst;
	tmp_in->sin_family = AF_INET;
	tmp_in->sin_addr.s_addr = INADDR_ANY;

	tmp_in = (struct sockaddr_in*)&rt.rt_genmask;
	tmp_in->sin_family = AF_INET;
	tmp_in->sin_addr.s_addr = INADDR_ANY;

	tmp_in = (struct sockaddr_in*)&rt.rt_gateway;
	tmp_in->sin_family = AF_INET;
	tmp_in->sin_addr.s_addr = *(uint32_t*)rip;

	rt.rt_flags = RTF_UP | RTF_GATEWAY;
	rt.rt_dev = options.interface_name;

	if(-1 == ioctl(s, SIOCADDRT, &rt)){
		perror("ioctl set route error");
		exit(-1);
	}

	if(options.debug){
		printf("IP addr set to: " PRINT_IP(ip));
		printf("Subnet mask: " PRINT_IP(net_mask));
		printf("Gateway IP: " PRINT_IP(rip));
		printf("Lease hours: %d\n", lease_secs/60/60);
	}
}

