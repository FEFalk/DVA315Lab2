#include <stdio.h>
#include <windows.h>
#include <string.h>
#include "wrapper.h"
#include "resource.h"

#define TIMERID			100  /* id for timer that is used by the thread that manages the window where graphics is drawn */
#define DEFAULT_STACK_SIZE	1024
#define TIME_OUT			MAILSLOT_WAIT_FOREVER 


/* ATTENTION!!! calls that require a time out, use TIME_OUT constant, specifies that calls are blocked forever */



DWORD threadCreate(LPTHREAD_START_ROUTINE threadFunc, LPVOID threadParams) {

	/* Creates a thread running threadFunc */
	/* optional parameters (NULL otherwise)and returns its id! */

	HANDLE thread = CreateThread(
		NULL,
		DEFAULT_STACK_SIZE,
		threadFunc,
		threadParams,
		0,
		NULL);

	if (thread == NULL)
	{
		MessageBox(NULL, NULL, TEXT("Error"), MB_OK);
		ExitProcess(3);
	}
	
	return (DWORD)thread;
}


HANDLE mailslotCreate(char *name) {

	/* Creates a mailslot with the specified name and returns the handle */
	/* Should be able to handle a messages of any size */

	HANDLE newMailSlot;

	newMailSlot = CreateMailslot(
		name,
		0,
		TIME_OUT,
		(LPSECURITY_ATTRIBUTES)NULL);

	if (newMailSlot == INVALID_HANDLE_VALUE)
	{
		printf("CreateMailslot failed with %d\n", GetLastError());
		MessageBox(NULL, "CreateMailslot failed", "Error!!", MB_OK);
		return FALSE;
	}

	return newMailSlot;
}

HANDLE mailslotConnect(char * name) {

	/* Connects to an existing mailslot for writing */
	/* and returns the handle upon success     */

	HANDLE newFile;

	newFile = CreateFile(name,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		0,
		(HANDLE)NULL);
	if (newFile == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile failed with %d.\n", GetLastError());
		return FALSE;
	}

	return newFile;
}

int mailslotWrite(HANDLE mailSlot, void *msg, int msgSize) {

	/* Write a msg to a mailslot, return nr */
	/* of successful bytes written         */
	BOOL fResult;
	DWORD cbWritten;

	fResult = WriteFile(mailSlot,
		msg,
		msgSize,
		&cbWritten,
		(LPOVERLAPPED)NULL);

	if (!fResult)
	{
		printf("WriteFile failed with %d.\n", GetLastError());
		return FALSE;
	}

	printf("Slot written to successfully.\n");

	return cbWritten;

}

int	mailslotRead(HANDLE mailbox, void *msg, int msgSize) {

	/* Read a msg from a mailslot, return nr */
	/* of successful bytes read              */

	/* Read a msg from a mailslot, return nr */
	/* of successful bytes read              */
	DWORD nextSize, messages, cbRead;
	BOOL fResult;

	fResult = GetMailslotInfo(mailbox, (LPDWORD)NULL, &nextSize, &messages, (LPDWORD)NULL);
	if (!fResult) {
		MessageBox(NULL, "GetMailslotInfo Error", "Error!!", MB_OK);
		printf("GetMailslotInfo Error: %d\n", GetLastError());
		return 0;
	}


	if (messages != 0)
	{
		fResult = ReadFile(mailbox, (char*)msg, nextSize, &cbRead, (LPOVERLAPPED)NULL);
		if (!fResult) {
			printf("ReadFile Error: %d\n", GetLastError());
			MessageBox(NULL, "ReadFile Error", "Error!!", MB_OK);
			return 0;
		}
		return cbRead;
	}
	return 0;
}

int mailslotClose(HANDLE mailSlot) {

	/* close a mailslot, returning whatever the service call returns */

	return CloseHandle(mailSlot);
}


/******************** Wrappers for window management, used for lab 2 and 3 ***********************/
/******************** DONT CHANGE!!! JUST FYI ******************************************************/


HWND windowCreate(HINSTANCE hPI, HINSTANCE hI, int ncs, char *title, WNDPROC callbackFunc, int bgcolor) {

	HWND hWnd;
	WNDCLASS wc;

	/* initialize and create the presentation window        */
	/* NOTE: The only important thing to you is that we     */
	/*       associate the function 'MainWndProc' with this */
	/*       window class. This function will be called by  */
	/*       windows when something happens to the window.  */
	if (!hPI) {
		wc.lpszClassName = "GenericAppClass";
		wc.lpfnWndProc = callbackFunc;          /* (this function is called when the window receives an event) */
		wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
		wc.hInstance = hI;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)bgcolor;
		wc.lpszMenuName = "GenericAppMenu";

		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;

		RegisterClass(&wc);
	}

	/* NOTE: This creates a window instance. Don't bother about the    */
	/*       parameters to this function. It is sufficient to know     */
	/*       that this function creates a window in which we can draw. */
	hWnd = CreateWindow("GenericAppClass",
		title,
		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
		0,
		0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hI,
		NULL
		);

	/* NOTE: This makes our window visible. */
	ShowWindow(hWnd, ncs);
	/* (window creation complete) */

	return hWnd;
}

