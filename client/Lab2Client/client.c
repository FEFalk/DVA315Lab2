/*********************************************
* client.c
*
* Desc: lab-skeleton for the client side of an
* client-server application
* 
* Revised by Dag Nystrom & Jukka Maki-Turja
* NOTE: the server must be started BEFORE the
* client.
*********************************************/
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include "wrapper.h"

#define MESSAGE "Hello!"
#define MAX_THREADS 1

planet_type *createPlanet(LPWORD Params) {
	DWORD ThreadId = GetCurrentThreadId();
	if (ThreadId == NULL)
	{
		MessageBox(NULL, NULL, TEXT("Error"), MB_OK);
		return 0;
	}
	planet_type *planet;
	planet = malloc(sizeof(planet_type));
	char *planetName;
	planetName = (char*)calloc(20, sizeof(char *));
	int planetProcess = 0;
	while (planetProcess == 0) 
	{
		printf("Planet Name: ");
		gets(planetName);
		sprintf(planet->name, planetName);
		if (strlen(planetName) > 20)printf("Error too big!\n");
		else 
		{
			fflush(stdin);
			planet->pid[(char)ThreadId] = ThreadId;
			printf("Position X: ");
			scanf("%lf", &planet->sx);
			fflush(stdin);
			printf("Position Y: ");
			scanf("%lf", &planet->sy);
			fflush(stdin);
			printf("Velocity X: ");
			scanf("%lf", &planet->vx);
			fflush(stdin);
			printf("Velocity Y: ");
			scanf("%lf", &planet->vy);
			fflush(stdin);
			printf("Mass: ");
			scanf("%lf", &planet->mass);
			fflush(stdin);
			printf("Life: ");
			scanf("%d", &planet->life);
			fflush(stdin);
			planetProcess = 1;
		}
	}
	return planet;
}

void main(void) {

	HANDLE mailSlot;	
	DWORD bytesWritten=0;
	int loops = 2000;
	
	mailSlot = mailslotConnect("\\\\.\\mailslot\\mailbox"); 

	if (mailSlot == INVALID_HANDLE_VALUE) {
		printf("Failed to get a handle to the mailslot!!\nHave you started the server?\n");
		return;
	}
	else {
		printf("Successfully got the mailslot\n");
	}
	
	/*Previous Process ID next pointer or linked list next pointer*/
	planet_type *planet;
	planet = malloc(sizeof(planet_type));
	HANDLE threads[MAX_THREADS] = {
		threadCreate(planet=createPlanet, NULL)
		//threadCreate(planet=createPlanet, NULL)
	};



		/* NOTE: replace code below for sending planet data to the server. */
	
		/* send a friendly greeting to the server */
		/* NOTE: The messages sent to the server need not to be of equal size.       */
		/* Messages can be of different sizes as long as they don't exceed the       */
		/* maximum message size that the mailslot can handle (defined upon creation).*/
		/*planet_type planet = { planetName, sx, sy, vx, vy, mass, NULL, life, NULL};*/
	bytesWritten = mailslotWrite(mailSlot, planet, sizeof(*planet));
	if (bytesWritten!=-1)
		printf("data sent to server (bytes = %d)\n", bytesWritten);
			
	else
		printf("failed sending data to server\n");

	mailslotClose (mailSlot);

	WaitForMultipleObjects(MAX_THREADS, threads, TRUE, INFINITE);

	CloseHandle(threads[0]);
	//CloseHandle(threads[1]);

		/* (sleep for a while, enables you to catch a glimpse of what the */
		/*  client prints on the console)                                 */
	Sleep(2000);

	return;
}
