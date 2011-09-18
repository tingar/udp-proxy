#include <stdlib.h>
#include <pcap.h>
#include <stdio.h>

void process_pkt(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
	int ethlen = 14;
	int iplen = 4 * (packet[ethlen] & 0x0f);
	int udplen = header->len - (ethlen + iplen);

	u_char *ipstart  = packet + ethlen;
	u_char *udpstart = ipstart + iplen;
	u_char *payload  = udpstart + 4;

	printf("Packet of length %d received\n", header->len);
	printf("  Ethernet: 0-%d\n", ethlen);
	printf("    DST: %02x:%02x:%02x:%02x:%02x:%02x\n", packet[0], packet[1], packet[2], packet[3], packet[4], packet[5]);
	printf("    SRC: %02x:%02x:%02x:%02x:%02x:%02x\n", packet[6], packet[7], packet[8], packet[9], packet[10], packet[11]);

	printf("  IP: %d (v %u)\n", iplen, ((int)ipstart >> 4));

	printf("  UDP: %d\n", udplen);
	printf("Summary: %02x%02x%02x%02x\n", udpstart[0], udpstart[1], udpstart[2], udpstart[3]);

}

int main(int argc, char **argv, char **envp) {
	pcap_t *handle;
	char   errbuf[PCAP_ERRBUF_SIZE], filter_exp[] = "udp port 6111", dev[] = "wlan0";
	struct bpf_program fp;
	bpf_u_int32 mask, net;

	if (NULL == dev) {
		fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
		return 2;
	}

	if (-1 == pcap_lookupnet(dev, &net, &mask, errbuf)) {
		fprintf(stderr, "Couldn't get netmask for device %s: %s\n", dev, errbuf);
		net  = 0;
		mask = 0; 
	}

	handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
	if (NULL == handle) {
		fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
		return 2;
	}

	if (-1 == pcap_compile(handle, &fp, filter_exp, 0, net)) {
		fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
		return 1;
	}

	if (-1 == pcap_setfilter(handle, &fp)) {
		fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
		return 2;
	}

	pcap_loop(handle, 0, process_pkt, NULL);

	pcap_close(handle);
	return 0;
}
