/************************************************************************

SensorBox

Obtains the Oculus HMD (Head-Mounted Display) orientation quaternion and
prints the corresponding Euler angles.

The HMD is recenterd everytime the program is executed
To End the Orientation reading press Ctrl-C in the console

Developed with Oculus PC SDK 0.8

Author: Raymundo Cassani
2016

************************************************************************/

#include "stdafx.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h> 
#include <windows.h>
#include <OVR.h>
#include <OVR_CAPI.h> // Include the OculusVR SDK

volatile bool isRunnung = true;

BOOL CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal. 
	case CTRL_C_EVENT:
		printf("Ctrl-C event\n\n");
		Beep(750, 300);
		isRunnung = false;
		return TRUE;
	
	default:
		return FALSE;

	}
}


int main()
{
	system("cls");
	float yaw, pitch, roll;
	//////////////////////////////////////////
	// Oculus HMD Initialization  
	//////////////////////////////////////////

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

	//////////////////////////////////////////
	// Query the HMD for ts current tracking state
	//////////////////////////////////////////
	
	// Recenters HMD Orientation
	ovr_RecenterPose(HMD);
	// Tracking State
	ovrTrackingState ts;
	// Orientation Quaternion
	OVR::Quatf orientation;
	
	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE))
	{
		while (isRunnung)
		{
			ts = ovr_GetTrackingState(HMD, ovr_GetTimeInSeconds(), ovrTrue);

			if ((ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked)))
			{
				// Quaternion with the HMD orientation
				orientation = ts.HeadPose.ThePose.Orientation;
				//system("cls");
				//std::cout << orientation.x << "," << orientation.y << "," << orientation.z << std::endl;
				orientation	.GetEulerAngles< OVR::Axis_Y, OVR::Axis_X, OVR::Axis_Z>(&yaw, &pitch, &roll);
				std::cout << "Yaw: " << OVR::RadToDegree(yaw) << ", Pitch: " << OVR::RadToDegree(pitch) << ", Roll: " << OVR::RadToDegree(roll) << std::endl;
			}
		}
	}

	//////////////////////////////////////////
	// Oculus HMD Destroy
	//////////////////////////////////////////

	std::cout << "Destroying HMD";
	ovr_Destroy(HMD);
	ovr_Shutdown();

	return 0;
}