#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define CLIENTS_COUNT 30
#define BUFFER_SIZE 256 

#pragma region Functions_Name
int init(WSADATA* wsa);
#pragma endregion

int main()
{
	WSADATA wsa;
	SOCKET s_listener, new_socket, client_sockets[30] = {NULL}, max_sd; // sd = socket descriptor
	struct sockaddr_in server, client;
	int client_size = sizeof(client);
	int i,clients_count = 0;
	char *message;
	char* buf = (char*)malloc(1025*sizeof(char));

	//set of socket descriptors
	fd_set readfds;

	// Initialising 
	if (init(&wsa) == 0) return 0;

	//Create Socket
	if ((s_listener = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		perror("Error in Create Socket: ");
		return 0;
	}
	printf("Create socket successful\n");

	// init server
	server.sin_family = AF_INET;
	server.sin_port = htons(7);
	server.sin_addr.S_un.S_addr = INADDR_ANY;

	//Bind
	if (bind(s_listener, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		perror("Bind Failed: ");
		getch();
		return 0;
	}
	printf("Bind socket successful\n");

	//Listen to connection
	listen(s_listener, 3);



	while (1)
	{
		//clear the socket set
		FD_ZERO(&readfds);

		//add server socket to set(our listener)
		FD_SET(s_listener, &readfds);
		max_sd = s_listener; // update the max value of sd

		for (i = 0; i < CLIENTS_COUNT; i++)
		{
			if (client_sockets[i] != 0)
			{
				//add client socket to set
				FD_SET(client_sockets[i], &readfds);
				if (client_sockets[i] > max_sd)
					max_sd = client_sockets[i]; // update the max value of sd
			}
		}

		//wait for activity in readfds(fd_set list)
		if (select(max_sd + 1, &readfds, NULL, NULL, NULL) == -1)
		{
			perror("Select");
			getch();
			return 1;
		}
		
		if (FD_ISSET(s_listener, &readfds)) // check if something hanppened in the server socket 
		{
			//Accept connection
			if ((new_socket = accept(s_listener, (struct sockaddr*)&client, &client_size)) != INVALID_SOCKET)
			{
				printf("accept connection successful   = %s, port = %d\n", inet_ntoa(client.sin_addr),ntohs(client.sin_port));
				for (i = 0; i < CLIENTS_COUNT; i++)
				{
					printf("Client %d \n", i);
					if (client_sockets[i] == 0)
					{
						client_sockets[i] = new_socket;
						break;
					}
				}
				//Reply to the client
				message = "Start message from the Server\n";
				send(new_socket, message, strlen(message), 0);
			}
			else
			{
				perror("Accept Failed: ");
				getch();
				return 1;
			}
		}
		// check if something hanppened in the clients sockets
		for (i = 0; i < CLIENTS_COUNT; i++)
		{
			if (FD_ISSET(client_sockets[i], &readfds))
			{
				int read_size;
				//try to read, if cant to read so the client disconnected/has error
				if ((read_size = recv(client_sockets[i], buf, BUFFER_SIZE,0)) == SOCKET_ERROR)
				{
					// client disconnected/has error
					getpeername(client_sockets[i], (struct sockaddr*)&client, &client_size);// put socket-descriptor in client 
					printf("Host disconnected , ip %s , port %d \n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

					//Close/Clear Socket
					closesocket(client_sockets[i]);
					client_sockets[i] = 0;
				}
				else
				{ //Send to everybody
					buf[read_size] = '\0';

					getpeername(client_sockets[i], (struct sockaddr*)&client, &client_size);
					printf("Message from %s in Port %d : %s\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buf);


					char buff[BUFFER_SIZE];
					_itoa(ntohs(client.sin_port),buff,10);
					strcat(buff, " : ");
					strcat(buff, buf);
					for (int j = 0; j < CLIENTS_COUNT; j++)
					{
						if (client_sockets[j] != 0 && client_sockets[j] != client_sockets[i])
						{
							send(client_sockets[j], buff, strlen(buff), 0);
						}
					}
				}
				/*
				else //Echo  message 
				{
					buf[read_size] = '\0';
					send(client_sockets[i], buf, strlen(buf), 0);

					getpeername(client_sockets[i], (struct sockaddr*)&client, &client_size);
					printf("Message from %s in Port %d : %s\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port),buf);
				}*/
			}
		}
	}

	closesocket(s_listener);
	WSACleanup();

	getch();
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