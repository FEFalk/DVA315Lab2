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

planet_type *getPlanetWithPID(char pid[30]);
HANDLE semaphore;
CRITICAL_SECTION criticalSection;
int counter = 0;
void deletePlanetThread(int index);
planet_type *localDatabase = NULL;
HWND hDlgMain;
/****************************************************************
* Function: createPlanet  										*
* Purpose: Creates a new planet through I/O and sends it to		*
*			 server. Waits for incoming messages from server.	*
* @param threadsArrayIndex - The index of the thread in the		*
*					handle-array(used to know which element		*
*					to delete).									*
*****************************************************************/
void planetThread(int serverListIndex) {
	DWORD threadId = GetCurrentThreadId();
	HWND serverPlanetsList = GetDlgItem(hDlgMain, ID_LIST_SERVER_PLANETS);
	HWND serverMessagesList = GetDlgItem(hDlgMain, ID_LIST_SERVER_MESSAGES);
	BOOL loop = TRUE;
	DWORD bytesRead;
	char buffer[1024];
	HANDLE mailbox;
	int c;
	char *serverMessage = (char *)calloc(20, sizeof(char));
	/* create a mailslot that clients can use to pass requests through   */
	/* (the clients use the name below to get contact with the mailslot) */
	/* NOTE: The name of a mailslot must start with "\\\\.\\mailslot\\"  */

	//Create mailbox threadid
	char mailslotName[128];
	sprintf(mailslotName, "\\\\.\\mailslot\\%d", threadId);
	mailbox = mailslotCreate(mailslotName);


	//READ-LOOP
	for (;;) {

		bytesRead = mailslotRead(mailbox, &buffer, strlen(buffer));


		//If a message is recieved, print out the reason why the planet died. 
		//! TO BE FIXED !
		if (bytesRead != 0) {
			buffer[bytesRead] = '\0';

			SYSTEMTIME st;
			GetLocalTime(&st);

			planet_type *planet = getPlanetWithPID(threadId);
			if (strcmp(buffer, "Life") == 0)
			{
				sprintf(serverMessage, "[%d:%d:%d] Planet '%s' has died. Reason: Life = 0", st.wHour, st.wMinute, st.wSecond, planet->name);
				SendMessage(serverMessagesList, LB_ADDSTRING, 0, (LPARAM)serverMessage);
				SendMessage(serverPlanetsList, LB_DELETESTRING, serverListIndex, NULL);
				/*printf("\nPlanet '%s' has died. Reason: Life = 0\n", planet->name);*/
				break;
			}
			else if (strcmp(buffer, "OOBX") == 0)
			{
				SendMessage(serverMessagesList, LB_ADDSTRING, 0, (LPARAM)TEXT("Planet '%s' has died. Reason: OOB in X", p));
				SendMessage(serverPlanetsList, LB_DELETESTRING, serverListIndex, NULL);
				/*printf("\nPlanet '%s' has died. Reason: OOB in X\n", planet->name);*/
				break;
			}
			else
			{
				SendMessage(serverMessagesList, LB_ADDSTRING, 0, (LPARAM)TEXT("Planet '%s' has died. Reason: OOB in Y", p));
				SendMessage(serverPlanetsList, LB_DELETESTRING, serverListIndex, NULL);
				/*printf("\nPlanet '%s' has died. Reason: OOB in Y\n", planet->name);*/

				break;
			}

		}
	}

	mailslotClose(mailbox);
}

///********************************************************************\
//* Function: addThread											     *
//* Purpose: Adds a thread to the thread-handles array.                *
//* @return - Returns TRUE or FALSE depending on if the array had an	 *
//*			open spot or not.										 *
///********************************************************************/
//BOOL addThread()
//{
//	for (int i = 0; i < MAX_THREADS; i++)
//	{
//		//If spot in array is open, create thread there
//		if (threads[i]==NULL)
//		{
//			threads[i] = threadCreate((LPTHREAD_START_ROUTINE)createPlanet, i);
//			return TRUE;
//		}
//	}
//	return FALSE;
//}
//
///********************************************************************\
//* Function: deleteThread											 *
//* Purpose: Deletes a thread-handle from the array and closes it.     *
//* @param index - Index of the thread to delete.						 *
///********************************************************************/
//void deletePlanetThread(int index)
//{
//	CloseHandle(threads[index]);
//	threads[index] = NULL;
//}

