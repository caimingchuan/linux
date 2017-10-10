#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <sys/un.h>
#include <time.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include <pthread.h>

#ifdef QNX_SYSTEM
	#include <net/if_ether.h>
	#include <machine/endian.h>
	#include <netinet/in.h>
	#include <net/if_dl.h>
	#define AF_PACKET AF_INET
	#define ETH_P_IP	ETHERTYPE_IP
	#define ETH_P_ARP 	ETHERTYPE_ARP
	#define ETH_P_RARP	ETHERTYPE_REVARP
	#define ETH_P_ALL	ETHERTYPE_8023
#else
	#include <linux/if_ether.h>
	#include <linux/net.h>	
	#include <net/ethernet.h>	
	#include <netpacket/packet.h>	
	#include <asm/types.h>
	#include <endian.h>	
	#include <byteswap.h>
#endif

int rarp_count = 0;
int ip_count = 0;
int arp_count = 0;
int all_count = 0;

pthread_mutex_t ip_mutex;
pthread_mutex_t arp_mutex;
pthread_mutex_t rarp_mutex;
pthread_mutex_t all_mutex;


int recv_count = 0;
int send_count = 0;

void set_timer(int interval)
{ /* set the timing interval */

        struct itimerval itv, oldtv;
        itv.it_interval.tv_sec = interval;
        itv.it_interval.tv_usec = 0;
        itv.it_value.tv_sec = interval;
        itv.it_value.tv_usec = 0;
        setitimer(ITIMER_REAL, &itv, &oldtv);
}

void time_handle()
{
	int i;
	
#ifndef QNX_SYSTEM
	system("cat /proc/net/dev");
#endif

	pthread_mutex_lock(&ip_mutex);
	pthread_mutex_lock(&arp_mutex);
	pthread_mutex_lock(&rarp_mutex);
	pthread_mutex_lock(&all_mutex);

	recv_count = rarp_count + ip_count + arp_count;
	send_count = all_count - recv_count;

	pthread_mutex_unlock(&ip_mutex);
	pthread_mutex_unlock(&arp_mutex);
	pthread_mutex_unlock(&rarp_mutex);
	pthread_mutex_unlock(&all_mutex);

	printf("recv is %d\n", recv_count);
	printf("send is %d\n", send_count);	
}

void *ip_count_thread(void *arg)
{
	int sock;
	struct ifreq ifstruct;

#ifdef QNX_SYSTEM
	struct sockaddr_dl sll;
#else
	struct sockaddr_ll sll;
#endif
	
	struct sockaddr_in addr;
	char buf[2000];
	int r;
	int len;
	len = sizeof(addr);
	//recv: ETH_P_IP ETH_P_ARP ETH_P_RARP
	//recv and send: ETH_P_ALL
	
	printf("%s: ++\n", __func__);
	if((sock = socket(AF_PACKET, SOCK_RAW, /*htons(ETH_P_IP)*/0)) == -1)  
	{
		printf("%s: %s\n", __func__, strerror(errno));
		return NULL;
	}
	printf("%s: ++\n", __func__);

#ifndef QNX_SYSTEM
	sll.sll_family = AF_PACKET;
	sll.sll_protocol = htons(ETH_P_IP);
	strcpy(ifstruct.ifr_name, "eth0");
	ioctl(sock, SIOCGIFINDEX, &ifstruct);
	sll.sll_ifindex = ifstruct.ifr_ifindex;
#else
	sll.sdl_len = sizeof(sll);
	sll.sdl_family = AF_PACKET;
	sll.sdl_index = if_nametoindex("tiw_sta0");
	sll.sdl_type = htons(ETH_P_IP);
	printf("%s: %s\n", __func__, if_indextoname(sll.sdl_index, buf));
#endif

	if (-1 == bind(sock, (struct sockaddr*)&sll,sizeof(sll)))
	{
		printf("%s: %s\n", __func__, strerror(errno));
	}
	ip_count=0;
	for(; ;)
	{
		r = recvfrom(sock, (char *)buf, sizeof(buf), 0, (struct sockaddr *)&addr, &len);
		if (r == -1)
		{
			perror("IP: ");
			break;
		}
		printf("%s : r %d\n", __func__, r);
		pthread_mutex_lock(&ip_mutex);
		ip_count += r;
		pthread_mutex_unlock(&ip_mutex);
	}
	return arg;
}

void *arp_count_thread(void *arg)
{
	int sock;
	struct ifreq ifstruct;
	
#ifdef QNX_SYSTEM
	struct sockaddr_dl sll;
#else
	struct sockaddr_ll sll;
#endif

	struct sockaddr_in addr;
	char buf[2000];
	int r;
	int len;
	len = sizeof(addr);
	//recv: ETH_P_IP ETH_P_ARP ETH_P_RARP
	//recv and send: ETH_P_ALL
	
	printf("%s: ++\n", __func__);
	if((sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP))) == -1)  
	{
		printf("%s: %s\n", __func__, strerror(errno));
		return NULL;
	}
	
	printf("%s: ++\n", __func__);
#ifndef QNX_SYSTEM
	sll.sll_family = AF_PACKET;
	sll.sll_protocol = htons(ETH_P_ARP);

	strcpy(ifstruct.ifr_name, "eth0");
	ioctl(sock, SIOCGIFINDEX, &ifstruct);
	sll.sll_ifindex = ifstruct.ifr_ifindex;
#else
	sll.sdl_family = AF_PACKET;
	sll.sdl_index = if_nametoindex("tiw_sta0");
	sll.sdl_type = htons(ETH_P_ARP);
	printf("%s: %s\n", __func__, if_indextoname(sll.sdl_index, buf));
