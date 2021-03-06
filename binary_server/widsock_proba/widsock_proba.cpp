// niektore fragmenty z:
// https://msdn.microsoft.com/pl-pl/library/windows/desktop/ms737593(v=vs.85).aspx
// http://cpp0x.pl/kursy/Kurs-WinAPI-C++/Zaawansowane/Winsock/371
#pragma once
#include "stdafx.h"

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

#define DEFAULT_BUFLEN 6
#define DEFAULT_PORT 27015 //port tcp


class serwer {
public:

	bool error = false;


	std::bitset<3> operations = 1;
	std::bitset<32> liczba1 = 000;//otrzymana liczba
	std::bitset<32> liczba2 = 001;//wynik
	std::bitset<2> status = 00;//blad 01= poza zakres, 10= /0, 
	std::bitset<8> id = 01;//losowy numer
	std::bitset<2> flags = 00;//znak i czy ostatnie
	std::bitset<1> padding = 0;

	
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

	void odkompresowanie_str_na_bit();// nadanie nowej wartosci bitsetom
	void operacje_na_danych();// zmiana liczba2
	std::string kompresowanie_bitow_na_str();

};

void serwer::odkompresowanie_str_na_bit()
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
			liczba1 = std::bitset<32>(temp); std::cout << temp << " ";
			temp = "";
		}
		if (i == 2 + 32 + 2) {
			status = std::bitset<2>(temp); std::cout << temp << " ";
			temp = "";
		}
		if (i == 8 + 2 + 32 + 2) {
			//id = std::bitset<8>(temp); 
			std::cout << temp << " ";
			temp = "";
		}
		if (i == 46)
		{
			flags = std::bitset<2>(temp); std::cout << temp << " ";
			temp = "";
		}

	}
	std::cout << "\n";
	if (flags[1] == 0)
		std::cout << "nie jest ostatnie \n";
	else
		std::cout << "Jest ostatnie\n";
}

void serwer::operacje_na_danych()
{
	std::string str;//wynik
	std::string se;//string bitow


	//wlasciwe dzialania na danych
	//patrzenie, czy liczba2 nie wyjdzie za okres
	long a = liczba1.to_ulong();

	

	if (operations.to_ulong() == 0) //dodawanie
	{
		std::cout << "operacja sumy";
		if(flags[0] == 0)
		a = liczba2.to_ulong() + liczba1.to_ulong();
		else
		{
			a= liczba2.to_ulong() - liczba1.to_ulong();
			if(a<=0)
			{
				a = -a;
				flags[0] = 0;
			}
		}
	}
	if (operations.to_ulong() == 1) //odejmowanie
	{
		std::cout << "operacja roznicy";
		if (flags[0] == 1)
		{
			std::cout << liczba2.to_ulong();
			long b;
			b = liczba2.to_ulong() + liczba1.to_ulong();//2147483647
			a = -b;
			if (b > 2147483647)
				status = 01;
		}
		else
			a = liczba2.to_ulong() - liczba1.to_ulong();

		if (a <= -1)
		{
			flags[0] = 1;
			a = -a;
		}
	}
	if (operations.to_ulong() == 2) //mnozenie
	{
		std::cout << "operacja iloczynu";
		if (std::to_string(liczba2.to_ulong()).size() + std::to_string(liczba2.to_ulong()).size()> 10)
			status = 01;
		long long c = liczba2.to_ulong();
		//for(int i=0;i<liczba1.to_ulong();i++)
		{ 
		c *= liczba1.to_ulong();
		if (c > 2147483647 && c < -2147483647)
		{
			status = 01;
			//break;
		}
		}
		a = c;
	}
	if (operations.to_ulong() == 3) //dzielenie
	{
		std::cout << "operacja ilorazu";
		if (liczba1.to_ulong() != 0)
			a = liczba2.to_ulong() / liczba1.to_ulong();
		else
			status = 10;
	}

	if (operations.to_ulong() == 4) //przyrownanie
	{
		std::cout << "operacja przyrownania";
		a = liczba1.to_ulong();
	}

	if (operations.to_ulong() == 5) //potegowanie
	{
		std::cout << "operacja potegowania";
		if (liczba1.to_ulong() > 32 && liczba2.to_ulong() > 1)
			status = 01;
		else
		{
			long d = liczba2.to_ulong();
			long long e = liczba2.to_ulong();

			if (d == 0) { if (liczba1.to_ulong() == 0)status = 10;else e = 1; }
			
			for (int x = 1; x < liczba1.to_ulong(); x++)
			{ 
				//for (int j = 0; j < d; j++)
				{ 
				e *= d;
				if (e > 2147483647 && e < -2147483647 )
				{ 
					status = 01;
					break;
				}
				if (std::to_string(e).size()>10)
				{
					status = 01;
					break;
				}
				}
			}
			a = e;
			if (liczba1.to_ulong() % 2 == 0 && status==00)flags[0] = 0;
		}
	}

	if (operations.to_ulong() == 6) //przyblizanie danej liczby
	{
		std::cout << "operacja przyblizania";
		std::string b = std::to_string(a);
		int c = pow(10, b.size() - 1);
		
		
		if (a%c < c / 10 * 4)a -= a % c;
		else
			a = a - a % c + c; 
	}

	if (operations.to_ulong() == 7) //losowa liczba z zakresu
	{
		std::cout << "operacja przyrownania do losowej liczby z zakresu";
		if (a == 0)
			status = 10;
		else
			a = rand() % liczba1.to_ulong();

		flags[0] = 0;
	}


	if (a > 2147483647 && a < -2147483647)
		status = 01;
	if (std::to_string(a).size()>10)
		status = 01;

	std::cout << " na liczbach "<<liczba2.to_ulong()<<" i "<<liczba1.to_ulong();
	if (status == 00)
	{
		if(flags[0]==0)	std::cout << "\nnowa liczba: " << a << "\n";
		else				std::cout << "\nnowa liczba: -" << a << "\n";
		liczba2 = std::bitset<32>(a);
	}
	else std::cout << " blad operacji.\n";

}

std::string serwer::kompresowanie_bitow_na_str() {

	//nowe bitsety w 1 string
	std::string se = operations.to_string() + liczba2.to_string() + status.to_string() + id.to_string() + flags.to_string() + padding.to_string();
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

	std::cout << "spakowane dane to\n" << str << '\n';

	return str;
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
			s = "      ";
			for (int i = 0; i < 6; i++)
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
	odkompresowanie_str_na_bit();
	operacje_na_danych();
	s = kompresowanie_bitow_na_str();


	char sendb[6];
	for (int i = 0; i < 6; i++)
		sendb[i] = s[i];
	sendbuflen = s.size();

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

	s.id = std::bitset<8>(rand() % 256);

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