#include  <X11/Xlib.h>
#include  <X11/Intrinsic.h>
#include  <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/PanedW.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>
#include <Xm/Scale.h>
#include <Xm/SelectioB.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/PushBG.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include "/usr/people/4Dgifts/iristools/include/image.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FALLBACK_RESOURCES "X.DrawTracks"
#define TOP_DOWN
#define NO_DEBUG
int MAXLEN=-1;
#define OK 1
#define CANCEL 2
#define LIGHT 1
#define DARK 2
#define INVALID_COLOR USHRT_MAX
/*************************************************************/
XmStringCharSet char_set=XmSTRING_DEFAULT_CHARSET;

typedef struct {
   unsigned short    falseAlarms,
                     groupedMeas,
                     smoothContour,
                     startFrame,
                     endFrame,
                     cornerFrame,
                     pointsOnTrajectory,
                     corners,
                     minContourLength;
}  Options;

typedef struct {
    char flag;
    int mx;
    int my;
    int sx;
    int sy;
    int fno;
}  PixelDetail;

typedef struct {
     int id; 
     char color[25];
     int color_index;
     int len;
     int fno;
     unsigned short red,green,blue;
     PixelDetail *array;
} Contour;

typedef struct {
    int x;
    int y;
} FalseAlarm;

typedef struct {
   char *colorName;
   unsigned long lightPixel;
   unsigned long darkPixel;
} ColorInfo;

typedef struct {
  int ncnt[7];        /* number of neighborhoods */
  int x[7][8];       /* x offsets for neighborhoods (clockwise) */
  int y[7][8];       /* y offsets */
} Searchvol, *SearchvolPtr;
/*******************************************************/

Searchvol svol = {{1,4,4,4,8,4,4},

/* x offsets */   {{0,0,0,0,0,0,0,0},        /* 0         */
                   {-1,0,1,0,0,0,0,0},       /* 1         */
                   {-1,-1,1,1,0,0,0,0},      /* Sqrt[2]   */
                   {-2,0,2,0,0,0,0,0},       /* 2         */
                   {-2,-1,1,2,2,1,-1,-2},    /* Sqrt[5]   */
                   {-2,2,2,-2,0,0,0,0},      /* 2*Sqrt[2] */
                   {-3,0,3,0,0,0,0,0}},      /* 3         */

/* y offsets */   {{0,0,0,0,0,0,0,0},        /* 0         */
                   {0,1,0,-1,0,0,0,0},       /* 1         */
                   {-1,1,1,-1,0,0,0,0},      /* Sqrt[2]   */
                   {0,2,0,-2,0,0,0,0},       /* 2         */
                   {1,2,2,1,-1,-2,-2,-1},    /* Sqrt[5]   */
                   {2,2,-2,-2,0,0,0,0},      /* 2*Sqrt[2] */

                   {0,3,0,-3,0,0,0,0}}};     /* 3         */

struct {
      GC  gc;
      ColorInfo colors[100];
      int ncolors;
} GcInfo;

static char infoTxtBuf[2000];
static XImage *xi;
static FalseAlarm *falseAlarmList;
static int nContours,nFalseAlarms;
static Contour **contourList;
static Contour ***contourLayout;
static Options displayOptions,tmpOptions;
static int currentContour = -1;
static char **data;
static int currentCornerFrame=0;
static int currentFrame=0;
static int **xcorners;
static int **ycorners;
static int *nCorners;
int start,end;

Widget displayPB,infoPB, endPB,savePB,sourceInfoPB,pdialog,sourceInfoDialog;
Widget colorList;
Widget colorSelectionDialog;
Widget changeColorPB=NULL;
Widget dialogForm, dialogFrame,checkBox,scale,minLenLabel,minLenValLabel,startFrameLabel,
       startFrameValLabel,endFrameLabel,endFrameValLabel,cornerFrameLabel,cornerFrameValLabel;
Widget root,canvas;
Widget toggle1,toggle2,toggle3,toggle4,toggle5;
Widget CreatePushButton();
Widget CreateToggle();
Widget CreateBulletinDialog();
Widget CreateDisplayOptionDialog(Widget , char*);
Widget displayOptionDialog,infoDialog;
Widget infoDisplayLabel=NULL;
Widget infoDisplayLabel1=NULL;
Widget CreateContourInfoDialog(Widget parent, char *name);
Widget GetTopShell(Widget w);
Colormap  cmap;
IMAGE     *SgiImage;
Dimension width, height;
Display   *display;
Window	window;
Screen	*screen;
void noMatchCB(Widget w,int client_data,
	       XmSelectionBoxCallbackStruct *call_data);
void get_gray_data(char* data,unsigned short* buf,IMAGE* SgiImage,
                   Dimension,Dimension);

static char grayCmap[256];
void makeCmap();
void changeGC(char *name, unsigned short type);
void drawContour( Contour* contour, char* c);
void infoUpdate();
void CreateSourceInfoDialog(Widget w);
void getInfoTxt();
void get_gray_cmap_data(char* data,unsigned short* buf);
void writeImage(char *name,int xsize, int ysize, char *buf);
void readCorners(char *basename, int start, int end);

XtCallbackProc          displayContourCB(Widget,XtPointer, XtPointer);
XtCallbackProc          displayInfoCB(Widget, XtPointer, XtPointer);
XtCallbackProc          quit(Widget, XtPointer, XtPointer);
XtCallbackProc		saveImgCB();
XtCallbackProc		pdialogCB(Widget,int,XmSelectionBoxCallbackStruct*);
XtCallbackProc		displaySourceInfoCB();
XtCallbackProc          scaleCB(Widget ,int,XmScaleCallbackStruct*);
XtCallbackProc          toggle1CB(),toggle2CB(),toggle3CB(),toggle4CB(),
			 toggle5CB(),applyCB(),closeCB(),
			 minLenValCB(),startFrameValCB(),endFrameValCB(),
			 cornerFrameValCB(),cornerFrameValCB();
XtCallbackProc          sortByLenToggleCB(),
			sortByIdToggleCB(),
			displayAllCB(),
			firstCB(),
			nextCB(),
			changeColorCB(),
			closeInfoCB();

XtCallbackProc          colorSelectionDialogCB(Widget,int,XmSelectionBoxCallbackStruct*);
	  
XtCallbackProc          destroyShellCB(Widget, Widget);
XtCallbackProc          drawImageCB(Widget,XtPointer,XtPointer);


