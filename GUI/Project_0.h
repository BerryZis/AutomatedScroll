/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                            1       /* callback function: Quit */
#define  PANEL_DISCONNECT_ARDU            2       /* control type: command, callback function: DisconnectToArdu */
#define  PANEL_CONNECT_ARDU               3       /* control type: command, callback function: connectToArdu */
#define  PANEL_TEXTBOX                    4       /* control type: textBox, callback function: (none) */
#define  PANEL_CABLE                      5       /* control type: numeric, callback function: (none) */
#define  PANEL_SORTER_RPM                 6       /* control type: numeric, callback function: (none) */
#define  PANEL_DRUM_RPM                   7       /* control type: numeric, callback function: (none) */
#define  PANEL_COMPORT                    8       /* control type: numeric, callback function: (none) */
#define  PANEL_STRING                     9       /* control type: string, callback function: (none) */
#define  PANEL_GRAPH_SORTER               10      /* control type: graph, callback function: (none) */
#define  PANEL_GRAPH_DRUM                 11      /* control type: graph, callback function: (none) */
#define  PANEL_SEND_STRING                12      /* control type: command, callback function: stringFunc */
#define  PANEL_CLEAR_SORTER               13      /* control type: command, callback function: clearGraph_Sorter */
#define  PANEL_CLEAR_DRUM                 14      /* control type: command, callback function: clearGraph */
#define  PANEL_DECORATION_5               15      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION_4               16      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION_3               17      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION_7               18      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION_6               19      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION_2               20      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION                 21      /* control type: deco, callback function: (none) */
#define  PANEL_EXP_PNG_DRUM               22      /* control type: command, callback function: exportGraph */
#define  PANEL_TEXTMSG_4                  23      /* control type: textMsg, callback function: (none) */
#define  PANEL_TEXTMSG_3                  24      /* control type: textMsg, callback function: (none) */
#define  PANEL_TEXTMSG_2                  25      /* control type: textMsg, callback function: (none) */
#define  PANEL_STATE                      26      /* control type: binary, callback function: (none) */
#define  PANEL_TEST_SORTER                27      /* control type: binary, callback function: testSorter */
#define  PANEL_TEST                       28      /* control type: binary, callback function: (none) */
#define  PANEL_SPEED                      29      /* control type: binary, callback function: (none) */
#define  PANEL_STOP                       30      /* control type: pictButton, callback function: StopFunc */
#define  PANEL_SET_STATE                  31      /* control type: command, callback function: SetState */
#define  PANEL_SORTER_DIRECTION           32      /* control type: binary, callback function: sorterDirection */
#define  PANEL_TEXTMSG                    33      /* control type: textMsg, callback function: (none) */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK clearGraph(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK clearGraph_Sorter(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK connectToArdu(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DisconnectToArdu(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK exportGraph(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Quit(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SetState(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK sorterDirection(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK StopFunc(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK stringFunc(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK testSorter(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
