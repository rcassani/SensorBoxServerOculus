/************************************************************************

SensorBoxServer

Obtains the Oculus HMD (Head-Mounted Display) orientation quaternion and
prints the corresponding Euler angles. Data is streamed 

The HMD is recenterd everytime the program is executed
To End the Orientation reading press Ctrl-C in the console

Developed with Oculus PC SDK 0.8

Author: Raymundo Cassani
2016

************************************************************************/

#define WIN32_LEAN_AND_MEAN

#include "stdafx.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h> 
#include <windows.h>
#include <OVR.h>
#include <OVR_CAPI.h> 
#include <winsock2.h>
#include <ws2tcpip.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

volatile bool isRunning = true;

BOOL CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal. 
		case CTRL_C_EVENT:
			isRunning = false;
			return TRUE;
	
		default:
			return FALSE;
	}
}

SOCKET OpenClientSocket(char *ip, char* port)
{
	////////////////////////////////////////////////////////
	// Creates Socket and Connects to the server at ip:port
	//
	////////////////////////////////////////////////////////	

	// 
	// Creating Socket
	WSADATA wsaData;        //structure is used to store Windows Sockets initialization information		
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) 
	{ 
		//printf("Error at WSAStartup(), error %d\n", iResult); 
		return INVALID_SOCKET;
	}
	struct addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(ip, port, &hints, &result);
	if (iResult != 0) 
	{
		//printf("Error at getaddrinfo(), error %d\n", iResult); 
		WSACleanup(); 
		return INVALID_SOCKET;
	}

	ptr = result;
	SOCKET theSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	if (theSocket == INVALID_SOCKET)
	{
		//printf("Error at socket(), error %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return INVALID_SOCKET;
	}

	// Connect to the server
	iResult = connect(theSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) 
	{
		closesocket(theSocket);
		theSocket = INVALID_SOCKET;
	}
	freeaddrinfo(result);
	if (theSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to server at %s : %s\n", ip, port);
		WSACleanup();
		return INVALID_SOCKET;
	}

	printf("Successful connection with server at %s : %s\n", ip, port);
	return theSocket;
}

void CloseClientSocket(SOCKET theSocket)
{
	////////////////////////////////////////////////////////
	// Closes the Socket
	//
	////////////////////////////////////////////////////////
	closesocket(theSocket);
	WSACleanup();
}

void SendIntClientSocket(SOCKET theSocket, int value)
{
	////////////////////////////////////////////////////////
	// Formats an Int32 from host to TCP/IP network byte order (which is big-endian)
	// and sends it thought the Socket
	////////////////////////////////////////////////////////
	
	int htonvalue[] = { htonl(value) };
	send(theSocket, (char*)htonvalue, sizeof(int), 0);
}

void SendFloatClientSocket(SOCKET theSocket, float value)
{
	////////////////////////////////////////////////////////
	// Formats an Float (32bits) from host to TCP/IP network byte order (which is big-endian)
	// and sends it thought the Socket
	////////////////////////////////////////////////////////

	int htonvalue[] = { htonf(value) };
	send(theSocket, (char*)htonvalue, sizeof(int), 0);
}

int main(int argc, char* argv[])
{

	char *ip = NULL;
	char *port = NULL;
	bool streaming = false;
	int iResult = 0;
	float yaw, pitch, roll;
	SOCKET clientSocket = INVALID_SOCKET;

	system("cls");
	
	////////////////////////////////////////////////////////
	// Check input arguments for IP and PORT
	// and open client socket if required
	////////////////////////////////////////////////////////	
	
	if (argc != 3) 
	{
		printf("Usage: %s <IP> <PORT>\n", argv[0]);
		printf("Orientation data will not be streamed\n");
	}
	else
	{
		ip = argv[1];
		port = argv[2];
		printf("Trying connection with %s : %s\n", ip, port);
		clientSocket = OpenClientSocket(ip, port);
		if (clientSocket == INVALID_SOCKET)
		{
			printf("Error at connection with %s : %s\n", ip, port);
			printf("Orientation data will not be streamed\n");
		}
		else 
		{
			streaming = true;
		}
	}
	Sleep(2000);


	////////////////////////////////////////////////////////
	// Oculus HMD Initialization  
	////////////////////////////////////////////////////////
	ovrResult result = ovr_Initialize(nullptr);
	if (OVR_FAILURE(result))
		return 1;

	ovrGraphicsLuid luid;
	ovrHmd HMD;

	result = ovr_Create(&HMD, &luid);
	if (OVR_FAILURE(result))
	{
		ovr_Shutdown();
		return 1;
	}
	ovrHmdDesc desc = ovr_GetHmdDesc(HMD);
	std::cout << "Product Name: " << desc.ProductName << std::endl;
	std::cout << "Firmware: " << desc.FirmwareMajor << "." << desc.FirmwareMinor << std::endl;
	Sleep(2000);

	////////////////////////////////////////////////////////
	// Query the HMD for ts current tracking state
	////////////////////////////////////////////////////////

	// Recenters HMD Orientation
	ovr_RecenterPose(HMD);
	// Tracking State
	ovrTrackingState ts;
	// Orientation Quaternion
	OVR::Quatf orientation;
	
	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE))
	{
		while (isRunning)
		{
			ts = ovr_GetTrackingState(HMD, ovr_GetTimeInSeconds(), ovrTrue);

			if ((ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked)))
			{
				// Quaternion with the HMD orientation
				orientation = ts.HeadPose.ThePose.Orientation;
				//system("cls");
				//std::cout << orientation.x << "," << orientation.y << "," << orientation.z << std::endl;
				orientation.GetEulerAngles< OVR::Axis_Y, OVR::Axis_X, OVR::Axis_Z>(&yaw, &pitch, &roll);
				yaw = OVR::RadToDegree(yaw);
				pitch = OVR::RadToDegree(pitch);
				roll = OVR::RadToDegree(roll);
				printf("Yaw: %f, Pitch: %f, Roll: %f\n", yaw, pitch, roll);
				if (streaming)
				{
					SendIntClientSocket(clientSocket, 12); // Tells the Server how many bytes to expect
					SendFloatClientSocket(clientSocket, yaw);
					SendFloatClientSocket(clientSocket, pitch);
					SendFloatClientSocket(clientSocket, roll);
				}
			}
		}
	}

	////////////////////////////////////////////////////////
	// Oculus HMD Destroy
	////////////////////////////////////////////////////////

	Beep(750, 300);
	std::cout << "Destroying HMD";
	ovr_Destroy(HMD);
	ovr_Shutdown();

	if (streaming)
	{
		SendIntClientSocket(clientSocket, -1);
		CloseClientSocket(clientSocket);
	}
	return 0;
}