/******************************************************************/
void main(int argc, char **argv)
 {
	 int                    i;
	 char                   prog[80];
         Widget                 menu,create_menu(char*);
	 XtAppContext		app;
         XImage                 *createXimage(unsigned short*),*edgeImg;
	 void  create_canvas(char*), postMenuCB(),handleClickCB();
         void readData();
	 void createContourLayout();
         unsigned short *inBuf;
         void clearTmpOptions(),clearDisplayOptions();
         int nframes,maxCorners;
         char firstImageName[80];
         void readImages(char *basename,int start, int end);


         if (argc != 6) {
            fprintf(stderr,"Usage: %s start end Corner_file_stem Image_file_stem maxCorners <trackFile\n",argv[0]);
            exit(-1);
         }
         start = atoi(argv[1]);
         end = atoi(argv[2]);
         maxCorners=atoi(argv[5]);
         nframes=end-start+1;
         data=(char**)malloc(nframes*sizeof(char*));
         if (data == NULL) {
             fprintf(stderr,"Couldn't allocate space for data\n");
             exit(-1);
         }
   
         xcorners = (int**)malloc(nframes*sizeof(int*));
         if (xcorners == NULL) {
             fprintf(stderr,"Couldn't allocate space for xcorners\n");
             exit(-1);
         }
         ycorners = (int**)malloc(nframes*sizeof(int*));
         if (ycorners == NULL) {
             fprintf(stderr,"Couldn't allocate space for ycorners\n");
             exit(-1);
         }
         nCorners = (int*)malloc(nframes*sizeof(int));
         if (nCorners == NULL) {
             fprintf(stderr,"Couldn't allocate space for nCorners\n");
             exit(-1);
         }

         strcpy(prog,argv[0]);
	 root = XtAppInitialize(&app, FALLBACK_RESOURCES, NULL, 0, &argc, argv, NULL, NULL, 0); 

         sprintf(firstImageName,"%s%d",argv[4],start);
	 create_canvas(firstImageName);

         printf("Reading corners from %d %d\n",start,end);fflush(stdout);
         for (i=0; i<nframes; i++) {
            data[i] = (char*)malloc(width*height*sizeof(char));
            xcorners[i]=(int*)malloc(maxCorners*sizeof(int));
            ycorners[i]=(int*)malloc(maxCorners*sizeof(int));
         }

         clearDisplayOptions(); clearTmpOptions();

	 getInfoTxt();
	 readData();
	 createContourLayout();
     
	 menu = create_menu(prog); 
         makeCmap();
         readImages(argv[4],start,end);
         readCorners(argv[3],start,end);

	 XtAddEventHandler(canvas, ButtonPressMask, FALSE, postMenuCB, menu); 
	 XtAddEventHandler(canvas, ButtonReleaseMask, FALSE, handleClickCB, NULL); 

	 XtAddCallback(canvas, XmNexposeCallback, drawImageCB, NULL); 


	 XtRealizeWidget(root);
	 XtAppMainLoop(app);

	 }
      /***********************************************/
	void  redraw_image()
	{
	   int i,x,y;
	   char *c;

           if (displayOptions.cornerFrame) currentFrame=displayOptions.cornerFrame-start;
           else currentFrame=0;
           if (!(xi = XCreateImage(display,DefaultVisualOfScreen(screen),
                            DefaultDepthOfScreen(screen),ZPixmap,0,data[currentFrame],width,height,32,width))) {
                   fprintf(stderr,"Could not create Image\n");
                   exit(-1);
           }
	   XPutImage(XtDisplay(canvas),XtWindow(canvas),GcInfo.gc,xi,
		     0,0,0,0,width,height);

	   if (currentContour == -1) {
	     for (i=0; i<nContours; i++) {
		  c = contourList[i]->color;
		  drawContour(contourList[i],c);
	     }
	   } else {
		  c = contourList[currentContour]->color;
		  drawContour(contourList[currentContour],c);
	   }
	   if (displayOptions.falseAlarms) {
	      for (i=0; i<nFalseAlarms; i++) {
		 x=falseAlarmList[i].x;
		 y=height - falseAlarmList[i].y -1;
		 changeGC("white",LIGHT);
		 XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x,y);
	      }
	   }
		 

	 }
	/**********************************************************/
       void drawCorner(int x, int y)
       {

	 changeGC("red",LIGHT);
         XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x,y);
	 XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x-1,y-1);
	 XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x+1,y-1);
	 XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x-1,y+1);
	 XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x+1,y+1);

	 XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x-2,y-2);
	 XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x+2,y-2);
	 XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x-2,y+2);
	 XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x+2,y+2);
       }

        void redrawCorners()
        {
           int i,j,colorIndex;
           if(!displayOptions.corners) return;
           if (displayOptions.cornerFrame) currentCornerFrame=displayOptions.cornerFrame-start;
           else currentCornerFrame=0;

           for (i=0;i<nCorners[currentCornerFrame];i++) {
#ifdef TOP_DOWN
              drawCorner(xcorners[currentCornerFrame][i],height - 1- ycorners[currentCornerFrame][i]);
#else
              drawCorner(xcorners[currentCornerFrame][i], ycorners[currentCornerFrame][i]);
#endif
           }

        }
/**************************************************/
	void clearTmpOptions() {

	  tmpOptions.falseAlarms=
		 tmpOptions.groupedMeas=
		      tmpOptions.smoothContour=
		      tmpOptions.startFrame=
		      tmpOptions.endFrame=
		      tmpOptions.cornerFrame=
		      tmpOptions.pointsOnTrajectory=
		      tmpOptions.corners=0;
	  tmpOptions.minContourLength =0;
	}
       /**************************************************/

	void clearDisplayOptions() {

	  displayOptions.falseAlarms=
		 displayOptions.groupedMeas=
		      displayOptions.smoothContour=
		      displayOptions.startFrame=
		      displayOptions.endFrame=
		      displayOptions.cornerFrame=
		      displayOptions.corners=
		      displayOptions.pointsOnTrajectory=0;
	  displayOptions.minContourLength =0;
	}

	/********************CANVAS********************************/

	unsigned short getColor(char *name, unsigned short type)
	{
	   XColor rgb;
	   int i;
	   unsigned short pix;

	   for (i=0; i<GcInfo.ncolors; i++) {
	       if ( strcmp(name,GcInfo.colors[i].colorName)== 0 ) {
		  break;
	       }
	   }

	   /* If not found add the color to the list */
	   if (i >= GcInfo.ncolors) {
	     if (XParseColor(display,cmap,name,&rgb)==0) { /* get r,g,b values */
		return (INVALID_COLOR);
	     }
	     XAllocColor(display,cmap,&rgb); /* get cmap index */
	     GcInfo.colors[i].colorName = strdup(name);
	     GcInfo.colors[i].lightPixel = rgb.pixel;
	     rgb.red /= 2;
	     rgb.green /=2;
	     rgb.blue /=2;
	     XAllocColor(display,cmap,&rgb); /* get cmap index */
	     GcInfo.colors[GcInfo.ncolors].darkPixel = rgb.pixel;
	     GcInfo.ncolors++;
	   }

	   if (type == LIGHT)
	     pix = (unsigned short)(GcInfo.colors[i].lightPixel);
	   else if (type == DARK )
	     pix = (unsigned short)(GcInfo.colors[i].darkPixel);

	   return pix;
	}
	/*------------------------------*/

	void changeGC(char *name, unsigned short type)
	{
	   XGCValues  gcv;
	   gcv.foreground = getColor(name,type);
	   XChangeGC(display,GcInfo.gc,GCForeground,&gcv);
	}

	/*------------------------------*/

	void create_canvas(char *img_name)
	 {
		 char		  name[64];
		 Arg		  args[9];
		 int  		  foreground, background,i;
		 XGCValues        gcv;
		 XColor           rgb;
		 unsigned short   pix;
                 IMAGE *SgiImage;


	         if ( (SgiImage=iopen(img_name,"r")) == NULL) {
        	    fprintf(stderr,"Can't open Sgi Image %s\n",img_name);
		    exit(-1);
		  }
		 width = SgiImage->xsize;
		 height = SgiImage->ysize;
		 iclose(SgiImage);


		 XtSetArg(args[0], XmNwidth, width);
		 XtSetArg(args[1], XmNheight, height);
		 XtSetArg(args[2], XmNmaxWidth, width);
		 XtSetArg(args[3], XmNmaxHeight, height);

		 XtSetValues(root,args,4);

		 /* --- Create Drawing Area --- */
		 XtSetArg(args[0], XmNwidth, width);
		 XtSetArg(args[1], XmNheight, height);
		 XtSetArg(args[2], XmNmaxWidth, width);
		 XtSetArg(args[3], XmNmaxHeight, height);
		 XtSetArg(args[4],XmNresizePolicy,XmRESIZE_NONE);
		 canvas = XmCreateDrawingArea(root, "canvas", args, 5);
		 XtManageChild(canvas);


		 display = XtDisplay(canvas);
		 window  = XtWindow(canvas);
		 screen  = XtScreen(canvas);
		 cmap    = DefaultColormap(display,DefaultScreen(display));
		 GcInfo.ncolors = 0;
		 pix = getColor("Gray",LIGHT);
		 gcv.foreground = pix;
                 gcv.line_width = 2;


		 if (!(GcInfo.gc=XCreateGC(XtDisplay(root),
		       RootWindowOfScreen(XtScreen(root)),
		       GCForeground |  GCLineWidth,&gcv))) {
		    fprintf(stderr,"Couldn't create GC\n");
		    exit(-1);
		  }

	 }
	/************CALLBACKS FOR CANVAS**********************************/

	XtCallbackProc drawImageCB(Widget w,XtPointer junk,XtPointer call_data)
	{
           redraw_image();
           redrawCorners();
	}
	/*------------------------------*/
	void drawContour( Contour* contour, char* c)
	{
	     int x,y,j,x1,y1;

	     if (contour->len < displayOptions.minContourLength) {
		 return;
             }
	     if (displayOptions.smoothContour) {
	       changeGC("red",DARK);
  	       x1 = contour->array[0].sx;
	       y1 = height - contour->array[0].sy-1;
#ifdef CROSS
	       XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1,y1);
	       XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1-1,y1-1);
	       XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1+1,y1-1);
	       XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1+1,y1+1);
	       XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1-1,y1+1);
	       XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1-2,y1-2);
	       XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1+2,y1-2);
	       XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1+2,y1+2);
	       XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1-2,y1+2);
