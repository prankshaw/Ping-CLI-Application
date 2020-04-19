// Header files
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <time.h> 
#include <signal.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/ip_icmp.h> 
#include <netinet/in.h> 
#include <netdb.h> 
#include <fcntl.h> 
#include <arpa/inet.h> 
#include <unistd.h> 

#define TIMEOUT_RECEIVE_PACKET 1 //Timeout delay for receiving packets( In Seconds)
#define PORT_NO 0 
#define SIZE_PING_PACKET 64  
#define PING_SLEEP_RATE 1000000 // Sleep Rate of Ping

int loop_ping=1; 

struct ping_pkt // Defining Ping packet structure 
{ 
	struct icmphdr hdr; 
	char msg[SIZE_PING_PACKET-sizeof(struct icmphdr)]; 
}; 

void intHandler(int dummy) //Defining Interrupt handler 
{ 
	loop_ping=0; 
} 

unsigned short checksum(void *buffered, int len) // Calculating the Check Sum 
{ unsigned short *buf = buffered; 
	unsigned int sum=0; 
	unsigned short result; 

	for ( sum = 0; len > 1; len -= 2 ) 
		sum += *buf++; 
	if ( len == 1 ) 
		sum += *(unsigned char*)buf; 
	sum = (sum >> 16) + (sum & 0xFFFF); 
	sum += (sum >> 16); 
	result = ~sum; 
	return result; 
} 

char *dns_lookup(char *addr_host, struct sockaddr_in *addr_con) // Performing a DNS lookup 
{ 
	struct hostent *host_entity; 
	char *ip=(char*)malloc(NI_MAXHOST*sizeof(char)); 
	int i; 

	if ((host_entity = gethostbyname(addr_host)) == NULL) 
	{ 
		return NULL; // If no ip is found for a hostname 
	} 

	strcpy(ip, inet_ntoa(*(struct in_addr *) host_entity->h_addr)); 

	(*addr_con).sin_family = host_entity->h_addrtype; 
	(*addr_con).sin_port = htons (PORT_NO); 
	(*addr_con).sin_addr.s_addr = *(long*)host_entity->h_addr; 

	return ip; 
	
} 

char* reverse_dns_lookup(char *ip_addr)  // Reverse DNS Lookup for the hostname 
{ 
	struct sockaddr_in temp_addr;	 
	socklen_t len; 
	char buf[NI_MAXHOST], *ret_buf; 

	temp_addr.sin_family = AF_INET; 
	temp_addr.sin_addr.s_addr = inet_addr(ip_addr); 
	len = sizeof(struct sockaddr_in); 

	if (getnameinfo((struct sockaddr *) &temp_addr, len, buf, sizeof(buf), NULL, 0, NI_NAMEREQD)) 
	{ 
		printf("Could not resolve reverse lookup of hostname\n"); 
		return NULL; 
	} 
	ret_buf = (char*)malloc((strlen(buf) +1)*sizeof(char) ); 
	strcpy(ret_buf, buf); 
	return ret_buf; 
} 

