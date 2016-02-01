/********************************************************************\
* server.c                                                           *
*                                                                    *
* Desc: example of the server-side of an application                 *
* Revised: Dag Nystrom & Jukka Maki-Turja                     *
*                                                                    *
* Based on generic.c from Microsoft.                                 *
*                                                                    *
*  Functions:                                                        *
*     WinMain      - Application entry point                         *
*     MainWndProc  - main window procedure                           *
*                                                                    *
* NOTE: this program uses some graphic primitives provided by Win32, *
* therefore there are probably a lot of things that are unfamiliar   *
* to you. There are comments in this file that indicates where it is *
* appropriate to place your code.                                    *
* *******************************************************************/

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "wrapper.h"

							/* the server uses a timer to periodically update the presentation window */
							/* here is the timer id and timer period defined                          */

#define UPDATE_FREQ     10	/* update frequency (in ms) for the timer */

							/* (the server uses a mailslot for incoming client requests) */



/*********************  Prototypes  ***************************/
/* NOTE: Windows has defined its own set of types. When the   */
/*       types are of importance to you we will write comments*/ 
/*       to indicate that. (Ignore them for now.)             */
/**************************************************************/

LRESULT WINAPI MainWndProc( HWND, UINT, WPARAM, LPARAM );
DWORD WINAPI mailThread(LPVOID);
planet_type *database;
void planetThread(planet_type *planet);
void addPlanet(planet_type *newPlanet);
void deletePlanet(planet_type *planetToRemove, char *deleteMessage);
CRITICAL_SECTION criticalSection;

HDC hDC;		/* Handle to Device Context, gets set 1st time in MainWndProc */
				/* we need it to access the window for printing and drawin */