#endif
/*               XDrawRectangle(display,XtWindow(canvas),GcInfo.gc,x1-2,y1-2,4,4);*/
	       if (displayOptions.groupedMeas)
		   changeGC(c,DARK);
		else
		   changeGC(c,LIGHT);
                changeGC("white",LIGHT); 
		for (j=1; j<contour->len;j++) {
                     if (displayOptions.startFrame) 
                        if (contour->fno != displayOptions.startFrame)break;
                     if (displayOptions.endFrame)
                        if (contour->array[j].fno > displayOptions.endFrame) continue;
		     x = contour->array[j].sx;
		     y = height - contour->array[j].sy-1;
                     if (x==0 ) continue;
		     XDrawLine(display,XtWindow(canvas),GcInfo.gc,x1,y1,x,y);
		     x1 = x;
		     y1 = y;
	          if (displayOptions.pointsOnTrajectory) {
	            if (displayOptions.cornerFrame  && contour->array[j].fno == displayOptions.cornerFrame) 
                       changeGC("yellow",LIGHT);   
                    else
                       changeGC("red",LIGHT);   
/*                     XDrawRectangle(display,XtWindow(canvas),GcInfo.gc,x1-2,y1-2,4,4); */
 		     changeGC(c,DARK);
		   }
	          if (displayOptions.cornerFrame  && contour->array[j].fno == displayOptions.cornerFrame) {
		     XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1,y1);
		     XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1-1,y1-1);
		     XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1+1,y1-1);
		     XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1+1,y1+1);
		     XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1-1,y1+1);
		     XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1-2,y1-2);
		     XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1+2,y1-2);
		     XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1+2,y1+2);
		     XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1-2,y1+2);
		   }
	       }
               XDrawArc(display,XtWindow(canvas),GcInfo.gc,x1,y1,5,5,0,360*64);
	     }
	     if (displayOptions.groupedMeas) {
  	        x1 = contour->array[0].mx;
	        y1 = height - contour->array[0].my-1;
                changeGC("white",LIGHT); 
                if (!displayOptions.startFrame || contour->fno==displayOptions.startFrame)
/*                  XDrawRectangle(display,XtWindow(canvas),GcInfo.gc,x1-2,y1-2,4,4); */
		for (j=1; j<contour->len;j++) {
                     if (displayOptions.startFrame) {
                        if (contour->fno != displayOptions.startFrame) {
                          printf("StartFrame=%d\n",contour->fno);
                          break;
                        }
		      }
                     if (displayOptions.endFrame)
                        if (contour->array[j].fno >= displayOptions.endFrame) continue;
		     x = contour->array[j].mx;
		     y = height - contour->array[j].my-1;
                     if (x==0 ) {
                       if ( displayOptions.pointsOnTrajectory) {
	                 if (displayOptions.cornerFrame  && contour->array[j].fno == displayOptions.cornerFrame) 
                           changeGC("magenta",LIGHT);   
                         else
                           changeGC("red",LIGHT);   
 	   	         x = contour->array[j].sx;
		         y = height - contour->array[j].sy-1;
/*	                 XDrawRectangle(display,XtWindow(canvas),GcInfo.gc,x-2,y-2,4,4); */
                         changeGC("white",LIGHT);
                       }
                       continue;
		     }
                     XDrawLine(display,XtWindow(canvas),GcInfo.gc,x1,y1,x,y);
                     x1=x; y1=y;
                     if ( displayOptions.pointsOnTrajectory) {
	               if (displayOptions.cornerFrame  && contour->array[j].fno == displayOptions.cornerFrame) 
                         changeGC("magenta",LIGHT);   
/*	               XDrawRectangle(display,XtWindow(canvas),GcInfo.gc,x1-2,y1-2,4,4); */
                       changeGC("white",LIGHT);
                      }
	             if (displayOptions.cornerFrame && contour->array[j].fno == displayOptions.cornerFrame) {
                        XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1,y1);
    		        XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1-1,y1-1);
		        XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1+1,y1-1);
		        XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1+1,y1+1);
		        XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1-1,y1+1);
		        XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1-2,y1-2);
		        XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1+2,y1-2);
		        XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1+2,y1+2);
		        XDrawPoint(display,XtWindow(canvas),GcInfo.gc,x1-2,y1+2);
		      }
		   }
              if (!displayOptions.startFrame || contour->fno==displayOptions.startFrame)
                 XDrawArc(display,XtWindow(canvas),GcInfo.gc,x1,y1,5,5,0,360*64);
	      }
	     }

	/******************EVENT HANDLERS FOR CANVAS************************/

	void handleClickCB(Widget w,XtPointer client_data,XEvent *event)
	{
	  int newX,newY,i;

	   if (getContour(event->xbutton.x,event->xbutton.y,&newX,&newY)) {
	      for (i=0; i<nContours; i++) {
		 if (contourList[i] == contourLayout[newY][newX]) {
		    currentContour = i;
		    break;
		  }
	      }
	      infoUpdate();
	      redraw_image();
              redrawCorners();
	      if (changeColorPB)
	      XtSetSensitive(changeColorPB,True);
	   }
	}

	/*------------------------------*/

	void postMenuCB(Widget canvas,Widget menu, XButtonPressedEvent *event)
	{
 /*	 if (event->button == Button3) {*/
	    XmMenuPosition(menu,event);
	    XtManageChild(menu);
/*	  }*/
	}    

	/****************POPUP MENU********************************/

	/* --- Create a popup menu --- */

	Widget create_menu( char *menu_title)
	{
		int			i;
		char			name[64];
		Arg			arg;
		Widget		        menu,tmp;
		XmString		xmstr;
		menu = XmCreatePopupMenu(canvas, "menu", NULL, 0);
		
		xmstr = XmStringCreateSimple(menu_title);
		XtSetArg(arg, XmNlabelString, xmstr);
		tmp = XmCreateLabel(menu, "menuTitle", &arg, 1);
		XtManageChild(tmp);
		XmStringFree(xmstr);
		tmp = XmCreateSeparator(menu, "menuSeparator", NULL, 0);
		XtManageChild(tmp);

		sprintf(name, "Display Options");
		xmstr = XmStringCreateSimple(name);
		XtSetArg(arg, XmNlabelString, xmstr);
		sprintf(name, "DisplayButton");
		displayPB = XmCreatePushButton(menu, name, &arg, 1);
		XtAddCallback(displayPB, XmNactivateCallback, displayContourCB, NULL);
		XtManageChild(displayPB);

		sprintf(name, "Contour Info");
		xmstr = XmStringCreateSimple(name);
		XtSetArg(arg, XmNlabelString, xmstr);
		sprintf(name, "InfoButton");
		infoPB = XmCreatePushButton(menu, name, &arg, 1);
		XtAddCallback(infoPB, XmNactivateCallback, displayInfoCB, NULL);
		XtManageChild(infoPB);

		sprintf(name, "Source Info");
		xmstr = XmStringCreateSimple(name);
		XtSetArg(arg, XmNlabelString, xmstr);
		sprintf(name, "SourceInfoButton");
		sourceInfoPB = XmCreatePushButton(menu, name, &arg, 1);
		XtAddCallback(sourceInfoPB, XmNactivateCallback, displaySourceInfoCB, NULL);
		XtManageChild(sourceInfoPB);

		sprintf(name, "Save Image/Exit");
		xmstr = XmStringCreateSimple(name);
		XtSetArg(arg, XmNlabelString, xmstr);
		savePB = XmCreatePushButton(menu, "SaveButton", &arg, 1);
		XtAddCallback(savePB, XmNactivateCallback, saveImgCB, NULL);
		XtManageChild(savePB);

		XtSetArg(arg, XmNselectionLabelString,
		XmStringCreateLtoR("Type in filename. ",char_set));
		pdialog = XmCreatePromptDialog(savePB,"pdialog",&arg,1);
		XtAddCallback(pdialog,XmNokCallback,pdialogCB,OK);
		XtAddCallback(pdialog,XmNcancelCallback,pdialogCB,CANCEL);
		XtUnmanageChild(XmSelectionBoxGetChild(pdialog,
			XmDIALOG_HELP_BUTTON));
			XmStringFree(xmstr);


		endPB = XmCreatePushButton(menu, "Quit", NULL, 0);
		XtAddCallback(endPB, XmNactivateCallback, quit, NULL);
		XtManageChild(endPB);

		return(menu);
	}

	/***********CALLBACKS FOR POPMENU********************************/

	void saveFile(char* filename)
	{
	  IMAGE *saveImg;
	  XImage *screenImg;
	  int i,j;
	  unsigned short *redBuf, *greenBuf,*blueBuf,*rb,*gb,*bb;
	  XColor colPix,rgb;
	  char *buf, *buf_ptr;

	  screenImg = XGetImage(display,XtWindow(canvas),0,0,
			width,height,AllPlanes,ZPixmap);
	  redBuf = (unsigned short*) malloc(width * height*sizeof(unsigned short));
	  greenBuf = (unsigned short*) malloc(width * height*sizeof(unsigned short));
	  blueBuf = (unsigned short*) malloc(width * height*sizeof(unsigned short));
	  buf = (char*) malloc (width*height*3*sizeof(char));


	  buf_ptr=buf;
	  rb=redBuf;gb=greenBuf;bb=blueBuf;
	  for (i=height-1; i>=0;i-- ) {
	    for (j=0;j<width;j++) {
	      colPix.pixel= XGetPixel(screenImg,j,i);
	      if (colPix.pixel) {
		XQueryColor(display,cmap,&colPix);
		*rb++ = colPix.red / 256;
		*gb++ = colPix.green / 256;
		*bb++ = colPix.blue / 256;
	      } else {
		*rb++ = 0;
		*gb++ =0;
		*bb++ = 0;
	      }
	    }
	  }
	  for (j=0;j<width*height;j++) 
	    *buf_ptr++ = (char)redBuf[j];
	  for (j=0;j<width*height;j++) 
	    *buf_ptr++ = (char)greenBuf[j];
	  for (j=0;j<width*height;j++) 
	    *buf_ptr++ = (char)blueBuf[j];

	  writeImage(filename,width,height,buf);
	  XtDestroyWidget(root);
	  exit(-1);
	}

	/*------------------------------*/
	XtCallbackProc displaySourceInfoCB() 
	{
	  CreateSourceInfoDialog(canvas);
	  XtPopup(sourceInfoDialog,XtGrabNone);
	}

	/*------------------------------*/
	XtCallbackProc pdialogCB(Widget w,int client_data,
		    XmSelectionBoxCallbackStruct *call_data)
	{
	    char *saveFileName;

	    switch (client_data)
	    {
		case OK:
		    /* get the string from the event structure. */
		  XmStringGetLtoR(call_data->value,char_set,&saveFileName);
		  printf("Saving File='%s'\n",saveFileName);
		  saveFile(saveFileName);
		  break;
		case CANCEL:
		 printf("CANCEL selected\n");
		 break;
	    }
	    /* make the dialog box invisible */
	    XtUnmanageChild(w);
	}
	/*------------------------------*/

	XtCallbackProc saveImgCB()
	{

	   XtManageChild(pdialog);
	}
	  

	/*------------------------------*/
	int getContour(int xc, int yc, int *newX, int *newY)
	{
	 int x,y,i;
	 int survcnt = 0;

	 /* Search for closest Contour */
	  for (survcnt = 0; survcnt < 7 ; survcnt++) {
	     for(i=0;i<svol.ncnt[survcnt];i++) {
	       x = xc + svol.x[survcnt][i];
	       y = yc + svol.y[survcnt][i];
	       if (contourLayout[y][x] ) {
		    *newX =x;
		    *newY = y;
		    return(1);
	       }
	     }
	  }

	  return(0);
	}
	/*------------------------------*/

	XtCallbackProc displayContourCB(Widget button, 
		    XtPointer junk, XtPointer cb)
	{

	  XtUnmanageChild(XtParent(button));
	  displayOptionDialog = CreateDisplayOptionDialog(canvas,
				       "DisplayOptions");
	  XtSetSensitive(button,False);
	}
	/*------------------------------*/

	XtCallbackProc displayInfoCB(Widget button, XtPointer junk, XtPointer cb)
	{

	  XtUnmanageChild(XtParent(button));
	  infoDialog = CreateContourInfoDialog(canvas,"InfoDialog");
	  XtSetSensitive(button,False);
	}
	/*------------------------------*/

	XtCallbackProc quit(Widget canvas, XtPointer junk, XtPointer cb)
	{
	    XtDestroyWidget(root);
	    exit(-1);
	  }


	/**********************DISPLAY OPTIONS DIALOG **************/

	Widget CreateDisplayOptionDialog(Widget parent, char* name)
	{
	  Widget         displayMenu;
	  Widget         apply,close,displayTitleLabel;
	  void displayDefaultValues();

	  displayMenu = CreateBulletinDialog(parent,name);
	 
	  dialogForm = XtCreateManagedWidget( "form",
				xmFormWidgetClass,
				displayMenu,
				NULL,
				0 );
	  displayTitleLabel = XtVaCreateManagedWidget("displayTitleLabel",
					  xmLabelWidgetClass,
					  dialogForm,
					  NULL);
	  dialogFrame = XtVaCreateManagedWidget("dialogFrame",
				xmFrameWidgetClass,
				dialogForm,
				NULL,
				0);
	  checkBox = XmCreateSimpleCheckBox(dialogFrame,"checkBox",NULL,0);
	  XtManageChild(checkBox);

	  toggle1 = CreateToggle(checkBox,"Toggle1","False Alarms",
				 0,XmONE_OF_MANY ,toggle1CB);
	  toggle2 = CreateToggle(checkBox,"Toggle2","Grouped Meas",
				 0,XmONE_OF_MANY ,toggle2CB);
	  toggle3 = CreateToggle(checkBox,"Toggle3","Smooth Contour",
				 0,XmONE_OF_MANY ,toggle3CB);

	  toggle4 = CreateToggle(checkBox,"Toggle4","PointsOnTrajectory",
				 0,XmONE_OF_MANY ,toggle4CB);


	  toggle5 = CreateToggle(checkBox,"Toggle5","Corners",
				 0,XmONE_OF_MANY ,toggle5CB);


	  startFrameLabel = XtVaCreateManagedWidget("startFrameLabel",
					  xmLabelWidgetClass,
					  dialogForm,
					  NULL);
	  startFrameValLabel = XtVaCreateManagedWidget("startFrameValLabel",
					  xmTextFieldWidgetClass,
					  dialogForm,
					  NULL);
	  XtAddCallback(startFrameValLabel,XmNactivateCallback,startFrameValCB,NULL);


	  
	  endFrameLabel = XtVaCreateManagedWidget("endFrameLabel",
					  xmLabelWidgetClass,
					  dialogForm,
					  NULL);
	  endFrameValLabel = XtVaCreateManagedWidget("endFrameValLabel",
					  xmTextFieldWidgetClass,
					  dialogForm,
					  NULL);
	  XtAddCallback(endFrameValLabel,XmNactivateCallback,endFrameValCB,NULL);


	  
	  cornerFrameLabel = XtVaCreateManagedWidget("cornerFrameLabel",
					  xmLabelWidgetClass,
					  dialogForm,
					  NULL);
	  cornerFrameValLabel = XtVaCreateManagedWidget("cornerFrameValLabel",
					  xmTextFieldWidgetClass,
					  dialogForm,
					  NULL);
	  XtAddCallback(cornerFrameValLabel,XmNactivateCallback,cornerFrameValCB,NULL);


	  
	  minLenLabel = XtVaCreateManagedWidget("minLenLabel",
					  xmLabelWidgetClass,
					  dialogForm,
					  NULL);
	  minLenValLabel = XtVaCreateManagedWidget("minLenValLabel",
					  xmTextFieldWidgetClass,
					  dialogForm,
					  NULL);
	  XtAddCallback(minLenValLabel,XmNactivateCallback,minLenValCB,NULL);

	  
	  scale = XtVaCreateManagedWidget("scale",
					  xmScaleWidgetClass,
					  dialogForm,
					  XmNorientation,XmHORIZONTAL,NULL);
	  XtAddCallback(scale,XmNdragCallback,scaleCB,NULL);
	 
	  apply=CreatePushButton(displayMenu,"Apply",NULL,NULL,applyCB);
	  close=CreatePushButton(displayMenu,"Close",NULL,NULL,closeCB);
	  
	  /* Set Various Resources */

	  XtVaSetValues(dialogFrame,
			      XmNleftAttachment,XmATTACH_POSITION,
			      XmNleftPosition,25,
			      XmNrightAttachment,XmATTACH_POSITION,
			      XmNrightPosition,75,
			      NULL);
	  XtVaSetValues(displayTitleLabel,
			      XmNbottomAttachment,XmATTACH_WIDGET,
			      XmNbottomWidget,dialogFrame,
			      NULL);


	  XtVaSetValues(startFrameLabel,
			      XmNtopAttachment,XmATTACH_WIDGET,
			      XmNtopWidget,dialogFrame,
                              XmNtopOffset, 25,
			      NULL);
	  XtVaSetValues(startFrameValLabel,
			      XmNmaxLength,4,
			      XmNwidth,45,
			      XmNleftAttachment,XmATTACH_WIDGET,
                	      XmNleftWidget,startFrameLabel,
                              XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
			      XmNtopWidget, startFrameLabel,
			      NULL);


          XtVaSetValues(endFrameLabel,
			      XmNtopAttachment,XmATTACH_WIDGET,
			      XmNtopWidget,startFrameLabel,
                              XmNtopOffset, 25,
			      NULL);
	  XtVaSetValues(endFrameValLabel,
			      XmNmaxLength,4,
                              XmNwidth,45,
			      XmNleftAttachment,XmATTACH_WIDGET,
			      XmNleftWidget,endFrameLabel,
			      XmNtopAttachment,XmATTACH_OPPOSITE_WIDGET,
			      XmNtopWidget,endFrameLabel,
			      NULL);



          XtVaSetValues(cornerFrameLabel,
			      XmNtopAttachment,XmATTACH_WIDGET,
			      XmNtopWidget,endFrameLabel,
                              XmNtopOffset, 25,
			      NULL);
	  XtVaSetValues(cornerFrameValLabel,
			      XmNmaxLength,4,
                              XmNwidth,45,
			      XmNleftAttachment,XmATTACH_WIDGET,
			      XmNleftWidget,cornerFrameLabel,
			      XmNtopAttachment,XmATTACH_OPPOSITE_WIDGET,
			      XmNtopWidget,cornerFrameLabel,
			      NULL);



	  XtVaSetValues(minLenLabel,
			      XmNtopAttachment,XmATTACH_WIDGET,
			      XmNtopWidget,cornerFrameLabel,
                              XmNtopOffset, 25,
			      NULL);
	  XtVaSetValues(minLenValLabel,
			      XmNmaxLength,4,
			      XmNwidth,45,
			      XmNleftAttachment,XmATTACH_WIDGET,
			      XmNleftWidget,minLenLabel,
			      XmNtopAttachment,XmATTACH_OPPOSITE_WIDGET,
			      XmNtopWidget,minLenLabel,
			      NULL);

	  XtVaSetValues(scale,
			      XmNleftAttachment,XmATTACH_POSITION,
			      XmNleftPosition,25,
			      XmNrightAttachment,XmATTACH_POSITION,
			      XmNrightPosition,75,
			      NULL);
	  displayDefaultValues();

	  XtManageChild(displayMenu);
	}

	/*------------------------------*/

	void displayDefaultValues()
	{
	   char s[80];

	   sprintf(s,"%d",displayOptions.startFrame);
	   XmTextFieldSetString(startFrameValLabel,s);

	   sprintf(s,"%d",displayOptions.endFrame);
	   XmTextFieldSetString(endFrameValLabel,s);

	   sprintf(s,"%d",displayOptions.cornerFrame);
	   XmTextFieldSetString(cornerFrameValLabel,s);

	   sprintf(s,"%d",displayOptions.minContourLength);
	   XmTextFieldSetString(minLenValLabel,s);
	   XmScaleSetValue(scale,100*displayOptions.minContourLength / MAXLEN);

	   if (displayOptions.falseAlarms)
	       SetToggle(toggle1,True);
	   if (displayOptions.groupedMeas)
	       SetToggle(toggle2,True);
	   if (displayOptions.smoothContour)
	       SetToggle(toggle3,True);
	   if (displayOptions.pointsOnTrajectory)
	       SetToggle(toggle4,True);
	   if (displayOptions.corners)
	       SetToggle(toggle5,True);
	}

	/***************CALLBACKS FOR DISPLAY OPTIONS DIALOG*********************/

	XtCallbackProc toggle1CB(Widget displayMenu, XtPointer junk, 
				 XmToggleButtonCallbackStruct *toggle_struct)
	{
	  if (toggle_struct->set == True)
	     tmpOptions.falseAlarms=1;
	  else
	     tmpOptions.falseAlarms=0;
	 
	}

	/*------------------------------*/

	XtCallbackProc toggle2CB(Widget displayMenu, XtPointer junk,
				 XmToggleButtonCallbackStruct *toggle_struct)
	{
	  if (toggle_struct->set == True)
	     tmpOptions.groupedMeas=1;
	  else
	     tmpOptions.groupedMeas=0;
	}

	/*------------------------------*/

	XtCallbackProc toggle3CB(Widget displayMenu, XtPointer junk, 
				 XmToggleButtonCallbackStruct *toggle_struct)
	{
	  if (toggle_struct->set == True)
	      tmpOptions.smoothContour=1;
	  else
	      tmpOptions.smoothContour=0;
	}
	/*------------------------------*/
	XtCallbackProc toggle4CB(Widget displayMenu, XtPointer junk, 
				 XmToggleButtonCallbackStruct *toggle_struct)
	{
	  if (toggle_struct->set == True)
	      tmpOptions.pointsOnTrajectory=1;
	  else
	      tmpOptions.pointsOnTrajectory=0;
	}
	/*------------------------------*/

	XtCallbackProc toggle5CB(Widget displayMenu, XtPointer junk, 
				 XmToggleButtonCallbackStruct *toggle_struct)
	{
	  if (toggle_struct->set == True)
	      tmpOptions.corners=1;
	  else
	      tmpOptions.corners=0;
	}
	/*------------------------------*/

	XtCallbackProc minLenValCB(Widget minLenValLabel, XtPointer junk, 
			 XtPointer cb)
	{
	  char  *text =XmTextFieldGetString(minLenValLabel);
	  tmpOptions.minContourLength=atoi(text);
	  XmScaleSetValue(scale,100*atoi(text)/MAXLEN);
	}

	/*------------------------------*/

	XtCallbackProc startFrameValCB(Widget startFrameValLabel, XtPointer junk, 
			 XtPointer cb)
	{
	  char  *text =XmTextFieldGetString(startFrameValLabel);
	  tmpOptions.startFrame=atoi(text);
          if (!tmpOptions.cornerFrame)
	    tmpOptions.cornerFrame=atoi(text);
	}

	/*------------------------------*/

	XtCallbackProc endFrameValCB(Widget endFrameValLabel, XtPointer junk, 
			 XtPointer cb)
	{
	  char  *text =XmTextFieldGetString(endFrameValLabel);
	  tmpOptions.endFrame=atoi(text);
	}

	/*------------------------------*/

	XtCallbackProc cornerFrameValCB(Widget cornerFrameValLabel, XtPointer junk, 
			 XtPointer cb)
	{
	  char  *text =XmTextFieldGetString(cornerFrameValLabel);
	  tmpOptions.cornerFrame=atoi(text);
	}

	/*------------------------------*/

	XtCallbackProc applyCB(Widget displayMenu, XtPointer junk,
			       XtPointer cb)
	{
	  displayOptions.falseAlarms=tmpOptions.falseAlarms;
	  displayOptions.groupedMeas=tmpOptions.groupedMeas;
	  displayOptions.smoothContour=tmpOptions.smoothContour;
	  displayOptions.pointsOnTrajectory=tmpOptions.pointsOnTrajectory;
	  displayOptions.corners=tmpOptions.corners;
	  displayOptions.minContourLength=tmpOptions.minContourLength;
	  displayOptions.startFrame=tmpOptions.startFrame;
	  displayOptions.endFrame=tmpOptions.endFrame;
	  displayOptions.cornerFrame=tmpOptions.cornerFrame;
	  redraw_image();
          redrawCorners();
	}

	/*------------------------------*/

	XtCallbackProc closeCB(Widget closeB, XtPointer junk, 
			   XtPointer cb)
	{
	  XtDestroyWidget(XtParent(closeB));
	  XtSetSensitive(displayPB,True);
	}

	/*------------------------------*/

	/* handles the scale's callback. */
	XtCallbackProc scaleCB(Widget w, int client_data, 
		       XmScaleCallbackStruct *call_data)
	{
	    char s[100];
	    Arg al[10];
	    int ac;

	    sprintf(s,"%d",call_data->value*MAXLEN / 100);
	    XmTextFieldSetString(minLenValLabel, s);
	    tmpOptions.minContourLength=atoi(s);
	}

	/***************CONTOUR INFO DIALOG************************/

	Widget CreateContourInfoDialog(Widget parent, char *name)
	{
	  int   ac;
	  Arg   args[10];
	  char s[60];
	  Widget infoBulletin,infoTitleLabel,
		 sortFrame,sortRowCol,
		 sortRadioBox,sortByLenToggle,sortByIdToggle,
		 actionFrame,actionRowCol,actionRadioBox,
		 displayAllPB,firstPB,nextPB, closeInfoPB;

	  infoBulletin = CreateBulletinDialog(parent,name);
	  infoTitleLabel = XtVaCreateManagedWidget("infoTitleLabel",
					  xmLabelWidgetClass,
					  infoBulletin,
					  NULL);
	  infoDisplayLabel = XtVaCreateManagedWidget("infoDisplayLabel",
					  xmLabelWidgetClass,
					  infoBulletin,
					  NULL);
	  infoDisplayLabel1 = XtVaCreateManagedWidget("infoDisplayLabel1",
					  xmLabelWidgetClass,
					  infoBulletin,
					  NULL);
	  sortFrame = XtVaCreateManagedWidget("sortFrame",
				xmFrameWidgetClass,
				infoBulletin,
				NULL,
				0);
	  XtSetArg(args[0],XmNorientation,XmHORIZONTAL);
	  XtSetArg(args[1],XmNradioBehavior,True);
	  sortRowCol = XmCreateRowColumn(sortFrame,
				   "sortRowCol",
				    args,2);
	  XtManageChild(sortRowCol);
	  sortByLenToggle = CreateToggle(sortRowCol,"sortByLenToggle",
			    "Sort_By_Len", 0,XmONE_OF_MANY ,
			    sortByLenToggleCB);
	  sortByIdToggle = CreateToggle(sortRowCol,"sortByIdToggle",
			    "Sort_By_Id", 0,XmONE_OF_MANY ,
			    sortByIdToggleCB);

	  actionFrame = XtVaCreateManagedWidget("actionFrame",
				xmFrameWidgetClass,
				infoBulletin,
				NULL,
				0);
	  XtSetArg(args[0],XmNorientation,XmHORIZONTAL);
	/*
	  XtSetArg(args[1],XmNradioBehavior,True); */
	  actionRowCol = XmCreateRowColumn(actionFrame,
				   "actionRowCol",
				    args,1);
	  XtManageChild(actionRowCol);
	  displayAllPB=CreatePushButton(actionRowCol,"displayAllPB",
		       NULL,NULL,displayAllCB);
	  firstPB=CreatePushButton(actionRowCol,"firstPB",
		       NULL,NULL,firstCB);
	  nextPB=CreatePushButton(actionRowCol,"nextPB",
		       NULL,NULL,nextCB);
	  changeColorPB=CreatePushButton(infoBulletin,"changeColorPB",
		       NULL,NULL,changeColorCB);
	  closeInfoPB=CreatePushButton(infoBulletin,"closeInfoPB",
		       NULL,NULL,closeInfoCB);


	    ac = 0;
	    /* XtSetArg(args[ac],XmNautoUnmanage,False); ac++;             */
	    XtSetArg(args[ac],XmNmustMatch,True); ac++;
	    XtSetArg(args[ac],XmNselectionLabelString,
		XmStringCreateLtoR("Pick a color. ",char_set));  ac++;
	    colorSelectionDialog=XmCreateSelectionDialog(infoBulletin,"colorSelectionDialog",args,ac);
	    XtAddCallback(colorSelectionDialog,XmNokCallback,colorSelectionDialogCB,OK);
	    XtAddCallback(colorSelectionDialog,XmNcancelCallback,colorSelectionDialogCB,CANCEL);
	    XtAddCallback(colorSelectionDialog,XmNnoMatchCallback,noMatchCB,NULL);
	    XtUnmanageChild(XmSelectionBoxGetChild(colorSelectionDialog,
		XmDIALOG_APPLY_BUTTON));
	    XtUnmanageChild(XmSelectionBoxGetChild(colorSelectionDialog,
		XmDIALOG_HELP_BUTTON));


	  /* Set Resources */
	 if (currentContour == -1) XtSetSensitive(changeColorPB,FALSE);

	 sprintf(s,"ID:____          COLOR:            LENGTH:       ");
	 XtSetArg(args[0],XmNlabelString,XmStringCreate(s,char_set));
	 XtSetValues(infoDisplayLabel,args,1);

	 sprintf(s,"START FRAME:____          X:            Y:       ");
	 XtSetArg(args[0],XmNlabelString,XmStringCreate(s,char_set));
	 XtSetValues(infoDisplayLabel1,args,1);

	 XtVaSetValues(sortFrame,
			      XmNtopAttachment,XmATTACH_WIDGET,
			      XmNtopWidget,infoTitleLabel,
			      XmNbottomAttachment,XmATTACH_WIDGET,
			      XmNbottomWidget,actionFrame,NULL);
	 XtVaSetValues(actionFrame,
			      XmNtopAttachment,XmATTACH_WIDGET,
			      XmNtopWidget,sortFrame,NULL);
	 XtVaSetValues(changeColorPB,
			      XmNtopAttachment,XmATTACH_WIDGET,
			      XmNtopWidget,actionFrame,NULL);

	 XtManageChild(infoBulletin);
	 infoUpdate();


	}

	/**********CALLBACKS FOR CONTOURINFO********************************/

	static int compareByLen(const void *el1, const void *el2)
	{
	    Contour **a = (Contour**)el1;
	    Contour **b = (Contour**)el2;

	    if ( (*a)->len > (*b)->len)
	       return(-1);
	    else if ( (*a)->len < (*b)->len)
	       return 1;
	    else
	       return 0;
	}

	/*------------------------------*/

	XtCallbackProc sortByLenToggleCB(Widget w, XtPointer junk,
					 XtPointer cb)
	{
	   qsort(contourList,nContours,sizeof(Contour*),compareByLen);
	}

	/*------------------------------*/

	static int compareById(const void *el1, const void *el2)
	{
	    Contour **a = (Contour**)el1;
	    Contour **b = (Contour**)el2;

	    if ( (*a)->id < (*b)->id )
	       return(-1);
	    else if ( (*a)->id > (*b)->id )
	       return 1;
	    else
	       return 0;
	}

	/*------------------------------*/

	XtCallbackProc sortByIdToggleCB(Widget w, XtPointer junk,
					XtPointer cb)
	{
	   qsort(contourList,nContours,sizeof(Contour*),compareById);
	}
	/*------------------------------*/

	void displayContourInfo()
	{
	    char s[60];
	    Arg  args[10];

		 sprintf(s,"ID:%d       COLOR:%s       LENGTH:%d",
		 contourList[currentContour]->id,contourList[currentContour]->color,
		 contourList[currentContour]->len);
		 XtSetArg(args[0],XmNlabelString,XmStringCreate(s,char_set));
		 XtSetValues(infoDisplayLabel,args,1);

		 sprintf(s,"START FRAME:%d       X:%d       Y:%d",
		 contourList[currentContour]->fno,
                             contourList[currentContour]->array[0].mx,
                             contourList[currentContour]->array[0].my);
		 XtSetArg(args[0],XmNlabelString,XmStringCreate(s,char_set));
		 XtSetValues(infoDisplayLabel1,args,1);
	}
	/*------------------------------*/

	void infoUpdate()
	{
	   char s[60];
	   Arg  args[10];

	   if (currentContour != -1 && infoDisplayLabel) 
		displayContourInfo();
	   else if (infoDisplayLabel)  {
		 sprintf(s,"ID:____          COLOR:            LENGTH:       ");
		 XtSetArg(args[0],XmNlabelString,XmStringCreate(s,char_set));
		 XtSetValues(infoDisplayLabel,args,1);

 	         sprintf(s,"START FRAME:____          X:            Y:       ");
	         XtSetArg(args[0],XmNlabelString,XmStringCreate(s,char_set));
	         XtSetValues(infoDisplayLabel1,args,1);

	   }

	}
	/*------------------------------*/

	XtCallbackProc displayAllCB(Widget w, XtPointer junk,
				    XtPointer cb)
	{
	   currentContour = -1;
	   XtSetSensitive(changeColorPB,False);
	   infoUpdate();
	   redraw_image();
           redrawCorners();
	}
	/*------------------------------*/

	XtCallbackProc firstCB(Widget w, XtPointer junk, 
			       XtPointer cb)
	{
	   currentContour = 0;
	   while (contourList[currentContour]->len
			  < displayOptions.minContourLength)
	   {
	     currentContour++;
	     if (currentContour >= nContours)
		    currentContour = 0;
	   }
	   XtSetSensitive(changeColorPB,True);
	   infoUpdate();
	   redraw_image();
           redrawCorners();
	}
	/*------------------------------*/

	XtCallbackProc nextCB(Widget w, XtPointer junk,
			      XtPointer cb)
	{
	   do {
	     currentContour++;
	     if (currentContour >= nContours)
		    currentContour = 0;
	   } while (contourList[currentContour]->len 
			  < displayOptions.minContourLength);
	   
	   XtSetSensitive(changeColorPB,True);

	   infoUpdate();
	   redraw_image();
           redrawCorners();
	}

	/*------------------------------*/

	void noMatchCB(Widget w,int client_data,
		       XmSelectionBoxCallbackStruct *call_data)
	{
	     char *s;
	     int pos;
	     unsigned short pix;

	     XmStringGetLtoR(call_data->value,char_set,&s);
	      
	     if ( (pix = getColor(s,LIGHT)) == INVALID_COLOR) {
	       fprintf(stderr,"Invalid color\n");
	       return;
	     }
	     XmListAddItem(colorList,call_data->value,GcInfo.ncolors);
	     pos = XmListItemPos(colorList,call_data->value);
	     contourList[currentContour]->color_index = pos;
	     strcpy(contourList[currentContour]->color, 
	     GcInfo.colors[pos].colorName);
	     infoUpdate();
	     redraw_image();
             redrawCorners();
	}

	/*------------------------------*/

	XtCallbackProc colorSelectionDialogCB(Widget w,int client_data,
				    XmSelectionBoxCallbackStruct *call_data)
	{
	    char *s;
	    int  pos;

	    switch (client_data)
	    {
		case OK:     /* OK*/
		    /* get the string from the event structure. */
		    XmStringGetLtoR(call_data->value,char_set,&s);
		    XtFree(s);
		    pos = XmListItemPos(colorList,call_data->value);
		    contourList[currentContour]->color_index = pos;
		    strcpy(contourList[currentContour]->color, 
			   GcInfo.colors[pos].colorName);
		    infoUpdate();
		    redraw_image();
                    redrawCorners();
		    break;
		case CANCEL:
		    printf("CANCEL selected\n");
		    break;
	    }
	    XtUnmanageChild(w);
	}


	/*------------------------------*/

	XtCallbackProc colorListCB(Widget w, XtPointer junk, 
		   XmListCallbackStruct  *list_data)
	{
	}

	/*------------------------------*/

	XtCallbackProc changeColorCB(Widget w, XtPointer junk,
				     XtPointer cb)
	{
	   Arg    args[10];
	   int status,i;
	   XtCallbackProc colorListCB();
	   Widget CreateScrolledList();
	   XmString s;

	   colorList = XmSelectionBoxGetChild(colorSelectionDialog,XmDIALOG_LIST);
	   XmListDeleteAllItems(colorList);
	   for (i=1; i<GcInfo.ncolors; i++) {
	     status = (contourList[currentContour]->color_index == i) ? 1 : 0;
	     s=XmStringCreate(GcInfo.colors[i].colorName,char_set);
	     XmListAddItem(colorList,s,i);
	     XmStringFree(s);
	   }
	   XtSetArg(args[0],XmNvisibleItemCount, 10);
	   XtSetValues(w,args,1);
	   XtManageChild(colorSelectionDialog);
	}

	/*------------------------------*/

	XtCallbackProc closeInfoCB(Widget closeB, XtPointer junk,
				   XtPointer cb)
	{
	  XtDestroyWidget(XtParent(closeB));
	  XtSetSensitive(infoPB,True);
	}






	/**********SourceInfoDialog********************************/

	void CreateSourceInfoDialog(Widget w)
	{
	 Widget pane,form,infoTxtW,OkWidget;
	 Arg args[10];

	  sourceInfoDialog=XtVaCreatePopupShell("SourceInfo",
		  xmDialogShellWidgetClass,GetTopShell(w),
		  XmNdeleteResponse, XmDESTROY,
		  XmNwidth,600,
		  XmNx,200,
		  XmNy,200,
		  NULL);

	  pane = XtVaCreateWidget("pane",
		  xmPanedWindowWidgetClass,
		  sourceInfoDialog,NULL);
	  form = XtVaCreateWidget("form1",
		  xmFormWidgetClass,
		  pane,NULL);
	  XtSetArg(args[0],XmNscrollVertical, True);
	  XtSetArg(args[1],XmNscrollHorizontal, False);
	  XtSetArg(args[2],XmNeditMode, XmMULTI_LINE_EDIT);
	  XtSetArg(args[3],XmNeditable, False);
	  XtSetArg(args[4],XmNcursorPositionVisible, False);
	  XtSetArg(args[5],XmNwordWrap, True);
	  XtSetArg(args[6],XmNvalue, infoTxtBuf);
	  XtSetArg(args[7],XmNrows, 5);

	  infoTxtW = XmCreateScrolledText(form,"Infotxt",args,8);
	  XtVaSetValues(XtParent(infoTxtW),
	      XmNtopAttachment, XmATTACH_FORM,
	      XmNleftAttachment, XmATTACH_FORM,
	      XmNrightAttachment, XmATTACH_FORM,
	      XmNbottomAttachment, XmATTACH_FORM,
	      NULL);

	  XtManageChild(infoTxtW);
	  XtManageChild(form);
	  
	  /* Create another form to act as action area for dialog*/
	  form = XtVaCreateWidget("form2",
		xmFormWidgetClass,pane,
		XmNfractionBase, 5,
		NULL);
	  OkWidget= XtVaCreateManagedWidget("Ok",
	    xmPushButtonGadgetClass,form,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNbottomAttachment,XmATTACH_FORM,
	    XmNleftAttachment,XmATTACH_POSITION,
	    XmNleftPosition,1,
	    XmNrightAttachment,XmATTACH_POSITION,
	    XmNrightPosition,2,
	    XmNshowAsDefault,True,
	    XmNdefaultButtonShadowThickness,1,
	    NULL);
	  XtAddCallback(OkWidget,XmNactivateCallback,destroyShellCB,sourceInfoDialog);
	  XtManageChild(form);
	  {
	    Dimension h;
	    XtVaGetValues(OkWidget,XmNheight, &h, NULL);
	    XtVaSetValues(form,XmNpaneMaximum,h,
		  XmNpaneMinimum,h,NULL);
	  }
	  XtManageChild(pane);
	}

	/*------------------------------*/
	XtCallbackProc destroyShellCB(Widget widget, Widget shell)
	{
	   XtDestroyWidget(shell);
	 }
	    
	/*------------------------------*/
	void getInfoTxt()
	{
	  
	   int COMMENT = '#';
	   int c;
	   int n = 80;
	   char infoLine[80];
	   char *p;

	   p = infoTxtBuf;
	   for (c = getchar(); c==COMMENT && c != EOF; c=getchar()) {
	     gets(infoLine);
	     p += strlen(strcpy(p,infoLine));
	     p += strlen(strcat(p,"\n"));
	   }
	   ungetc(c,stdin);
	}


	/*------------------------------*/
	Widget GetTopShell(Widget w)
	{
	  while (w && !XtIsWMShell(w) )
	    w=XtParent(w);
	  return w;
	}