#endif
	
	if (-1 == bind(sock, (struct sockaddr*)&sll,sizeof(sll)))
	{
		printf("%s: %s\n", __func__, strerror(errno));
	}

	arp_count=0;
	for(; ;)
	{
		r = recvfrom(sock, (char *)buf, sizeof(buf), 0, (struct sockaddr *)&addr, &len);
		if (r == -1)
		{
			perror("ARP: ");
			break;
		}
		
		printf("%s : r %d\n", __func__, r);
		pthread_mutex_lock(&arp_mutex);
		arp_count += r;
		pthread_mutex_unlock(&arp_mutex);
	}

	return arg;
}

void *rarp_count_thread(void *arg)
{
	int sock;
	struct ifreq ifstruct;
#ifdef QNX_SYSTEM
	struct sockaddr_dl sll;
#else
	struct sockaddr_ll sll;
#endif

	struct sockaddr_in addr;
	char buf[2000];
	int r;
	int len;
	len = sizeof(addr);
	//recv: ETH_P_IP ETH_P_ARP ETH_P_RARP
	//recv and send: ETH_P_ALL
	
	printf("%s: ++\n", __func__);
	if((sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_RARP))) == -1)  
	{
		printf("%s: %s\n", __func__, strerror(errno));
		return NULL;
	}
	
	printf("%s: ++\n", __func__);
#ifndef QNX_SYSTEM
	sll.sll_family = AF_PACKET;
	sll.sll_protocol = htons(ETH_P_RARP);

	strcpy(ifstruct.ifr_name, "eth0");
	ioctl(sock, SIOCGIFINDEX, &ifstruct);
	sll.sll_ifindex = ifstruct.ifr_ifindex;
#else
	sll.sdl_family = AF_PACKET;
	sll.sdl_index = if_nametoindex("tiw_sta0");
	sll.sdl_type = htons(ETH_P_RARP);
	printf("%s: %s\n", __func__, if_indextoname(sll.sdl_index, buf));
#endif

	if (-1 == bind(sock, (struct sockaddr*)&sll,sizeof(sll)))
	{
		printf("%s: %s\n", __func__, strerror(errno));
	}
	rarp_count=0;
	for(; ;)
	{
		r = recvfrom(sock, (char *)buf, sizeof(buf), 0, (struct sockaddr *)&addr, &len);
		if (r == -1)
		{
			perror("RARP: ");
			break;
		}
		
		printf("%s : r %d\n", __func__, r);
		pthread_mutex_lock(&rarp_mutex);
		rarp_count += r;
		pthread_mutex_unlock(&rarp_mutex);
	}

	return arg;
}

void *all_count_thread(void *arg)
{
	int sock;
	struct ifreq ifstruct;
#ifdef QNX_SYSTEM
	struct sockaddr_dl sll;
#else
	struct sockaddr_ll sll;
#endif
	struct sockaddr_in addr;
	char buf[2000];
	int r;
	int len;
	len = sizeof(addr);
	//recv: ETH_P_IP ETH_P_ARP ETH_P_RARP
	//recv and send: ETH_P_ALL
	
	printf("%s: ++\n", __func__);
	if((sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1)  
	{
		printf("%s: %s\n", __func__, strerror(errno));
		return NULL;
	}
	printf("%s: ++\n", __func__);
#ifndef QNX_SYSTEM
	sll.sll_family = AF_PACKET;
	sll.sll_protocol = htons(ETH_P_ALL);

	strcpy(ifstruct.ifr_name, "eth0");
	ioctl(sock, SIOCGIFINDEX, &ifstruct);
	sll.sll_ifindex = ifstruct.ifr_ifindex;
#else
	sll.sdl_family = AF_PACKET;
	sll.sdl_index = if_nametoindex("tiw_sta0");
	sll.sdl_type = htons(ETH_P_ALL);
	printf("%s: %s\n", __func__, if_indextoname(sll.sdl_index, buf));
#endif

	if (-1 == bind(sock, (struct sockaddr*)&sll,sizeof(sll)))
	{
		printf("%s: %s\n", __func__, strerror(errno));
	}
	all_count=0;
	for(; ;)
	{
		r = recvfrom(sock, (char *)buf, sizeof(buf), 0, (struct sockaddr *)&addr, &len);
		if (r == -1)
		{
			perror("ALL: ");
			break;
		}
		
		printf("%s : r %d\n", __func__, r);
		pthread_mutex_lock(&all_mutex);
		all_count += r;
		pthread_mutex_unlock(&all_mutex);
	}

	return arg;
}

int main(int argc, char *argv[])
{
	pthread_t ip_pid;
	pthread_t arp_pid;
	pthread_t rarp_pid;
	pthread_t all_pid;
	printf("%s %s\n", __TIME__, __DATE__);

	pthread_mutex_init(&ip_mutex, NULL);
	pthread_mutex_init(&arp_mutex, NULL);
	pthread_mutex_init(&rarp_mutex, NULL);
	pthread_mutex_init(&all_mutex, NULL);

	pthread_create(&ip_pid, NULL, ip_count_thread, NULL);
	usleep(500 * 1000);
	pthread_create(&arp_pid, NULL, arp_count_thread, NULL);
	usleep(500 * 1000);
	pthread_create(&rarp_pid, NULL, rarp_count_thread, NULL);
	usleep(500 * 1000);
	pthread_create(&all_pid, NULL, all_count_thread, NULL);
	
	signal(SIGALRM, time_handle);

	set_timer(1);

	while (1)
	{
		sleep(10);
	}
	
	return 0;
}