void fillArrayFromDatabase(planet_type *buf)
{

	planet_type *traverser = localDatabase;
	int i;
	if (traverser != 0)
	{
		for (i = 0; traverser != 0; i++)
		{
			memcpy(buf + i, traverser, sizeof(planet_type));
			traverser = traverser->next;
		}
	}
}
int getDatabaseSize()
{
	planet_type *traverser = localDatabase;
	int i;
	if (traverser != 0)
	{
		for (i = 0; traverser->next != 0; i++)
		{
			traverser = traverser->next;
		}
	}
	return i;
}
planet_type *getPlanetWithPID(DWORD pid)
{
	char *threadIDString = calloc(20, sizeof(char));
	_itoa(pid, threadIDString, 10);
	planet_type *traverser = localDatabase;
	if (traverser != 0)
	{
		while (strncmp(traverser->pid, threadIDString, strlen(threadIDString)) != 0)
		{
			traverser = traverser->next;
		}
	}

	return traverser;

}
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
planet_type *getPlanetAt(int index)
{
	planet_type *traverser = localDatabase;
	if (traverser != 0)
	{
		for (int i = 0; i < index; i++)
		{
			traverser = traverser->next;
		}
	}

	return traverser;
}
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
			planet_type *planet = (planet_type*)calloc(1, sizeof(planet_type));
			planet->next = NULL;
			if (checkFields(hDlg, localPlanetsList, TRUE, planet))
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
		if (checkFieldsEmpty(hDlg) == FALSE)
		{
			if (MessageBox(hDlg,
				TEXT("Close the window?"), TEXT("Close"),
				MB_ICONQUESTION | MB_YESNO) == IDYES)
			{
				ShowWindow(hDlg, SW_HIDE);
			}
		}
		else
		{
			ShowWindow(hDlg, SW_HIDE);
		}
		return TRUE;

	case WM_DESTROY:
		return TRUE;
	}
	return FALSE;
}

void showEditPlanet(HWND hDlg, BOOL hide) {
	if (!hide)
	{
		ShowWindow(GetDlgItem(hDlg, ID_BUTTON_EDIT_OK), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, ID_BUTTON_EDIT_CANCEL), SW_SHOW);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_NAME), EM_SETREADONLY, hide, 0); //hide is a bool
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_LIFE), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_MASS), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_MASS2), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONY), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONX), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYY), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYX), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONY2), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONX2), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYY2), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYX2), EM_SETREADONLY, hide, 0);
	}
	else {
		ShowWindow(GetDlgItem(hDlg, ID_BUTTON_EDIT_OK), SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, ID_BUTTON_EDIT_CANCEL), SW_HIDE);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_NAME), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_LIFE), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_MASS), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_MASS2), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONY), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONX), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYY), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYX), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONY2), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONX2), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYY2), EM_SETREADONLY, hide, 0);
		SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYX2), EM_SETREADONLY, hide, 0);
	}

}

