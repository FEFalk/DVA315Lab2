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
#include "resource.h"
#include "Strsafe.h"

#define MESSAGE "Hello!"
#define MAX_THREADS 2
HANDLE semaphore;
CRITICAL_SECTION criticalSection;
HANDLE threads[MAX_THREADS];
int counter = 0;
void deletePlanetThread(int index);
planet_type *localDatabase=NULL;
/****************************************************************
* Function: createPlanet  										*
* Purpose: Creates a new planet through I/O and sends it to		*
*			 server. Waits for incoming messages from server.	*
* @param threadsArrayIndex - The index of the thread in the		*
*					handle-array(used to know which element		*
*					to delete).									*
*****************************************************************/
void createPlanet(int threadsArrayIndex) {
	HANDLE mailSlot;
	DWORD bytesWritten = 0;
	DWORD threadId = GetCurrentThreadId();
	planet_type *planet = (planet_type*)calloc(1, sizeof(planet_type));
	BOOL loop = TRUE;
	DWORD bytesRead;
	char buffer[1024];
	HANDLE mailbox;
	int c;
	char *p;
	/* create a mailslot that clients can use to pass requests through   */
	/* (the clients use the name below to get contact with the mailslot) */
	/* NOTE: The name of a mailslot must start with "\\\\.\\mailslot\\"  */

	//Create mailbox threadid
	char mailslotName[128];
	sprintf(mailslotName, "\\\\.\\mailslot\\%d", threadId);
	mailbox = mailslotCreate(mailslotName);
	
	//Connect to the server mailbox
	mailSlot = mailslotConnect("\\\\.\\mailslot\\mailbox");
	
	if (mailSlot == INVALID_HANDLE_VALUE) {
		printf("Failed to get a handle to the mailslot!!\nHave you started the server?\n");
		return;
	}
	else {
		printf("Successfully got the mailslot\n");
	}
	//I/O for creating a new planet
	EnterCriticalSection(&criticalSection);
	printf("Planet Name: ");
	fgets(planet->name, 20, (stdin));
	if ((p = strchr(planet->name, '\n')) != NULL)
		*p = '\0';
	if (strlen(planet->name) > 20)printf("Error too big!\n");
	else 
	{
		sprintf(planet->pid, "%d", threadId);
		printf("Position X: ");
		scanf(" %lf", &planet->sx);
		printf("Position Y: ");
		scanf(" %lf", &planet->sy);
		printf("Velocity X: ");
		scanf(" %lf", &planet->vx);
		printf("Velocity Y: ");
		scanf(" %lf", &planet->vy);
		printf("Mass: ");
		scanf(" %lf", &planet->mass);
		printf("Life: ");
		scanf(" %d", &planet->life);
		loop = FALSE;
		while ((c = getchar()) != '\n' && c != EOF);
	}
	LeaveCriticalSection(&criticalSection);

	//Sends the planet to server through the mailslot
	bytesWritten = mailslotWrite(mailSlot, planet, sizeof(*planet));
	if (bytesWritten != -1)
		printf("data sent to server (bytes = %d)\n", bytesWritten);

	else
		printf("failed sending data to server\n");

	//Gives main-thread permission to continue
	ReleaseSemaphore(semaphore, 1, NULL);

	//READ-LOOP
	for (;;) {

		bytesRead = mailslotRead(mailbox, &buffer, strlen(buffer));


		//If a message is recieved, print out the reason why the planet died.
		if (bytesRead != 0) {
			buffer[bytesRead] = '\0';
			if (strcmp(buffer, "Life") == 0)
			{
				printf("\nPlanet '%s' has died. Reason: Life = 0\n", planet->name);
				break;
			}
			else if (strcmp(buffer, "OOBX") == 0)
			{
				printf("\nPlanet '%s' has died. Reason: OOB in X\n", planet->name);
				break;
			}
			else
			{
				printf("\nPlanet '%s' has died. Reason: OOB in Y\n", planet->name);
				break;
			}
		}
	}
	free(planet);
	deletePlanetThread(threadsArrayIndex);
	mailslotClose(mailSlot);
	mailslotClose(mailbox);

}

/********************************************************************\
* Function: addThread											     *
* Purpose: Adds a thread to the thread-handles array.                *
* @return - Returns TRUE or FALSE depending on if the array had an	 *
*			open spot or not.										 *
/********************************************************************/
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

/********************************************************************\
* Function: deleteThread											 *
* Purpose: Deletes a thread-handle from the array and closes it.     *
* @param index - Index of the thread to delete.						 *
/********************************************************************/
void deletePlanetThread(int index)
{
	CloseHandle(threads[index]);
	threads[index] = NULL;
}

