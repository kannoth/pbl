/*
 pblStringBuilder.c - C implementation of a mutable sequence of characters.

 Copyright (C) 2010, 2015   Peter Graf

 This file is part of PBL - The Program Base Library.
 PBL is free software.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 For more information on the Program Base Library or Peter Graf,
 please see: http://www.mission-base.com/.

 $Log: pblStringBuilder.c,v $
 Revision 1.5  2016/06/03 21:13:30  peter
 Syncing with GIT version.

 Revision 1.3  2015/11/19 23:31:43  peter
 Just formatting the source.

 Revision 1.2  2015/04/06 14:33:11  peter
 Some cleanup.

 Revision 1.1  2015/03/23 15:53:10  peter
 Added the string builder.

 */
/*
 * Make sure "strings <exe> | grep Id | sort -u" shows the source file versions
 */
char* pblStringBuilder_c_id =
        "$Id: pblStringBuilder.c,v 1.5 2016/06/03 21:13:30 peter Exp $";

#include <stdio.h>
#include <memory.h>
#include <stdarg.h>
#include <errno.h>

#ifndef __APPLE__
#include <malloc.h>
#endif

#include <stdlib.h>

#include "pbl.h"

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

/**
 * Creates a new string builder.
 *
 * This function has a time complexity of O(1).
 *
 * @return PblStringBuilder * retPtr != NULL: A pointer to the new string builder.
 * @return PblStringBuilder * retPtr == NULL: An error, see pbl_errno:
 *
 * <BR>PBL_ERROR_OUT_OF_MEMORY - Out of memory.
 */
PblStringBuilder * pblStringBuilderNew( void )
{
	PblStringBuilder * stringBuilder = (PblStringBuilder *)pbl_malloc0( "pblStringBuilderNew",
                                       sizeof(PblStringBuilder) );
    if( !stringBuilder )
    {
        return NULL;
    }

    stringBuilder->list = pblListNewArrayList();
    if( !stringBuilder->list )
    {
    	PBL_FREE(stringBuilder);
        return NULL;
    }

    return stringBuilder;
}

/**
 * Removes all of the elements from the string builder.
 *
 * This function has a time complexity of O(N),
 * with N being the number of elements in the string builder.
 *
 * @return void
 */
void pblStringBuilderClear(      /*                              */
PblStringBuilder * stringBuilder /** The string builder to clear */
)
{
	while( pblListSize( stringBuilder->list ) > 0 )
	{
		void * element = pblListRemoveLast( stringBuilder->list );
		if( element != NULL && element != (void *) -1)
		{
			PBL_FREE( element );
		}
	}
    stringBuilder->length = 0;
}

/**
 * Frees the string builder's memory from heap.
 *
 * This function has a time complexity of O(N),
 * with N being the number of elements in the string builder.
 *
 * @return void
 */
void pblStringBuilderFree(       /*                             */
PblStringBuilder * stringBuilder /** The string builder to free */
)
{
    pblStringBuilderClear( stringBuilder );
    pblListFree( stringBuilder->list );
    PBL_FREE( stringBuilder );
}

/**
 * Returns the number of elements in the string builder.
 *
 * This function has a time complexity of O(1).
 *
 * @return int rc: The number of elements in the string builder.
 */
int pblStringBuilderSize(        /*                            */
PblStringBuilder * stringBuilder /** The string builder to use */
)
{
    return pblListSize( stringBuilder->list );
}

/**
 * Returns the number of characters in the string builder.
 *
 * This function has a time complexity of O(1).
 *
 * @return size_t rc: The number of characters in the string builder.
 */
size_t pblStringBuilderLength(   /*                            */
PblStringBuilder * stringBuilder /** The string builder to use */
)
{
    return stringBuilder->length;
}