/********************************************************************\
*  Function: int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)    *
*                                                                    *
*   Purpose: Initializes Application                                 *
*                                                                    *
*  Comments: Register window class, create and display the main      *
*            window, and enter message loop.                         *
*                                                                    *
*                                                                    *
\********************************************************************/

							/* NOTE: This function is not too important to you, it only */
							/*       initializes a bunch of things.                     */
							/* NOTE: In windows WinMain is the start function, not main */

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow ) {

	HWND hWnd;
	DWORD threadID;
	MSG msg;


							/* Create the window, 3 last parameters important */
							/* The tile of the window, the callback function */
							/* and the backgrond color */

	hWnd = windowCreate (hPrevInstance, hInstance, nCmdShow, "Himmel", MainWndProc, COLOR_WINDOW+1);

							/* start the timer for the periodic update of the window    */
							/* (this is a one-shot timer, which means that it has to be */
							/* re-set after each time-out) */
							/* NOTE: When this timer expires a message will be sent to  */
							/*       our callback function (MainWndProc).               */
  
	windowRefreshTimer (hWnd, UPDATE_FREQ);
  

							/* create a thread that can handle incoming client requests */
							/* (the thread starts executing in the function mailThread) */
							/* NOTE: See online help for details, you need to know how  */ 
							/*       this function does and what its parameters mean.   */
							/* We have no parameters to pass, hence NULL				*/
  

	threadID = threadCreate (mailThread, NULL); 
  

							/* (the message processing loop that all windows applications must have) */
							/* NOTE: just leave it as it is. */
	while( GetMessage( &msg, NULL, 0, 0 ) ) {
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	return msg.wParam;
}


/********************************************************************\
* Function: mailThread                                               *
* Purpose: Handle incoming requests from clients                     *
* NOTE: This function is important to you.                           *
/********************************************************************/

DWORD WINAPI mailThread(LPVOID arg) {

	char buffer[1024];
	DWORD bytesRead;
	static int posY = 0;
	int flag = 0;
	HANDLE mailbox;
							/* create a mailslot that clients can use to pass requests through   */
							/* (the clients use the name below to get contact with the mailslot) */
							/* NOTE: The name of a mailslot must start with "\\\\.\\mailslot\\"  */

	
	mailbox = mailslotCreate ("\\\\.\\mailslot\\mailbox");

	InitializeCriticalSection(&criticalSection);
	for(;;) {				
							/* (ordinary file manipulating functions are used to read from mailslots) */
							/* in this example the server receives strings from the client side and   */
							/* displays them in the presentation window                               */
							/* NOTE: binary data can also be sent and received, e.g. planet structures*/
		
		bytesRead = mailslotRead (mailbox, &buffer, strlen(buffer)); 
		//TESTING PLANETS
		//if(flag==0)
		//{
		//	planet_type p1 = { "p1", 300, 300, 0, 0, 1000000000, NULL, 1000, NULL };
		//	planet_type p2 = { "p2", 200, 300, 0, 0.008, 1000, NULL, 1000, NULL};
		//	planet_type p3 = { "p3", 210, 300, 0.008, 0.008, 5000000, NULL, 1000, NULL };
		//	planet_type *p11 = malloc(sizeof(planet_type));
		//	memcpy(p11, &p1, sizeof(planet_type));
		//	p11->next = NULL;
		//	planet_type *p22 = malloc(sizeof(planet_type));
		//	memcpy(p22, &p2, sizeof(planet_type));
		//	p22->next = NULL;
		//	planet_type *p33 = malloc(sizeof(planet_type));
		//	memcpy(p33, &p3, sizeof(planet_type));
		//	p33->next = NULL;
		//	addPlanet(p11);
		//	addPlanet(p22);
		//	addPlanet(p33);
		//	threadCreate((LPTHREAD_START_ROUTINE)planetThread, p11);
		//	threadCreate((LPTHREAD_START_ROUTINE)planetThread, p22);
		//	threadCreate((LPTHREAD_START_ROUTINE)planetThread, p33);
		//	flag = 1;
		//}
		//Create planet
		if (bytesRead != 0) {
			planet_type *p = malloc(sizeof(planet_type));
			memcpy(p, buffer, sizeof(planet_type));
			p->next = NULL;
			addPlanet(p);
			threadCreate((LPTHREAD_START_ROUTINE)planetThread, p);
			
		

							/* NOTE: It is appropriate to replace this code with something */
							/*       that match your needs here.                           */
		posY++;  
							/* (hDC is used reference the previously created window) */	
		TextOut(hDC, 10, 50+posY%200, p->name, sizeof(strlen(p->name)));

		
		}
		else {
								/* failed reading from mailslot                              */
								/* (in this example we ignore this, and happily continue...) */
		}
	}
	DeleteCriticalSection(&criticalSection);
  return 0;
}

/********************************************************************\
* Function: planetThread                                             *
* Purpose: Handle calculation of planets' new position               *
* @param planet - The planet to calculate the new position for       *
/********************************************************************/
void planetThread(planet_type *planet)
{
	double atotx = 0;
	double atoty = 0;
	double r, x1, x2, y1, y2, cos, sin, G, a1, ax, ay, dt;
	planet_type *comparePlanet = database;
	G = 6.67259*pow(10, -11);
	dt = 10;
	BOOL firstLoop = TRUE;
	if (comparePlanet != 0) {
		while (1)
		{
			//	Using critical section for each whole "database-loop"
			//	as it could get messy with the pointers.
			if (firstLoop)
			{
				EnterCriticalSection(&criticalSection);
			}
			
			if (comparePlanet != planet)
			{
				x1 = planet->sx;
				y1 = planet->sy;
				x2 = comparePlanet->sx;
				y2 = comparePlanet->sy;

				r = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
				a1 = G*(comparePlanet->mass / pow(r, 2));
				cos = (x2 - x1) / r;
				sin = (y2 - y1) / r;
				ax = a1*cos;
				ay = a1*sin;
				atotx += ax;
				atoty += ay;


			}
			// If we reach the end of the loop we enter a new position for the 
			// planet and reset all the values for the next loop. 
			// Also death-check of the planet.
			if (comparePlanet->next == 0)
			{
				planet->vx += atotx*dt;
				planet->vy += atoty*dt;

				planet->sx += planet->vx*dt;
				planet->sy += planet->vy*dt;
				planet->life--;
				atotx = atoty = 0;
				
				comparePlanet = database;
				firstLoop = TRUE;
				if (planet->life <= 0)
				{
					//Remove because of life == 0
					deletePlanet(planet, "Life");
					break;
				}
				else if (planet->sx >= 800 || planet->sx <= 0)
				{
					//Remove because out of bounds in x
					deletePlanet(planet, "OOBX");
					break;
				}
				else if (planet->sy >= 600 || planet->sy <= 0)
				{
					//Remove because out of bounds in y
					deletePlanet(planet, "OOBY");
					break;
				}
				LeaveCriticalSection(&criticalSection);
				Sleep(10);
			}
			else
			{
				firstLoop = FALSE;
				comparePlanet = comparePlanet->next;
			}
			

		}
	}

	
}

/********************************************************************\
* Function: addPlanet												 *
* Purpose: Adds new planet to the database linked list               *
* @param newPlanet - The new planet to be added				         *
/********************************************************************/
void addPlanet(planet_type *newPlanet)
{
	planet_type *traverser = database;
	EnterCriticalSection(&criticalSection);
	if (traverser != 0)
	{
		while (traverser->next != 0)
		{
			traverser = traverser->next;
		}
		traverser->next = newPlanet;
	}
	else
	{
		database = newPlanet;
	}
	LeaveCriticalSection(&criticalSection);
	

}
/********************************************************************\
* Function: deletePlanet											 *
* Purpose: Deletes a planet from the database linked list            *
* @param planetToRemove - The planet to be removed					 *
* @param deleteMessage - The message to be sent to the client about	 *
*						  how the planet died						 *
/********************************************************************/
void deletePlanet(planet_type *planetToRemove, char *deleteMessage)
{
	planet_type *prev = database;
	char mailslotName[128];
	sprintf(mailslotName, "\\\\.\\mailslot\\%s", planetToRemove->pid);

	EnterCriticalSection(&criticalSection);

	planet_type *traverser = database;
	while (traverser != planetToRemove)
	{
		prev = traverser;
		traverser = traverser->next;
	}
	prev->next = planetToRemove->next;
	//If the planet to remove is the root-planet
	if (traverser == database)
		database = traverser->next;
	traverser->next = NULL;

	free(traverser);
	LeaveCriticalSection(&criticalSection);

	HANDLE mail = mailslotConnect(mailslotName);
	mailslotWrite(mail, deleteMessage, 4);
	mailslotClose(mail);
}


/********************************************************************\
* Function: LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM) *
*                                                                    *
* Purpose: Processes Application Messages (received by the window)   *
* Comments: The following messages are processed                     *
*                                                                    *
*           WM_PAINT                                                 *
*           WM_COMMAND                                               *
*           WM_DESTROY                                               *
*           WM_TIMER                                                 *
*                                                                    *
\********************************************************************/
/* NOTE: This function is called by Windows when something happens to our window */

LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
  
	PAINTSTRUCT ps;
	HANDLE context;
	static DWORD color = 0;
	static DWORD colors[5];
  
	switch( msg ) {
							/**************************************************************/
							/*    WM_CREATE:        (received on window creation)
							/**************************************************************/
		case WM_CREATE:       
			hDC = GetDC(hWnd);  
			break;   
							/**************************************************************/
							/*    WM_TIMER:         (received when our timer expires)
							/**************************************************************/
		case WM_TIMER:

							/* NOTE: replace code below for periodic update of the window */
							/*       e.g. draw a planet system)                           */
							/* NOTE: this is referred to as the 'graphics' thread in the lab spec. */


			//Creating a simple color-array.
			colors[0] = RGB(255, 0, 0);
			colors[1] = RGB(0, 255, 0);
			colors[2] = RGB(0, 0, 255);
			colors[3] = RGB(255, 255, 0);
			colors[4] = RGB(255, 153, 0);

			//Critical section so we don't get messy pointers
			EnterCriticalSection(&criticalSection);
			int i;
			planet_type *traverser = database;

			//A loop to draw all the planets with different colors.
			if (traverser != 0)
			{
				i = 0;
				while (traverser != 0)
				{
					i %= 5;
					SetPixel(hDC, traverser->sx, traverser->sy, (COLORREF)colors[i]);
					//FATTEN LINES
					SetPixel(hDC, traverser->sx + 1, traverser->sy, (COLORREF)colors[i]);
					SetPixel(hDC, traverser->sx, traverser->sy + 1, (COLORREF)colors[i]);
					SetPixel(hDC, traverser->sx + 1, traverser->sy + 1, (COLORREF)colors[i]);
					traverser = traverser->next;
					i++;
				}
			}
			LeaveCriticalSection(&criticalSection);
			
			windowRefreshTimer (hWnd, UPDATE_FREQ);
			break;
							/****************************************************************\
							*     WM_PAINT: (received when the window needs to be repainted, *
							*               e.g. when maximizing the window)                 *
							\****************************************************************/

		case WM_PAINT:
							/* NOTE: The code for this message can be removed. It's just */
							/*       for showing something in the window.                */
			context = BeginPaint( hWnd, &ps ); /* (you can safely remove the following line of code) */
			TextOut( context, 10, 10, "Solar System", 13 ); /* 13 is the string length */
			EndPaint( hWnd, &ps );
			break;
							/**************************************************************\
							*     WM_DESTROY: PostQuitMessage() is called                  *
							*     (received when the user presses the "quit" button in the *
							*      window)                                                 *
							\**************************************************************/
		case WM_DESTROY:
			PostQuitMessage( 0 );
							/* NOTE: Windows will automatically release most resources this */
     						/*       process is using, e.g. memory and mailslots.           */
     						/*       (So even though we don't free the memory which has been*/     
     						/*       allocated by us, there will not be memory leaks.)      */

			ReleaseDC(hWnd, hDC); /* Some housekeeping */
			break;

							/**************************************************************\
							*     Let the default window proc handle all other messages    *
							\**************************************************************/
		default:
			return( DefWindowProc( hWnd, msg, wParam, lParam )); 
   }
   return 0;
}




