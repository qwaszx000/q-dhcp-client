#ifndef NETIF
#define NETIF

extern void get_interface_hwaddr(const int s, char *dst);

extern void update_interface(const int s);

#endif