/**********************IMAGE ROUTINES**********************************/

	XImage* createXimage(unsigned short *buf)
{
        XImage                                  *xi;
	char                                    *data;

        /* --- Get Image Data  --- */
        data = (char*) malloc(width*height*sizeof(char));

        get_gray_cmap_data(data,buf);

       /* -- Create Image --- */
	if (!(xi = XCreateImage(display,DefaultVisualOfScreen(screen),
                   DefaultDepthOfScreen(screen),ZPixmap,0,data,width,height,32,width))) {
             fprintf(stderr,"Could not create Image\n");
             exit(-1);
	   }
        return xi;
      }

/*------------------------------*/

void get_gray_cmap_data(char* data,unsigned short* buf)
 {
   char* dp = data;
   int i,j;
   XColor xc_gray,xc_black;
   unsigned short pix ;
   unsigned short *ptr;
   int numPixels=width*height;

   makeCmap();
   for (i=height-1; i>0; i--) {
      ptr = buf+ i*width;
      for (j=0; j<width; j++) {
	if (*ptr) 
	     *dp++ = (char)(grayCmap[*ptr++]);
	else
	     *dp++ = (char)(grayCmap[*ptr++]);

      }
    }
 }

/******************INPUT ROUTINES*************************************/

void readData()
{

   int i,j;
   char flag;
   float mx,my,mdx,mdy,sx,sy,sdx,sdy,likelihood;
   char model[15];
   char str[90];
   int time,fno,ind;
static char *colorNames[12] =
{
  "orange", "red", "yellow", "cyan", "blue", "green", "purple",
  "coral", "tomato", "magenta", "navy", "violet"
};


   scanf("%d\n",&nContours);
   scanf("%d\n",&nFalseAlarms);
   contourList = (Contour**) malloc(nContours*sizeof(Contour*));
   falseAlarmList = (FalseAlarm*) malloc(nFalseAlarms*sizeof(FalseAlarm));
   for (i=0; i<nContours; i++) {
     contourList[i] = (Contour*) malloc(sizeof(Contour));

     scanf("%d%d\n",&(contourList[i]->id), &(contourList[i]->len) );
     ind = i % 12;
     strcpy(contourList[i]->color,colorNames[ind]);
     if (contourList[i]->len >MAXLEN) MAXLEN = contourList[i]->len;
     if(!(contourList[i]->array = (PixelDetail*) malloc(contourList[i]->len * sizeof(PixelDetail)))) {
         fprintf(stderr,"Couldn't allocate memory for array %d\n",i);
         exit(-1);
     }
     for (j=0; j<contourList[i]->len; j++) {
        gets(str);
        sscanf(str,"%c %f %f %f %f %f %d %d %s",
               &flag, &mx,&my,&sx,&sy,&likelihood,&time,&fno,model);
        if (j==0)  contourList[i]->fno=fno;
        contourList[i]->array[j].fno=fno;
        contourList[i]->array[j].flag=flag;
        contourList[i]->array[j].mx=(int)mx;
        contourList[i]->array[j].my=(int)my;
        contourList[i]->array[j].sx =(int)sx;
        contourList[i]->array[j].sy =(int)sy;
     }
   }
   for (i=0; i<nFalseAlarms; i++)  {
     scanf("%f %f %d\n",&mx,&my,&fno);
     falseAlarmList[i].x = (int)mx;
     falseAlarmList[i].y = (int)my;
   }
   printf("MAXLEN=%d\n",MAXLEN);fflush(stdout);
}
/*------------------------------*/
static void **alloc_block(int w,int h,int size)
{
   register char **pel,**p,**fp;
   register char *area;
   int rsize=w*size;            /* full length of a row */

   pel = (char **) malloc(h*sizeof(*pel));
   if (!pel) return (void **) NULL;

   area = (char *) malloc(w*h*size);
   if (!area) return (void **) NULL;
   for (p=pel,fp=p+h; p < fp; p++,area += rsize)
     *p = (char *) area;
   return (void **) pel;
}
/*------------------------------*/
void createContourLayout()
{
 int x,y,i,j;

  if( !(contourLayout = 
      (Contour ***) alloc_block(width,height,sizeof(**contourLayout)) )) {
    fprintf(stderr,"Couldn't allocate contourLayout\n");
    exit(-1);
  }
  for (y=0; y < height; y++)
    for (x=0; x < width; x++)
      contourLayout[y][x] = 0;


  for (i=0; i < nContours; i++) {
    for (j=0; j < contourList[i]->len ; j++) {
      x = contourList[i]->array[j].mx;
      y = height - contourList[i]->array[j].my  -1;
      contourLayout[y][x] = contourList[i];
    }
  }
}

