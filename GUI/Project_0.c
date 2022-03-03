#include <analysis.h>
#include "toolbox.h"
#include <rs232.h>
#include <ansi_c.h>
#include <cvirte.h>		
#include <userint.h>
#include "Project_0.h"



static int panelHandle;
int COM=3;
int motor_direction=1,motor_speed=1,cable_length=0;
char string[30], file_name[200];
int state=1,rpm=0,run_time=0, connect_flag=0,test_mode=0,sorter_on=0,sorter_direction=0;


int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((panelHandle = LoadPanel (0, "Project_0.uir", PANEL)) < 0)
		return -1;
	DisplayPanel (panelHandle);
	RunUserInterface ();
	DiscardPanel (panelHandle);
	CloseCom (COM);
	return 0;
}
//--------------------------------------------------- 

void checkVal(char data[]);

//--------------------------------------------------- 
int CVICALLBACK Quit (int panel, int event, void *callbackData,
					  int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:
			if(connect_flag){
				sprintf(string,"%d,%d",0,0);			  //Goes to Idle if the program closed
				ComWrt (COM, string, strlen(string)+1);
			}
			QuitUserInterface (0);
			break;
	}
	return 0;
}
//--------------------------------------------------- 
void CVICALLBACK SerialFunc (int portNumber, int eventMask, void *callbackData)
{
	char message[100];
	int bytesRead;
	static int lines = 0;
	
	while(GetInQLen (COM))
	{
		bytesRead = ComRdTerm (COM, message, 100, '\n');
		
		message[bytesRead-1]='\0';
		
		InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, message);
		lines++;
		
		SetCtrlAttribute (panelHandle, PANEL_TEXTBOX, ATTR_FIRST_VISIBLE_LINE, lines);
		
		if((message[0]>= '3' && message[0]<= '5'))
			checkVal(message);
		
		
	}
	
}
//--------------------------------------------------- 
int CVICALLBACK connectToArdu (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle, PANEL_COMPORT, &COM);
			OpenComConfig (COM, "", 9600, 0, 8, 1, 512, 512);
			FlushInQ (COM);
			InstallComCallback (COM, LWRS_RXFLAG, 0, '\n', SerialFunc, 0);
			connect_flag=1;
			break;														
	}
	return 0;
}
//--------------------------------------------------- 
void checkVal(char data[])
{
	if(data[0]!='5')
		sscanf(data,"%d,%d,%d",&state,&rpm,&run_time);
	if(data[0]=='5')
		sscanf(data,"%d,%d",&state,&cable_length);
	switch(state){
		
	case 3:
		SetCtrlVal (panelHandle, PANEL_DRUM_RPM, rpm);
		PlotPoint (panelHandle, PANEL_GRAPH_DRUM, run_time, rpm, VAL_SOLID_CIRCLE, VAL_RED);
		
		
	break;
    case 4:
		SetCtrlVal (panelHandle, PANEL_SORTER_RPM, rpm);
		PlotPoint (panelHandle, PANEL_GRAPH_SORTER, run_time, rpm, VAL_SOLID_CIRCLE, VAL_RED);
	break;
	case 5:
		SetCtrlVal (panelHandle, PANEL_CABLE, cable_length);
	break;
	}
}
//---------------------------------------------------
int CVICALLBACK SetState (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(connect_flag==1){
				GetCtrlVal (panelHandle, PANEL_STATE, &motor_direction);
				GetCtrlVal (panelHandle, PANEL_SPEED, &motor_speed);
				GetCtrlVal (panelHandle, PANEL_TEST, &test_mode);
				
				sprintf(string,"%d,%d,%d",motor_direction,motor_speed,test_mode);
				ComWrt (COM, string, strlen(string)+1);
			}
			
			
			break;
	}
	return 0;
}
//---------------------------------------------------
int CVICALLBACK clearGraph (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			DeleteGraphPlot (panelHandle, PANEL_GRAPH_DRUM, -1, VAL_IMMEDIATE_DRAW);

			break;
	}
	return 0;
}
//---------------------------------------------------
int CVICALLBACK clearGraph_Sorter (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			DeleteGraphPlot (panelHandle, PANEL_GRAPH_SORTER, -1, VAL_IMMEDIATE_DRAW);
			

			break;
	}
	return 0;
}

//---------------------------------------------------
int CVICALLBACK exportGraph (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			sprintf(file_name,"Exproted Graph Sorter Speed.png");
			SaveCtrlDisplayToFile (panelHandle, PANEL_GRAPH_SORTER, 1, -1, -1, file_name);
			sprintf(file_name,"Exproted Graph Drum Speed.png");
			SaveCtrlDisplayToFile (panelHandle, PANEL_GRAPH_DRUM, 1, -1, -1, file_name);

			break;
	}
	return 0;
}
//---------------------------------------------------
int CVICALLBACK StopFunc (int panel, int control, int event,
						  void *callbackData, int eventData1, int eventData2)
{
	
	switch (event)
	{
		case EVENT_COMMIT:
			if(connect_flag==1){
				sprintf(string,"%d,%d",0,0);
				ComWrt (COM, string, strlen(string)+1);
			}
			break;
	}
	return 0;
}

int CVICALLBACK DisconnectToArdu (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(connect_flag!=0){
				CloseCom (COM);
			   	connect_flag=0;
				InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, "Arduino is Disconnected");
			}
			
			
			break;
	}
	return 0;
}

int CVICALLBACK testSorter (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(connect_flag==1){
				GetCtrlVal (panelHandle, PANEL_TEST_SORTER, &sorter_on);
				if(sorter_on==1)
				{
					sprintf(string,"3,%d",sorter_on);
					ComWrt (COM, string, strlen(string)+1);
				}
				else 
				{
					sprintf(string,"%d,%d",0,0);
					ComWrt (COM, string, strlen(string)+1);
				}
			}
			break;
	}
	return 0;
}

int CVICALLBACK sorterDirection (int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(connect_flag==1){
				GetCtrlVal (panelHandle, PANEL_SORTER_DIRECTION, &sorter_direction);
				sprintf(string,"*,%d",sorter_direction);
				ComWrt (COM, string, strlen(string)+1);
			}

			break;
	}
	return 0;
}

int CVICALLBACK stringFunc (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(connect_flag==1){
				GetCtrlVal (panelHandle, PANEL_STRING, string);
				ComWrt (COM, string, strlen(string)+1);
			}

			break;
	}
	return 0;
}
