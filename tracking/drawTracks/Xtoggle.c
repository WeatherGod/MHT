
/*
 *      toggle.c
 *      Motif Toggle Button test
 *
 *      Written for Power Programming Motif
 *
 */

#include  <X11/Intrinsic.h>
#include  <X11/StringDefs.h>
#include  <Xm/Xm.h>
#include  <Xm/RowColumn.h>
#include  <Xm/ToggleB.h> 


Widget CreateToggle( parent, name, message, state, type, callback_func )

Widget  parent;
char    name[];
char    message[];              /* text to appear next to toggle */
Boolean state;                  /* initial state */
int     type;                   /* XmN_OF_MANY or XmONE_OF_MANY */
void    (*callback_func)();     /* state change callback */

/*
 *      Creates and manages a toggle button widget. The initial
 *      state of the toggle button is set to state. When the
 *      state changes, callback_func() will be called. The
 *      type specifies the radio button diamond XmONE_OF_MANY or the
 *      square XmN_OF_MANY shape for the toggle button. 
 */

{       /* CreateToggle */
        Widget          toggle_widget;
        Arg             args[10];
        int             n;
        XmString        motif_string; 
        XmString        Str2XmString(); /* string.c */


        /*
         * Convert message to
         * an XmString.
         */
        motif_string = Str2XmString( message );


        n = 0;
        XtSetArg( args[n], XmNlabelString, motif_string ); n++;
        XtSetArg( args[n], XmNindicatorType, type ); n++;


        /*
         * Try XmNindicatorOn with True and False
         */
        XtSetArg( args[n], XmNindicatorOn, True ); n++;
        /* XtSetArg( args[n], XmNindicatorOn, False ); n++; */


        XtSetArg( args[n], XmNset, state ); n++; /* on or off */

        toggle_widget = XmCreateToggleButton( parent,
                                name,
                                args, 
                                n );


        XtManageChild( toggle_widget );



        /*
         * Add a callback
         * for when the value
         * is changed.
         */
        XtAddCallback( toggle_widget,
                XmNvalueChangedCallback,
                callback_func,
                NULL );


        XmStringFree( motif_string );
        

        return( toggle_widget );

}       /* CreateToggle */


SetToggle( widget, state )

Widget  widget;
int     state;

/*
 *      Sets the value of a toggle button to
 *      True of False, depending on the 
 *      value of state. If state != 0,
 *      then the toggle is set to True.
 *      If state == 0, then the toggle is
 *      set to False.
 */

{       /* SetToggle */
        Arg     args[10];
        int     n;

        if ( state != 0 )
                {
                n = 0;
                XtSetArg( args[n], XmNset, True ); n++; /* on */
                }
        else
                {
                n = 0;
                XtSetArg( args[n], XmNset, False ); n++; /* off */
                }

        XtSetValues( widget, args, n );

}       /* SetToggle */


Widget CreateRadioBox( parent, name )

Widget  parent;
char    name[];

/*
 *      Creates a RowColumn widget to oversee a group
 *      of radio buttons.
 */

{       /* CreateRadioBox */
        Widget  radio;
        Arg     args[10];
        int     n;

        /*
         * Set up RowColumn to manage radio buttons.
         */
        n = 0;
        XtSetArg( args[n], XmNradioBehavior,  True ); n++;
        XtSetArg( args[n], XmNradioAlwaysOne, True ); n++;

        radio = XmCreateRowColumn( parent,
                        name,
                        args,
                        n );

        XtManageChild( radio );

        return( radio );

}       /* CreateRadioBox */

/* end of file */



