
/*
 *      push.c
 *      Motif push-button widget utility function.
 *
 *      Written for Power Programming Motif
 */

#include  <X11/Intrinsic.h>
#include  <X11/StringDefs.h>
#include  <Xm/Xm.h>
#include  <Xm/PushB.h>


Widget CreatePushButton( parent, name, args, n, callback_func )

Widget  parent;
char    name[];
Arg     *args;
int     n;
void    (*callback_func)();

/*
 *      CreatePushButton() creates a Motif
 *      XmPushButton widget, using whatever
 *      args are passed to the function.
 *      callback_func() is set up
 *      as the activateCallback function
 *      for this push-button widget.
 */

{       /* function CreatePushButton */
        Widget  push_widget;

        push_widget = XtCreateManagedWidget( name,
                        xmPushButtonWidgetClass,
                        parent, 
                        args,
                        n );


        /*
         * Set up a callback function
         * to be called whenever
         * the push button is
         * "activated".
         */
        XtAddCallback( push_widget,
                XmNactivateCallback,
                callback_func,
                NULL );


        return( push_widget );
        
}       /* function CreatePushButton */

/* end of file */

