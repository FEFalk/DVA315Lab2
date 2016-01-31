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
#define MAX_THREADS 2
HANDLE semaphore;
CRITICAL_SECTION criticalSection;

void createPlanet(LPWORD Params) {
	HANDLE mailSlot;
	DWORD bytesWritten = 0;
	DWORD ThreadId = GetCurrentThreadId();
	planet_type *planet = (planet_type*)calloc(50, sizeof(planet_type));
	BOOL loop = TRUE;

	mailSlot = mailslotConnect("\\\\.\\mailslot\\mailbox");
	releaseSemaphore(semaphore, 1, NULL);
	if (mailSlot == INVALID_HANDLE_VALUE) {
		printf("Failed to get a handle to the mailslot!!\nHave you started the server?\n");
		return;
	}
	else {
		printf("Successfully got the mailslot\n");
	}
	
	if (ThreadId == NULL)
	{
		MessageBox(NULL, NULL, TEXT("Error"), MB_OK);
		return 0;
	}

	while (loop == TRUE) 
	{
		EnterCriticalSection(&criticalSection);
		printf("Planet Name: ");
		gets(planet->name);
		if (strlen(planet->name) > 20)printf("Error too big!\n");
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
			loop = FALSE;
			
		}
	}

	bytesWritten = mailslotWrite(mailSlot, planet, sizeof(*planet));
	if (bytesWritten != -1)
		printf("data sent to server (bytes = %d)\n", bytesWritten);

	else
		printf("failed sending data to server\n");
	WaitForSingleObject(semaphore, INFINITE);
	mailslotClose(mailSlot);
	LeaveCriticalSection(&criticalSection);
	Sleep(100);

	return;
}

void main(void) {
	int loops = 2000;
	BOOL planetmaking = TRUE;
	char quit;
	semaphore = CreateSemaphore(NULL, 0, 10, NULL);
	if (semaphore == NULL)
	{
		printf("CreateSemaphore Error: %d\n", GetLastError());
		return 1;

	}
	while (planetmaking == TRUE)
	{
		prinft("Do you want to make a new planet? \n");
		getch(quit);
		if (quit != 'y' || quit != 'n')printf("Wrong input!");
		else if (quit == 'y')
		{	
			HANDLE threads[MAX_THREADS] = {
			threadCreate(createPlanet, NULL)
			//threadCreate(planet=createPlanet, NULL)
			}; 
		}
		else planetmaking = FALSE;
	}
	/*Previous Process ID next pointer or linked list next pointer*/

		/* NOTE: replace code below for sending planet data to the server. */
	
		/* send a friendly greeting to the server */
		/* NOTE: The messages sent to the server need not to be of equal size.       */
		/* Messages can be of different sizes as long as they don't exceed the       */
		/* maximum message size that the mailslot can handle (defined upon creation).*/


	//CloseHandle(threads[0]);
	//CloseHandle(threads[1]);

		/* (sleep for a while, enables you to catch a glimpse of what the */
		/*  client prints on the console)                                 */
	Sleep(2000);

	return;
}