/**
 * Appends a '\0' terminated string to the
 * end of the data of the string builder.
 *
 * This function has a time complexity of O(N),
 * with N beeing the length of the string to append.
 *
 * @return size_t rc >= 0: The length of the string builder.
 * @return size_t rc == -1: An error, see pbl_errno:
 *
 * <BR>PBL_ERROR_OUT_OF_MEMORY - Out of memory.
 */
size_t pblStringBuilderAppendStr(    /*                             */
PblStringBuilder * stringBuilder,    /** The string builder to use  */
const char * data  /** The data to be added to the string builder   */
)
{
	size_t length = 0;
    char * element = NULL;

    if( data )
    {
    	length = strlen( data );
    	element = (char *)pbl_memdup( "pblStringBuilderAppendStr", (void *)data, length + 1 );
        if( !element )
        {
            return (size_t) -1;
        }
    }

    if( pblListAdd( stringBuilder->list, element ) < 0 )
    {
    	PBL_FREE( element );
    	return (size_t) -1;
    }

    return stringBuilder->length += length;
}

/**
 * Appends at most n bytes of a string to the
 * end of the data of the string builder.
 *
 * This function has a time complexity of O(N),
 * with N beeing the number of characters to append.
 *
 * @return size_t rc >= 0: The length of the string builder.
 * @return size_t rc == -1: An error, see pbl_errno:
 *
 * <BR>PBL_ERROR_OUT_OF_MEMORY - Out of memory.
 */
size_t pblStringBuilderAppendStrN(     /*                            */
PblStringBuilder * stringBuilder,      /** The string builder to use */
size_t n,           /** The maximum number of bytes to append        */
const char * data   /** The data to be added to the string builder   */
)
{
	size_t length = 0;
    char * element = NULL;

    if( data && n > 0 )
    {
    	element = (char *)pbl_malloc( "pblStringBuilderAppendStrN", n + 1 );
        if( !element )
        {
            return -1;
        }

        strncpy( element, data, n );
        element[ n ] = '\0';
        length = strlen( element );
    }

    if( pblListAdd( stringBuilder->list, element ) < 0 )
    {
    	PBL_FREE( element );
    	return -1;
    }

    return stringBuilder->length += length;
}

#define _PBL_BUFFER_ON_STACK_SIZE_MAX  4096


/**
 * Appends a variable string defined by the format parameter to the
 * end of the data of the string builder.
 *
 * This function will append at most 4096 bytes of data to the string buffer.
 * If you need to append more bytes, use the function pblStringBuilderAppendN.
 *
 * For the possible values of the format parameter,
 * see the documentation of the printf(3) family of functions.
 *
 * This function has a time complexity of O(N),
 * with N being the length of the string being appended.
 *
 * @return size_t rc >= 0: The length of the string builder.
 * @return size_t rc == -1: An error, see pbl_errno:
 *
 * <BR>PBL_ERROR_OUT_OF_MEMORY - Out of memory.
 * <BR>PBL_ERROR_PARAM_FORMAT  - The parameter format is not valid.
 */
size_t pblStringBuilderAppend(    /*                                        */
PblStringBuilder * stringBuilder, /** The string builder to use             */
const char *format,               /** The format of the arguments to append */
... )                             /** The variable arguments to append      */
{
	char * tag = "pblStringBuilderAppend";
	size_t size = _PBL_BUFFER_ON_STACK_SIZE_MAX;
	char buffer[ _PBL_BUFFER_ON_STACK_SIZE_MAX ];
	int rc;
	va_list args;

	if( !format )
	{
		return pblStringBuilderAppendStr( stringBuilder, NULL );
	}

	va_start( args, format );
    rc = vsnprintf( buffer, size, format, args );
    va_end( args );

    if( rc < 0 )
    {
        snprintf( pbl_errstr, PBL_ERRSTR_LEN,
        		"%s: vsnprintf of format '%s' failed with errno %d\n", tag, format, errno );
        pbl_errno = PBL_ERROR_PARAM_FORMAT;

        return (size_t) -1;
    }

    return pblStringBuilderAppendStr( stringBuilder, buffer );
}

