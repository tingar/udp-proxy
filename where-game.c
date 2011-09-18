#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char *argv[]) {
	int sDST;
	int nLen = 20;
	int on = 1;
	char cBuf[] = {
		0xd9, 0xfa, 0x14, 0x00,
		0x02, 0x00, 0x00, 0x00,
		0x50, 0x58, 0x45, 0x53, /* PXES */
		0xd3, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00  /* padding? */
	};

	struct sockaddr_in saDST, saSRC;

	if (argc != 1) {
		printf("Usage: %s\n", argv[0]);
		return 1;
	}

	saDST.sin_family = AF_INET;
	saDST.sin_port = htons(6111);
	saDST.sin_addr.s_addr = ~0;

	saSRC.sin_family = AF_INET;
	saSRC.sin_port = saDST.sin_port;
	saSRC.sin_addr.s_addr = 0;

	sDST = socket(AF_INET, SOCK_DGRAM, 0);

	if (bind(sDST, (struct sockaddr *)&saSRC, sizeof (saSRC))) {
		printf("Unable to bind to socket\n");
		return 1;
	}

	setsockopt(sDST, SOL_SOCKET, SO_BROADCAST, &on, sizeof (on));

	sendto(sDST, cBuf, nLen, 0, (struct sockaddr *)&saDST, sizeof (saDST));

	return 0;
}
