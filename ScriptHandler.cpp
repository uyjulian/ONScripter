/* -*- C++ -*-
 *
 *  ScriptHandler.cpp - Script manipulation class
 *
 *  Copyright (c) 2001-2004 Ogapee. All rights reserved.
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

static struct {
    char *first_file;
    int  encrypt_mode;
    char *format;
} script_file_mode[] = {
    {"0.txt", 0, "%d.txt"},
    {"00.txt", 0, "%2.2d.txt"},
    {"nscr_sec.dat", 2, ""},
    {"nscript.dat", 1, ""},
    {NULL}
};

ScriptHandler::ScriptHandler()
{
    int i;
    
    script_buffer = NULL;
    end_status = END_NONE;
    log_info[LABEL_LOG].current_log = &log_info[LABEL_LOG].root_log;
    log_info[LABEL_LOG].num_logs = 0;
    log_info[LABEL_LOG].filename = "NScrllog.dat";
    log_info[FILE_LOG].current_log = &log_info[FILE_LOG].root_log;
    log_info[FILE_LOG].num_logs = 0;
    log_info[FILE_LOG].filename = "NScrflog.dat";
    kidokuskip_flag = false;
    kidoku_buffer = NULL;
    skip_enabled = false;

    text_flag = true;
    line_head_flag = true;
    linepage_flag = false;

    string_buffer_length = 512;
    string_buffer       = new char[ string_buffer_length ];
    str_string_buffer   = new char[ string_buffer_length ];
    saved_string_buffer = new char[ string_buffer_length ];

    num_of_labels = 0;
    last_stack_info = &root_stack_info;

    /* ---------------------------------------- */
    for ( i=0 ; i<VARIABLE_RANGE ; i++ ){
        num_variables[i] = 0;
        num_limit_flag[i] = false;
        str_variables[i] = NULL;
    }

    screen_size = SCREEN_SIZE_640x480;
    global_variable_border = 200;

    last_name_alias = &root_name_alias;
    last_name_alias->next = NULL;
    last_str_alias = &root_str_alias;
    last_str_alias->next = NULL;
}

ScriptHandler::~ScriptHandler()
{
    if ( script_buffer )       delete[] script_buffer;
    if ( string_buffer )       delete[] string_buffer;
    if ( str_string_buffer )   delete[] string_buffer;
    if ( saved_string_buffer ) delete[] saved_string_buffer;
    deleteLog( LABEL_LOG );
    deleteLog( FILE_LOG );
}

FILE *ScriptHandler::fopen( const char *path, const char *mode )
{
    char * file_name = new char[strlen(archive_path)+strlen(path)+1];
    sprintf( file_name, "%s%s", archive_path, path );

    FILE *fp = ::fopen( file_name, mode );
    delete[] file_name;

    return fp;
}

char *ScriptHandler::saveStringBuffer()
{
    strcpy( saved_string_buffer, string_buffer );
    return saved_string_buffer;
}

int ScriptHandler::getOffset( char *pos )
{
    return pos - script_buffer;
}

char *ScriptHandler::getAddress( int offset )
{
    return script_buffer + offset;
}

char *ScriptHandler::getCurrent()
{
    return current_script;
}

void ScriptHandler::setCurrent( char *pos, bool reread_flag )
{
    text_flag = true;
    line_head_flag = true;
    
    current_script = pos;
    if ( reread_flag ) 
        rereadToken();
    else
        next_script = pos;
}

int ScriptHandler::getLineByAddress( char *address )
{
    LabelInfo label = getLabelByAddress( address );

    char *addr = label.start_address;
    int line = 0;
    while ( address > addr ){
        if ( *addr == 0x0a ) line++;
        addr++;
    }
    return line;
}

char *ScriptHandler::getAddressByLine( int line )
{
    LabelInfo label = getLabelByLine( line );
    
    int l = line - label.start_line;
    char *addr = label.start_address;
    while ( l > 0 ){
        while( *addr != 0x0a ) addr++;
        addr++;
        l--;
    }
    return addr;
}

ScriptHandler::LabelInfo ScriptHandler::getLabelByAddress( char *address )
{
    int i;
    for ( i=0 ; i<num_of_labels-1 ; i++ ){
        if ( label_info[i+1].start_address > address )
            return label_info[i];
    }
    return label_info[i];
}

ScriptHandler::LabelInfo ScriptHandler::getLabelByLine( int line )
{
    int i;
    for ( i=0 ; i<num_of_labels-1 ; i++ ){
        if ( label_info[i+1].start_line > line )
            return label_info[i];
    }
    return label_info[i];
}

char *ScriptHandler::getNext()
{
    return next_script;
}

void ScriptHandler::pushCurrent( char *pos )
{
    StackInfo *info = new StackInfo( current_script );

    last_stack_info->next = info;
    last_stack_info = last_stack_info->next;

    current_script = pos;
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
    return text_flag;
}