void windowRefreshTimer(HWND hWnd, int updateFreq) {

	if (SetTimer(hWnd, TIMERID, updateFreq, NULL) == 0) {
		/* NOTE: Example of how to use MessageBoxes, see the online help for details. */
		MessageBox(NULL, "Failed setting timer", "Error!!", MB_OK);
		exit(1);
	}
}


/******************** Wrappers for window management, used for lab  3 ***********************/
/*****  Lab 3: Check in MSDN GetOpenFileName and GetSaveFileName  *********/
/**************  what the parameters mean, and what you must call this function with *********/

void loadPlanets(HWND hDlg)
{
	//LOAD FILE
	OPENFILENAME ofn;
	char szFileName[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hDlg;
	ofn.lpstrFilter = "(*.txt); (*.dat)\0*.txt;*.dat\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "txt";
	if (GetOpenFileName(&ofn))
	{
		// Do something usefull with the filename stored in szFileName 
		FILE *ptr_myfile;
		planet_type *buf = (planet_type*)calloc(1, sizeof(planet_type));
		planet_type *p;
		ptr_myfile = fopen(szFileName, "rb");
		if (!ptr_myfile)
		{
			printf("Unable to open file!");
			return 1;
		}
		while (fread(buf, sizeof(planet_type), 1, ptr_myfile) != sizeof(planet_type))
		{
			if (feof(ptr_myfile))
				break;

			p = (planet_type*)calloc(1, sizeof(planet_type));
			memcpy(p, buf, sizeof(planet_type));
			p->next = NULL;
			addPlanet(p);
			SendMessage(GetDlgItem(hDlg, ID_LIST_LOCAL_PLANETS), LB_ADDSTRING, 0, (LPARAM)p->name);
			MessageBox(hDlg, "Successfully added planets to local list!", "Warning!",
				MB_OK | MB_ICONINFORMATION);
		}
		fclose(ptr_myfile);
	}
}

void savePlanets(HWND hDlg)
{
	//LOAD FILE
	OPENFILENAME ofn;
	char szFileName[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hDlg;
	ofn.lpstrFilter = "(*.txt); (*.dat)\0*.txt;*.dat\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = "txt";
	if (GetOpenFileName(&ofn))
	{
		// Do something usefull with the filename stored in szFileName 
		FILE *ptr_myfile;
		planet_type *buf;
		int dSize = getDatabaseSize()+1;
		buf = (planet_type*)calloc(dSize, sizeof(planet_type));
		fillArrayFromDatabase(buf);

		ptr_myfile = fopen(szFileName, "wb");
		if (!ptr_myfile)
		{
			printf("Unable to open file!");
			return 1;
		}

		fwrite(buf, sizeof(planet_type), dSize, ptr_myfile);
		

		MessageBox(hDlg, "Successfully added planets to file!", "Warning!",
			MB_OK | MB_ICONINFORMATION);
		fclose(ptr_myfile);
	}
}
BOOL checkFieldsEmpty(HWND hDlg) 
{
	char *p;
	int i, bufInt = 0;
	char* buf;
	int editBoxArray[7] =
	{
		ID_EDIT_PLANET_NAME,
		ID_EDIT_PLANET_X_P,
		ID_EDIT_PLANET_Y_P,
		ID_EDIT_PLANET_X_V,
		ID_EDIT_PLANET_Y_V,
		ID_EDIT_PLANET_MASS,
		ID_EDIT_PLANET_LIFE
	};

	int len;
	for (int i = 0; i < 7; i++)
	{
		len = GetWindowTextLength(GetDlgItem(hDlg, editBoxArray[i]));
		if (len > 0)
		{
			return FALSE;
		}
		else
		{
			
		}
	}
	return TRUE;
}
BOOL checkFields(HWND hDlg, HWND localPlanetsList, BOOL isAddingPlanet, planet_type *planet)
{
	char *p;
	int i, bufInt = 0;
	char* buf, *buf2;
	int *editBoxArray = calloc(12, sizeof(int));
	if (isAddingPlanet)
	{
		editBoxArray[0] = ID_EDIT_PLANET_NAME;
		editBoxArray[1] = ID_EDIT_PLANET_LIFE;
		editBoxArray[2] = ID_EDIT_PLANET_MASS;
		editBoxArray[3] = ID_EDIT_PLANET_MASS2;
		editBoxArray[4] = ID_EDIT_PLANET_X_P;
		editBoxArray[5] = ID_EDIT_PLANET_X_P2;
		editBoxArray[6] = ID_EDIT_PLANET_Y_P;
		editBoxArray[7] = ID_EDIT_PLANET_Y_P2;
		editBoxArray[8] = ID_EDIT_PLANET_X_V;
		editBoxArray[9] = ID_EDIT_PLANET_X_V2;
		editBoxArray[10] = ID_EDIT_PLANET_Y_V;
		editBoxArray[11] = ID_EDIT_PLANET_Y_V2;
	}
	else
	{
		editBoxArray[0] = ID_EDIT_LOCAL_PLANET_INFO_NAME;
		editBoxArray[1] = ID_EDIT_LOCAL_PLANET_INFO_LIFE;
		editBoxArray[2] = ID_EDIT_LOCAL_PLANET_INFO_MASS;
		editBoxArray[3] = ID_EDIT_LOCAL_PLANET_INFO_MASS2;
		editBoxArray[4] = ID_EDIT_LOCAL_PLANET_INFO_POSITIONX;
		editBoxArray[5] = ID_EDIT_LOCAL_PLANET_INFO_POSITIONX2;
		editBoxArray[6] = ID_EDIT_LOCAL_PLANET_INFO_POSITIONY;
		editBoxArray[7] = ID_EDIT_LOCAL_PLANET_INFO_POSITIONY2;
		editBoxArray[8] = ID_EDIT_LOCAL_PLANET_INFO_VELOCITYX;
		editBoxArray[9] = ID_EDIT_LOCAL_PLANET_INFO_VELOCITYX2;
		editBoxArray[10] = ID_EDIT_LOCAL_PLANET_INFO_VELOCITYY;
		editBoxArray[11] = ID_EDIT_LOCAL_PLANET_INFO_VELOCITYY2;
	}

	int len, totlen;
	for (int i = 0; i < 12; i++)
	{
		len = totlen = GetWindowTextLength(GetDlgItem(hDlg, editBoxArray[i]));
		
		if (len <= 0 || len > 20)
		{
			//error
			char msg[100];
			char fieldName[20];
			switch (i)
			{
			case 0:
				sprintf(fieldName, "Name");
				break;
			case 1:
				sprintf(fieldName, "Life");
				break;
			case 2:
				sprintf(fieldName, "Mass");
				break;
			case 3:
				sprintf(fieldName, "Mass (decimals)");
				break;
			case 4:
				sprintf(fieldName, "X-Position");
				break;
			case 5:
				sprintf(fieldName, "X-Position (decimals)");
				break;
			case 6:
				sprintf(fieldName, "Y-Position");
				break;
			case 7:
				sprintf(fieldName, "Y-Position (decimals)");
				break;
			case 8:
				sprintf(fieldName, "X-Velocity");
				break;
			case 9:
				sprintf(fieldName, "X-Velocity (decimals");
				break;
			case 10:
				sprintf(fieldName, "Y-Velocity");
				break;
			case 11:
				sprintf(fieldName, "Y-Velocity (decimals)");
				break;
			default:
				break;

			}
			if (len <= 0)
			{
				sprintf(msg, "The field '%s' is empty! Please fill out all information.", fieldName);
				MessageBox(hDlg, msg, "Warning!",
					MB_OK | MB_ICONINFORMATION);
			}
			else if (len > 20)
			{
				sprintf(msg, "The field '%s' contains too many digits! Please enter values with a maximum of 8 digits.", fieldName);
				MessageBox(hDlg, msg, "Warning!",
					MB_OK | MB_ICONINFORMATION);
			}
			return FALSE;
		}
	}
	for (int i = 0; i < 12; i += 2)
	{
		len = totlen = GetWindowTextLength(GetDlgItem(hDlg, editBoxArray[i]));

		switch (editBoxArray[i])
		{
		case ID_EDIT_PLANET_NAME:
		case ID_EDIT_LOCAL_PLANET_INFO_NAME:
		{
			buf = (char*)GlobalAlloc(GPTR, len + 1);
			GetDlgItemText(hDlg, editBoxArray[i], buf, len + 1);
			if (strlen(buf) > 20)
				MessageBox(hDlg, TEXT("Error too big!\n"), TEXT("Error!"), MB_OK);
			else
			{
				sprintf(planet->name, buf);
				if ((p = strchr(planet->name, '\n')) != NULL)
					*p = '\0';
			}
			i--;
		}
		break;
		case ID_EDIT_PLANET_LIFE:
		case ID_EDIT_LOCAL_PLANET_INFO_LIFE:
		{
			buf = (char*)GlobalAlloc(GPTR, len + 1);
			GetDlgItemText(hDlg, editBoxArray[i], buf, len + 1);

			planet->life = atoi(buf);
			i--;
		}
		break;
		default:
		{
			len = GetWindowTextLength(GetDlgItem(hDlg, editBoxArray[i + 1]));
			totlen += len;
			buf = (char*)GlobalAlloc(GPTR, totlen + 2);
			buf2 = (char*)GlobalAlloc(GPTR, len + 1);
			GetDlgItemText(hDlg, editBoxArray[i], buf, len + 1);
			GetDlgItemText(hDlg, editBoxArray[i + 1], buf2, len + 1);

			strcat(buf, ".");
			strcat(buf, buf2);
			switch (i) {
			case 2: planet->mass = atof(buf); break;
			case 4: planet->sx = atof(buf); break;
			case 6: planet->sy = atof(buf); break;
			case 8: planet->vx = atof(buf); break;
			case 10: planet->vy = atof(buf); break;
			default: break;
			}
		}
		break;
		}
	}

	if (isAddingPlanet)
	{
		planet->next = NULL;
		addPlanet(planet);
	}
	
	GlobalFree((HANDLE)buf);
	return TRUE;
}

void selectedPlanet(HWND hDlg)
{
	HWND localPlanetsList = GetDlgItem(hDlg, ID_LIST_LOCAL_PLANETS);
	char *buff = (char *)calloc(MAX_PATH, sizeof(char));
	char *buff2 = (char *)calloc(MAX_PATH, sizeof(char));

	// Get selected index. (In ListBox)
	int lbItem = (int)SendMessage(localPlanetsList, LB_GETCURSEL, 0, 0);
	planet_type *selectedPlanet = getPlanetAt(lbItem);
	char str[128];
	int i = 0;

	SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_NAME), WM_SETTEXT, 0, selectedPlanet->name);
	_itoa(selectedPlanet->life, str, 10);
	SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_LIFE), WM_SETTEXT, 0, str);

	sprintf(str, "%.3f", selectedPlanet->mass);
	stringSeparate(&str, buff, &buff2);

	SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_MASS), WM_SETTEXT, 0, buff);
	SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_MASS2), WM_SETTEXT, 0, buff2);

	sprintf(str, "%.3f", selectedPlanet->sx);
	stringSeparate(&str, buff, &buff2);

	SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONX), WM_SETTEXT, 0, buff);
	SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONX2), WM_SETTEXT, 0, buff2);

	sprintf(str, "%.3f", selectedPlanet->sy);
	stringSeparate(&str, buff, &buff2);

	SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONY), WM_SETTEXT, 0, buff);
	SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_POSITIONY2), WM_SETTEXT, 0, buff2);

	sprintf(str, "%.3f", selectedPlanet->vx);
	stringSeparate(&str, buff, &buff2);

	SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYX), WM_SETTEXT, 0, buff);
	SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYX2), WM_SETTEXT, 0, buff2);

	sprintf(str, "%.3f", selectedPlanet->vy);
	stringSeparate(&str, buff, &buff2);

	SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYY), WM_SETTEXT, 0, buff);
	SendMessage(GetDlgItem(hDlg, ID_EDIT_LOCAL_PLANET_INFO_VELOCITYY2), WM_SETTEXT, 0, buff2);

}

HANDLE OpenFileDialog(char* string, DWORD accessMode, DWORD howToCreate)
{

	OPENFILENAME opf;
	char szFileName[_MAX_PATH] = "";

	opf.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT;
	opf.lpstrDefExt = "dat";
	opf.lpstrCustomFilter = NULL;
	opf.lStructSize = sizeof(OPENFILENAME);
	opf.hwndOwner = NULL;
	opf.lpstrFilter = NULL;
	opf.lpstrFile = szFileName;
	opf.nMaxFile = _MAX_PATH;
	opf.nMaxFileTitle = _MAX_FNAME;
	opf.lpstrInitialDir = NULL;
	opf.lpstrTitle = string;
	opf.lpstrFileTitle = NULL;

	if (accessMode == GENERIC_READ)
		GetOpenFileName(&opf);
	else
		GetSaveFileName(&opf);

	return CreateFile(szFileName,
		accessMode,
		0,
		NULL,
		howToCreate,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
}

void stringSeparate(char *s, char *digitString, char **decimalString)
{
	strcpy(digitString, s);
	*decimalString = s;
	for (int i = 0;; i++)
	{
		if (s[i] == '.')
		{
			*decimalString += i + 1;
			digitString[i] = '\0';
			break;
		}
	}
}