/*------------------------------*/
/*
 * readImage reads an image called "name" into an unsigned short
 * array pointed to by "buffer".  The x and y dimensions are
 * returned in "xsize" and "ysize"
 */

void readImage(char *name, unsigned short **buf)
{
  IMAGE *image;
  int numPixels,f,i;
  char *charbuf;


  image = (IMAGE*)malloc(sizeof(IMAGE));
  memset(image,0,sizeof(IMAGE));

  if (name)
       f = open(name, 0);
   if (f < 0) {
       fprintf(stderr, "iopen: Couldn't open file " );
       exit(-1);
   }

   if (read(f,image,sizeof(IMAGE)) != sizeof(IMAGE)) {
       fprintf(stderr, "iopen: error on read of image header" );
       exit(-1);
   }

   if (image->imagic != IMAGIC) {
      fprintf(stderr, "iopen: bad magic in image file" );
       exit(-1);
   }

   if(ISRLE(image->type)) {
      fprintf(stderr, "iopen: RLE not supported");
      exit(-1);
   }

   image->x = image->y = image->z = 0;
   image->file = f;
   image->offset = 512L;                   /* set up for img_optseek */
   lseek((int)(image->file), 512L, 0);

   width = image->xsize;
   height = image->ysize;
   numPixels = image->xsize * image->ysize;
   *buf=(unsigned short*)malloc(numPixels*sizeof(unsigned short));
   charbuf=(char*)malloc(numPixels*sizeof(char));
   read((int)(image->file),charbuf,numPixels);
   for (i=0; i<numPixels; i++)
     (*buf)[i] = (unsigned short)charbuf[i];
   close(f);
}