HANDLE mailSlot;
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{

		//Initializes the "Add Planet"-window(dialog)
		addPlanetDialog = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_DIALOG_ADD_PLANET), hDlg, addPlanetProc);
		if (addPlanetDialog == NULL)
		{
			MessageBox(hDlg, "CreateDialog returned NULL", "Warning!",
				MB_OK | MB_ICONINFORMATION);
		}

		//Connect to the server mailbox
		mailSlot = mailslotConnect("\\\\.\\mailslot\\mailbox");
		if (mailSlot == NULL) {
			MessageBox(hDlg, "Failed to get a handle to the mailslot!!\nHave you started the server?", "Warning!",
				MB_OK | MB_ICONINFORMATION);
			PostQuitMessage(0);
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
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_NAME), EM_SETSEL, 0, -1);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_NAME), WM_CLEAR, 0, 0);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_LIFE), EM_SETSEL, 0, -1);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_LIFE), WM_CLEAR, 0, 0);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_X_P), EM_SETSEL, 0, -1);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_X_P), WM_CLEAR, 0, 0);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_Y_P), EM_SETSEL, 0, -1);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_Y_P), WM_CLEAR, 0, 0);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_X_V), EM_SETSEL, 0, -1);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_X_V), WM_CLEAR, 0, 0);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_Y_V), EM_SETSEL, 0, -1);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_Y_V), WM_CLEAR, 0, 0);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_MASS), EM_SETSEL, 0, -1);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_MASS), WM_CLEAR, 0, 0);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_X_P2), EM_SETSEL, 0, -1);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_X_P2), WM_CLEAR, 0, 0);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_Y_P2), EM_SETSEL, 0, -1);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_Y_P2), WM_CLEAR, 0, 0);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_X_V2), EM_SETSEL, 0, -1);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_X_V2), WM_CLEAR, 0, 0);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_Y_V2), EM_SETSEL, 0, -1);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_Y_V2), WM_CLEAR, 0, 0);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_MASS2), EM_SETSEL, 0, -1);
			SendMessage(GetDlgItem(addPlanetDialog, ID_EDIT_PLANET_MASS2), WM_CLEAR, 0, 0);

			ShowWindow(addPlanetDialog, SW_SHOW);
			return TRUE;
		}
		case ID_BUTTON_EDIT:
		{
			HWND localPlanetsList = GetDlgItem(hDlg, ID_LIST_LOCAL_PLANETS);
			// Get selected index. (In ListBox)
			int lbItem = (int)SendMessage(localPlanetsList, LB_GETCURSEL, 0, 0);
			if (lbItem == LB_ERR) {
				MessageBox(hDlg, "Select a planet", "Warning!",
					MB_OK | MB_ICONINFORMATION);
				break;
			}
			showEditPlanet(hDlg, FALSE); // Enable editboxes in edit window
			return TRUE;
		}
		case ID_BUTTON_EDIT_OK:
		{
			HWND localPlanetsList = GetDlgItem(hDlg, ID_LIST_LOCAL_PLANETS);
			// Get selected index. (In ListBox)
			int lbItem = (int)SendMessage(localPlanetsList, LB_GETCURSEL, 0, 0);

			TCHAR tempString[128];


			if (checkFields(hDlg, localPlanetsList, FALSE, getPlanetAt(lbItem))) //Check fields for wrong input and store floats in one variable
			{
				////store the input from the boxes in the struct
				//GetWindowText(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_NAME), tempString, 128);
				//sprintf(getPlanetAt(lbItem)->name, tempString);
				//GetWindowText(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_LIFE), tempString, 128);
				//getPlanetAt(lbItem)->life = atoi(tempString);
				//GetWindowText(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_MASS), tempString, 128);
				//getPlanetAt(lbItem)->mass = atof(tempString);
				//GetWindowText(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONX), tempString, 128);
				//getPlanetAt(lbItem)->sx = atof(tempString);
				//GetWindowText(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONY), tempString, 128);
				//getPlanetAt(lbItem)->sy = atof(tempString);
				//GetWindowText(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYX), tempString, 128);
				//getPlanetAt(lbItem)->vx = atof(tempString);
				//GetWindowText(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYY), tempString, 128);
				//getPlanetAt(lbItem)->vy = atof(tempString);

				int pos = (int)SendMessage(localPlanetsList, LB_GETCURSEL, 0, 0);
				SendMessage(localPlanetsList, LB_DELETESTRING, pos, (LPARAM)tempString);
				SendMessage(localPlanetsList, LB_ADDSTRING, pos, (LPARAM)getPlanetAt(lbItem)->name);
				SendMessage(localPlanetsList, LB_SETSEL, TRUE, pos);
			}
			else
			{
				//Do something when a field is empty
			}


			////Clear all input from edit boxes.
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_NAME), EM_SETSEL, 0, -1);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_NAME), WM_CLEAR, 0, 0);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_LIFE), EM_SETSEL, 0, -1);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_LIFE), WM_CLEAR, 0, 0);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_MASS), EM_SETSEL, 0, -1);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_MASS), WM_CLEAR, 0, 0);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONX), EM_SETSEL, 0, -1);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONX), WM_CLEAR, 0, 0);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONY), EM_SETSEL, 0, -1);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONY), WM_CLEAR, 0, 0);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYX), EM_SETSEL, 0, -1);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYX), WM_CLEAR, 0, 0);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYY), EM_SETSEL, 0, -1);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYY), WM_CLEAR, 0, 0);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_MASS2), EM_SETSEL, 0, -1);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_MASS2), WM_CLEAR, 0, 0);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONX2), EM_SETSEL, 0, -1);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONX2), WM_CLEAR, 0, 0);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONY2), EM_SETSEL, 0, -1);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONY2), WM_CLEAR, 0, 0);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYX2), EM_SETSEL, 0, -1);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYX2), WM_CLEAR, 0, 0);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYY2), EM_SETSEL, 0, -1);
			//SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYY2), WM_CLEAR, 0, 0);

			showEditPlanet(hDlg, TRUE);
			return TRUE;
		}
		case ID_BUTTON_EDIT_CANCEL:
		{
			showEditPlanet(hDlg, TRUE);
			return TRUE;
		}
		case ID_FILE_LOAD:
		{
			loadPlanets(hDlg);

			return TRUE;
		}
		case ID_FILE_SAVE:
		{
			//SAVE FILE
			savePlanets(hDlg);
			return TRUE;
		}
		case ID_FILE_EXIT:
		{
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return TRUE;
		}
		case ID_BUTTON_SEND:
		{
			HWND localPlanetsList = GetDlgItem(hDlg, ID_LIST_LOCAL_PLANETS);
			HWND serverPlanetsList = GetDlgItem(hDlg, ID_LIST_SERVER_PLANETS);

			// Get selected index. (In ListBox)
			int lbItem = SendMessage(localPlanetsList, LB_GETCURSEL, 0, 0);
			if (lbItem == LB_ERR)
			{
				MessageBox(hDlg, TEXT("No planet is selected! Please select a planet to send first.", bytesWritten), "Error!", MB_OK | MB_ICONINFORMATION);
				return TRUE;
			}

			planet_type *selectedPlanet = getPlanetAt(lbItem);

			int pos = (int)SendMessage(serverPlanetsList, LB_ADDSTRING, 0, (LPARAM)selectedPlanet->name);
			HANDLE thread;
			thread = threadCreate((LPTHREAD_START_ROUTINE)planetThread, pos);

			char *temp = calloc(20, sizeof(char));
			_itoa(GetThreadId(thread), temp, 10);
			strncpy(selectedPlanet->pid, temp, strlen(temp) + 1);
			DWORD bytesWritten = 0;
			//Sends the planet to server through the mailslot
			bytesWritten = mailslotWrite(mailSlot, selectedPlanet, sizeof(*localDatabase));
			if (bytesWritten != -1)
			{
				char msg[100];
				sprintf(msg, "Data sent to server (bytes = %d)", bytesWritten);
				MessageBox(hDlg, TEXT(msg), "Success!", MB_OK | MB_ICONINFORMATION);
			}
			else
			{
				CloseHandle(thread);
				SendMessage(serverPlanetsList, LB_DELETESTRING, pos, NULL);
				MessageBox(hDlg, TEXT("Failed to send data to server!", bytesWritten), "Error!", MB_OK | MB_ICONINFORMATION);
				return TRUE;
			}
			return TRUE;
		}
		case ID_LIST_LOCAL_PLANETS:
			{
				switch (HIWORD(wParam))
				{
				case LBN_SELCHANGE:
					{
						selectedPlanet(hDlg);
						return TRUE;
					}
				}
				return TRUE;
			}
		}
		return FALSE;
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
		mailslotClose(mailSlot);
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{

	BOOL ret;
	MSG msg;
	HMENU hMenu;
	hDlgMain = CreateDialogParam(hInstance, MAKEINTRESOURCE(ID_DIALOG_MAIN), 0, DialogProc, 0);
	ShowWindow(hDlgMain, nCmdShow);

	hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1));
	SetMenu(hDlgMain, hMenu);

	while ((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
		if (ret == -1) /* error found */
			return -1;
		if (!IsDialogMessage(addPlanetDialog, &msg))
		{
			TranslateMessage(&msg); /* translate virtual-key messages */
			DispatchMessage(&msg); /* send it to dialog procedure */
		}
	}
	return 0;
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
