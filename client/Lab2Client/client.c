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
	planet_type *planet = malloc(sizeof(planet_type));
	char *planetName;
	double sx=0;
	double sy=0;
	double vx=0;
	double vy=0;
	double mass=0;
	int life=0;
	char pid[30];
	planetName = (char*)calloc(20, sizeof(char *));
	int planetProcess = 0;
	int i = 0;
	while (planetProcess == 0){
		printf("Planet Name: ");
		gets(planetName);
		sprintf(planet->name, planetName);
		if (strlen(planetName) > 20)printf("Error too big!\n");
		else{
			printf("Position X: ");
			scanf("%lf", &planet->sx);
			printf("Position Y: ");
			scanf("%lf", &planet->sy);
			printf("Velocity X: ");
			scanf("%lf", &planet->vx);
			printf("Velocity Y: ");
			scanf("%lf", &planet->vy);
			printf("Mass: ");
			scanf("%lf", &planet->mass);
			printf("Life: ");
			scanf("%d", &planet->life);
			pid[i] = i;
			i++;
			planetProcess = 1;
		}
	}
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

		/* (sleep for a while, enables you to catch a glimpse of what the */
		/*  client prints on the console)                                 */
	Sleep(2000);

	return;
}