/*------------------------------*/
/*
 * writeImage writes a image of dimension "xsize" by "ysize" held
 * in an unsighned short array "buf" to a file called "name"
 * NOTE that the wirtten file is a BYTE array (not unsigned
 * short.
 */

void writeImage(char *name,int xsize, int ysize, char *buf)
{
  IMAGE *image;
  int f,numPixels,i;

  image = (IMAGE*)malloc(sizeof(IMAGE));
  memset(image,0,sizeof(IMAGE));

  if (name)
       f = creat(name, 0666);
   if (f < 0) {
       fprintf(stderr, "iopen: Couldn't open file %s for writing ",name );
       exit(-1);
   }
   image->imagic = IMAGIC;
   image->type = VERBATIM(BPP(1));
   image->xsize=xsize;
   image->ysize=ysize;
   image->zsize=3;
   image->dim=3;
   image->min = 10000000;
   image->max = 0;
   image->wastebytes = 0;
   strncpy(image->name,name,80);
   image->dorev = 0;
   if (write(f,image,sizeof(IMAGE)) != sizeof(IMAGE)) {
       fprintf(stderr, "writeImage: error on write of image header");
       exit(-1);
   }
   image->x = image->y = image->z = 0;
   image->file = f;
   image->offset = 512L;       /* set up for img_optseek */
   lseek((int)(image->file) , 512L, 0);
   numPixels = xsize * ysize * 3;
   write((int)(image->file),buf,numPixels);
   close(f);
}

