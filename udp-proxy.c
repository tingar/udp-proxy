/*
 * I cannot claim credit for this program; it was shamelessly stolen from:
 *
 * http://www.vttoth.com/tunnel.htm
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <syslog.h>

int main(int argc, char *argv[]) {
	struct sockaddr_in saSRC, saDST, saRCV;
	char cBuf[1 << 16];
	int sSRC, sDST;
	struct hostent *ph;
	int nLen;
	int bDebug = 0;
	int on = 1;
	pid_t pid;
	int nAS;
	unsigned long aRCV, aDST;
	int i;

	char *pszApp = *argv++;

	if (argc > 1 && !strcmp(*argv, "-d")) {
		printf("Debug mode.\n");
		bDebug = 1;
		argc--;
		argv++;
	}

	if (argc != 3) {
		printf("Usage: %s [-d] port-number ip-address\n", pszApp);
		return 1;
	}

	ph = gethostbyname(argv[1]);
	if (ph == NULL) {
		printf("Invalid address\n");
		return 1;
	}
	saDST.sin_family = AF_INET;
	saDST.sin_port = htons(atoi(argv[0]));
	saDST.sin_addr.s_addr = *((unsigned long *)ph->h_addr);

	saSRC.sin_family = AF_INET;
	saSRC.sin_port = saDST.sin_port;
	saSRC.sin_addr.s_addr = 0;

	sSRC = socket(AF_INET, SOCK_DGRAM, 0);
	sDST = socket(AF_INET, SOCK_DGRAM, 0);

	if (bind(sSRC, (struct sockaddr *)&saSRC, sizeof (saSRC))) {
		printf("Unable to bind to socket\n");
		return 1;
	}

	setsockopt(sDST, SOL_SOCKET, SO_BROADCAST, &on, sizeof (on));

	if (!bDebug) {
		close(0);
		close(1);
		close(2);
		pid = fork();
		if (pid < 0) {
			syslog(LOG_ERR, "Could not go into background.");
		}
		if (pid > 0) {
			return 0;
		}
	}

	aDST = htonl(saDST.sin_addr.s_addr);

	while (1) {
		nAS = sizeof (saRCV);
		nLen = recvfrom(sSRC, cBuf, sizeof (cBuf), 0, (struct sockaddr *)&saRCV, &nAS);

		// Imperfect method for filtering loopback of broadcast packets;
		// it may also filter packets from certain hosts on the local
		// network, but for our purposes, that's irrelevant.
		aRCV = htonl(saRCV.sin_addr.s_addr);
		for (i = 0; i < 32; i++) {
			if (!(aDST & (1 << i))) {
				break;
			}
			aRCV |= (1 << i);
		}

		if (nLen > 0 && aRCV != aDST) {
			if (bDebug) {
				printf("Relaying a packet of length %d from %d.%d.%d.%d.\n",
					nLen,
					((unsigned char *)&saRCV.sin_addr.s_addr)[0],
					((unsigned char *)&saRCV.sin_addr.s_addr)[1],
					((unsigned char *)&saRCV.sin_addr.s_addr)[2],
					((unsigned char *)&saRCV.sin_addr.s_addr)[3]
				);
			}
			sendto(sDST, cBuf, nLen, 0, (struct sockaddr *)&saDST, sizeof (saDST));
		}
	}
}