/********************************************************************\
* Function: addPlanet												 *
* Purpose: Adds new planet to the database linked list               *
* @param newPlanet - The new planet to be added				         *
/********************************************************************/
int getPlanetIndex(planet_type *p)
{
	planet_type *traverser = localDatabase;
	int i;
	if (traverser != 0)
	{
		for (i = 0; traverser->next == p; i++)
		{
			traverser = traverser->next;
		}
	}

	return i;

}

/********************************************************************\
* Function: getPlanet												 *
* Purpose: Returns a planet from the database linked list            *
* @param newPlanet - The new planet to be added				         *
/********************************************************************/
planet_type *getPlanetAt(int index)
{
	planet_type *traverser = localDatabase;
	if (traverser != 0)
	{
		while (traverser->next != 0)
		{
			traverser = traverser->next;
		}
	}

	return traverser;
}
/********************************************************************\
* Function: getPlanet												 *
* Purpose: Returns a planet from the database linked list            *
* @param newPlanet - The new planet to be added				         *
/********************************************************************/
planet_type *getLastPlanet()
{
	planet_type *traverser = localDatabase;
	if (traverser != 0)
	{
		while (traverser->next != 0)
		{
			traverser = traverser->next;
		}
	}

	return traverser;
}

/********************************************************************\
* Function: addPlanet												 *
* Purpose: Adds new planet to the database linked list               *
* @param newPlanet - The new planet to be added				         *
/********************************************************************/
void addPlanet(planet_type *newPlanet)
{
	planet_type *traverser = localDatabase;
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
		localDatabase = newPlanet;
	}


}
/********************************************************************\
* Function: deletePlanet											 *
* Purpose: Deletes a planet from the database linked list            *
* @param planetToRemove - The planet to be removed					 *
/********************************************************************/
void deletePlanet(planet_type *planetToRemove, char *deleteMessage)
{
	planet_type *prev = localDatabase;

	EnterCriticalSection(&criticalSection);

	planet_type *traverser = localDatabase;
	while (traverser != planetToRemove)
	{
		prev = traverser;
		traverser = traverser->next;
	}
	prev->next = planetToRemove->next;
	//If the planet to remove is the root-planet
	if (traverser == localDatabase)
		localDatabase = traverser->next;
	traverser->next = NULL;

	//!UPDATE GETITEMDATA/SETITEMDATA!


	free(traverser);
	LeaveCriticalSection(&criticalSection);
}

HWND addPlanetDialog = NULL;
INT_PTR CALLBACK addPlanetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			return TRUE;
		}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return TRUE;
		case IDOK:
			{
				HWND localPlanetsList = GetDlgItem(GetParent(hDlg), ID_LIST_LOCAL_PLANETS);
				if (checkFields(hDlg))
				{
					int pos = (int)SendMessage(localPlanetsList, LB_ADDSTRING, 0, (LPARAM)getLastPlanet()->name);
					SendMessage(localPlanetsList, LB_SETITEMDATA, pos, getPlanetIndex(getLastPlanet()));
					ShowWindow(hDlg, SW_HIDE);
				}
				else
				{
					//Do something when a field is empty
				}

				return TRUE;
			}
		}
		return TRUE;
	}
	case WM_CLOSE:
		if (MessageBox(hDlg,
			TEXT("Close the window?"), TEXT("Close"),
			MB_ICONQUESTION | MB_YESNO) == IDYES)
		{
			ShowWindow(hDlg, SW_HIDE);
		}
		return TRUE;

	case WM_DESTROY:
		return TRUE;
	}
	return FALSE;
}


INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND localPlanetsList = GetDlgItem(hDlg, ID_LIST_LOCAL_PLANETS);
	// Get selected index. (In ListBox)
	int lbItem = (int)SendMessage(localPlanetsList, LB_GETCURSEL, 0, 0);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			//Initializes the "Add Planet"-window(dialog)
			addPlanetDialog = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_DIALOG_ADD_PLANET), hDlg, addPlanetProc);
			if(addPlanetDialog == NULL)
			{
				MessageBox(hDlg, "CreateDialog returned NULL", "Warning!",
					MB_OK | MB_ICONINFORMATION);
			}
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDCANCEL:
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				return TRUE;
			case ID_BUTTON_ADD:
				{
					ShowWindow(addPlanetDialog, SW_SHOW);
					return TRUE;
				}
			case ID_BUTTON_EDIT:
			{
				if (lbItem == NULL) {
					MessageBox(hDlg, "You must target a planet", "Warning!",
						MB_OK | MB_ICONINFORMATION);
					break;
				}
				ShowWindow(GetDlgItem(hDlg, ID_BUTTON_EDIT_OK), SW_SHOW);
				ShowWindow(GetDlgItem(hDlg, ID_BUTTON_EDIT_CANCEL), SW_SHOW);
				SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_NAME), EM_SETREADONLY, FALSE, 0);
				SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONY), EM_SETREADONLY, FALSE, 0);
				SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONX), EM_SETREADONLY, FALSE, 0);
				SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYY), EM_SETREADONLY, FALSE, 0);
				SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYX), EM_SETREADONLY, FALSE, 0);
				return TRUE;
			}
			case ID_BUTTON_EDIT_OK:
			{
				planet_type *editPlanet = (planet_type*)calloc(1, sizeof(planet_type));
				TCHAR tempString[128];
				
				
				editPlanet = getPlanetAt(lbItem);
				GetWindowText(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_NAME), tempString, 128);
				sprintf(editPlanet->name, tempString);
				free(editPlanet);
				return TRUE;
			}
			case ID_BUTTON_EDIT_CANCEL:
			{
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				return TRUE;
			}
			}

			case ID_LIST_LOCAL_PLANETS:
				{
					switch (HIWORD(wParam))
					{
					case LBN_SELCHANGE:
						{
							HWND localPlanetsList = GetDlgItem(hDlg, ID_LIST_LOCAL_PLANETS);

							// Get selected index. (In ListBox)
							int lbItem = (int)SendMessage(localPlanetsList, LB_GETCURSEL, 0, 0);

							// Get item data of selected index. (item data = Index of planet linked list)
							int i = (int)SendMessage(localPlanetsList, LB_GETITEMDATA, lbItem, 0);

							// Do something with the data from Roster[i]
							TCHAR buff[MAX_PATH];
							StringCbPrintf(buff, ARRAYSIZE(buff),
								TEXT("Planet Name: %s\Planet X-Position: %d\Planet Y-Position: %d"),
								localDatabase[i].name, (int)localDatabase[i].sx,
								(int)localDatabase[i].sy);

							SetDlgItemText(hDlg, ID_STATIC_LOCAL_PLANET_INFO, buff);

							return TRUE;
						}
					}

					return TRUE;
				}
			return TRUE;
		}
	case WM_CLOSE:
		if (MessageBox(hDlg,
			TEXT("Close the window?"), TEXT("Close"),
			MB_ICONQUESTION | MB_YESNO) == IDYES)
		{
			DestroyWindow(hDlg);
		}
		return TRUE;

	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	HWND hDlg;
	BOOL ret;
	MSG msg;

	hDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(ID_DIALOG_MAIN), 0, DialogProc, 0);
	ShowWindow(hDlg, nCmdShow);


	while ((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
		if (ret == -1) /* error found */
			return -1;
		if (!IsDialogMessage(addPlanetDialog, &msg)) 
		{
			TranslateMessage(&msg); /* translate virtual-key messages */
			DispatchMessage(&msg); /* send it to dialog procedure */
		}
	}
}



//void main(void) {
//	int loops = 2000;
//	BOOL planetmaking = TRUE;
//	char create=0;
//	semaphore = CreateSemaphore(NULL, 1, 10, NULL);
//	if (semaphore == NULL)
//	{
//		printf("CreateSemaphore Error: %d\n", GetLastError());
//		return 1;
//	}
//	InitializeCriticalSection(&criticalSection);
//
//	// I/O-loop for creating planets or quitting main-process.
//	while(planetmaking == TRUE)
//	{ 
//		WaitForSingleObject(semaphore, INFINITE);
//		while (1)
//		{
//			printf("Do you want to make a new planet? y/n\nEnter: \n");
//			create = getch();
//			if (create == 'y') {
//				//If adding the thread was successful, break and wait...
//				if (addThread())
//				{
//					break;
//				}
//				else {
//					printf("Too many threads active! Try again later.\n");
//				}
//			}
//			else if (create == 'n')
//			{
//				planetmaking = FALSE;
//				break;
//			}
//			else printf("Wrong input!\n");
//		}
//	}
//
//	for (int i = 0; i < MAX_THREADS; i++)
//	{
//		if(threads[i]!=NULL)
//			CloseHandle(threads[i]);
//	}
//	
//	DeleteCriticalSection(&criticalSection);
//	printf("\nShutting down main-process...\n");
//	Sleep(5);
//
//	return 0;
//}
