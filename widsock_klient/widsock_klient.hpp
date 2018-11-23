// widsock_proba.cpp : Defines the entry point for the console application.
//
// niektore fragmenty z:
// https://msdn.microsoft.com/pl-pl/library/windows/desktop/ms737593(v=vs.85).aspx
// http://cpp0x.pl/kursy/Kurs-WinAPI-C++/Zaawansowane/Winsock/371
#pragma once
//#include "stdafx.h"

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


#define DEFAULT_BUFLEN 6
#define DEFAULT_PORT 27015 //port tcp


class klient {
public:

	bool error = false;

	std::bitset<3> operations = 1;
	std::bitset<32> liczba1 = 000;
	std::bitset<32> liczba2 = 000;//wynik posredni
	std::bitset<2> status = 00;
	std::bitset<8> id = 1011101;//przesyla losowy numer?
	std::bitset<3> padding = 000;//liczba ujemna i ostatnia


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

	std::string kompresowanie_bitow_na_str();
	void odkompresowanie_str_na_bit();
};


void klient::odkompresowanie_str_na_bit()
{
	std::string se;//string bitow
	std::string temp;// jeden 8bitowy char w str

// rozpakowanie bitow z charow i wstawienie ich w jeden, duzy string
	for (auto e : s)
		se += std::bitset<8>(e).to_string();

	std::cout << "odkompresowane dane: ";
	// wpakowanie bitow w odpowiednie bitsety 
	for (int i = 0; i < 48; i++)
	{
		temp += se[i];

		if (i == 2) {
			operations = std::bitset<3>(temp); std::cout << temp << " ";
			temp = "";
		}
		if (i == 32 + 2)
		{
			liczba2 = std::bitset<32>(temp); std::cout << temp << " ";
			temp = "";
		}
		if (i == 2 + 32 + 2) {
			status = std::bitset<2>(temp); std::cout << temp << " ";
			temp = "";
		}
		if (i == 8 + 2 + 32 + 2) {
			id = std::bitset<8>(temp); std::cout << temp << " ";
			temp = "";
		}
		if (i == 47)
		{
			padding = std::bitset<3>(temp); std::cout << temp << " ";
			temp = "";
		}
	}
	if (padding[0] == 0)
		std::cout << "\nwynik posredni: " << liczba2.to_ulong();
	else
		std::cout << "\nwynik posredni: -" << liczba2.to_ulong();
}

std::string klient::kompresowanie_bitow_na_str() {

	std::string se = operations.to_string() + liczba1.to_string() + status.to_string() + id.to_string() + padding.to_string();
	std::bitset<48> wynik(se);


	std::string str;//string 8bitowych charow
	std::string temp;//zawartosc jednego chara


	for (int i = 0; i < 48; i++) {
		temp += se[i];

		if (i % 8 == 7) {
			auto a = std::bitset<8>(temp);// string na bitset, i ten bitset spakowany jako 1 znak w char		
			str += a.to_ulong();
			temp.clear();
		}

	}

	std::cout << "\nspakowane dane to\n" << str << '\n';

	return str;
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
		return;
	}
}


void klient::sending()
{
	// Send a buffer			(int)strlen(sendbuf)		//strcpy_s(sendbuf, s.c_str());		s.c_str()
	status = 0;

	s = kompresowanie_bitow_na_str();
	char sendb[6];
	for (int i = 0; i < 6; i++)
		sendb[i] = s[i];
	sendbuflen = s.size();


	iResult = send(ConnectSocket, sendb, sendbuflen, 0);
	std::cout << "Bytes Sent: " << iResult << "\n";
	std::cout << "String sent: " << s << '\n';


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

		for (int i = 0; i < 6; i++)
			s[i] = recvbuf[i];

		std::cout << "\nreceived data:" << s;
		printf("\nBytes received: %d\n", iResult);
		odkompresowanie_str_na_bit();
		//std::cout << "\nwynik posredni: " << liczba2.to_ulong();
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





int __cdecl main_() {
	klient k;
	k.id = std::bitset<8>(rand() % 256);


	if (!k.error)
		k.validation();
	if (!k.error)
		k.connectsocket();

	if (!k.error)
	{
		int ok = 1;
		std::cout << "000 +, 001 -, 010 *, 011 /, 100 =, 101 ^, 110 round , 111 rand z zakresu ";
		do {
			ok = 1;

			std::cout << "jaka operacja? ";
			std::string temp;
			std::cin >> temp;

			for (auto e : temp)
				if (e != '0' && e != '1')
					ok = 0;
			if (ok == 1)
				k.operations = std::bitset<3>(temp);
		} while (ok == 0);
	}

	int next = 1;
	while (next != 0 && k.error == false)
	{
		std::cout << "\n\n\n";
		int ok = 1; int a;
		do {
			ok = 1;
			printf(" daj liczbe do wyslania ");
			std::string temp;
			std::cin >> temp;

			for (auto e : temp)
			{
				if (int(e) < int('0') || e > int('9') || temp.size() > 10)
					ok = 0;
			}

			long long l = std::stoll(temp);// nie dziala to
			if (ok == 1 && l > 2147483647)
				ok = 0;


			if (ok == 1)
				a = std::stoi(temp);
		} while (ok == 0);

		k.liczba1 = std::bitset<32>(a);
		std::cout << "liczba w bitach: " << k.liczba1 << '\n';

		printf("1 dalej, 0 koniec ");

		std::cin >> next;//to na padding
		if (next == 0)
			k.padding[2] = 1;
		else k.padding[2] = 0;



		if (!k.error)
			k.sending();

		if (!k.error && k.status != 00)
		{
			std::cout << "ERROR. nie dokonano operacji. TYP BLEDU: ";
			if (k.status == 01)
				std::cout << "poza zakresem\n";
			if (k.status == 10)
				std::cout << "podano 0\n";
		}
	}

	if (!k.error)
	{
		k.cleanup();
		if (k.padding[0] == 0)
			std::cout << "\nwynik koncowy: " << k.liczba2.to_ulong();
		else
			std::cout << "\nwynik koncowy: -" << k.liczba2.to_ulong();
	}

	if (k.error) { std::cout << "error"; }

	return 0;
}
