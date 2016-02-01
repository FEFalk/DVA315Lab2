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
HANDLE threads[MAX_THREADS];
int counter = 0;
void deletePlanetThread(int index);

void createPlanet(int threadsArrayIndex) {
	HANDLE mailSlot;
	DWORD bytesWritten = 0;
	DWORD threadId = GetCurrentThreadId();
	planet_type *planet = (planet_type*)calloc(50, sizeof(planet_type));
	BOOL loop = TRUE;
	DWORD bytesRead;
	char buffer[1024];
	HANDLE mailbox;
	/* create a mailslot that clients can use to pass requests through   */
	/* (the clients use the name below to get contact with the mailslot) */
	/* NOTE: The name of a mailslot must start with "\\\\.\\mailslot\\"  */

	//create mailbox threadid
	char mailslotName[128];
	sprintf(mailslotName, "\\\\.\\mailslot\\%d", threadId);
	mailbox = mailslotCreate(mailslotName);
	
	mailSlot = mailslotConnect("\\\\.\\mailslot\\mailbox");
	
	if (mailSlot == INVALID_HANDLE_VALUE) {
		printf("Failed to get a handle to the mailslot!!\nHave you started the server?\n");
		return;
	}
	else {
		printf("Successfully got the mailslot\n");
	}

	EnterCriticalSection(&criticalSection);
	while (loop) 
	{
		printf("Planet Name: ");
		fgets(planet->name, 20, (stdin));
		if (strlen(planet->name) > 20)printf("Error too big!\n");
		else 
		{
			fflush(stdin);
			sprintf(planet->pid, "%d", threadId);
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
	LeaveCriticalSection(&criticalSection);

	bytesWritten = mailslotWrite(mailSlot, planet, sizeof(*planet));
	if (bytesWritten != -1)
		printf("data sent to server (bytes = %d)\n", bytesWritten);

	else
		printf("failed sending data to server\n");

	ReleaseSemaphore(semaphore, 1, NULL);

	for (;;) {

		bytesRead = mailslotRead(mailbox, &buffer, strlen(buffer));
		//Create planet
		if (bytesRead != 0) {
			if (buffer == "Life")
			{
				printf("Planet Life = 0\n");
				deletePlanetThread(threadsArrayIndex);
				break;
			}
			else if (buffer == "OOBX")
			{
				printf("Planet OOB = X\n");
				deletePlanetThread(threadsArrayIndex);
				break;
			}
			else
			{
				printf("Planet OOB = Y\n");
				deletePlanetThread(threadsArrayIndex);
				break;
			}
		}
	}
	mailslotClose(mailSlot);
	Sleep(100);

	return;
}


BOOL addThread()
{
	for (int i = 0; i < MAX_THREADS; i++)
	{
		//If spot in array is open, create thread there
		if (threads[i]==NULL)
		{
			threads[i] = threadCreate((LPTHREAD_START_ROUTINE)createPlanet, i);
			return TRUE;
		}
	}
	return FALSE;
}

void deletePlanetThread(int index)
{
	CloseHandle(threads[index]);
	threads[index] = NULL;
}

void main(void) {
	int loops = 2000;
	BOOL planetmaking = TRUE;
	char create=0;
	semaphore = CreateSemaphore(NULL, 1, 10, NULL);
	if (semaphore == NULL)
	{
		printf("CreateSemaphore Error: %d\n", GetLastError());
		return 1;
	}
	InitializeCriticalSection(&criticalSection);
	while(planetmaking == TRUE)
	{ 
		WaitForSingleObject(semaphore, INFINITE);
		while (1)
		{
			printf("Do you want to make a new planet? y/n\nEnter: \n");
			create = getch();
			if (create == 'y') {
				//If adding the thread was successful, break and wait...
				if (addThread())
				{
					break;
				}
				else {
					printf("Too many threads active! Try again later.\n");
				}
			}
			else if (create == 'n')
			{
				planetmaking = FALSE;
				break;
			}
			else printf("Wrong input!\n");
		}
	}
		/* NOTE: replace code below for sending planet data to the server. */
	
		/* send a friendly greeting to the server */
		/* NOTE: The messages sent to the server need not to be of equal size.       */
		/* Messages can be of different sizes as long as they don't exceed the       */
		/* maximum message size that the mailslot can handle (defined upon creation).*/


	
	//CloseHandle(threads[1]);

		/* (sleep for a while, enables you to catch a glimpse of what the */
		/*  client prints on the console)    
		*/
	for (int i = 0; i < MAX_THREADS; i++)
	{
		if(threads[i]!=NULL)
			CloseHandle(threads[i]);
	}
	
	DeleteCriticalSection(&criticalSection);
	Sleep(2000);

	return;
}