void ScriptHandler::setText( bool val )
{
    line_head_flag = val;
    text_flag = val;
}

int ScriptHandler::getEndStatus()
{
    return end_status;
}

void ScriptHandler::skipLine( int no )
{
    for ( int i=0 ; i<no ; i++ ){
        while ( *current_script != 0x0a ) current_script++;
        current_script++;
    }
    line_head_flag = true;
    rereadToken();
}

void ScriptHandler::setLinepage( bool val )
{
    linepage_flag = val;
}

bool ScriptHandler::isKidoku()
{
    return skip_enabled;
}

void ScriptHandler::markAsKidoku( char *address )
{
    if ( !kidokuskip_flag ) return;
    int offset = current_script - script_buffer;
    if ( address ) offset = address - script_buffer;
    //printf("mark (%c)%x:%x = %d\n", *current_script, offset /8, offset%8, kidoku_buffer[ offset/8 ] & ((char)1 << (offset % 8)));
    if ( kidoku_buffer[ offset/8 ] & ((char)1 << (offset % 8)) )
        skip_enabled = true;
    else
        skip_enabled = false;
    kidoku_buffer[ offset/8 ] |= ((char)1 << (offset % 8));
}

int ScriptHandler::rereadToken()
{
    next_script = current_script;
    return readToken();
}

