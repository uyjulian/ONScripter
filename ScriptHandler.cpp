/* -*- C++ -*-
 *
 *  ScriptHandler.cpp - Script manipulation class
 *
 *  Copyright (c) 2001-2002 Ogapee. All rights reserved.
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

#include "ScriptHandler.h"

#define SKIP_SPACE(p) while ( *(p) == ' ' || *(p) == '\t' ) (p)++

ScriptHandler::ScriptHandler()
{
    int i;
    
    script_buffer = NULL;
    svga_flag = false;
    end_with_comma_flag = false;
    num_of_labels = 0;
    num_of_labels_accessed = 0;
    kidoku_buffer = NULL;

    text_flag = true;
    linepage_flag = false;

    string_buffer_length = 512;
    string_buffer = new char[ string_buffer_length ];
    str_string_buffer  = new char[ string_buffer_length ];
    saved_string_buffer  = new char[ string_buffer_length ];

    last_stack_info = &root_stack_info;

    /* ---------------------------------------- */
    for ( i=0 ; i<VARIABLE_RANGE ; i++ ){
        num_variables[i] = 0;
        num_limit_flag[i] = false;
        str_variables[i] = new char[1];
        str_variables[i][0] = '\0';
    }

    last_name_alias = &root_name_alias;
    last_name_alias->next = NULL;
    last_str_alias = &root_str_alias;
    last_str_alias->next = NULL;
}

ScriptHandler::~ScriptHandler()
{
    if ( script_buffer ) delete[] script_buffer;
    if ( string_buffer ) delete[] string_buffer;
    if ( str_string_buffer ) delete[] str_string_buffer;
    if ( saved_string_buffer ) delete[] saved_string_buffer;
}

FILE *ScriptHandler::fopen( const char *path, const char *mode )
{
    char file_name[256];

    sprintf( file_name, "%s%s", archive_path, path );
    return ::fopen( file_name, mode );
}

char *ScriptHandler::saveStringBuffer()
{
    strcpy( saved_string_buffer, string_buffer );
    return saved_string_buffer;
}

char *ScriptHandler::getCurrent()
{
    return current_script;
}

void ScriptHandler::setCurrent( char *pos, bool reread_flag )
{
    text_flag = true;
    text_line_flag = next_text_line_flag = false;
    current_script = pos;
    if ( reread_flag ) 
        rereadToken();
    else
        next_script = pos;
}

char *ScriptHandler::getNext()
{
    return next_script;
}

void ScriptHandler::pushCurrent( char *pos, bool reread_flag )
{
    StackInfo *info = new StackInfo( current_script );

    last_stack_info->next = info;
    last_stack_info = last_stack_info->next;

    current_script = pos;
    if ( reread_flag ) 
        rereadToken();
    else
        next_script = pos;
}

void ScriptHandler::popCurrent()
{
    StackInfo *info = &root_stack_info;

    if ( last_stack_info == &root_stack_info )
        errorAndExit( "StackInfo: under flow." );
    
    while( info->next != last_stack_info ) info = info->next;

    current_script = info->next->current_script;
    
    last_stack_info = info;
    delete last_stack_info->next;
    last_stack_info->next = NULL;

    rereadToken();
}

bool ScriptHandler::isName( const char *name )
{
    return (strncmp( name, string_buffer, strlen(name) )==0)?true:false;
}

bool ScriptHandler::isText()
{
    return text_line_flag;
}

void ScriptHandler::setText( bool val )
{
    text_flag = val;
}

bool ScriptHandler::isQuat()
{
    return quat_flag;
}

bool ScriptHandler::isEndWithComma()
{
    return end_with_comma_flag;
}

void ScriptHandler::skipLine( int no )
{
    for ( int i=0 ; i<no ; i++ ){
        if ( *current_script != 0x0a )
        {
            //current_script = next_script;
            while ( *current_script != 0x0a ) current_script++;
        }
        current_script++;
    }
    next_text_line_flag = false;
    text_flag = true;
    rereadToken();
}

