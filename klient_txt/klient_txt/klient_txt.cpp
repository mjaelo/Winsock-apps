// widsock_proba.cpp : Defines the entry point for the console application.
//
// niektore fragmenty z:
// https://msdn.microsoft.com/pl-pl/library/windows/desktop/ms737593(v=vs.85).aspx
// http://cpp0x.pl/kursy/Kurs-WinAPI-C++/Zaawansowane/Winsock/371
#pragma once
#include "pch.h"

#define WIN32_LEAN_AND_MEAN

//#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <bitset>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning(disable:4996) 

//podawanie danych poczatkowych
//zmienic liczby na tekst
//znaczniki czasu


#define DEFAULT_BUFLEN 18
#define DEFAULT_PORT 27015 //port tcp


class klient {
public:

	bool error = false;
	
	std::string OP = "0";
	std::string L1 = "0000000000";//otrzymana liczba
	std::string L2 = "0000000001";//wynik
	std::string ST = "00";//blad 01= poza zakres, 10= /0, 
	std::string ID = "001";//losowy numer
	std::string padding = "00";//czy ostatnie i znak



	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	sockaddr_in service;

	char sendbuf[DEFAULT_BUFLEN];
	int sendbuflen;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	int iResult;
	std::string s;


	klient() {}


	void validation();
	void connectsocket();
	void sending();
	void receive();
	void cleanup();

	void dekompresja();

};

void klient::dekompresja() {

	std::string temp;
	std::cout << "dane ";
	for (int i = 0;i < s.size();i++)
	{
		temp += s[i];
		if (i == 0)
		{
			std::cout << temp << " ";
			OP = temp;
			temp = "";
		}
		if (i == 10)
		{
			std::cout << temp << " ";
			L2 = temp;
			temp = "";
		}
		if (i == 10 + 2)
		{
			std::cout << temp << " ";
			ST = temp;
			temp = "";
		}
		if (i == 10 + 2 + 3)
		{
			std::cout << temp << " ";
			ID = temp;
			temp = "";
		}
		if (i == 10 + 2 + 3 + 2)
		{
			std::cout << temp << " ";
			padding = temp;
			temp = "";
		}

	}


}



void klient::validation()
{
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		error = true;
	}

	ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	std::cout << "Podaj adres IP serwera: ";
	std::string IP;
	std::cin >> IP;

	memset(&service, 0, sizeof(service));
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(IP.c_str());
	service.sin_port = DEFAULT_PORT;

	if (!error)printf("validation completed\n");
}


void klient::connectsocket() {
	if (connect(ConnectSocket, (SOCKADDR *)& service, sizeof(service)) == SOCKET_ERROR)
	{
		printf("Failed to connect.\n");
		WSACleanup();
		error = true;
	}
}


void klient::sending()
{
	// Send a buffer			
	s = OP+L1+ST+ID+padding;

	sendbuflen = s.size();
	char sendb[52];
	for (int i = 0; i < s.size(); i++)
		sendb[i] = s[i];
	


	iResult = send(ConnectSocket, sendb, sendbuflen, 0);
	std::cout << "Bytes Sent: " << iResult << "\n";
	std::cout << "String sent: " << '\n';


	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		error = true;
	}
	else
	{
		printf("sending completed\n");
		receive();
	}

}


void klient::receive()
{
	// Receive 1 data

	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	if (iResult > 0)
	{

		s = "0123456789012345678";
		for (int i = 0; i < 18; i++)
			s[i] = recvbuf[i];

		std::cout << "\nreceived data:" << s;
		printf("\nBytes received: %d\n", iResult);

		dekompresja();

		std::cout << "\nwynik posredni: " << std::stoi(L2);
	}
	else if (iResult == 0)
		printf("\nConnection closed\n");
	else
		printf("recv failed with error: %d\n", WSAGetLastError());

	//iResult = shutdown(ConnectSocket, SD_SEND);




	if (!error)printf("\nreceiving completed\n");
}


void klient::cleanup()
{
	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		error = true;
	}

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

}





int  main() {
	klient k;

	std::string t,temp;
	temp = std::to_string(rand() % 256);
	for (int i = 0;i < 3 - temp.size();i++)
		t += "0";
	k.ID = t + temp;

	


	if (!k.error)
		k.validation();
	if (!k.error)
		k.connectsocket();



	int next = 1;
	while (next != 0 && k.error == false)
	{
		std::cout << "\n\n\n";

		if (!k.error)
		{
			int ok = 1;
			std::cout << "0 ^, 1 log, 2 mnozenie , 3 dzielenie \n";
			do {
				ok = 1;

				std::cout << "jaka operacja? ";
				std::string temp;
				std::cin >> temp;

				for (auto e : temp)
					if (int(e) < int('0') || e > int('9'))
						ok = 0;
				if (ok == 1)
					k.OP = temp;
			} while (ok == 0);
		}

		int ok = 1; int a;std::string temp;

		do {
			ok = 1;
			printf(" daj liczbe do wyslania ");
			std::cin >> temp;

			for (auto e : temp)
			{
				if (int(e) < int('0') || int(e) > int('9') || temp.size() > 10)
					ok = 0;
			}
			if (ok == 1)
			{
				 long l = std::stoll(temp);
				if (ok == 1 && l > 2147483647)
					ok = 0;
				
			}
		} while (ok == 0);

		std::string t;
		for (int i = 0;i < 10-temp.size();i++)
			t += "0";
		k.L1 = t+temp;

		std::cout << "liczba: " << k.L1 << '\n';

		ok = 0;
		do {
			printf("1 dalej, 0 koniec ");
			std::cin >> temp;//to na padding
			if (temp == "0" || temp == "1")
				ok = 1;

		} while (ok == 0);

		next = std::stoi(temp);
		



		if (!k.error)
			k.sending();

		if (!k.error && k.ST != "00")
		{
			std::cout << "ERROR. nie dokonano operacji. TYP BLEDU: ";
			if (k.ST == "01")
				std::cout << "poza zakresem\n";
			if (k.ST == "10")
				std::cout << "podano 0\n";
		}
	}

	if (!k.error)
	{
		k.cleanup();
		
			std::cout << "\nwynik koncowy: " << k.L2;
		
	}

	if (k.error) { std::cout << "error"; }

	return 0;
}
