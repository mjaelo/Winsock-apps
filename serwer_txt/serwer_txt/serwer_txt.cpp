// niektore fragmenty z:
// https://msdn.microsoft.com/pl-pl/library/windows/desktop/ms737593(v=vs.85).aspx
// http://cpp0x.pl/kursy/Kurs-WinAPI-C++/Zaawansowane/Winsock/371
#pragma once
#include "pch.h"

#undef UNICODE
#define WIN32_LEAN_AND_MEAN

//#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <bitset>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable:4996) 

//inne operacje


#define DEFAULT_BUFLEN 18
#define DEFAULT_PORT 27015 //port tcp


class serwer {
public:

	bool error = false;


	std::string OP = "0";
	std::string L1 = "0000000000";//otrzymana liczba
	std::string L2 = "0000000002";//wynik
	std::string ST = "00";//blad 01= poza zakres, 10= /0, 
	std::string ID = "001";//losowy numer
	std::string padding = "00";//czy ostatnie i znak


	WSADATA wsaData;
	int iResult;
	std::string s;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	sockaddr_in service;

	int iSendResult;
	char sendbuf[DEFAULT_BUFLEN];
	unsigned int sendbuflen;
	char recvbuf[DEFAULT_BUFLEN];
	unsigned int recvbuflen = DEFAULT_BUFLEN;


	void validation();
	void connectsocket();
	void receive();
	void sending();
	void cleanup();

	void operacje_na_danych();// zmiana L2
	void dekompresja();

};

void serwer::dekompresja() {
	std::cout << "\ndane to ";
	std::string temp;
		for (int i = 0;i < s.size();i++)
	{
			temp += s[i];
		if(i==0)
		{ 
			std::cout << temp << " ";
		OP = temp;
		temp = "";
		}
		if (i == 10)
		{
			std::cout << temp << " ";
			L1 = temp;
			temp = "";
		}
		if (i == 10+2)
		{
			std::cout << temp << " ";
			ST = temp;
			temp = "";
		}
		if (i == 10+2+3)
		{
			std::cout << temp << " ";
			ID = temp;
			temp = "";
		}
		if (i == 10+2+3+2)
		{
			std::cout << temp << " ";
			padding = temp;
			temp = "";
		}

	}
		std::cout << '\n';

}


void serwer::operacje_na_danych()
{
	std::string str;//wynik
	std::string se;//string bitow


	//wlasciwe dzialania na danych
	//patrzenie, czy L2 nie wyjdzie za okres
	long l1 = std::stoll(L1);
	long l2 = std::stoll(L2);
	long a =  l1;


	
	if (OP  == "0") //potegowanie
	{
		if (l1 == 0)
			a = 1;
		else if (l1 > 32 && l2 > 1)
			ST = 01;
		else
		{
			long d = std::stoi(L2);
			long long e = std::stoi(L2);

			for (int x = 1; x < std::stoi(L1); x++)
			{
				//for (int j = 0; j < d; j++)
				{

					e *= d;
					if (e > 2147483647 || e < -2147483647)
					{
						ST = "01";
						break;
					}
					if (std::to_string(e).size() > 10)
					{
						ST = "01";
						break;
					}
				}
			}
			a = e;
		}
		
	}

	if (OP  == "1") //logarytm
	{
#include "math.h"
		int x = std::stoi(L2);
		int base = std::stoi(L1);
		a = log(x) / log(base);
	}

	if (OP == "2") //mnozenie
	{

		if (std::to_string(l1).size() + std::to_string(l2).size() > 10)
			ST = "01";
		long long c = stoi(L2);
		//for(int i=0;i<liczba1.to_ulong();i++)
		{
			c *= stoi(L1);
			if (c > 2147483647 && c < -2147483647)
			{
				ST = "01";
				//break;
			}
		}
		a = c;
	}
	if (OP == "3") //dzielenie
	{
		if (stoi(L1) != 0)
			a = stoi(L2) / stoi(L1);
		else
			ST = "10";
	}


	if (a > 2147483647 && a < -2147483647)
		ST = "01";
	if (std::to_string(a).size() > 10)
		ST = "01";

	if (ST == "00")
	{
		std::cout << "\nnowa liczba: " << a << "\n";
		L2 =std::to_string(a);
	}
	else std::cout << "error\n";

}




void serwer::validation()
{
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		error = true;
	}

	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	memset(&service, 0, sizeof(service));
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = DEFAULT_PORT;

	if (bind(ListenSocket, (SOCKADDR *)& service, sizeof(service)) == SOCKET_ERROR)
	{
		printf("bind() failed.\n");
		closesocket(ListenSocket);
		error = true;
	}

	if (!error)printf("validation completed\n");
}


void serwer::connectsocket() {
	if (listen(ListenSocket, 1) == SOCKET_ERROR)
		printf("Error listening on socket.\n");

	printf("Waiting for a client to connect...\n");

	while (ClientSocket == SOCKET_ERROR)
	{
		ClientSocket = accept(ListenSocket, NULL, NULL);
	}

	printf("Client connected.\n");
	ListenSocket = ClientSocket;

	if (!error)printf("connecting completed\n\n");
}


void serwer::receive()
{
	// Receive until the peer shuts down the connection
	do {
		std::cout << "\n\n\n";
		//otrzymanie
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			s="0123456789012345678";
			for (int i = 0; i < 18; i++)				
				s[i] = recvbuf[i];

			std::cout << "Otrzymano slowo: \n" << s << '\n';
			printf("Bytes received: %d\n", iResult);

			
			sending();//dawanie danych do przetworzenia i odeslania

		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			error = true;
		}
	} while (iResult > 0);


	if (!error)printf("receiving completed\n\n");
}


void serwer::sending()
{
	//wysylanie
	dekompresja();
	operacje_na_danych();

	std::string t, temp;
	temp = L2;
	for (int i = 0;i < 10 - temp.size();i++)
		t += "0";
	L2 = t + temp;

	s = OP + L2 + ST + ID + padding;

	sendbuflen = s.size();
	char sendb[52];
	for (int i = 0; i < s.size(); i++)
		sendb[i] = s[i];


	// send the buffer back to the sender
	iSendResult = send(ClientSocket, sendb, sendbuflen, 0);
	if (iSendResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		error = true;
	}
	std::cout << "wyslane slowo: " << s << '\n';
	printf("Bytes sent: %d\n", iSendResult);
}


void serwer::cleanup()
{
	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);



	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		error = true;
	}


	// cleanup
	closesocket(ClientSocket);
	WSACleanup();



	if (!error)printf("sending completed\n\n");

}



int  main() {
	serwer s;

	if (!s.error)
		s.validation();
	if (!s.error)
		s.connectsocket();
	if (!s.error)
		s.receive();
	if (!s.error)
		s.cleanup();

	if (s.error) { std::cout << "error"; }


	return 0;
}