void ScriptHandler::setLinepage( bool val )
{
    linepage_flag = val;
}

bool ScriptHandler::isKidoku()
{
    int offset = current_script - script_buffer;
    
    return (kidoku_buffer[ offset/8 ] & ((char)1 << (offset % 8)))?true:false;
}

void ScriptHandler::markAsKidoku()
{
    int offset = current_script - script_buffer;
    
    kidoku_buffer[ offset/8 ] |= ((char)1 << (offset % 8));
}

bool ScriptHandler::rereadToken()
{
    next_script = current_script;
    return readToken();
}

bool ScriptHandler::readToken()
{
    current_script = next_script;
    char *buf = current_script;
    unsigned int i;
    end_with_comma_flag = false;
    quat_flag = false;

    int string_counter=0, no;
    bool op_flag = true;
    bool num_flag = false;
    bool variable_flag = false;
    char num_buf[10], num_sjis_buf[3];

    char ch = *buf++;
    //printf("read %x %x %c(%x)\n", buf-1, script_buffer, ch, ch );
    while ( ch == ' ' || ch == '\t' || ch == ':' ){
        if ( ch == ':' ) next_text_line_flag = false;
        ch = *buf++;
    }

    text_line_flag = next_text_line_flag;

    if ( ch == 0x0a || ch == '\0' )
    {
        text_flag = true;
        //text_line_flag = text_flag;
        next_text_line_flag = false;
        
        addStringBuffer( ch, string_counter++ );
        addStringBuffer( '\0', string_counter++ );
        next_script = buf;

        //printf("readToken 0x0a is text %d script %x\n", text_line_flag, current_script  );
        return end_with_comma_flag;
    }

    if ( ch == ';' ){
        addStringBuffer( ch, string_counter++ );
        do{
            ch = *buf++;
        } while ( ch != 0x0a && ch != '\0' );
    }
    else if ( ch == '"' )
    {
        quat_flag = true;
        ch = *buf++;
        while ( ch != '"' && ch != 0x0a ){
            addStringBuffer( ch, string_counter++ );
            ch = *buf++;
        }
        buf++;
    }
    else while ( ch != 0x0a && ch != '\0' && ch != '"' )
    {
        if ( op_flag && !text_flag ){
            while ( ch == '<' || ch == '>' ||
                    ch == '=' || ch == '!' ||
                    ch == '&')
            {
                addStringBuffer( ch, string_counter++ );
                ch = *buf++;
            }
            if ( string_counter > 0 &&
                 (string_buffer[0] != '!' || string_counter != 1 ) ){
                break;
            }
            else{
                op_flag = false;
            }
        }
        else{
            op_flag = false;
        }

        if ( (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') )
        {
            if ( num_flag ) break; // if the first letter is a number, the token is a number
            if ( string_counter == 0 ) text_flag = false;
            if ( !text_flag && ch >= 'A' && ch <= 'Z' ) ch += 'a' - 'A';
            addStringBuffer( ch, string_counter++ );
        }
        else if ( ch >= '0' && ch <= '9' || ch == '-' ){
            if ( string_counter == 0 ){
                text_flag = false;
                num_flag = true;
            }
            else if ( ch == '-' && !text_flag ) break;
            addStringBuffer( ch, string_counter++ );
        }
        else if ( ch == ',' && string_counter == 0 ) // for select
        {
            addStringBuffer( ch, string_counter++ );
            buf++;
            text_flag = false;
            break;
        }
        else if ( ch == '*' && string_counter == 0 )
        {
            text_flag = false;
            addStringBuffer( ch, string_counter++ );
        }
        else if ( ch == '$' )
        {
            if ( text_flag )
            {
                no = parseInt( &buf );
                if ( str_variables[no] ){
                    for ( i=0 ; i<strlen( str_variables[no] ) ; i++ ){
                        addStringBuffer( str_variables[no][i], string_counter++ );
                    }
                }
                text_line_flag = true;
                next_text_line_flag = true;
            }
            else{
                if ( string_counter != 0 ) break;
                variable_flag = true;
                addStringBuffer( '$', string_counter++ );
            }
                
        }
        else if ( ch == '%' || ch == '?'){
            if ( text_flag ){
                buf--;
                no = parseInt( &buf );
                if ( no<0 ){
                    addStringBuffer( "Å|"[0], string_counter++ );
                    addStringBuffer( "Å|"[1], string_counter++ );
                    sprintf( num_buf, "%d", -no );
                }
                else{
                    sprintf( num_buf, "%d", no );
                }
                for ( i=0 ; i<strlen( num_buf ) ; i++ ){
                    getSJISFromInteger( num_sjis_buf, num_buf[i] - '0', false );
                    addStringBuffer( num_sjis_buf[0], string_counter++ );
                    addStringBuffer( num_sjis_buf[1], string_counter++ );
                }
                text_line_flag = true;
                next_text_line_flag = true;
            }
            else{
                if ( string_counter != 0 && !variable_flag ) break;
                variable_flag = true;
                addStringBuffer( ch, string_counter++ );
            }
        }
        else if ( text_flag && (ch == '@' || ch == '\\') )
        {
            addStringBuffer( ch, string_counter++ );
            buf++;
#if 0
            // should be processed in caller ??
            if ( textgosub_label )
            {
                text_flag = true;
            }
#endif            
            break;
        }
        else if ( ch & 0x80 ){
            // if text appears immediately after a command, break.
            if ( string_counter != 0 && !text_flag ) break;
            
            addStringBuffer( ch, string_counter++ );
            ch = *buf++;
            addStringBuffer( ch, string_counter++ );
            text_flag = true;
            text_line_flag = true;
            next_text_line_flag = true;
        }
        else{
            if ( !text_flag &&
                 ( ch == ',' ||
                   ch == ' ' ||
                   ch == '\t' ||
                   ch == '<' ||
                   ch == '>' ||
                   ch == '=' ||
                   ch == '!' ||
                   ch == '&' ||
                   ch == ':' ||
                   ch == ';' ||
                   ch == ')' || 
                   ch == '*' ) ) break;
            addStringBuffer( ch, string_counter++ );
        }
            
        ch = *buf++;
    }

    buf--;
    SKIP_SPACE( buf );
    if ( *buf == ',' )
    {
        end_with_comma_flag = true;
        buf++;
        SKIP_SPACE( buf );
    }

    if ( linepage_flag && text_line_flag )
        addStringBuffer( '\\', string_counter++ );
    
    addStringBuffer( '\0', string_counter++ );
    next_script = buf;

    //printf("readToken [%s](%x) is %s script %x, %d\n", string_buffer, *string_buffer, text_line_flag?"text":"command", current_script, end_with_comma_flag );
    return end_with_comma_flag;
}

char *ScriptHandler::getStringBuffer()
{
    return string_buffer;
}

const char *ScriptHandler::readStr( char *dst_buf, bool reread_flag )
{
    if ( !dst_buf ) dst_buf = str_string_buffer;
    if ( reread_flag ) next_script = current_script;
    
    bool condition_flag = false;
    
    if ( next_script[0] == '(' ){
        condition_flag = true;
        next_script++;
        readStr( dst_buf );
        if ( *next_script == ')' ) next_script++;
    }
    else{
        readToken();
        strcpy( dst_buf, string_buffer );
    }

    if ( condition_flag ){ // check condition code
        if ( cBR->getAccessFlag( dst_buf ) ){
            readStr( dst_buf );
            char *buf = new char[ strlen( dst_buf ) + 1 ];
            strcpy( buf, dst_buf );
            readStr( dst_buf );
            strcpy( dst_buf, buf );
            delete[] buf;
        }
        else{
            readStr( dst_buf );
            readStr( dst_buf );
        }
    }
    
    if ( string_buffer[0] == '$' ){
        char *p_dst_buf = string_buffer+1;
        int no = parseInt( &p_dst_buf );
        strcpy( dst_buf, str_variables[ no ] );
    }
    else{
        /* ---------------------------------------- */
        /* Solve str aliases */
        StringAlias *p_str_alias = root_str_alias.next;

        while( p_str_alias ){
            if ( !strcmp( p_str_alias->alias, (const char*)dst_buf ) ){
                strcpy( dst_buf, p_str_alias->str );
                break;
            }
            p_str_alias = p_str_alias->next;
        }
    }
    return dst_buf;
}

int ScriptHandler::readInt( bool reread_flag )
{
    if ( reread_flag ) next_script = current_script;
    readToken();
    char *buf = string_buffer;
    int ret = parseInt( &buf );
    return ret;
}

int ScriptHandler::parseInt( char **buf )
{
    int no;

    SKIP_SPACE( *buf );

    if ( (*buf)[0] == '\0' || (*buf)[0] == ':' || (*buf)[0] == ';' ){
        no = 0;
    }
    else if ( (*buf)[0] == '%' ){
        (*buf)++;
        no = num_variables[ parseInt( buf ) ];
    }
    else if ( (*buf)[0] == '?' ){
        no = *decodeArray( buf );
    }
    else{
        char ch, alias_buf[256];
        int alias_buf_len = 0, alias_no = 0;
        bool first_flag = true;
        bool num_alias_flag = true;
        bool minus_flag = false;
        
        while( 1 ){
            ch = **buf;
            
            if ( (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_' ){
                if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
                if ( !first_flag && num_alias_flag == false ) break;
                alias_buf[ alias_buf_len++ ] = ch;
            }
            else if ( ch >= '0' && ch <= '9' ){
                if ( first_flag ) num_alias_flag = false;
                if ( num_alias_flag ) alias_buf[ alias_buf_len++ ] = ch;
                else alias_no = alias_no * 10 + ch - '0';
            }
            else if ( ch == '-' ){
                if ( first_flag ) num_alias_flag = false;
                if ( num_alias_flag ) alias_buf[ alias_buf_len++ ] = ch;
                else minus_flag = true;
            }
            else break;
            (*buf)++;
            first_flag = false;
        }
        if ( minus_flag ) alias_no = -alias_no;
        
        /* ---------------------------------------- */
        /* Solve num aliases */
        if ( num_alias_flag ){
            alias_buf[ alias_buf_len ] = '\0';
            NameAlias *p_name_alias = root_name_alias.next;

            while( p_name_alias ){
                /* In case of constant */
                if ( !strcmp( p_name_alias->alias,
                              (const char*)alias_buf ) ){
                    alias_no = p_name_alias->num;
                    break;
                }
                p_name_alias = p_name_alias->next;
            }
            if ( !p_name_alias ){
                printf("can't find name alias for %s... assume 0.\n", alias_buf );
                alias_no = 0;
            }
        }
        no = alias_no;
    }

    SKIP_SPACE( *buf );

    return no;
}

void ScriptHandler::setInt( char *buf, int val, int offset )
{
    char *p_buf;

    if ( buf[0] == '%' ){
        p_buf = buf + 1;
        setNumVariable( parseInt( &p_buf ) + offset, val );
    }
    else if ( buf[0] == '?' ){
        p_buf = buf;
        *(decodeArray( &p_buf, offset )) = val;
    }
    else{
        errorAndExit( "setInt: no variables." );
    }
}

void ScriptHandler::setNumVariable( int no, int val )
{
    if ( no >= VARIABLE_RANGE ) errorAndExit( "setNumVariable: range exceeds." );
    if ( num_limit_flag[no] ){
        if      ( val < num_limit_lower[no] ) val = num_limit_lower[no];
        else if ( val > num_limit_upper[no] ) val = num_limit_upper[no];
    }
    num_variables[no] = val;
}

int ScriptHandler::decodeArraySub( char **buf, struct ArrayVariable *array )
{
    SKIP_SPACE( *buf );
    
    (*buf)++; // skip '?'
    int no = parseInt( buf );

    SKIP_SPACE( *buf );
    array->num_dim = 0;
    while ( **buf == '[' ){
        (*buf)++;
        array->dim[array->num_dim] = parseInt( buf );
        array->num_dim++;
        SKIP_SPACE( *buf );
        if ( **buf != ']' ) errorAndExit( "decodeArraySub: no ]." );
        (*buf)++;
    }
    for ( int i=array->num_dim ; i<20 ; i++ ) array->dim[i] = 0;

    return no;
}

void ScriptHandler::getSJISFromInteger( char *buffer, int no, bool add_space_flag )
{
    int c = 0;
    char num_str[] = "ÇOÇPÇQÇRÇSÇTÇUÇVÇWÇX";
    if ( no >= 10 ){
        buffer[c++] = num_str[ no / 10 % 10 * 2];
        buffer[c++] = num_str[ no / 10 % 10 * 2 + 1];
    }
    else if ( add_space_flag ){
        buffer[c++] = ((char*)"Å@")[0];
        buffer[c++] = ((char*)"Å@")[1];
    }
    buffer[c++] = num_str[ no % 10 * 2];
    buffer[c++] = num_str[ no % 10 * 2 + 1];
    buffer[c++] = '\0';
}

int ScriptHandler::readScriptSub( FILE *fp, char **buf, bool encrypt_flag )
{
    bool newline_flag = true;
    bool cr_flag = false;

    while(1)
    {
        int ch = fgetc( fp );
        if ( ch != EOF && encrypt_flag ) ch ^= 0x84;
        
        if ( cr_flag && ch != 0x0a ){
            *(*buf)++ = 0x0a;
            newline_flag = true;
            cr_flag = false;
        }
        if ( ch == EOF ) break;
    
        if ( ch == '*' && newline_flag ) num_of_labels++;
        if ( ch == 0x0d ){
            cr_flag = true;
            continue;
        }
        if ( ch == 0x0a ){
            *(*buf)++ = 0x0a;
            newline_flag = true;
            cr_flag = false;
        }
        else{
            *(*buf)++ = ch;
            if ( ch != ' ' && ch != '\t' )
                newline_flag = false;
        }
    }

    *(*buf)++ = 0x0a;
    return 0;
}

int ScriptHandler::readScript( char *path )
{
    archive_path = new char[ strlen(path) + 1 ];
    strcpy( archive_path, path );

    FILE *fp;
    char file_name[10];
    int  i, file_counter = 0;
    char *p_script_buffer;
    int estimated_buffer_length = 0;
    bool encrypt_flag = false;
    
    if ( (fp = fopen( "0.txt", "rb" )) != NULL ){
        do{
            fseek( fp, 0, SEEK_END );
            estimated_buffer_length += ftell( fp ) + 1;
            sprintf( file_name, "%d.txt", ++file_counter );
            fclose( fp );
        }
        while( (fp = fopen( file_name, "rb" )) != NULL );
    }
    else if ( (fp = fopen( "nscript.dat", "rb" )) != NULL ){
        encrypt_flag = true;
        fseek( fp, 0, SEEK_END );
        estimated_buffer_length = ftell( fp ) + 1;
        fclose( fp );
    }
    else{
        fprintf( stderr, "can't open file 0.txt or nscript.dat\n" );
        return -1;
    }
    
    if ( script_buffer ) delete[] script_buffer;
    if ( ( script_buffer = new char[ estimated_buffer_length ]) == NULL ){
        fprintf( stderr, " *** can't allocate memory for the script ***\n");
        exit( -1 );
    }
    current_script = p_script_buffer = script_buffer;
    
    if ( encrypt_flag ){
        fp = fopen( "nscript.dat", "rb" );
        readScriptSub( fp, &p_script_buffer, encrypt_flag );
        fclose( fp );
    }
    else{
        for ( i=0 ; i<file_counter ; i++ ){
            sprintf( file_name, "%d.txt", i );
            fp = fopen( file_name, "rb" );
            readScriptSub( fp, &p_script_buffer, encrypt_flag );
            fclose( fp );
        }
    }

    script_buffer_length = p_script_buffer - script_buffer;
    
    /* ---------------------------------------- */
    /* 800 x 600 check */
    if ( !strncmp( script_buffer, ";mode800", 8 ) )
        svga_flag = true;
        
    return labelScript();
}

int ScriptHandler::labelScript()
{
    int  label_counter = -1;
    char *buf = script_buffer;
    label_info = new LabelInfo[ num_of_labels ];

    while ( buf < script_buffer + script_buffer_length ){
        SKIP_SPACE( buf );
        if ( *buf == '*' )
        {
            setCurrent( buf );
            label_info[ ++label_counter ].name = new char[ strlen(string_buffer) ];
            for ( unsigned int i=0 ; i<strlen(string_buffer) ; i++ ){
                label_info[ label_counter ].name[i] = string_buffer[i+1];
                if ( 'a' <= label_info[ label_counter ].name[i] && label_info[ label_counter ].name[i] <= 'z' )
                    label_info[ label_counter ].name[i] += 'A' - 'a';
            }
            while( *buf != 0x0a ) buf++;
            buf++;
            label_info[ label_counter ].start_address = buf;
            label_info[ label_counter ].num_of_lines  = 0;
            label_info[ label_counter ].access_flag   = false;
        }
        else{
            if ( label_counter >= 0 )
                label_info[ label_counter ].num_of_lines++;
            while( *buf != 0x0a ) buf++;
            buf++;
        }
        text_flag = true;
    }
    return 0;
}

struct ScriptHandler::LabelInfo ScriptHandler::lookupLabel( const char *label )
{
    int i = findLabel( label );
    if ( i >= 0 ){
        if ( !label_info[i].access_flag ){
            num_of_labels_accessed++;
            label_info[i].access_flag = true;
        }
        return label_info[i];
    }

    errorAndExit( "Label is not found." );
    return label_info[0];
}

struct ScriptHandler::LabelInfo ScriptHandler::lookupLabelNext( const char *label )
{
    int i = findLabel( label );
    if ( i >= 0 && i+1 < num_of_labels ){
        if ( !label_info[i+1].access_flag ){
            num_of_labels_accessed++;
            label_info[i+1].access_flag = true;
        }
        return label_info[i+1];
    }

    errorAndExit( "Next Label is not found." );
    return label_info[0];
}

bool ScriptHandler::getLabelAccessFlag( const char *label )
{
    int i = findLabel( label );
    if ( i >= 0 ){
        return label_info[i].access_flag;
    }

    return false;
}

void ScriptHandler::saveLabelLog()
{
    FILE *fp;
    int  i;
    char buf[10];

    if ( ( fp = fopen( "NScrllog.dat", "wb" ) ) == NULL ){
        fprintf( stderr, "can't write NScrllog.dat\n" );
        exit( -1 );
    }
    
    sprintf( buf, "%d", num_of_labels_accessed );
    for ( i=0 ; i<(int)strlen( buf ) ; i++ ) fputc( buf[i], fp );
    fputc( 0x0a, fp );
    
    for ( i=0 ; i<num_of_labels ; i++ ){
        if ( label_info[i].access_flag ){
            fputc( '"', fp );
            for ( unsigned j=0 ; j<strlen( label_info[i].name ) ; j++ )
                fputc( label_info[i].name[j] ^ 0x84, fp );
            fputc( '"', fp );
        }
    }
    
    fclose( fp );
}

void ScriptHandler::loadLabelLog()
{
    FILE *fp;
    int i, j, ch, count = 0;
    char buf[100];
    
    if ( ( fp = fopen( "NScrllog.dat", "rb" ) ) != NULL ){
        while( (ch = fgetc( fp )) != 0x0a ){
            count = count * 10 + ch - '0';
        }

        for ( i=0 ; i<count ; i++ ){
            fgetc( fp );
            j = 0;
            while( (ch = fgetc( fp )) != '"' ) buf[j++] = ch ^ 0x84;
            buf[j] = '\0';
            lookupLabel( buf );
        }
        
        fclose( fp );
    }
}

void ScriptHandler::saveKidokuData()
{
    FILE *fp;

    if ( ( fp = fopen( "kidoku.dat", "wb" ) ) == NULL ){
        fprintf( stderr, "can't write kidoku.dat\n" );
        return;
    }

    fwrite( kidoku_buffer, 1, script_buffer_length/8 + 1, fp );
    fclose( fp );
}

void ScriptHandler::loadKidokuData()
{
    FILE *fp;

    kidoku_buffer = new char[ script_buffer_length/8 + 1 ];
    memset( kidoku_buffer, 0, script_buffer_length/8 + 1 );

    if ( ( fp = fopen( "kidoku.dat", "rb" ) ) != NULL ){
        fread( kidoku_buffer, 1, script_buffer_length/8 + 1, fp );
        fclose( fp );
    }
}

void ScriptHandler::addNumAlias( const char *str, int no )
{
    NameAlias *p_name_alias = new NameAlias( str, no );
    last_name_alias->next = p_name_alias;
    last_name_alias = last_name_alias->next;
}

void ScriptHandler::addStrAlias( const char *str1, const char *str2 )
{
    StringAlias *p_str_alias = new StringAlias( str1, str2 );
    last_str_alias->next = p_str_alias;
    last_str_alias = last_str_alias->next;
}

void ScriptHandler::errorAndExit( char *str )
{
    fprintf( stderr, " **** Script error, %s [%s] ***\n", str, string_buffer );
    exit(-1);
}

void ScriptHandler::addStringBuffer( char ch, int string_counter )
{
    /* In case of string bufer over flow */
    if ( string_counter == string_buffer_length ){
        string_buffer_length += 512;
        char *tmp_buffer = new char[ string_buffer_length ];
        memcpy( tmp_buffer, string_buffer, string_counter );

        delete[] string_buffer;
        string_buffer = tmp_buffer;

        delete[] str_string_buffer;
        str_string_buffer = new char[ string_buffer_length ];

        delete[] saved_string_buffer;
        saved_string_buffer = new char[ string_buffer_length ];
    }

    string_buffer[string_counter] = ch;
}

int ScriptHandler::findLabel( const char *label )
{
    int i;
    char *capital_label = new char[ strlen( label ) + 1 ];

    for ( i=0 ; i<(int)strlen( label )+1 ; i++ ){
        capital_label[i] = label[i];
        if ( 'a' <= capital_label[i] && capital_label[i] <= 'z' ) capital_label[i] += 'A' - 'a';
    }

    for ( i=0 ; i<num_of_labels ; i++ )
        if ( !strcmp( label_info[i].name, capital_label ) ){
            delete[] capital_label;
            return i;
        }

    delete[] capital_label;
    return -1;
}

int *ScriptHandler::decodeArray( char **buf, int offset )
{
    struct ArrayVariable array;
    int dim, i;

    SKIP_SPACE( *buf );
    int no = decodeArraySub( buf, &array );
    
    if ( array_variables[ no ].data == NULL ) errorAndExit( "decodeArray: data = NULL." );
    if ( array_variables[ no ].dim[0] <= array.dim[0] ) errorAndExit( "dim[0] <= array.dim[0]." );
    dim = array.dim[0];
    for ( i=1 ; i<array_variables[ no ].num_dim ; i++ ){
        if ( array_variables[ no ].dim[i] <= array.dim[i] ) errorAndExit( "dim[i] <= array.dim[i]." );
        dim = dim * array_variables[ no ].dim[i] + array.dim[i];
    }
    if ( array_variables[ no ].dim[i-1] <= array.dim[i-1] + offset ) errorAndExit( "dim[i-1] <= array.dim[i-1] + offset." );
    dim += offset;

    return &array_variables[ no ].data[ dim ];
}

