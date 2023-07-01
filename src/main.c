#include<stdio.h>
#include<unistd.h>
#include"./structures.h"
#include"./args_handler.h"
#include"./netif.h"
#include"./helper.h"
#include"./defines.h"
#include"./dhcp_protocol_handler.h"
#include"./arp_checker.h"

extern ARGS_OPTIONS options;
extern int lease_secs;
extern uint8_t dhcp_result;

static char hwaddr[HWADDR_LEN];

int main(unsigned int args, char **argv){
	process_args(args, argv);
	
	const int main_socket = create_dhcp_socket(); 
	get_interface_hwaddr(main_socket, hwaddr);	

	if(options.release){
		dhcp_release(main_socket, hwaddr);
		goto done;
	}

	perform_dhcp(main_socket, hwaddr);
	printf("%d\n", lease_secs);

	if(MSGTYPE_NAK == dhcp_result){
		printf("Got DHCP_NAK, aborting...\n");
		close(main_socket);
		return -2;
	}
	

	if(options.arp_check){
		int tmp = check_arp(hwaddr);
		if(ARP_OCCUPIED_IP == tmp){
			printf("IP already occupied\n");
			dhcp_decline(main_socket, hwaddr);
			close(main_socket);
			return -2;
		} else if(-1 == tmp){
			printf("ARP check error\n");
			close(main_socket);
			return -1;
		}
	}

done:	if(!options.dry){
		update_interface(main_socket);
	}

	close(main_socket);
	return 0;
}