int ScriptHandler::readToken()
{
    current_script = next_script;
    char *buf = current_script;
    unsigned int i;
    end_status = END_NONE;
    current_variable.type = VAR_NONE;

    int string_counter=0, no;
    bool op_flag = true;
    bool comment_flag = false;
    char num_buf[10], num_sjis_buf[3];

    char ch = *buf++;
    //printf("read %x %x %c(%x)\n", buf-1, script_buffer, ch, ch );
    while ( ch == ' ' || ch == '\t' || ch == ':' ){
        if ( ch == ':' ){
            line_head_flag = true;
            text_flag = false;
            markAsKidoku( buf-1 );
            
            addStringBuffer( ch, string_counter++ );
            addStringBuffer( '\0', string_counter++ );
            next_script = buf;
            
            return end_status;
        }
        if ( line_head_flag &&
             last_stack_info == &root_stack_info ){
            markAsKidoku( buf-1 );
        }

        ch = *buf++;
    }

    if ( ch == 0x0a || ch == '\0' )
    {
        if ( last_stack_info == &root_stack_info )
            markAsKidoku( buf-1 );
        if (line_head_flag)
            text_flag = false;
        line_head_flag = true;

        addStringBuffer( ch, string_counter++ );
        addStringBuffer( '\0', string_counter++ );
        next_script = buf;

        //printf("readToken 0x0a is text %d script %p\n", text_flag, current_script  );
        return end_status;
    }

    if ( line_head_flag ){
        text_flag = true;
        if ( last_stack_info == &root_stack_info )
            markAsKidoku( buf-1 );
    }

    if ( ch == '*' ){ // FIXME: Is this a correct fix ?
        // to interpret "goto* label" or "goto * label"
        SKIP_SPACE(buf);
    }

    if ( ch == ';' ){
        if ( last_stack_info == &root_stack_info )
            markAsKidoku( buf-1 );
        addStringBuffer( ch, string_counter++ );
        do{
            ch = *buf++;
        } while ( ch != 0x0a && ch != '\0' );
        comment_flag = true;
    }
    else if ( ch == '"' )
    {
        current_variable.type = VAR_STR_CONST;
        ch = *buf++;
        while ( ch != '"' && ch != 0x0a ){
            addStringBuffer( ch, string_counter++ );
            ch = *buf++;
        }
        buf++;
        end_status |= END_QUAT;
    }
    else while ( ch != 0x0a && ch != '\0' && ch != '"' )
    {
        if ( op_flag && !text_flag ){
            while ( ch == '<' || ch == '>' ||
                    ch == '=' || ch == '!' ||
                    ch == '&' )
            {
                addStringBuffer( ch, string_counter++ );
                ch = *buf++;
            }
            if ( string_counter > 0 &&
                 (string_buffer[0] != '!' || 
                  string_counter != 1 ) ){ // avoid palette
                break;
            }
            else{
                op_flag = false;
            }
        }
        else{
            op_flag = false;
        }

        if ( (ch >= 'a' && ch <= 'z') || 
             (ch >= 'A' && ch <= 'Z') ||
             ch == '~' )
        {
            if ( string_counter == 0 ) text_flag = false;
            if ( !text_flag && ch >= 'A' && ch <= 'Z' ) ch += 'a' - 'A';
            addStringBuffer( ch, string_counter++ );
        }
        else if ( ch == ',' && string_counter == 0 ) // for select
        {
            addStringBuffer( ch, string_counter++ );
            buf++;
            text_flag = false;
            break;
        }
        else if ( (ch == '*' ||
                   ch == '#' || // color
                   ch == '!') && // for !w2000 line 
                  string_counter == 0 )
        {
            text_flag = false;
            addStringBuffer( ch, string_counter++ );
        }
        else if ( ch == '$' ){
            if ( !text_flag && string_counter != 0 ) break;

            no = parseInt(&buf);
            if ( str_variables[no] ){
                for ( i=0 ; i<strlen( str_variables[no] ) ; i++ ){
                    addStringBuffer( str_variables[no][i], string_counter++ );
                }
            }
            if ( text_flag ){
                current_variable.type = VAR_NONE;
            }
            else{
                current_variable.type = VAR_STR;
                current_variable.var_no = no;
                buf++;
                break;
            }
        }
        else if ( ch == '%' || ch == '?'){
            if ( !text_flag && string_counter != 0 ) break;

            buf--;
            
            if ( text_flag ){
                no = parseInt(&buf);
                if ( no<0 ){
                    addStringBuffer( "|"[0], string_counter++ );
                    addStringBuffer( "|"[1], string_counter++ );
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
                current_variable.type = VAR_NONE;
            }
            else{
                parseIntExpression(&buf);
                buf++;
                break;
            }
        }
        else if ( ch == '@' || ch == '\\' )
        {
            if ( text_flag || string_counter == 0 ){
                if ( string_counter == 0 ) text_flag = false;
                addStringBuffer( ch, string_counter++ );
                buf++;
            }
            break;
        }
        else if ( ch & 0x80 ){
            // if text appears immediately after a command, break.
            if ( string_counter != 0 && !text_flag ) break;
            
            addStringBuffer( ch, string_counter++ );
            ch = *buf++;
            addStringBuffer( ch, string_counter++ );
            text_flag = true;
        }
        else{ // separaters
            if ( !text_flag &&
                 ( ch == ',' ||
                   ch == ' ' ||
                   ch == '\t' ||
                   // conditions in if
                   ch == '<' ||
                   ch == '>' ||
                   ch == '=' ||
                   ch == '!' ||
                   ch == '&' ||
                   ch == ':' ||
                   ch == ';' ||
                   ch == ')' || 
                   ch == '*' ) ) break;
            if ( !text_flag && string_counter>0 &&
                 ( ch == '-' ||
                   ch == '/' ||
                   ch == '*' ) ) break;
            if ( ch != ' ' && ch != '\t' )
                addStringBuffer( ch, string_counter++ );
        }
            
        ch = *buf++;
    }

    if ( !comment_flag ) line_head_flag = false;

    buf--;
    SKIP_SPACE( buf );
    if ( *buf == ',' )
    {
        end_status |= END_COMMA;
        buf++;
        SKIP_SPACE( buf );
    }
#if 0
    else if ( *buf == ':' ){
        text_flag = true;
        end_status |= END_COLON;
    }
#endif
    if ( linepage_flag && text_flag )
        addStringBuffer( '\\', string_counter++ );
    
    addStringBuffer( '\0', string_counter++ );
    next_script = buf;

    //printf("readToken [%s](%x) is %s script %x, %d\n", string_buffer, *string_buffer, text_flag?"text":"command", current_script, end_status );
    return end_status;
}

void ScriptHandler::skipToken()
{
    current_script = next_script;
    char *buf = current_script;

    bool quat_flag = false;
    bool text_flag = false;
    while(1){
        if ( *buf == 0x0a || 
             (!quat_flag && !text_flag && (*buf == ':' || *buf == ';') ) ) break;
        if ( *buf == '"' ) quat_flag = !quat_flag;
        if ( *buf & 0x80 ){
            buf += 2;
            if ( !quat_flag ) text_flag = true;
        }
        else
            buf++;
    }
    next_script = buf;
}

int ScriptHandler::getIntVariable( VariableInfo *var_info )
{
    if ( var_info == NULL ) var_info = &current_variable;
    
    if ( var_info->type == VAR_INT )
        return num_variables[ var_info->var_no];
    else if ( var_info->type == VAR_PTR )
        return *getArrayPtr( var_info->var_no, var_info->array, 0 );
    return 0;
}

char *ScriptHandler::getStringBuffer()
{
    return string_buffer;
}

void ScriptHandler::parseStr( char **buf )
{
    SKIP_SPACE( *buf );

    if ( **buf == '(' ){
        (*buf)++;
        parseStr(buf);
        SKIP_SPACE( *buf );
        if ( (*buf)[0] != ')' ) errorAndExit("parseStr: ) is not found.");
        (*buf)++;

        if ( findAndAddLog( FILE_LOG, str_string_buffer, false ) ){
            parseStr(buf);
            char *tmp_buf = new char[ strlen( str_string_buffer ) + 1 ];
            strcpy( tmp_buf, str_string_buffer );
            parseStr(buf);
            strcpy( str_string_buffer, tmp_buf );
            delete[] tmp_buf;
        }
        else{
            parseStr(buf);
            parseStr(buf);
        }
        current_variable.type = VAR_STR_CONST;
    }
    else if ( **buf == '$' ){
        (*buf)++;
        int no = parseInt(buf);
        if ( str_variables[no] )
            strcpy( str_string_buffer, str_variables[no] );
        else
            str_string_buffer[0] = '\0';
        current_variable.type = VAR_STR;
        current_variable.var_no = no;
    }
    else if ( **buf == '"' ){
        int c=0;
        (*buf)++;
        while ( **buf != '"' && **buf != 0x0a )
            str_string_buffer[c++] = *(*buf)++;
        str_string_buffer[c] = '\0';
        if ( **buf == '"' ) (*buf)++;
        current_variable.type = VAR_STR_CONST;
        end_status |= END_QUAT;
    }
    else if ( **buf == '#' ){ // for color
        for ( int i=0 ; i<7 ; i++ )
            str_string_buffer[i] = *(*buf)++;
        str_string_buffer[7] = '\0';
        current_variable.type = VAR_NONE;
    }
    else{ // str alias
        char ch, alias_buf[512];
        int alias_buf_len = 0;
        bool first_flag = true;
        
        while(1){
            if ( alias_buf_len == 511 ) break;
            ch = **buf;
            
            if ( (ch >= 'a' && ch <= 'z') || 
                 (ch >= 'A' && ch <= 'Z') || 
                 ch == '_' ){
                if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
                first_flag = false;
                alias_buf[ alias_buf_len++ ] = ch;
            }
            else if ( ch >= '0' && ch <= '9' ){
                if ( first_flag ) errorAndExit("parseStr: number is not allowed for the first letter of str alias.");
                first_flag = false;
                alias_buf[ alias_buf_len++ ] = ch;
            }
            else break;
            (*buf)++;
        }
        alias_buf[alias_buf_len] = '\0';
        
        if ( alias_buf_len == 0 ){
            str_string_buffer[0] = '\0';
            current_variable.type = VAR_NONE;
            return;
        }
        
        StringAlias *p_str_alias = root_str_alias.next;

        while( p_str_alias ){
            if ( !strcmp( p_str_alias->alias, (const char*)alias_buf ) ){
                strcpy( str_string_buffer, p_str_alias->str );
                break;
            }
            p_str_alias = p_str_alias->next;
        }
        if ( !p_str_alias ){
            printf("can't find str alias for %s...\n", alias_buf );
            exit(-1);
        }
        current_variable.type = VAR_STR_CONST;
    }
}

const char *ScriptHandler::readStr( bool reread_flag )
{
    end_status = END_NONE;
    if ( reread_flag ) next_script = current_script;
    current_script = next_script;
    char *buf = current_script;
    
    parseStr(&buf);

    SKIP_SPACE( buf );
    if ( *buf == ',' )
    {
        end_status |= END_COMMA;
        buf++;
        SKIP_SPACE( buf );
    }
#if 0    
    else if ( *buf == ':' ){
        end_status |= END_COLON;
    }
#endif    
    next_script = buf;
    
    strcpy( string_buffer, str_string_buffer );
    return string_buffer;
}

int ScriptHandler::readInt( bool reread_flag )
{
    end_status = END_NONE;
    if ( reread_flag ) next_script = current_script;
    current_script = next_script;
    char *buf = current_script;
    
    int ret = parseIntExpression(&buf);
    
    SKIP_SPACE( buf );
    if ( *buf == ',' )
    {
        end_status |= END_COMMA;
        buf++;
        SKIP_SPACE( buf );
    }
#if 0    
    else if ( *buf == ':' ){
        end_status |= END_COLON;
    }
#endif    
    next_script = buf;

    return ret;
}

int ScriptHandler::parseInt( char **buf )
{
    int ret = 0;
    
    SKIP_SPACE( *buf );

    if ( **buf == '%' ){
        (*buf)++;
        current_variable.var_no = parseInt(buf);
        current_variable.type = VAR_INT;
        return num_variables[ current_variable.var_no ];
    }
    else if ( **buf == '?' ){
        current_variable.var_no = decodeArray( buf, current_variable.array );
        current_variable.type = VAR_PTR;
        return *getArrayPtr( current_variable.var_no, current_variable.array, 0 );
    }
    else{
        char ch, alias_buf[256];
        int alias_buf_len = 0, alias_no = 0;
        bool direct_num_flag = false;
        bool num_alias_flag = false;
        bool minus_flag = false;

        char *buf_start = *buf;
        while( 1 ){
            ch = **buf;
            
            if ( (ch >= 'a' && ch <= 'z') || 
                 (ch >= 'A' && ch <= 'Z') || 
                 ch == '_' ){
                if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
                if ( direct_num_flag ) break;
                num_alias_flag = true;
                alias_buf[ alias_buf_len++ ] = ch;
            }
            else if ( ch >= '0' && ch <= '9' ){
                if ( !num_alias_flag ) direct_num_flag = true;
                if ( direct_num_flag ) 
                    alias_no = alias_no * 10 + ch - '0';
                else
                    alias_buf[ alias_buf_len++ ] = ch;
            }
            else if ( ch == '-' ){
                if ( direct_num_flag || num_alias_flag ) break;
                minus_flag = true;
                direct_num_flag = true;
            }
            else break;
            (*buf)++;
        }
        if ( minus_flag ) alias_no = -alias_no;

        if ( *buf - buf_start  == 0 ){
            current_variable.type = VAR_NONE;
            return 0;
        }
        
        /* ---------------------------------------- */
        /* Solve num aliases */
        if ( num_alias_flag ){
            alias_buf[ alias_buf_len ] = '\0';
            NameAlias *p_name_alias = root_name_alias.next;

            while( p_name_alias ){
                if ( !strcmp( p_name_alias->alias,
                              (const char*)alias_buf ) ){
                    alias_no = p_name_alias->num;
                    break;
                }
                p_name_alias = p_name_alias->next;
            }
            if ( !p_name_alias ){
                //printf("can't find name alias for %s... assume 0.\n", alias_buf );
                current_variable.type = VAR_NONE;
                *buf = buf_start;
                return 0;
            }
        }
        current_variable.type = VAR_INT_CONST;
        ret = alias_no;
    }

    SKIP_SPACE( *buf );

    return ret;
}

/*
 * Internal buffer looks like this.
 *   num[0] op[0] num[1] op[1] num[2]
 * If priority of op[0] is higher than op[1], (num[0] op[0] num[1]) is computed,
 * otherwise (num[1] op[1] num[2]) is computed.
 * Then, the next op and num is read from the script.
 * Num is an immediate value, a variable or a bracketed expression.
 */
void ScriptHandler::readNextOp( char **buf, int *op, int *num )
{
    SKIP_SPACE(*buf);
    char *buf_start = *buf;
    
    if ( op ){
        if      ( (*buf)[0] == '+' ) *op = OP_PLUS;
        else if ( (*buf)[0] == '-' ) *op = OP_MINUS;
        else if ( (*buf)[0] == '*' ) *op = OP_MULT;
        else if ( (*buf)[0] == '/' ) *op = OP_DIV;
        else if ( (*buf)[0] == 'm' &&
                  (*buf)[1] == 'o' &&
                  (*buf)[2] == 'd' &&
                  ( (*buf)[3] == ' '  ||
                    (*buf)[3] == '\t' ||
                    (*buf)[3] == '$' ||
                    (*buf)[3] == '%' ||
                    (*buf)[3] == '?' ||
                    ( (*buf)[3] >= '0' && (*buf)[3] <= '9') ))
            *op = OP_MOD;
        else{
            *op = OP_INVALID;
            return;
        }
        if ( *op == OP_MOD ) *buf += 3;
        else                 (*buf)++;
        SKIP_SPACE(*buf);
    }

    if ( (*buf)[0] == '(' ){
        (*buf)++;
        *num = parseIntExpression( buf );
        SKIP_SPACE(*buf);
        if ( (*buf)[0] != ')' ) errorAndExit(") is not found.\n");
        (*buf)++;
    }
    else{
        *num = parseInt( buf );
        if ( current_variable.type == VAR_NONE ){
            if (op) *op = OP_INVALID;
            *buf = buf_start;
        }
    }
}

int ScriptHandler::calcArithmetic( int num1, int op, int num2 )
{
    int ret;
    
    if      ( op == OP_PLUS )  ret = num1+num2;
    else if ( op == OP_MINUS ) ret = num1-num2;
    else if ( op == OP_MULT )  ret = num1*num2;
    else if ( op == OP_DIV )   ret = num1/num2;
    else if ( op == OP_MOD )   ret = num1%num2;

    current_variable.type = VAR_INT_CONST;

    return ret;
}

int ScriptHandler::parseIntExpression( char **buf )
{
    int ret;
    int num[3], op[2]; // internal buffer

    SKIP_SPACE( *buf );

    readNextOp( buf, NULL, &num[0] );

    readNextOp( buf, &op[0], &num[1] );
    if ( op[0] == OP_INVALID )
        return num[0];

    while(1){
        readNextOp( buf, &op[1], &num[2] );
        if ( op[1] == OP_INVALID ) break;

        if ( !(op[0] & 0x04) && (op[1] & 0x04) ){ // if priorith of op[1] is higher than op[0]
            num[1] = calcArithmetic( num[1], op[1], num[2] );
        }
        else{
            num[0] = calcArithmetic( num[0], op[0], num[1] );
            op[0] = op[1];
            num[1] = num[2];
        }
    }
    ret = calcArithmetic( num[0], op[0], num[1] );

    return ret;
}

void ScriptHandler::setInt( VariableInfo *var_info, int val, int offset )
{
    if ( var_info->type == VAR_INT ){
        setNumVariable( var_info->var_no + offset, val );
    }
    else if ( var_info->type == VAR_PTR ){
        *getArrayPtr( var_info->var_no, var_info->array, offset ) = val;
    }
    else{
        errorAndExit( "setInt: no variables." );
    }
}

void ScriptHandler::pushVariable()
{
    pushed_variable = current_variable;
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

void ScriptHandler::getSJISFromInteger( char *buffer, int no, bool add_space_flag )
{
    int c = 0;
    char num_str[] = "‚O‚P‚Q‚R‚S‚T‚U‚V‚W‚X";
    if ( no >= 10 ){
        buffer[c++] = num_str[ no / 10 % 10 * 2];
        buffer[c++] = num_str[ no / 10 % 10 * 2 + 1];
    }
    else if ( add_space_flag ){
        buffer[c++] = ((char*)"@")[0];
        buffer[c++] = ((char*)"@")[1];
    }
    buffer[c++] = num_str[ no % 10 * 2];
    buffer[c++] = num_str[ no % 10 * 2 + 1];
    buffer[c++] = '\0';
}

int ScriptHandler::readScriptSub( FILE *fp, char **buf, int encrypt_mode )
{
    char magic[5] = {0x79, 0x57, 0x0d, 0x80, 0x04 };
    int  magic_counter = 0;
    bool newline_flag = true;
    bool cr_flag = false;

    while(1)
    {
        int ch = fgetc( fp );
        if ( ch != EOF ){
            if      ( encrypt_mode == 1 ) ch ^= 0x84;
            else if ( encrypt_mode == 2 ){
                ch = (ch ^ magic[magic_counter++]) & 0xff;
                if ( magic_counter == 5 ) magic_counter = 0;
            }
        }

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
    int  file_counter = 0;
    int estimated_buffer_length = 0;

    int no=0;
    while( script_file_mode[no].first_file ){
        if ( (fp = fopen( script_file_mode[no].first_file, "rb" )) != NULL ){
            while(1){
                fseek( fp, 0, SEEK_END );
                estimated_buffer_length += ftell( fp ) + 1;
                if (script_file_mode[no].encrypt_mode == 0){
                    fclose( fp );
                    sprintf( file_name, script_file_mode[no].format, ++file_counter );
                    if ((fp = fopen( file_name, "rb" )) != NULL) continue;
                }
                break;
            }
            break;
        }
        no++;
    }
    if (script_file_mode[no].first_file == NULL){
        fprintf( stderr, "can't open file 0.txt or 00.txt or nscript.dat\n" );
        return -1;
    }
    
    if ( script_buffer ) delete[] script_buffer;
    if ( ( script_buffer = new char[ estimated_buffer_length ]) == NULL ){
        fprintf( stderr, " *** can't allocate memory for the script ***\n");
        exit( -1 );
    }

    char *p_script_buffer;
    current_script = p_script_buffer = script_buffer;
    
    if (script_file_mode[no].encrypt_mode > 0){
        fseek( fp, 0, SEEK_SET );
        readScriptSub( fp, &p_script_buffer, script_file_mode[no].encrypt_mode );
        fclose( fp );
    }
    else{
        for ( int i=0 ; i<file_counter ; i++ ){
            sprintf( file_name, script_file_mode[no].format, i );
            fp = fopen( file_name, "rb" );
            readScriptSub( fp, &p_script_buffer, script_file_mode[no].encrypt_mode );
            fclose( fp );
        }
    }

    script_buffer_length = p_script_buffer - script_buffer;
    
    /* ---------------------------------------- */
    /* screen size and value check */
    char *buf = script_buffer+1;
    while( script_buffer[0] == ';' ){
        if ( !strncmp( buf, "mode", 4 ) ){
            buf += 4;
            if      ( !strncmp( buf, "800", 3 ) )
                screen_size = SCREEN_SIZE_800x600;
            else if ( !strncmp( buf, "400", 3 ) )
                screen_size = SCREEN_SIZE_400x300;
            else if ( !strncmp( buf, "320", 3 ) )
                screen_size = SCREEN_SIZE_320x240;
            else
                screen_size = SCREEN_SIZE_640x480;
            buf += 3;
        }
        else if ( !strncmp( buf, "value", 5 ) ){
            buf += 5;
            global_variable_border = 0;
            while ( *buf >= '0' && *buf <= '9' )
                global_variable_border = global_variable_border * 10 + *buf++ - '0';
        }
        else{
            break;
        }
        if ( *buf != ',' ) break;
        buf++;
    }

    return labelScript();
}

int ScriptHandler::labelScript()
{
    int label_counter = -1;
    int current_line = 0;
    char *buf = script_buffer;
    label_info = new LabelInfo[ num_of_labels ];

    while ( buf < script_buffer + script_buffer_length ){
        SKIP_SPACE( buf );
        if ( *buf == '*' )
        {
            setCurrent( buf );
            label_info[ ++label_counter ].name = new char[ strlen(string_buffer) ];
            label_info[ label_counter ].label_header = buf;
            for ( unsigned int i=0 ; i<strlen(string_buffer) ; i++ ){
                label_info[ label_counter ].name[i] = string_buffer[i+1];
                if ( 'a' <= label_info[ label_counter ].name[i] && label_info[ label_counter ].name[i] <= 'z' )
                    label_info[ label_counter ].name[i] += 'A' - 'a';
            }
            while( *buf != 0x0a && *buf != ';' && *buf != ':' ) buf++;
            label_info[ label_counter ].num_of_lines = 0; // ';' or ':'
            if ( *buf == 0x0a ){
                buf++;
                current_line++;
            }
            label_info[ label_counter ].start_address = buf;
            label_info[ label_counter ].start_line    = current_line;
        }
        else{
            if ( label_counter >= 0 )
                label_info[ label_counter ].num_of_lines++;
            while( *buf != 0x0a ) buf++;
            buf++;
            current_line++;
        }
        text_flag = true;
    }
    return 0;
}

struct ScriptHandler::LabelInfo ScriptHandler::lookupLabel( const char *label )
{
    int i = findLabel( label );
    if ( i >= 0 ){
        findAndAddLog( LABEL_LOG, label_info[i].name, true );
        return label_info[i];
    }

    char *p = new char[ strlen(label) + 32 ];
    sprintf(p, "Label \"%s\" is not found.", label);
    errorAndExit( p );
    delete[] p; // dummy
    return label_info[0];
}

struct ScriptHandler::LabelInfo ScriptHandler::lookupLabelNext( const char *label )
{
    int i = findLabel( label );
    if ( i >= 0 && i+1 < num_of_labels ){
        findAndAddLog( LABEL_LOG, label_info[i+1].name, true );
        return label_info[i+1];
    }

    errorAndExit( "Next Label is not found." );
    return label_info[0];
}

ScriptHandler::LogLink *ScriptHandler::findAndAddLog( LOG_TYPE type, const char *name, bool add_flag )
{
    char capital_name[256];
    for ( unsigned int i=0 ; i<strlen(name)+1 ; i++ ){
        capital_name[i] = name[i];
        if ( 'a' <= capital_name[i] && capital_name[i] <= 'z' ) capital_name[i] += 'A' - 'a';
        else if ( capital_name[i] == '/' ) capital_name[i] = '\\';
    }
    
    LogInfo *info = NULL;
    if ( type == LABEL_LOG )
        info = &log_info[LABEL_LOG];
    else
        info = &log_info[FILE_LOG];

    LogLink *cur = info->root_log.next;
    while( cur ){
        if ( !strcmp( cur->name, capital_name ) ) break;
        cur = cur->next;
    }
    if ( !add_flag || cur ) return cur;

    LogLink *link = new LogLink();
    link->name = new char[strlen(capital_name)+1];
    strcpy( link->name, capital_name );
    info->current_log->next = link;
    info->current_log = info->current_log->next;
    info->num_logs++;
    
    return link;
}

void ScriptHandler::deleteLog( LOG_TYPE type )
{
    LogInfo *info = NULL;
    if ( type == LABEL_LOG )
        info = &log_info[LABEL_LOG];
    else
        info = &log_info[FILE_LOG];
    info->current_log = &info->root_log;

    LogLink *link = info->root_log.next;

    while( link ){
        LogLink *tmp = link;
        link = link->next;
        delete tmp;
    }
    info->num_logs = 0;
    info->root_log.next = NULL;
}

void ScriptHandler::loadLog( LOG_TYPE type )
{
    deleteLog( type );
    
    int i, j, ch, count = 0;
    char buf[100];

    LogInfo *info = NULL;
    if ( type == LABEL_LOG )
        info = &log_info[LABEL_LOG];
    else
        info = &log_info[FILE_LOG];

    FILE *fp; 
    if ( ( fp = fopen( info->filename, "rb" ) ) != NULL ){
        while( (ch = fgetc( fp )) != 0x0a ){
            count = count * 10 + ch - '0';
        }

        for ( i=0 ; i<count ; i++ ){
            fgetc( fp );
            j = 0; 
            while( (ch = fgetc( fp )) != '"' ) buf[j++] = ch ^ 0x84;
            buf[j] = '\0';

            findAndAddLog( type, buf, true );
        }

        fclose( fp );
    }
}

void ScriptHandler::saveLog( LOG_TYPE type )
{
    int  i,j;
    char buf[10];

    LogInfo *info = NULL;
    if ( type == LABEL_LOG )
        info = &log_info[LABEL_LOG];
    else
        info = &log_info[FILE_LOG];

    FILE *fp;
    if ( ( fp = fopen( info->filename, "wb" ) ) == NULL ){
        fprintf( stderr, "can't write %s\n", info->filename );
        exit( -1 );
    }

    sprintf( buf, "%d", info->num_logs );
    for ( i=0 ; i<(int)strlen( buf ) ; i++ ) fputc( buf[i], fp );
    fputc( 0x0a, fp );

    LogLink *cur = info->root_log.next;
    for ( i=0 ; i<info->num_logs ; i++ ){
        fputc( '"', fp );
        for ( j=0 ; j<(int)strlen( cur->name ) ; j++ )
            fputc( cur->name[j] ^ 0x84, fp );
        fputc( '"', fp );
        cur = cur->next;
    }
    
    fclose( fp );
}

void ScriptHandler::setKidokuskip( bool kidokuskip_flag )
{
    this->kidokuskip_flag = kidokuskip_flag;
}

void ScriptHandler::saveKidokuData()
{
    FILE *fp;

    if ( ( fp = fopen( "kidoku.dat", "wb" ) ) == NULL ){
        fprintf( stderr, "can't write kidoku.dat\n" );
        return;
    }

    fwrite( kidoku_buffer, 1, script_buffer_length/8, fp );
    fclose( fp );
}

void ScriptHandler::loadKidokuData()
{
    FILE *fp;

    setKidokuskip( true );
    kidoku_buffer = new char[ script_buffer_length/8 + 1 ];
    memset( kidoku_buffer, 0, script_buffer_length/8 + 1 );

    if ( ( fp = fopen( "kidoku.dat", "rb" ) ) != NULL ){
        fread( kidoku_buffer, 1, script_buffer_length/8, fp );
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
    char capital_label[256];

    for ( i=0 ; i<(int)strlen( label )+1 ; i++ ){
        capital_label[i] = label[i];
        if ( 'a' <= capital_label[i] && capital_label[i] <= 'z' ) capital_label[i] += 'A' - 'a';
    }
    for ( i=0 ; i<num_of_labels ; i++ ){
        if ( !strcmp( label_info[i].name, capital_label ) )
            return i;
    }

    return -1;
}

int ScriptHandler::decodeArray( char **buf, struct ArrayVariable &array )
{
    SKIP_SPACE( *buf );
    
    (*buf)++; // skip '?'
    int no = parseInt( buf );

    SKIP_SPACE( *buf );
    array.num_dim = 0;
    while ( **buf == '[' ){
        (*buf)++;
        array.dim[array.num_dim] = parseIntExpression(buf);
        array.num_dim++;
        SKIP_SPACE( *buf );
        if ( **buf != ']' ) errorAndExit( "decodeArray: no ']' is found." );
        (*buf)++;
    }
    for ( int i=array.num_dim ; i<20 ; i++ ) array.dim[i] = 0;

    return no;
}

int *ScriptHandler::getArrayPtr( int no, ArrayVariable &array, int offset )
{
    int dim = 0, i;
    for ( i=0 ; i<array_variables[no].num_dim ; i++ ){
        if ( array_variables[no].dim[i] <= array.dim[i] ) errorAndExit( "dim[i] <= array.dim[i]." );
        dim = dim * array_variables[no].dim[i] + array.dim[i];
    }
    if ( array_variables[no].dim[i-1] <= array.dim[i-1] + offset ) errorAndExit( "dim[i-1] <= array.dim[i-1] + offset." );

    return &array_variables[no].data[dim+offset];
}

void ScriptHandler::declareDim()
{
    current_script = next_script;
    char *buf = current_script;

    ScriptHandler::ArrayVariable array;
    int no = decodeArray( &buf, array );

    int dim = 1;
    array_variables[no].num_dim = array.num_dim;
    for ( int i=0 ; i<array.num_dim ; i++ ){
        array_variables[no].dim[i] = array.dim[i]+1;
        dim *= (array.dim[i]+1);
    }
    array_variables[no].data = new int[dim];
    memset( array_variables[no].data, 0, sizeof(int) * dim );

    next_script = buf;
}
