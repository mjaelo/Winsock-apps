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
#include <vector>
#include<time.h>


#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning(disable:4996) 

//podawanie danych poczatkowych
//zmienic liczby na tekst
//znaczniki czasu
//dodac operacje do historii


#define DEFAULT_BUFLEN 100
#define DEFAULT_PORT 27015 //port tcp


class klient {
public:

	bool error = false;
	
	std::string OP = "0";
	std::string L1 = "0";//otrzymana liczba
	std::string L2 = "1";//wynik
	std::string ST = "0";//blad 1= poza zakres, 2= /0,
	std::string ID = "1";//losowy numer
	std::string TM = "00:00:00";//czas wyslania

	std::vector<std::string> historia;

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
	void hist();
	void dekompresja();

};

void klient::hist() {
	int l1 = stoi(L1);
	if (L1 == ID)
	{
		std::cout << " \n*** historia wysylek: ***\n";
		for (auto e : historia)
			std::cout << e << " \n";
		std::cout << "\n ";
	}
	else if (l1 > historia.size())
		ST = "1";
	else
	{
		std::cout << "\n*** wysylka o numerze " << l1 << ": ***\n" << historia[l1]<<"\n";
	}
}

void klient::dekompresja() {
	std::string temp;
	//std::cout << "dane ";
	int zn = 0;int plus = 0;
	for (int i = 0;i < s.size();i++)
	{

		if (s[i] == ',') { zn++;plus = 0; }
		if (plus == 1)temp += s[i];
		if (s[i] == ' ')plus = 1;

		if (s[i] == ',')
		{
			if (zn == 1)
			{
				//std::cout << temp << " ";
				OP = temp;
				temp = "";
			}
			if (zn == 2)
			{
				//std::cout << temp << " ";
				L2 = temp;
				temp = "";
			}
			if (zn == 3)
			{
				//std::cout << temp << " ";
				ST = temp;
				temp = "";
			}
			if (zn == 4)
			{
				//std::cout << temp << " ";
				ID = temp;
				temp = "";
			}
			if (zn == 5)
			{
				//std::cout << temp << " ";
				TM = temp;
				temp = "";
			}
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

	time_t sec;	sec = time(NULL)+3600;
	int temp = sec % 60;
	std::string temo=std::to_string(temp);
	if (temp > 9) { TM[7] = temo[1];	TM[6] = temo[0]; }
	else		 { TM[7] = temo[0];	TM[6] = '0'; }

	sec = (sec - temp) / 60;
	temp = sec % 60;
	temo = std::to_string(temp);
	if (temp > 9) { TM[4] = temo[1];	TM[3] = temo[0]; }
	else		 { TM[4] = temo[0];	TM[3] = '0'; }

	sec = (sec - temp) / 60;
	temp = sec % 24;
	temo = std::to_string(temp);
	if (temp > 9) { TM[1] = temo[1];	TM[0] = temo[0]; }
	else		  { TM[1] = temo[0];	TM[0] = '0'; }

	std::cout << '\n';





	ST = "0";
	s = "operacja: "+OP+",liczba: "+L1+",stan: "+ST+",id: "+ID+",czas: "+TM;
	historia.push_back(s);

	sendbuflen = s.size();
	char sendb[DEFAULT_BUFLEN];
	for (int i = 0; i < s.size(); i++)
		sendb[i] = s[i];
	


	iResult = send(ConnectSocket, sendb, sendbuflen, 0);
	std::cout << "Wyslane Slowo: \n" <<s<< '\n';
	std::cout << "Bytes Sent: " << iResult << "\n";

	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		error = true;
	}
	else
	{
		receive();
	}

}


void klient::receive()
{
	// Receive 1 data

	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	if (iResult > 0)
	{

		s.resize(iResult);
		for (int i = 0; i < iResult; i++)
			s[i] = recvbuf[i];

		std::cout << "\nOtrzymane Slowo:\n" << s;
		printf("\nBytes received: %d\n", iResult);

		dekompresja();

	}
	else if (iResult == 0)
		printf("\nConnection closed\n");
	else
		printf("recv failed with error: %d\n", WSAGetLastError());

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

	if (!k.error)
		k.validation();
	if (!k.error)
		k.connectsocket();




	//pierwona wartosc serwera
	std::string t,tempr;
	if(!k.error)
	{ 
		std::cout << "\n\n\n" << k.historia.size() << "\n";
		std::cout << "___________________________________________________________________________________\n";
	int oke = 1;
	do {
		oke = 1;
		std::cout << "podaj poczatkowa liczbe serwera ";
		std::cin >> tempr;

		for (auto e : tempr)
		{
			if (int(e) < int('0') || int(e) > int('9') || tempr.size() > 10)
				oke = 0;
		}
		if (oke == 1)
		{
			long l = std::stoll(tempr);
			if (oke == 1 && l > 2147483647)
				oke= 0;

		}
	} while (oke == 0);
	
	
	k.L1 = tempr;
	k.OP = "przyrownanie";
	k.sending();
}







	int next = 1;
	while (next != 0 && !k.error)
	{
		std::cout << "\n\n\n\n"<<k.historia.size()<<"\n";
std::cout << "___________________________________________________________________________________\n";
		if (!k.error)
		{
			int ok = 1;
			
			std::cout << " potegowanie , logarytm , mnozenie , dzielenie  , historia , przyrownanie \n";
			do {
				ok = 1;

				std::cout << "jaka operacja? ";
				std::string temp;
				std::cin >> temp;

				
				
				if (temp == "potegowanie" || temp == "logarytm" || temp == "mnozenie" 
					|| temp == "dzielenie" || temp == "historia" || temp == "przyrownanie")
					k.OP = temp;
				else
					ok = 0;
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
		k.L1 = temp;
	
	


		

		if (k.OP == "historia")
		{		k.hist();		}

		if (!k.error)
			k.sending();

		if (!k.error && k.ST != "0")
		{
			std::cout << "ERROR. nie dokonano operacji. TYP BLEDU: ";
			if (k.ST == "1")
				std::cout << "poza zakresem\n";
			if (k.ST == "2")
				std::cout << "podano 0\n";
		}






		ok = 0;
		do {
			printf("\n1 dalej, 0 koniec ");
			std::cin >> temp;//to na padding
			if (temp == "0" || temp == "1")
				ok = 1;
		} while (ok == 0);
		next = std::stoi(temp);
	}





	if (!k.error)
	{
		k.cleanup();
			std::cout << "\n\n\n ";
	}

	if (k.error) { std::cout << "error"; }

	return 0;
}
