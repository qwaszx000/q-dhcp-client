#include<stdio.h>
#include<unistd.h>
#include"./structures.h"
#include"./args_handler.h"
#include"./netif.h"
#include"./helper.h"
#include"./defines.h"
#include"./dhcp_protocol_handler.h"
#include"./arp_checker.h"

int main(int argc, char **argv)
{
	ARGS_OPTIONS options = {0, 0, 0, 0, 0, NULL};
	DHCP_RESULT dhcp_res;
	char hwaddr[HWADDR_LEN];
	int main_socket, tmp;

	process_args(argc, argv, &options);
	
	main_socket = create_dhcp_socket(&options); 
	get_interface_hwaddr(main_socket, hwaddr, &options);

	if(options.release){
		get_interface_ip(main_socket, &options, &dhcp_res);
		dhcp_release(main_socket, hwaddr, &options, &dhcp_res);
		goto done;
	}

	perform_dhcp(main_socket, hwaddr, &options, &dhcp_res);
	if(MSGTYPE_NAK == dhcp_res.answer){
		printf("Got DHCP_NAK, aborting...\n");
		close(main_socket);
		return -2;
	}
	

	if(options.arp_check){
		tmp = check_arp(hwaddr, &options, &dhcp_res);
		if(ARP_OCCUPIED_IP == tmp){
			printf("IP already occupied\n");
			dhcp_decline(main_socket, hwaddr, &options, &dhcp_res);
			close(main_socket);
			return -2;
		} else if(-1 == tmp){
			printf("ARP check error\n");
			close(main_socket);
			return -1;
		}
	}

done:	if(!options.dry){
		update_interface(main_socket, &options, &dhcp_res);
	}

	/*IP:MASK:GATEWAY:LEASE TIME*/
	printf("%hhu.%hhu.%hhu.%hhu:%hhu.%hhu.%hhu.%hhu:"
"%hhu.%hhu.%hhu.%hhu:%d\n",
dhcp_res.ip[0], dhcp_res.ip[1], dhcp_res.ip[2], dhcp_res.ip[3],
dhcp_res.net_mask[0], dhcp_res.net_mask[1], dhcp_res.net_mask[2], dhcp_res.net_mask[3],
dhcp_res.router_ip[0], dhcp_res.router_ip[1], dhcp_res.router_ip[2], dhcp_res.router_ip[3],
dhcp_res.lease_secs);

	close(main_socket);
	return 0;
}
