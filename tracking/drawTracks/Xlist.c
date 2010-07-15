
/*
 *      list.c
 *      Motif Scrolled List Widget Functions
 *
 *      Written for Power Programming Motif
 *
 */

#include  <X11/Intrinsic.h>
#include  <X11/StringDefs.h>
#include  <Xm/Xm.h>
#include  <Xm/List.h>



AddToList( widget, string, position )

Widget  widget;
char    string[];
int     position;

/*
 *      AddToList() adds a text string to a list
 *      widget at a given position in the list.
 */

{       /* AddToList */
        XmString        motif_string, Str2XmString();

        motif_string = Str2XmString( string );

        XmListAddItemUnselected( widget,
                motif_string,
                position );

        XmStringFree( motif_string );
        
}       /* AddToList */


ClearList( widget )

Widget  widget;

/*
 * Clears out all items in 
 * a list widget. In Motif 1.1,
 * we could use XmListDeleteAllItems(),
 * but that's missing in 1.0.
 */

{       /* ClearList */
        int     max, i;

        max = ListSize( widget );

        for( i = 2; i <= max; i++ )
                {
                XmListDeletePos( widget, 
                        0 );    /* delete last list item */
                }

        XmListDeletePos( widget, 1 ); /* first item */

}       /* ClearList */


Widget CreateScrolledList( parent, name, args, n, list_callback )

Widget  parent;
char    name[];
Arg     *args;
int     n;
void    (*list_callback)();     /* call back function */

/*
 *      Creates a scrolled list widget.
 */

{       /* CreateScrolledList */
        Widget  list_widget;

        /*
         * Set up size. Note that
         * n is passed as a parameter to
         * CreateScrolledList().
         */
        XtSetArg( args[n], XmNitemCount, 0 ); n++; 
        XtSetArg( args[n], XmNselectionPolicy, XmSINGLE_SELECT ); n++;

        list_widget = XmCreateScrolledList( parent,
                                name,
                                args,
                                n );

        XtManageChild( list_widget );

        XtAddCallback( list_widget,
                XmNsingleSelectionCallback,
                list_callback,
                NULL );

        return( list_widget );

}       /* CreateScrolledList */


ListSize( widget )

Widget  widget;

/*
 * Returns the number of elements in a list
 */

{       /* ListSize */
        int     size;
        Arg     args[10];

        /*
         * Get the number of items
         * in the list widget
         */
        XtSetArg( args[0], XmNitemCount, &size );

        XtGetValues( widget, args, 1 );

        return( size );

}       /* ListSize */

/* end of file */

