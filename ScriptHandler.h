/* -*- C++ -*-
 * 
 *  ScriptHandler.h - Script manipulation class
 *
 *  Copyright (c) 2001-2003 Ogapee. All rights reserved.
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __SCRIPT_HANDLER_H__
#define __SCRIPT_HANDLER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BaseReader.h"

#define VARIABLE_RANGE 4096
#define ARRAY_VARIABLE_RANGE 200

typedef unsigned char uchar3[3];

class ScriptHandler
{
public:
    enum { END_NONE  = 0,
           END_COMMA = 1,
           END_QUAT  = 2
    };
    struct LabelInfo{
        char *name;
        char *label_header;
        char *start_address;
        int  start_line;
        int  num_of_lines;
    };

    struct ArrayVariable{
        int num_dim;
        int dim[20];
        int *data;
        ArrayVariable(){
            data = NULL;
        };
    };

    enum { VAR_NONE,      // text, color, etc.
           VAR_INT,       // integer variable
           VAR_INT_CONST, // integer const
           VAR_STR,       // string variable
           VAR_STR_CONST, // string const
           VAR_PTR        // array variable
    };
    struct VariableInfo{
        int type;
        int var_no;   // for integer(%), string($)
        int *var_ptr; // for array(?)
    };

    ScriptHandler();
    ~ScriptHandler();
    FILE *fopen( const char *path, const char *mode );

    char *saveStringBuffer();
    int  getOffset( char *pos );
    char *getAddress( int offset );
    char *getCurrent();
    void setCurrent( char *pos, bool reread_flag=true );
    int getLineByAddress( char *address );
    char *getAddressByLine( int line );
    LabelInfo getLabelByAddress( char *address );
    LabelInfo getLabelByLine( int line );
    char *getNext();
    void pushCurrent( char *pos );
    void popCurrent();
    char *getStringBuffer();

    bool isName( const char *name );
    bool isText();
    void setText( bool val ); // exception: for select command to handle string variables in the second line or below
    int  getEndStatus();
    void skipLine( int no=1 );
    void setLinepage( bool val );

    bool isKidoku();
    void markAsKidoku( char *address=NULL );

    int  rereadToken();
    int  readToken();
    const char *readStr( bool reread_flag=false );
    int  readInt( bool reread_flag=false );
    void declareDim();
    void skipToken();

    void setInt( VariableInfo *var_info, int val, int offset=0 );
    void setNumVariable( int no, int val );
    void pushVariable();
    int  getIntVariable( VariableInfo *var_info=NULL );

    void getSJISFromInteger( char *buffer, int no, bool add_space_flag );

    int readScriptSub( FILE *fp, char **buf, int encrypt_mode );
    int readScript( char *path );
    int labelScript();

    LabelInfo lookupLabel( const char* label );
    LabelInfo lookupLabelNext( const char* label );
    void errorAndExit( char *str );

    void setKidokuskip( bool kidokuskip_flag );
    void saveKidokuData();
    void loadKidokuData();
    
    void addNumAlias( const char *str, int no );
    void addStrAlias( const char *str1, const char *str2 );

    typedef enum { LABEL_LOG = 0,
                   FILE_LOG = 1
    } LOG_TYPE;
    struct LogLink{
        LogLink *next;
        char *name;
        LogLink(){
            next = NULL;
            name = NULL;
        };
        ~LogLink(){
            if ( name ) delete[] name;
        };
    };
    LogLink *findAndAddLog( LOG_TYPE type, const char *name, bool add_flag );
    void deleteLog( LOG_TYPE type );
    void loadLog( LOG_TYPE type );
    void saveLog( LOG_TYPE type );
    
    /* ---------------------------------------- */
    /* Variable */
    int num_variables[ VARIABLE_RANGE ];
    int num_limit_upper[ VARIABLE_RANGE ];
    int num_limit_lower[ VARIABLE_RANGE ];
    bool num_limit_flag[ VARIABLE_RANGE ];
    char *str_variables[ VARIABLE_RANGE ];
    VariableInfo current_variable, pushed_variable;
    
    int screen_size;
    enum { SCREEN_SIZE_640x480 = 0,
           SCREEN_SIZE_800x600 = 1,
           SCREEN_SIZE_400x300 = 2,
           SCREEN_SIZE_320x240 = 3
    };

    BaseReader *cBR;
    
private:
    enum { OP_INVALID = 0, // 000
           OP_PLUS    = 2, // 010
           OP_MINUS   = 3, // 011
           OP_MULT    = 4, // 100
           OP_DIV     = 5, // 101
           OP_MOD     = 6  // 110
    };
    
    struct StackInfo{
        struct StackInfo *next;
        char *current_script;

        StackInfo(){
            StackInfo(NULL);
        };
        StackInfo( char *current ){
            next = NULL;
            current_script = current;
        };
    };

    struct NameAlias{
        struct NameAlias *next;
        char *alias;
        int  num;

        NameAlias(){
            next = NULL;
        };
        NameAlias( const char *str, int no ){
            next  = NULL;
            alias = new char[ strlen(str) + 1];
            strcpy( alias, str );
            num = no;
        };
    };
    
    struct StringAlias{
        struct StringAlias *next;
        char *alias;
        char *str;

        StringAlias(){
            next  = NULL;
        };
        StringAlias( const char *str1, const char *str2 ){
            next  = NULL;
            alias = new char[ strlen(str1) + 1];
            strcpy( alias, str1 );
            str = new char[ strlen(str2) + 1];
            strcpy( str, str2 );
        };
    };


    void addStringBuffer( char ch, int string_counter );
    int findLabel( const char* label );

    int parseInt( char **buf );
    int parseIntExpression( char **buf );
    void parseStr( char **buf );

    void readNextOp( char **buf, int *op, int *num );
    int calcArithmetic( int num1, int op, int num2 );
    int decodeArraySub( char **buf, ArrayVariable *array );
    int *decodeArray( char **buf, int offset = 0 );


    /* ---------------------------------------- */
    /* Variable */
    NameAlias root_name_alias, *last_name_alias;
    StringAlias root_str_alias, *last_str_alias;
    
    ArrayVariable array_variables[ ARRAY_VARIABLE_RANGE ], tmp_array_variable;

    char *archive_path;
    int script_buffer_length;
    char *script_buffer;
    
    int string_buffer_length;
    char *string_buffer; // update only be readToken
    char *saved_string_buffer; // updated only by saveStringBuffer
    char *str_string_buffer; // updated only by readStr

    LabelInfo *label_info;
    int num_of_labels;
    struct LogInfo{
        LogLink root_log;
        LogLink *current_log;
        int num_logs;
        char *filename;
    } log_info[2];

    bool kidokuskip_flag;
    char *kidoku_buffer;

    bool text_flag; // true if the current token is text
    bool line_head_flag; // true if the current token is the first token of the line
    int  end_status;
    bool linepage_flag;

    StackInfo root_stack_info, *last_stack_info;
    char *current_script;
    char *next_script;
    char *stack_script;
};

#endif // __SCRIPT_HANDLER_H__
