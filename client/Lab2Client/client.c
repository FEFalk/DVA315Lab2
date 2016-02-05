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

#define MESSAGE "Hello!"
#define MAX_THREADS 2
#define UWM_CHANGENAME (WM_APP + 1000)
HANDLE semaphore;
CRITICAL_SECTION criticalSection;
HANDLE threads[MAX_THREADS];
int counter = 0;
void deletePlanetThread(int index);

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

/*HERE STARTS THE LAB 3 CODE!!!!!!!!!!!!!!!!!!!!!!!*/

planet_type LocalPlanets[] = {
	{TEXT("Venus"), 300, 300, 0.01, 0.001, 1000, 2000},
	{ TEXT("Jupiter"), 300, 300, 0.01, 0.001, 1000, 2000 },
	{ TEXT("Neptunus"), 300, 300, 0.01, 0.001, 1000, 2000 }
};

INT_PTR CALLBACK ListBoxExampleProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
		// Add items to list. 
		HWND hwndList = GetDlgItem(hDlg, IDC_LIST2);
		for (int i = 0; i < ARRAYSIZE(LocalPlanets); i++)
		{
			int pos = (int)SendMessage(hwndList, LB_ADDSTRING, 0,
				(LPARAM)LocalPlanets[i].name);
			// Set the array index of the player as item data.
			// This enables us to retrieve the item from the array
			// even after the items are sorted by the list box.
			SendMessage(hwndList, LB_SETITEMDATA, pos, (LPARAM)i);
		}
		// Set input focus to the list box.
		SetFocus(hwndList);
		return TRUE;
	}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;

		case IDC_LIST2:
		{
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE:
			{
				HWND hwndList = GetDlgItem(hDlg, IDC_LIST2);

				// Get selected index.
				int lbItem = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0);

				// Get item data.
				int i = (int)SendMessage(hwndList, LB_GETITEMDATA, lbItem, 0);

				return TRUE;
			}
			}
		}
		return TRUE;
		}
	}
	return FALSE;
}

void onButtonClick(HWND hDlg) {
	char buffer[128];
	int msg = 1;
	sprintf(buffer, "I am MessageBox");
	MessageBox(hDlg, buffer, "Note", MB_OK);
	ListBoxExampleProc(hDlg, WM_INITDIALOG,
		IDC_LIST2, NULL);
	//SetWindowText(hDlg, TEXT("TESTING SETWINDOWTEXT")); /*Edits the name of the dialog window ( which is hDlg )*/
	//SendMessage(IDC_LIST2, msg, buffer, NULL); /*Queues a message in IDC_LIST2s messagebox*/
}
void onCancel(HWND hDlg) {
	SendMessage(hDlg, WM_CLOSE, 0, 0);
}
void onClose(HWND hDlg) {
	if (MessageBox(hDlg,
		TEXT("Close the window?"), TEXT("Close"),
		MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
		DestroyWindow(hDlg);
	}
}
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:	onCancel(hDlg);	return TRUE;
		case IDC_BUTTON3:	onButtonClick(hDlg);	return TRUE;
		}
		break;

	case WM_CLOSE:	onClose(hDlg);	return TRUE;

	case WM_DESTROY:	PostQuitMessage(0);	return TRUE;
	}  
	return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	HWND hDlg;
	BOOL ret;
	MSG msg;

	hDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG2), 0, DialogProc, 0);
	ShowWindow(hDlg, nCmdShow);


	while ((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
		if (ret == -1) /* error found */
			return -1;

		if (!IsDialogMessage(hDlg, &msg)) {
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
