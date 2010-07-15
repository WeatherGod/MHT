
/*
 *      string.c
 *      Motif utility function to convert a text string
 *      to a Motif compound string.
 *
 *      Written for Power Programming Motif
 *
 */

#include  <Xm/Xm.h>


XmString Str2XmString( string )

char    *string;

/*
 *      Str2XmString() converts a standard C-style
 *      null-terminated string into a Motif-style
 *      compound string. Newline characters (\n)
 *      in the C string become separators in the
 *      Motif string, because we use
 *      XmStringLtoRCreate().
 */

{       /* Str2XmString */
        XmString        motif_string;

        motif_string = XmStringCreateLtoR( string, 
                                XmSTRING_DEFAULT_CHARSET );

        return( motif_string );

}       /* Str2XmString */

/* end of file */

