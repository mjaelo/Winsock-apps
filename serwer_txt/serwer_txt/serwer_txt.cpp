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
#include <vector>
#include<time.h>
#include<ctime>
#include "math.h"
#include <random>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable:4996) 



#define DEFAULT_BUFLEN 100
#define DEFAULT_PORT 27015 //port tcp
//historia serwer i klient moze pytac, id polaczenia = wszystkie komunikaty, liczba = komunikat o danym id.
//zmienny rozmiar, brak dopelnien zerami
//string z opisem: Operacja = mnozenie, liczba 
//operacje i status slownie

class serwer {
public:

	bool error = false;
	bool pytanie_sie_o_historie = false;

	std::string OP = "0";
	std::string L1 = "0";//otrzymana liczba
	std::string L2 = "2";//wynik
	std::string ST = "0";//blad 1= poza zakres, 2 /0,
	std::string ID = "1";//losowy numer
	std::string TM = "00:00:00";//czas wyslania
	
	std::vector<std::string> historia;

	WSADATA wsaData;
	int iResult;
	std::string s;//przesylany komunikat

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
	void hist();
	void operacje_na_danych();// zmiana L2
	void dekompresja();

};


void serwer::hist() {
	int l1 = stoi(L1);
	if(L1==ID)
	{
	std::cout << "\n*** historia wysylek: ***\n"<<
		"nr    operacja        liczba status id     data i godzina\n";
	for (int e=0;e<historia.size();e++)
		std::cout << e <<":    "<<historia[e]<<'\n';
	}
	else if (l1 > historia.size())
	{
		ST = "1";std::cout << "\n*** wysylka o numerze: " << l1 << " nie istnieje ***\n\n";
	}
	else 
	{
		std::cout << "\n*** wysylka o numerze:"<<l1<<" ***\n"<<historia[l1]<<"\n\n";
	}
	
}


void serwer::dekompresja() {
	std::string temp;
	//std::cout << "dane ";
	int zn = 0;int plus = 0;
	for (int i = 0;i < s.size();i++)
	{
		
		if (s[i] == '$') { zn++;plus = 0; } //koniec wyrazu
		if (plus == 1)temp += s[i];
		if (s[i] == '=')plus = 1;//koniec opisu 
		
		if (s[i]=='$') 
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
				L1 = temp;
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
				//ID = temp;
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
	//std::cout << "\n";
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


	
	if (OP  == "potegowanie") //potegowanie
	{
		if (l1 == 0)
			a = 1;
		else if (l1 > 32 && l2 > 1)
			ST = "1";
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
						ST = "1";
						break;
					}
					if (std::to_string(e).size() > 10)
					{
						ST = "1";
						break;
					}
				}
			}
			a = e;
		}
		
	}

	if (OP  == "logarytm") //logarytm
	{
		if (L2 == "0000000000" || L1 == "0000000000")
			ST = "2";
		else
		{ 
		int x = std::stoi(L2);
		int base = std::stoi(L1);
		a = log(x) / log(base);
		}
	}

	if (OP == "mnozenie") //mnozenie
	{

		if (std::to_string(l1).size() + std::to_string(l2).size() > 10)
			ST = "1";
		long long c = stoi(L2);
		//for(int i=0;i<liczba1.to_ulong();i++)
		{
			c *= stoi(L1);
			if (c > 2147483647 && c < -2147483647)
			{
				ST = "1";
				//break;
			}
		}
		a = c;
	}
	
	if (OP == "dzielenie") //dzielenie
	{
		if (stoi(L1) != 0)
			a = stoi(L2) / stoi(L1);
		else
			ST = "2";
	}

	if (OP == "historia")
	{		hist();	}

	if (OP == "przyrownanie")
	{		a = stol(L1);	}




	if (a > 2147483647 && a < -2147483647)
		ST = "1";
	if (std::to_string(a).size() > 10)
		ST = "1";

	if (ST == "0")
	{
		//std::cout << "\nnowa liczba: " << a << "\n";
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


	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int > d(0, 256);
	ID = std::to_string(d(gen));
}


void serwer::receive()
{
	// Receive until the peer shuts down the connection
	do {
		
		//otrzymanie
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
		std::cout << "\n\n\n\n" << historia.size() << "\n";
		std::cout << "___________________________________________________________________________________\n\n\n";
			s.resize(DEFAULT_BUFLEN);
			for (int i = 0; i < iResult; i++)				
				s[i] = recvbuf[i];

			std::cout << "Otrzymane Slowo: \n" << s;// << '\n';
			printf("Bytes received: %d", iResult);

			
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
	std::cout << "\n";
	dekompresja();
	operacje_na_danych();


	//czas wyslania
	time_t sec;	sec = time(NULL) + 3600;
	int temp = sec % 60;
	std::string temo = std::to_string(temp);
	if (temp > 9) { TM[7] = temo[1];	TM[6] = temo[0]; }
	else { TM[7] = temo[0];	TM[6] = '0'; }

	sec = (sec - temp) / 60;
	temp = sec % 60;
	temo = std::to_string(temp);
	if (temp > 9) { TM[4] = temo[1];	TM[3] = temo[0]; }
	else { TM[4] = temo[0];	TM[3] = '0'; }

	sec = (sec - temp) / 60;
	temp = sec % 24;
	temo = std::to_string(temp);
	if (temp > 9) { TM[1] = temo[1];	TM[0] = temo[0]; }
	else { TM[1] = temo[0];	TM[0] = '0'; }

	//czas unixonwy w sec
	std::time_t resu = std::time(nullptr);
	//std::cout << std::asctime(std::localtime(&resu))
		//<< resu << " sekund od Epoch\n";
	TM = std::to_string(resu);

	char czas[26];
	time_t result = time(NULL);
		ctime_s(czas, sizeof czas, &result);
		//TM = czas;
	//std::cout << "czas wyslania: " <<TM << '\n';




	s = "OP=" + OP + "$L2=" + L2 + "$ST=" + ST + "$ID=" + ID + "$TM=" + TM+'$';
	historia.push_back(s);


	sendbuflen = s.size();
	char sendb[DEFAULT_BUFLEN];
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
	std::cout << "\nWyslane Slowo: \n" << s << '\n';
	printf("Bytes sent: %d\n", iSendResult);



	if (pytanie_sie_o_historie)
	{
		int ok = 1; int a;std::string temp;
		printf("chcesz sie pytac o historie? 1-tak, 0-nie ");
		std::cin >> temp;
		if (temp == "1")
		{ 
			do {
				ok = 1;
				printf(" podaj nr polaczenia lub id sesji ");
				std::cin >> temp;

				for (auto e : temp)
				{
					if (int(e) < int('0') || int(e) > int('9') || temp.size() > 3)
						ok = 0;
				}
				if (ok == 1)
				{
					long l = std::stoll(temp);
					if (ok == 1 && l > 2147483647)
						ok = 0;
				}
			} while (ok == 0);
			L1 = temp;
			hist();}
	}
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
	{
		int ok = 0; std::string temp;
		do {
			std::cout << "czy serwer bedzie chcial sie pytac o historie? 1-tak, 0-nie\n";
			std::cin >> temp;
			if (temp == "0" || temp == "1")
				ok = 1;
		} while (ok == 0);
		s.pytanie_sie_o_historie = std::stoi(temp);
	}
	if (!s.error)
		s.receive();
	if (!s.error)
		s.cleanup();

	if (s.error) { std::cout << "error"; }

	std::cout << "\n\n\n";
	return 0;
}