#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define SIZE 256

// Functions:
int init(WSADATA* wsa);
DWORD WINAPI receives_from_server(void* s_server);




int main(int argc, char *argv[])
{
	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server;
	char *message;
	HANDLE thread;

	// Initialising
	if (init(&wsa) == 0) return 0;

	//Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) //IPPORTO_TCP
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("Socket created.\n");

	server.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(7);

	//Connect to server
	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		puts("connect error");
		getch();
		return 1;
	}
	puts("Connected");

	thread = CreateThread(NULL, 0, receives_from_server, s, 0, NULL);


	while (1)
	{
		//Send message
		message = (char*)malloc(SIZE * sizeof(char));

		gets_s(message, SIZE);

		//printf("len: %d", strlen(message));
		if (send(s, message, strlen(message), 0) < 0)
		{
			puts("Send error");
			return 1;
		}
		//puts("Data Send");

	}


	CloseHandle(thread);

	puts("Clean Socket\n");
	closesocket(s);
	WSACleanup();

	getch();
	return 0;
}

int init(WSADATA* wsa)
{
	printf("\n Initialising Winsock \n");
	if (WSAStartup(MAKEWORD(2, 2), wsa) != 0)
	{
		printf("Initialising Winsock error code: %d \n", WSAGetLastError());
		return 0;
	}

	printf("Initialised. \n");
	return 1;
}


DWORD WINAPI receives_from_server(void* s_server) {

	while (1)
	{
		int recv_size;
		char server_reply[SIZE];
		if ((recv_size = recv(s_server, server_reply, SIZE, 0)) == SOCKET_ERROR)
		{
			puts("recv failed");
			return 0;
		}

		server_reply[recv_size] = '\0';
		puts(server_reply);
	}
}
