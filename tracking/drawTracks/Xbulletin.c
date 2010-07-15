
/*
 *      bulletin.c
 *      Utility functions for the Motif Bulletin Board Dialog.
 *
 *      Written for Power Programming Motif
 */


#include  <X11/Intrinsic.h>
#include  <X11/StringDefs.h>
#include  <Xm/Xm.h>
#include  <Xm/BulletinB.h>


BulletinDefaultButton( bulletin, button )

Widget  bulletin;
Widget  button;

/*
 *      BulletinDefaultButton() sets the given bulletin board
 *      dialog to have the given push button as its default
 *      button (the one that is triggered when the user presses
 *      the Return key).
 */

{       /* BulletinDefaultButton */
        Arg     args[10];
        int     n;

        /*
         *      Set button as default
         */
        n = 0;
        XtSetArg( args[n], XmNdefaultButton, button ); n++;

        XtSetValues( bulletin, args, n );

}       /* BulletinDefaultButton */


BulletinDialogTitle( bulletin, title )

Widget  bulletin;
char    title[];

/*
 *      BulletinDialogTitle() sets the title of
 *      the given bulletin board dialog.
 */

{       /* BulletinDialogTitle */ 
        XmString        motif_string;
        XmString        Str2XmString(); /* string.c */
        Arg             args[10];
        int             n;


        /*
         * Set up title of dialog box
         */
        motif_string = Str2XmString( title );

        n = 0;
        XtSetArg( args[n], XmNdialogTitle, motif_string );  n++;

        XtSetValues( bulletin, args, n );

        XmStringFree( motif_string );

}       /* BulletinDialogTitle */ 


Widget CreateBulletinDialog( parent, name )

Widget  parent;
char    name[];

/*
 *      CreateBulletinDialog() creates a bulletin board
 *      dialog, but does not manage it. The dialog
 *      is set up to forbid resizes.
 */

{       /* CreateBulletinDialog */
        Widget  bulletin;
        Arg     args[10];
        int     n;


        /*
         * Create a bulletin board dialog, but don't
         * manage it. A callback will manage
         * it later.
         */
        n = 0;
        XtSetArg( args[n], XmNautoUnmanage, False ); n++;
        XtSetArg( args[n], XmNnoResize, True ); n++;

        bulletin = XmCreateBulletinBoardDialog( parent,
                                name, 
                                args,
                                n );

        return( bulletin );

}       /* CreateBulletinDialog */

/* end of file */