// To make a ping request 
void send_ping(int ping_sockfd, struct sockaddr_in *ping_addr, char *ping_dom, char *ping_ip, char *rev_host, int ttl_input, int ispresent_ttl) 
{ 
	int ttl_val=64, msg_count=0, i, addr_len, flag=1, 
			msg_received_count=0; 
	
	struct ping_pkt pckt; 
	struct sockaddr_in r_addr; 
	struct timespec time_start, time_end, tfs, tfe; 
	long double rtt_msec=0, total_msec=0; 
	struct timeval tv_out; 
	tv_out.tv_sec = TIMEOUT_RECEIVE_PACKET; 
	tv_out.tv_usec = 0; 

	clock_gettime(CLOCK_MONOTONIC, &tfs); 

	if (setsockopt(ping_sockfd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0) 
	{ 
		printf("\nSetting socket options to TTL failed!\n"); 
		return; 
	} 

	setsockopt(ping_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out); 

	// To Send icmp packet in an infinite loop 
	while(loop_ping) 
	{ 
		flag=1; // To see if packet was sent or not 
	
		bzero(&pckt, sizeof(pckt)); 
		
		pckt.hdr.type = ICMP_ECHO; 
		pckt.hdr.un.echo.id = getpid(); 
		
		for ( i = 0; i < sizeof(pckt.msg)-1; i++ ) 
			pckt.msg[i] = i+'0'; 
		
		pckt.msg[i] = 0; 
		pckt.hdr.un.echo.sequence = msg_count++; 
		pckt.hdr.checksum = checksum(&pckt, sizeof(pckt)); 


		usleep(PING_SLEEP_RATE); 

		//send packet 
		clock_gettime(CLOCK_MONOTONIC, &time_start); 
		if ( sendto(ping_sockfd, &pckt, sizeof(pckt), 0, 
		(struct sockaddr*) ping_addr, 
			sizeof(*ping_addr)) <= 0) 
		{ 
			printf("\nPacket Sending Failed!\n"); 
			flag=0; 
		} 

		//receive packet 
		addr_len=sizeof(r_addr); 

		if ( recvfrom(ping_sockfd, &pckt, sizeof(pckt), 0, 
			(struct sockaddr*)&r_addr, &addr_len) <= 0 
			&& msg_count>1) 
		{ 
			printf("\nPacket receive failed!\n"); 
		} 

		else
		{ 
			clock_gettime(CLOCK_MONOTONIC, &time_end); 
			
			double timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec))/1000000.0 ;
			rtt_msec = (time_end.tv_sec- time_start.tv_sec) * 1000.0 + timeElapsed; 
			
			if(flag)  // if packet was not sent, don't receive 
			{ 
				if(!(pckt.hdr.type ==69 && pckt.hdr.code==0)) 
				{ 
					printf("Error..Packet received with ICMP type %d code %d\n", pckt.hdr.type, pckt.hdr.code); 
				} 
				else
				{ 
					printf("%d bytes from %s (h: %s) (%s) icmp_seq=%d ttl=%d rtt = %Lf ms.\n", SIZE_PING_PACKET, ping_dom, rev_host, ping_ip, msg_count, ttl_val, rtt_msec); 

					msg_received_count++; 

					if(ispresent_ttl==1)
					{
						if(ttl_input<=ttl_val)
						{
							printf("\n===TIME EXCEEDED===\n");
							break;
						}
					}

					if(msg_received_count%10==0)
					{
						clock_gettime(CLOCK_MONOTONIC, &tfe); 
						double timeElapsed = ((double)(tfe.tv_nsec - tfs.tv_nsec))/1000000.0; 
	
						total_msec = (tfe.tv_sec-tfs.tv_sec)*1000.0+ timeElapsed ;
					
						printf("\n===%s ping statistics===\n", ping_ip); 
						printf("\n%d packets sent, %d packets received, %d packets lost. Total time: %Lf ms.\n\n", msg_count, msg_received_count, (msg_count - msg_received_count), total_msec); 

					}	
					
				} 
			} 
		}	 
	} 
	clock_gettime(CLOCK_MONOTONIC, &tfe); 
	double timeElapsed = ((double)(tfe.tv_nsec - tfs.tv_nsec))/1000000.0; 
	
	total_msec = (tfe.tv_sec-tfs.tv_sec)*1000.0+ timeElapsed ;
					
	printf("\n===%s ping statistics===\n", ping_ip); 
	printf("\n%d packets sent, %d packets received, %d packets lost. Total time: %Lf ms.\n\n", msg_count, msg_received_count, (msg_count - msg_received_count), total_msec); 
} 

// MAIN Function
int main(int argc, char *argv[]) 
{ 
	int sockfd, ttl_input=-1,ispresent_ttl=0; 
	char *ip_addr, *reverse_hostname; 
	struct sockaddr_in addr_con; 
	int addrlen = sizeof(addr_con); 
	char net_buf[NI_MAXHOST]; 

	if(argc < 2) 
	{ 
		printf("\nToo few arguments-> %s\n", argv[0]); 
		return 0; 
	} 
	if(argc > 3) 
	{ 
		printf("\nToo many arguments\n"); 
		return 0; 
	} 
	if(argc==3)
	{
		char* isvalueinteger=argv[2];
		char* input_value=argv[2];
		int flag=0;
		while (*isvalueinteger != '\0')
		{
			if(*isvalueinteger=='-')
			{
				if(flag==0)
					flag=1;
				else
				{
					printf("%s is not an integer\n", argv[2]);
        			return 0;
				}
			}	
    		else if (*isvalueinteger<'0' || *isvalueinteger>'9')
    		{
        		printf("%s is not an integer\n", argv[2]);
        		return 0;
   			}
   	 		isvalueinteger++;
		}

		ttl_input=atoi(input_value);
		ispresent_ttl=1;
	}	

	ip_addr = dns_lookup(argv[1], &addr_con); 
	if(ip_addr==NULL) 
	{ 
		printf("\nDNS lookup failed! Could not resolve hostname!\n"); 
		return 0; 
	} 

	reverse_hostname = reverse_dns_lookup(ip_addr); 
	printf("\nConnecting......."); 
	printf("\nReverse Lookup domain: %s\n", reverse_hostname); 

	//socket() 
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); 
	if(sockfd<0) 
	{ 
		printf("\nSocket file descriptor not received!!\n"); 
		return 0; 
	} 
	
	signal(SIGINT, intHandler); //catching interrupt 

	// To send pings continuously 
	send_ping(sockfd, &addr_con, reverse_hostname, ip_addr, argv[1], ttl_input, ispresent_ttl); 
	
	return 0; 
} 