/**
 * Appends at most n bytes of a variable string defined by the format parameter to the
 * end of the data of the string builder.
 *
 * For the possible values of the format parameter,
 * see the documentation of the printf(3) family of functions.
 *
 * This function has a time complexity of O(N),
 * with N being the length of the string being appended.
 *
 * @return size_t rc >= 0: The length of the string builder.
 * @return size_t rc == -1: An error, see pbl_errno:
 *
 * <BR>PBL_ERROR_OUT_OF_MEMORY - Out of memory.
 * <BR>PBL_ERROR_PARAM_FORMAT  - The parameter format is not valid.
 */
size_t pblStringBuilderAppendN(    /*                                       */
PblStringBuilder * stringBuilder, /** The string builder to use             */
size_t n,                         /** The maximum number of bytes to append */
const char *format,               /** The format of the arguments to append */
... )                             /** The variable arguments to append      */
{
	char * tag = "pblStringBuilderAppendN";
	char * data = NULL;
	int rc;
	size_t size_t_rc;
	va_list args;

	if( !format )
	{
		return pblStringBuilderAppendStr( stringBuilder, NULL );
	}

	if( n < 1 )
	{
		return pblStringBuilderAppendStr( stringBuilder, "" );
	}

    data = (char *)pbl_malloc( tag, n + 1 );
    if( !data )
    {
        return (size_t) -1;
    }

    va_start( args, format );
    rc = vsnprintf( data, n + 1, format, args );
    va_end( args );

    if( rc < 0 )
    {
    	PBL_FREE( data );

    	snprintf( pbl_errstr, PBL_ERRSTR_LEN,
    		       "%s: vsnprintf of format '%s' failed with errno %d\n", tag, format, errno );
    	pbl_errno = PBL_ERROR_PARAM_FORMAT;

    	return (size_t) -1;
    }

    data[ n ] = '\0';
    size_t_rc = pblStringBuilderAppendStr( stringBuilder, data );
    PBL_FREE( data );
    return size_t_rc;
}

/**
 * Converts the string builder's data to a '\0' terminated string.
 *
 * This function has a time complexity of O(N),
 * with N beeing the length of the data of the string builder.
 *
 * Note: The memory for the data returned is malloced,
 * it is the caller's responsibility to free that memory!
 *
 * @return char * rc != NULL: The data.
 * @return char * rc == NULL: An error, see pbl_errno:
 *
 * <BR>PBL_ERROR_OUT_OF_MEMORY - Out of memory.
 */
char * pblStringBuilderToString( /*                            */
PblStringBuilder * stringBuilder /** The string builder to use */
)
{
	char * tag = "pblStringBuilderToString";
	int hasNext;
    char * data = NULL;
    char * ptr;
    PblIterator   iteratorBuffer;
    PblIterator * iterator = &iteratorBuffer;

    if( stringBuilder->length == 0 )
    {
    	return (char *)pbl_strdup( tag, "" );
    }

    data = (char *)pbl_malloc( tag, stringBuilder->length + 1 );
    if( !data )
    {
    	return (char*) NULL;
    }
    ptr = data;

    if( pblIteratorInit( (PblCollection *)stringBuilder->list, iterator ) < 0 )
    {
    	PBL_FREE( data );
        return (char*) NULL;
    }

    while( ( hasNext = pblIteratorHasNext( iterator ) ) > 0 )
    {
        char * element = (char*) pblIteratorNext( iterator );
        if( element == (void*)-1 )
        {
            // Concurrent modification
            //
        	PBL_FREE( data );
            return (char*) NULL;
        }
        if( element )
        {
        	size_t length = strlen( element );
        	memcpy( ptr, element, length + 1 );
        	ptr += length;
        }
    }
    if( hasNext < 0 )
    {
        // Concurrent modification
        //
    	PBL_FREE( data );
        return (char*) NULL;
    }

    return data;
}