/*------------------------------*/
void makeCmap()
{
  XColor xc;
  int i;

  for (i=0; i<190;i++) {
     xc.red=xc.green=xc.blue=i<<8;
     if ((XAllocColor(XtDisplay(canvas),
          DefaultColormapOfScreen(XtScreen(canvas)),&xc)) == 0) {
       fprintf(stderr,"XAlloc did not succeed for red=%d green=%d blue=%d i=%d\n",
       xc.red,xc.green,xc.blue,i);
       exit(-1);
     }
     grayCmap[i] = (char)(xc.pixel);
   }
   printf("Created Gray Cmap\n");fflush(stdout);
}


	/*****************************************************/


void readCorners(char *basename, int start, int end)
{
  int i,frameCount,cornerCount;
  int strSize=200;
  FILE *inDataFile;
  char fname[100], str[80], junk[200];
  float x,y;

  frameCount=0;
  printf("start=%d end=%d\n",start,end);fflush(stdout);
  for (i=start; i<=end; i++) {
      sprintf(fname,"%s%d",basename,i);
      printf("opening file %s\n",fname);fflush(stdout);
      if (!(inDataFile = fopen(fname,"r"))) {
         fprintf(stderr,"Error openingfile\n");
         exit(-1);
      }
      cornerCount=0;
      while (fgets(str,strSize,inDataFile)) {
#ifdef OXFORD
	 sscanf(str,"%f %f",&x,&y);
	 xcorners[frameCount][cornerCount] = rint(x);
	 ycorners[frameCount][cornerCount] = rint(y);
#else
         sscanf(str,"%d %d %s",&(xcorners[frameCount][cornerCount]),
                            &(ycorners[frameCount][cornerCount]), junk);
         printf("Read %d %d %s\n",(xcorners[frameCount][cornerCount]),
                            (ycorners[frameCount][cornerCount]), junk);
#endif
         cornerCount++;
      }
      nCorners[frameCount] =cornerCount-1;
      printf("Read %d corners in frame %d %s\n",nCorners[frameCount],frameCount,fname);
      frameCount++;
      fclose(inDataFile);
  }
}

/***********************************************/

void readImages(char *basename,int start, int end)
{
  int i,j,f,frameCount;
  int strSize=80;
  IMAGE *img;
  char fname[80], str[80];
  char *dp;
  unsigned short *buf;
  XColor xc;

  frameCount=0;
  printf("width=%d height=%d\n",width,height);fflush(stdout);
  buf = (unsigned short*)malloc(width*sizeof(unsigned short));
  for (f=start; f<=end; f++) {
     
      sprintf(fname,"%s%d",basename,f);
      printf("open Image %s\n",fname);fflush(stdout);
      if((img = iopen(fname,"r")) == NULL)
      {
	fprintf(stderr, "Error opening image file %s\n", fname);
	exit(1);
      }
      dp = data[frameCount];
      for (i=height-1; i>0; i--) {
        getrow(img,buf,i);
        for (j=0; j<width; j++) {
	  *dp++ = grayCmap[buf[j]];
        }
      }
      frameCount++;
      iclose(img);
    }

}
  











