/* -*- C++ -*-
 *
 *  ScriptParser.cpp - Define block analyzer of ONScripter
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

#include "ScriptParser.h"

#define READ_LENGTH 4096
#define VERSION_STR1 "ONScripter"
#define VERSION_STR2 "Copyright (C) 2001-2002 Studio O.G.A. All Rights Reserved."

#define DEFAULT_SAVE_MENU_NAME "���Z�[�u��"
#define DEFAULT_LOAD_MENU_NAME "�����[�h��"
#define DEFAULT_SAVE_ITEM_NAME "������"

#define DEFAULT_LOOKBACK_NAME0 "uoncur.bmp"
#define DEFAULT_LOOKBACK_NAME1 "uoffcur.bmp"
#define DEFAULT_LOOKBACK_NAME2 "doncur.bmp"
#define DEFAULT_LOOKBACK_NAME3 "doffcur.bmp"

#define DEFAULT_TEXT_SPEED_LOW    40
#define DEFAULT_TEXT_SPEED_MIDDLE 20
#define DEFAULT_TEXT_SPEED_HIGHT  10

#define DEFAULT_VOLUME 100

typedef int (ScriptParser::*FuncList)();
static struct FuncLUT{
    char command[40];
    FuncList method;
} func_lut[] = {
    {"windoweffect",      &ScriptParser::effectCommand},
    {"windowback",      &ScriptParser::windowbackCommand},
    {"versionstr",      &ScriptParser::versionstrCommand},
    {"usewheel", &ScriptParser::usewheelCommand},
    {"underline", &ScriptParser::underlineCommand},
    {"transmode", &ScriptParser::transmodeCommand},
    {"time", &ScriptParser::timeCommand},
    {"textgosub", &ScriptParser::textgosubCommand},
    {"sub", &ScriptParser::subCommand},
    {"stralias", &ScriptParser::straliasCommand},
    {"skip",     &ScriptParser::skipCommand},
    {"selectvoice",     &ScriptParser::selectvoiceCommand},
    {"selectcolor",     &ScriptParser::selectcolorCommand},
    {"savenumber",     &ScriptParser::savenumberCommand},
    {"savename",     &ScriptParser::savenameCommand},
    {"sar",    &ScriptParser::nsaCommand},
    {"roff",    &ScriptParser::roffCommand},
    {"rmenu",    &ScriptParser::rmenuCommand},
    {"return",   &ScriptParser::returnCommand},
    {"numalias", &ScriptParser::numaliasCommand},
    {"nsa",    &ScriptParser::nsaCommand},
    {"notif",    &ScriptParser::ifCommand},
    {"next",    &ScriptParser::nextCommand},
    {"nsa",    &ScriptParser::arcCommand},
    {"mul",      &ScriptParser::mulCommand},
    {"movl",      &ScriptParser::movCommand},
    {"mov10",      &ScriptParser::movCommand},
    {"mov9",      &ScriptParser::movCommand},
    {"mov8",      &ScriptParser::movCommand},
    {"mov7",      &ScriptParser::movCommand},
    {"mov6",      &ScriptParser::movCommand},
    {"mov5",      &ScriptParser::movCommand},
    {"mov4",      &ScriptParser::movCommand},
    {"mov3",      &ScriptParser::movCommand},
    {"mov",      &ScriptParser::movCommand},
    {"mode_saya", &ScriptParser::mode_sayaCommand},
    {"mod",      &ScriptParser::modCommand},
    {"mid",      &ScriptParser::midCommand},
    {"menusetwindow",      &ScriptParser::menusetwindowCommand},
    {"menuselectvoice",      &ScriptParser::menuselectvoiceCommand},
    {"menuselectcolor",      &ScriptParser::menuselectcolorCommand},
    {"lookbacksp",      &ScriptParser::lookbackspCommand},
    {"lookbackcolor",      &ScriptParser::lookbackcolorCommand},
    {"lookbackbutton",      &ScriptParser::lookbackbuttonCommand},
    {"len",      &ScriptParser::lenCommand},
    {"labellog",      &ScriptParser::labellogCommand},
    {"itoa", &ScriptParser::itoaCommand},
    {"intlimit", &ScriptParser::intlimitCommand},
    {"inc",      &ScriptParser::incCommand},
    {"if",       &ScriptParser::ifCommand},
    {"humanz",       &ScriptParser::humanzCommand},
    {"goto",     &ScriptParser::gotoCommand},
    {"gosub",    &ScriptParser::gosubCommand},
    {"globalon",    &ScriptParser::globalonCommand},
    //{"game",    &ScriptParser::gameCommand},
    {"for",   &ScriptParser::forCommand},
    {"filelog",   &ScriptParser::filelogCommand},
    {"effectblank",   &ScriptParser::effectblankCommand},
    {"effect",   &ScriptParser::effectCommand},
    {"div",   &ScriptParser::divCommand},
    {"dim",   &ScriptParser::dimCommand},
    {"defvoicevol",   &ScriptParser::defvoicevolCommand},
    {"defsevol",   &ScriptParser::defsevolCommand},
    {"defmp3vol",   &ScriptParser::defmp3volCommand},
    {"defaultspeed", &ScriptParser::defaultspeedCommand},
    {"dec",   &ScriptParser::decCommand},
    {"date",   &ScriptParser::dateCommand},
    {"cmp",      &ScriptParser::cmpCommand},
    {"clickvoice",   &ScriptParser::clickvoiceCommand},
    {"clickstr",   &ScriptParser::clickstrCommand},
    {"break",   &ScriptParser::breakCommand},
    {"atoi",      &ScriptParser::atoiCommand},
    {"arc",      &ScriptParser::arcCommand},
    {"add",      &ScriptParser::addCommand},
    {"", NULL}
};
    
ScriptParser::ScriptParser()
{
    unsigned int i;
    
    script_buffer = NULL;

    debug_level = 0;

    archive_path = "";
    globalon_flag = false;
    filelog_flag = false;
    labellog_flag = false;
    num_of_label_accessed = 0;
    rmode_flag = true;
    windowback_flag = false;
    usewheel_flag = false;
    text_line_flag = false;
    mode_saya_flag = false;
    
    num_of_labels = 0;
    label_info = NULL;

    string_buffer_offset = 0;
    string_buffer_length = 512;
    string_buffer = new char[ string_buffer_length ];
    tmp_string_buffer  = new char[ string_buffer_length ];
    line_cache = -1;
    
    /* ---------------------------------------- */
    /* Global definitions */
#if defined(PDA)
    screen_ratio1 = 1;
    screen_ratio2 = 2;
#else
    screen_ratio1 = 1;
    screen_ratio2 = 1;
#endif
    screen_width  = 640 * screen_ratio1 / screen_ratio2;
    screen_height = 480 * screen_ratio1 / screen_ratio2;
    version_str = new char[strlen(VERSION_STR1)+
                          strlen("\n")+
                          strlen(VERSION_STR2)+
                          strlen("\n")+
                          +1];
    sprintf( version_str, "%s\n%s\n", VERSION_STR1, VERSION_STR2 );
    jumpf_flag = false;
    srand( time(NULL) );
    z_order = 25;
    end_with_comma_flag = false;
    
    /* ---------------------------------------- */
    /* Lookback related variables */
    lookback_image_name[0] = new char[ strlen( DEFAULT_LOOKBACK_NAME0 ) + 1 ];
    memcpy( lookback_image_name[0], DEFAULT_LOOKBACK_NAME0, strlen( DEFAULT_LOOKBACK_NAME0 ) + 1 );
    lookback_image_name[1] = new char[ strlen( DEFAULT_LOOKBACK_NAME1 ) + 1 ];
    memcpy( lookback_image_name[1], DEFAULT_LOOKBACK_NAME1, strlen( DEFAULT_LOOKBACK_NAME1 ) + 1 );
    lookback_image_name[2] = new char[ strlen( DEFAULT_LOOKBACK_NAME2 ) + 1 ];
    memcpy( lookback_image_name[2], DEFAULT_LOOKBACK_NAME2, strlen( DEFAULT_LOOKBACK_NAME2 ) + 1 );
    lookback_image_name[3] = new char[ strlen( DEFAULT_LOOKBACK_NAME3 ) + 1 ];
    memcpy( lookback_image_name[3], DEFAULT_LOOKBACK_NAME3, strlen( DEFAULT_LOOKBACK_NAME3 ) + 1 );
    lookback_sp[0] = lookback_sp[1] = -1;
    lookback_color[0] = 0xff;
    lookback_color[1] = 0xff;
    lookback_color[2] = 0x00;

    /* ---------------------------------------- */
    /* Select related variables */
    select_on_color[0] = select_on_color[1] = select_on_color[2] = 0xff;
    select_off_color[0] = select_off_color[1] = select_off_color[2] = 0x80;
    
    /* ---------------------------------------- */
    /* For loop related variables */
    current_for_link = &root_for_link;
    for_stack_depth = 0;
    break_flag = false;
    
    /* ---------------------------------------- */
    /* Transmode related variables */
    trans_mode = AnimationInfo::TRANS_TOPLEFT;
    
    /* ---------------------------------------- */
    /* Save/Load related variables */
    save_menu_name = new char[ strlen( DEFAULT_SAVE_MENU_NAME ) + 1 ];
    memcpy( save_menu_name, DEFAULT_SAVE_MENU_NAME, strlen( DEFAULT_SAVE_MENU_NAME ) + 1 );
    load_menu_name = new char[ strlen( DEFAULT_LOAD_MENU_NAME ) + 1 ];
    memcpy( load_menu_name, DEFAULT_LOAD_MENU_NAME, strlen( DEFAULT_LOAD_MENU_NAME ) + 1 );
    save_item_name = new char[ strlen( DEFAULT_SAVE_ITEM_NAME ) + 1 ];
    memcpy( save_item_name, DEFAULT_SAVE_ITEM_NAME, strlen( DEFAULT_SAVE_ITEM_NAME ) + 1 );

    num_save_file = 9;
    for ( i=0 ; i<MAX_SAVE_FILE ; i++ ){
        save_file_info[i].valid = false;
        getSJISFromInteger( save_file_info[i].sjis_no, i+1 );
    }

    /* ---------------------------------------- */
    /* Text related variables */
    default_text_speed[0] = DEFAULT_TEXT_SPEED_LOW;
    default_text_speed[1] = DEFAULT_TEXT_SPEED_MIDDLE;
    default_text_speed[2] = DEFAULT_TEXT_SPEED_HIGHT;
    text_history_num = MAX_TEXT_BUFFER;
    for ( i=0 ; i<MAX_TEXT_BUFFER-1 ; i++ ){
        text_buffer[i].next = &text_buffer[i+1];
        text_buffer[i+1].previous = &text_buffer[i];
        text_buffer[i].buffer = NULL;
        text_buffer[i].xy[1] = -1; // It means an invalid text buffer.
    }
    text_buffer[0].previous = &text_buffer[MAX_TEXT_BUFFER-1];
    text_buffer[MAX_TEXT_BUFFER-1].next = &text_buffer[0];
    text_buffer[MAX_TEXT_BUFFER-1].buffer = NULL;
    text_buffer[MAX_TEXT_BUFFER-1].xy[1] = -1; // It means an invalid text buffer.
    current_text_buffer = &text_buffer[0];

    clickstr_num = 0;
    clickstr_list = NULL;
    clickstr_line = 0;
    clickstr_state = CLICK_NONE;
    
    /* ---------------------------------------- */
    /* Sound related variables */
    mp3_volume = voice_volume = se_volume = DEFAULT_VOLUME;
    for ( i=0 ; i<     CLICKVOICE_NUM ; i++ )
             clickvoice_file_name[i] = NULL;
    for ( i=0 ; i<    SELECTVOICE_NUM ; i++ )
            selectvoice_file_name[i] = NULL;
    for ( i=0 ; i<MENUSELECTVOICE_NUM ; i++ )
        menuselectvoice_file_name[i] = NULL;

    /* ---------------------------------------- */
    /* Menu related variables */
    menu_font.font_size_xy[0] = system_font.font_size_xy[0] = 18;
    menu_font.font_size_xy[1] = system_font.font_size_xy[1] = 18;
    menu_font.top_xy[0] = system_font.top_xy[0] = 0;
    menu_font.top_xy[1] = system_font.top_xy[1] = 16;
    menu_font.num_xy[0] = system_font.num_xy[0] = 32;
    menu_font.num_xy[1] = system_font.num_xy[1] = 23;
    menu_font.pitch_xy[0] = system_font.pitch_xy[0] = 2 + system_font.font_size_xy[0];
    menu_font.pitch_xy[1] = system_font.pitch_xy[1] = 2 + system_font.font_size_xy[1];
    system_font.window_color[0] = system_font.window_color[1] = system_font.window_color[2] = 0xcc;
    menu_font.window_color[0] = menu_font.window_color[1] = menu_font.window_color[2] = 0xcc;

    root_menu_link.next = NULL;
    root_menu_link.label = NULL;
    menu_link_num = 0;
    menu_link_width = 0;
    
    /* ---------------------------------------- */
    /* System customize related variables */
    textgosub_label = NULL;

    /* ---------------------------------------- */
    for ( i=0 ; i<VARIABLE_RANGE ; i++ ){
        num_variables[i] = 0;
        num_limit_flag[i] = false;
        str_variables[i] = new char[1];
        str_variables[i][0] = '\0';
    }

    effect_blank = MINIMUM_TIMER_RESOLUTION;
    
    last_name_alias = &root_name_alias;
    last_name_alias->next = NULL;
    last_str_alias = &root_str_alias;
    last_str_alias->next = NULL;

    /* ---------------------------------------- */
    /* Effect related variables */
    root_effect_link.num = 0;
    root_effect_link.effect = 0;
    root_effect_link.duration = 0;
    last_effect_link = &root_effect_link;
    last_effect_link->next = NULL;

    strcpy( string_buffer, "effect 1,1" );
    current_mode = DEFINE_MODE;
    effectCommand();
    current_mode = NORMAL_MODE;
}

ScriptParser::~ScriptParser()
{
    printf("Deconstructor\n");
    int i;

    for ( i=0 ; i<num_of_labels ; i++ )
        delete[] label_info[i].name;
    if ( label_info ) delete[] label_info;
    if ( script_buffer ) delete[] script_buffer;
    delete[] string_buffer;
    delete[] tmp_string_buffer;
}

int ScriptParser::open( char *path )
{
    if ( path ){
        archive_path = new char[ strlen(path) + 2 ];
        sprintf( archive_path, "%s%c", path, DELIMITER );
    }
    
    cBR = new DirectReader( archive_path );
    cBR->open();
    
    if ( readScript() ) return -1;
    if ( labelScript() ) return -1;
    
    label_stack_depth = 0;

    root_link_label_info.previous = NULL;
    root_link_label_info.next = NULL;
    root_link_label_info.current_line = 0;
    root_link_label_info.offset = 0;
    root_link_label_info.label_info = lookupLabel("define");
    current_mode = DEFINE_MODE;
    current_link_label_info = &root_link_label_info;

    last_tilde.label_info = lookupLabel( "start" );
    last_tilde.current_line = 0;
    last_tilde.offset = 0;
    
    return 0;
}

void ScriptParser::getSJISFromInteger( char *buffer, int no, bool add_space_flag )
{
    int c = 0;
    char num_str[] = "�O�P�Q�R�S�T�U�V�W�X";
    if ( no >= 10 ){
        buffer[c++] = num_str[ no / 10 % 10 * 2];
        buffer[c++] = num_str[ no / 10 % 10 * 2 + 1];
    }
    else if ( add_space_flag ){
        buffer[c++] = ((char*)"�@")[0];
        buffer[c++] = ((char*)"�@")[1];
    }
    buffer[c++] = num_str[ no % 10 * 2];
    buffer[c++] = num_str[ no % 10 * 2 + 1];
    buffer[c++] = '\0';
}

int ScriptParser::readScript()
{
    FILE *fp;
    char file_name[10];
    int  i, j, file_counter = 0, c;
    long len;
    bool encrypt_flag = false;
    char *p_script_buffer;
    
    script_buffer_length = 0;
    
    if ( (fp = fopen( "0.txt", "rb" )) != NULL ){
        do{
            fseek( fp, 0, SEEK_END );
            script_buffer_length += ftell( fp ) + 2;
            sprintf( file_name, "%d.txt", ++file_counter );
            fclose( fp );
        }
        while( (fp = fopen( file_name, "rb" )) != NULL );
    }
    else if ( (fp = fopen( "nscript.dat", "rb" )) != NULL ){
        encrypt_flag = true;
        fseek( fp, 0, SEEK_END );
        script_buffer_length += ftell( fp );
        fclose( fp );
    }
    else{
        fprintf( stderr, "can't open file 0.txt or nscript.dat\n" );
        return -1;
    }
    
    if ( script_buffer ) delete[] script_buffer;
    if ( ( script_buffer = new char[ script_buffer_length ]) == NULL ){
        printf(" *** can't allocate memory for the script ***\n");
        exit( -1 );
    }
    p_script_buffer = script_buffer;
    
    if ( encrypt_flag ){
        fp = fopen( "nscript.dat", "rb" );
        fseek( fp, 0, SEEK_END );
        len = ftell( fp );
        fseek( fp, 0, SEEK_SET );
        while( len > 0 ){
            if ( len > READ_LENGTH ) c = READ_LENGTH;
            else                     c = len;
            len -= c;
            fread( p_script_buffer, 1, c, fp );
            for ( j=0 ; j<c ; j++ ) p_script_buffer[j] ^= 0x84;
            p_script_buffer += c;
        }
        fclose( fp );
    }
    else{
        for ( i=0 ; i<file_counter ; i++ ){
            sprintf( file_name, "%d.txt", i );
            fp = fopen( file_name, "rb" );
            printf("opening %s\n",file_name);
            fseek( fp, 0, SEEK_END );
            len = ftell( fp );
            fseek( fp, 0, SEEK_SET );
            while( len > 0 ){
                if ( len > READ_LENGTH ) c = READ_LENGTH;
                else                     c = len;
                len -= c;
                fread( p_script_buffer, 1, c, fp );
                p_script_buffer += c;
            }
            fclose( fp );
            *p_script_buffer++ = 0x0d;
            *p_script_buffer++ = 0x0a;
        }
    }

    /* ---------------------------------------- */
    /* 800 x 600 check */
    if ( !strncmp( script_buffer, ";mode800", 8 ) ){
#if defined(PDA)
        screen_ratio1 = 2;
        screen_ratio2 = 5;
#else
        screen_ratio1 = 1;
        screen_ratio2 = 1;
#endif
        screen_width  = 800 * screen_ratio1 / screen_ratio2;
        screen_height = 600 * screen_ratio1 / screen_ratio2;
    }
    underline_value = screen_height - 1;
         
    return 0;
}

void ScriptParser::addStringBuffer( char ch, int string_counter )
{
    /* In case of string bufer over flow */
    if ( string_counter == string_buffer_length ){
        string_buffer_length += 512;
        char *tmp_buffer = new char[ string_buffer_length ];
        memcpy( tmp_buffer, string_buffer, string_counter );

        delete[] string_buffer;
        string_buffer = tmp_buffer;

        delete[] tmp_string_buffer;
        tmp_string_buffer = new char[ string_buffer_length ];
    }

    string_buffer[string_counter] = ch;
}

unsigned char ScriptParser::convHexToDec( char ch )
{
    if ( '0' <= ch && ch <= '9' ) return ch - '0';
    else if ( 'a' <= ch && ch <= 'f' ) return ch - 'a' + 10;
    else if ( 'A' <= ch && ch <= 'F' ) return ch - 'A' + 10;
    else return 0;
}

void ScriptParser::readColor( uchar3 *color, char *buf ){
    (*color)[0] = convHexToDec( buf[0] ) << 4 | convHexToDec( buf[1] );
    (*color)[1] = convHexToDec( buf[2] ) << 4 | convHexToDec( buf[3] );
    (*color)[2] = convHexToDec( buf[4] ) << 4 | convHexToDec( buf[5] );
}

int ScriptParser::skipLine( char **buf )
{
    char *end_point = script_buffer + script_buffer_length;

    while( 1 ){
        if ( *buf == end_point ) return -1;
        if ( *(*buf)++ == 0x0a ) break;
    }

    return 0;
}

int ScriptParser::readLine( char **buf, bool raw_flag )
{
    char ch;
    int string_counter=0, no;
    char *end_point = script_buffer + script_buffer_length;
    bool head_flag = true;
    bool text_flag = true;
    char num_buf[10], num_sjis_buf[3];
    bool quat_flag = false, comment_flag = false;
    unsigned int i;

    text_line_flag = false;
    
    while( 1 ){
        if ( *buf == end_point ) return -1;
        ch = *(*buf)++;
        
        if ( head_flag && (ch == ' ' || ch == '\t' || ch == ':' ) ){
            continue;
        }

        if ( ch == 0x0d ) continue;
        if ( ch == 0x0a ) break;

        if ( comment_flag ) continue;
        if ( raw_flag ){
            addStringBuffer( ch, string_counter++ );
            head_flag = false;
            continue;
        }

        /* Parser */
        if ( !quat_flag && ch == ';' ){
            if ( head_flag ) addStringBuffer( ch, string_counter++ );
            comment_flag = true;
        }
        else if ( (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ){
            if ( head_flag ) text_flag = false;
            if ( !text_flag && !quat_flag && ch >= 'A' && ch <= 'Z' ) ch += 'a' - 'A';
            addStringBuffer( ch, string_counter++ );
        }
        else if ( ch == '$' ){
            if ( text_flag ){
                no = readInt( buf );
                if ( str_variables[no] ){
                    for ( i=0 ; i<strlen( str_variables[no] ) ; i++ ){
                        addStringBuffer( str_variables[no][i], string_counter++ );
                    }
                }
                if ( end_with_comma_flag ) addStringBuffer( ',', string_counter++ );
            }
            else{
                addStringBuffer( '$', string_counter++ );
            }
        }
        else if ( ch == '%' || ch == '?'){
            if ( text_flag ){
                (*buf)--;
                no = readInt( buf );
                if ( no<0 ){
                    addStringBuffer( "�|"[0], string_counter++ );
                    addStringBuffer( "�|"[1], string_counter++ );
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
                if ( end_with_comma_flag ) addStringBuffer( ',', string_counter++ );
            }
            else{
                addStringBuffer( ch, string_counter++ );
            }
        }
        else if ( ch == '"' ){
            quat_flag = !quat_flag;
            addStringBuffer( ch, string_counter++ );
        }
        else if ( ch & 0x80 ){
            addStringBuffer( ch, string_counter++ );
            ch = *(*buf)++;
            addStringBuffer( ch, string_counter++ );
            if ( !quat_flag )
                text_flag = true;
        }
        else if ( (ch == '@' || ch == '\\') && textgosub_label ){
            addStringBuffer( ch, string_counter++ );
            if ( !quat_flag ){
                head_flag = true;
                text_flag = true;
                continue;
            }
        }
        else{
            addStringBuffer( ch, string_counter++ );
        }

        head_flag = false;
        if ( ch == ':' ) head_flag = true;
    }

    addStringBuffer( '\0', string_counter++ );

    string_buffer_offset = 0;
    while ( string_buffer[ string_counter ] == ' ' || string_buffer[ string_counter ] == '\t' )
        string_counter++;

    return 0;
}

int ScriptParser::findLabel( const char *label )
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

bool ScriptParser::getLabelAccessFlag( const char *label )
{
    int i = findLabel( label );
    if ( i >= 0 ){
        return label_info[i].access_flag;
    }

    return false;
}

struct ScriptParser::LabelInfo ScriptParser::lookupLabel( const char *label )
{
    int i = findLabel( label );
    if ( i >= 0 ){
        if ( !label_info[i].access_flag ) num_of_label_accessed++;
        label_info[i].access_flag = true;
        return label_info[i];
    }

    errorAndExit( string_buffer + string_buffer_offset, "Label is not found." );
    return label_info[0];
}

struct ScriptParser::LabelInfo ScriptParser::lookupLabelNext( const char *label )
{
    int i = findLabel( label );
    if ( i >= 0 && i+1 < num_of_labels ){
        if ( !label_info[i+1].access_flag ) num_of_label_accessed++;
        label_info[i+1].access_flag = true;
        return label_info[i+1];
    }

    errorAndExit( string_buffer + string_buffer_offset, "Label is not found." );
    return label_info[0];
}

int ScriptParser::labelScript()
{
    char *p_script_buffer = script_buffer, *p_script_buffer_old, *p_start_buffer = NULL;
    char *p_string_buffer = NULL;
    bool first_flag = true;
    int  label_counter = -1;
    
    num_of_labels = 0;

    p_script_buffer_old = p_script_buffer;
    while( !readLine( &p_script_buffer, true ) ){
        if ( string_buffer[0] == '*' ){
            if ( first_flag ){
                first_flag = false;
                p_start_buffer = p_script_buffer_old;
            }
            num_of_labels++;
        }
        p_script_buffer_old = p_script_buffer;
    }

    label_info = new struct LabelInfo[ num_of_labels ];

    p_script_buffer = p_start_buffer;
    while( !readLine( &p_script_buffer, true ) ){
        if ( string_buffer[0] == '*' ){
            p_string_buffer = string_buffer + 1;
            readToken( &p_string_buffer, tmp_string_buffer );
            label_info[ ++label_counter ].name = new char[ strlen(tmp_string_buffer) + 1 ];
            for ( unsigned int i=0 ; i<strlen(tmp_string_buffer) + 1 ; i++ ){
                label_info[ label_counter ].name[i] = tmp_string_buffer[i];
                if ( 'a' <= label_info[ label_counter ].name[i] && label_info[ label_counter ].name[i] <= 'z' )
                    label_info[ label_counter ].name[i] += 'A' - 'a';
            }
            label_info[ label_counter ].start_address = p_script_buffer;
            label_info[ label_counter ].num_of_lines  = 0;
            label_info[ label_counter ].access_flag   = false;
        }
        else{
            label_info[ label_counter ].num_of_lines++;
        }
    }

    return 0;
}

int ScriptParser::parseLine()
{
    int ret, lut_counter = 0;

    while( string_buffer[ string_buffer_offset ] == ' ' ||
           string_buffer[ string_buffer_offset ] == '\t' ) string_buffer_offset++;
    if ( text_line_flag ) return RET_NOMATCH;
    if ( string_buffer[ string_buffer_offset ] == ':' ){
        string_buffer_offset++;
        return RET_CONTINUE;
    }
    if ( string_buffer[ string_buffer_offset ] == '\0' || string_buffer[ string_buffer_offset ] == ';' ) return RET_COMMENT;
    else if ( string_buffer[ string_buffer_offset ] & 0x80 ) return RET_NOMATCH;

    char *p_string_buffer = string_buffer + string_buffer_offset;
    if ( debug_level > 0 ) printf("ScriptParser::Parseline %d %d %s\n",text_line_flag, string_buffer_offset,p_string_buffer );
    readToken( &p_string_buffer, tmp_string_buffer );

    while( func_lut[ lut_counter ].method ){
        if ( !strcmp( func_lut[ lut_counter ].command, tmp_string_buffer ) ){
            ret = (this->*func_lut[ lut_counter ].method)();
            if ( ret == RET_CONTINUE || ret == RET_WAIT_NEXT ){
                skipToken();
            }
            break;
        }
        lut_counter++;
    }

    if ( func_lut[ lut_counter ].method == 0 ) return RET_NOMATCH;

    return ret;
}


bool ScriptParser::readToken( char **src_buf, char *dst_buf, bool skip_space_flag )
{
    bool first_flag = true;
    bool quat_flag = false;

    end_with_comma_flag = false;
#if 0
    /* If it reaces to the end of the buffer, just return. */
    if ( *src_buf >= string_buffer + strlen(string_buffer) ){
        dst_buf[0] = '\0';
        return end_with_comma_flag;
    }
#endif
    /* ---------------------------------------- */
    /* Get a single word */
    SKIP_SPACE( *src_buf );

    //printf("token start %c:\n", **src_buf);
    if ( **src_buf == '"' ){
        quat_flag = true;
        (*src_buf)++;
    }
    while ( **src_buf != '\0' &&
            **src_buf != '"' &&
            (quat_flag || ( **src_buf != ',' &&
                            **src_buf != ':' &&
                            **src_buf != ';' &&
                            **src_buf != '<' &&
                            **src_buf != '>' &&
                            **src_buf != '=' &&
                            **src_buf != '!' &&
                            **src_buf != '&' &&
                            **src_buf != ')' &&
                            !(**src_buf & 0x80) ) ) &&
            ( quat_flag || skip_space_flag || ( **src_buf != ' ' &&
                                                **src_buf != '\t' ) ) &&
            ( quat_flag || first_flag || (**src_buf != '#' &&
                                          **src_buf != '-' &&
                                          **src_buf != '*' ) )
            ){
        if ( **src_buf & 0x80 ) {
            *dst_buf++ = *(*src_buf)++;
            *dst_buf++ = *(*src_buf)++;
        }
        else{
            *dst_buf++ = *(*src_buf)++;
        }
        first_flag = false;
    }

    if ( quat_flag && **src_buf == '"' ) (*src_buf)++;
    SKIP_SPACE( *src_buf );

    if ( **src_buf == ')' ) (*src_buf)++;
    SKIP_SPACE( *src_buf );
    
    if ( **src_buf == ',' ){
        end_with_comma_flag = true;
        (*src_buf)++;
        SKIP_SPACE( *src_buf );
    }
    *dst_buf++ = '\0';
    
    return end_with_comma_flag;
}

bool ScriptParser::readStr( char **src_buf, char *dst_buf )
{
    bool condition_flag = false;
    bool ret;
    
    SKIP_SPACE( *src_buf );
    if ( (*src_buf)[0] == '(' ){
        condition_flag = true;
        (*src_buf)++;
        readStr( src_buf, dst_buf );
    }
    else{
        ret = readToken( src_buf, dst_buf );
    }

    if ( condition_flag ){ // check condition code
        if ( cBR->getAccessFlag( dst_buf ) ){
            ret = readStr( src_buf, dst_buf );
            char *buf = new char[ strlen( dst_buf ) + 1 ];
            memcpy( buf, dst_buf, strlen( dst_buf ) + 1 );
            readStr( src_buf, dst_buf );
            memcpy( dst_buf, buf, strlen( buf ) + 1 );
            delete[] buf;
        }
        else{
            readStr( src_buf, dst_buf );
            ret = readStr( src_buf, dst_buf );
        }
    }
    
    if ( dst_buf[0] == '$' ){
        char *p_dst_buf = dst_buf+1;
        int no = readInt( &p_dst_buf );
        sprintf( dst_buf, "%s", str_variables[ no ] );
    }
    else{
        /* ---------------------------------------- */
        /* Solve str aliases */
        StringAlias *p_str_alias = root_str_alias.next;

        while( p_str_alias ){
            if ( !strcmp( p_str_alias->alias, (const char*)dst_buf ) ){
                memcpy( dst_buf, p_str_alias->str, strlen( p_str_alias->str ) + 1 );
                break;
            }
            p_str_alias = p_str_alias->next;
        }
    }
    return ret;
}

void ScriptParser::skipToken()
{
    bool quat_flag = false;
    bool first_flag = true;
    bool comma_flag = false;
    char *buf = string_buffer + string_buffer_offset;

    if ( *buf & 0x80 ){
        buf += 2;
    }
    else while(1){
        if ( *buf == '\0' ||
             ( !quat_flag && *buf == ':' ) ||
             ( !quat_flag && *buf == ';' )) break;
        else if ( !quat_flag && *buf & 0x80 ){
            //buf += 2;
            break;
        }
        else if ( *buf == '"' ){
            quat_flag = !quat_flag;
        }
        else if ( !quat_flag && (*buf == ' ' || *buf == '\t') ){
            SKIP_SPACE( buf );
            if ( *buf == ',' ){
                buf++;
                comma_flag = true;
                SKIP_SPACE( buf );
            }
            if ( !first_flag && !comma_flag ) break;
            first_flag = false;
            continue;
        }
        else if ( !quat_flag && *buf == ',' ){
            comma_flag = true;
        }
        else if ( !quat_flag && (*buf == '%' || *buf == '$' ) ){
            buf++;
            SKIP_SPACE( buf );
            continue;
        }
        else if ( !quat_flag && (*buf == '?' ) ){
            while ( *(buf++) != '[' );
            int c = 1;
            while ( c > 0 ){
                if ( *buf == '[' ) c++;
                else if ( *buf == ']' ) c--;
                buf++;
            }
            continue;
        }
        else if ( !quat_flag && (*buf == '@' || *buf == '\\') && textgosub_label ){
            comma_flag = false;
            break;
        }
        else
            comma_flag = false;
        buf++;
    }
    while ( *buf == ':' ){
        buf++;
        SKIP_SPACE( buf );
    }
    if ( *buf == ';' ) while ( *buf != '\0' ) buf++;
    //printf("skipToken [%s] to [%s]\n",string_buffer + string_buffer_offset, buf );
    string_buffer_offset = buf - string_buffer;
}

struct ScriptParser::EffectLink ScriptParser::getEffect( int effect_no )
{
    if      ( effect_no == WINDOW_EFFECT ) return window_effect;
    else if ( effect_no == PRINT_EFFECT )  return print_effect;
    else if ( effect_no == TMP_EFFECT )    return tmp_effect;
    else if ( effect_no == QUAKE_EFFECT )  return quake_effect;
    
    EffectLink *p_effect_link = &root_effect_link;
    while( p_effect_link ){
        if ( p_effect_link->num == effect_no ) return *p_effect_link;
        p_effect_link = p_effect_link->next;
    }

    fprintf( stderr, "no effect was found [%d]\n", effect_no );
    exit(-1);
}

int ScriptParser::getSystemCallNo( char *buffer )
{
    if      ( !strcmp( buffer, "skip" ) )        return SYSTEM_SKIP;
    else if ( !strcmp( buffer, "reset" ) )       return SYSTEM_RESET;
    else if ( !strcmp( buffer, "save" ) )        return SYSTEM_SAVE;
    else if ( !strcmp( buffer, "load" ) )        return SYSTEM_LOAD;
    else if ( !strcmp( buffer, "lookback" ) )    return SYSTEM_LOOKBACK;
    else if ( !strcmp( buffer, "windowerase" ) ) return SYSTEM_WINDOWERASE;
    else if ( !strcmp( buffer, "rmenu" ) )       return SYSTEM_MENU;
    else{
        printf("Unsupported system call %s\n", buffer );
        return -1;
    }
}

void ScriptParser::saveGlovalData()
{
    if ( !globalon_flag ) return;
    printf(" saveGlobalData\n" );

    FILE *fp;

    if ( ( fp = fopen( "global.sav", "wb" ) ) == NULL ){
        fprintf( stderr, "can't write global.sav\n" );
        exit( -1 );
    }

    saveVariables( fp, 200, VARIABLE_RANGE );
    fclose( fp );
}

void ScriptParser::loadFileLog()
{
    FILE *fp; 
    int i, j, ch, count = 0;
    char buf[100];

    if ( ( fp = fopen( "NScrflog.dat", "rb" ) ) != NULL ){
        while( (ch = fgetc( fp )) != 0x0a ){
            count = count * 10 + ch - '0';
        }

        for ( i=0 ; i<count ; i++ ){
            fgetc( fp );
            j = 0; 
            while( (ch = fgetc( fp )) != '"' ) buf[j++] = ch ^ 0x84;
            buf[j] = '\0';

            cBR->getFileLength( buf );
        }

        fclose( fp );
    }
}

void ScriptParser::saveFileLog()
{
    if ( !filelog_flag ) return;
    printf(" saveFileLog\n" );

    FILE *fp;
    int  i,j;
    char buf[10];

    if ( ( fp = fopen( "NScrflog.dat", "wb" ) ) == NULL ){
        fprintf( stderr, "can't write NScrflog.dat\n" );
        exit( -1 );
    }

    sprintf( buf, "%d", cBR->getNumAccessed() );
    for ( i=0 ; i<(int)strlen( buf ) ; i++ ) fputc( buf[i], fp );
    fputc( 0x0a, fp );
    
    SarReader::FileInfo fi;
    for ( i=0 ; i<cBR->getNumFiles() ; i++ ){
        fi = cBR->getFileByIndex( i );
        if ( fi.access_flag ){
            fputc( '"', fp );
            for ( j=0 ; j<(int)strlen( fi.name ) ; j++ )
                fputc( fi.name[j] ^ 0x84, fp );
            fputc( '"', fp );
        }
    }
    
    fclose( fp );
}

void ScriptParser::saveLabelLog()
{
    if ( !labellog_flag ) return;
    printf(" saveLabelLog\n" );

    FILE *fp;
    int  i;
    char buf[10];

    if ( ( fp = fopen( "NScrllog.dat", "wb" ) ) == NULL ){
        fprintf( stderr, "can't write NScrllog.dat\n" );
        exit( -1 );
    }

    sprintf( buf, "%d", num_of_label_accessed );
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

void ScriptParser::saveInt( FILE *fp, int var )
{
    char tmp_int[4];
    
    tmp_int[0] = var & 0xff;
    tmp_int[1] = (var >> 8) & 0xff;
    tmp_int[2] = (var >> 16) & 0xff;
    tmp_int[3] = (var >> 24) & 0xff;
        
    fwrite( tmp_int, 1, 4, fp );
}

void ScriptParser::loadInt( FILE *fp, int *var )
{
    unsigned char tmp_int[4];
    
    fread( tmp_int, 1, 4, fp );
    *var =
        (unsigned int)tmp_int[3] << 24 |
        (unsigned int)tmp_int[2] << 16 |
        (unsigned int)tmp_int[1] << 8 |
        (unsigned int)tmp_int[0];
}

void ScriptParser::saveStr( FILE *fp, char *str )
{
    if ( str && str[0] ) fprintf( fp, "%s", str );
    fputc( 0, fp );
}

#define INITIAL_LOAD_STR 128
void ScriptParser::loadStr( FILE *fp, char **str )
{
    int counter = 0, counter_max = INITIAL_LOAD_STR;
    char tmp_buffer[INITIAL_LOAD_STR], *p_buffer = tmp_buffer, *p_buffer2;
    
    while( (p_buffer[ counter++ ] = fgetc( fp )) != '\0' ){
        if ( counter >= counter_max ){
            p_buffer2 = p_buffer;
            p_buffer = new char[ counter_max + INITIAL_LOAD_STR ];
            memcpy( p_buffer, p_buffer2, counter_max );
            if ( counter_max != INITIAL_LOAD_STR ) delete[] p_buffer2;
            counter_max += INITIAL_LOAD_STR;
        }
    }
    if ( *str ) delete[] *str;
    if ( counter == 1 ){
        *str = NULL;
    }
    else{
        *str = new char[ counter ];
        memcpy( *str, p_buffer, counter );
        if ( counter_max != INITIAL_LOAD_STR ) delete[] p_buffer;
    }
}

void ScriptParser::saveVariables( FILE *fp, int from, int to )
{
    for ( int i=from ; i<to ; i++ ){
        saveInt( fp, num_variables[i] );
        saveStr( fp, str_variables[i] );
    }
}

void ScriptParser::loadVariables( FILE *fp, int from, int to )
{
    for ( int i=from ; i<to ; i++ ){
        loadInt( fp, &num_variables[i] );
        loadStr( fp, &str_variables[i] );
        if ( str_variables[i] == NULL ){
            str_variables[i] = new char[1];
            str_variables[i][0] = '\0';
        }
    }
}

void ScriptParser::errorAndExit( char *str, char *reason )
{
    if ( reason )
        fprintf( stderr, " *** Invalid command [%s]; %s ***\n", str, reason );
    else
        fprintf( stderr, " *** Invalid command [%s] ***\n", str );
    exit(-1);
}

int ScriptParser::decodeArraySub( char **buf, struct ArrayVariable *array )
{
    SKIP_SPACE( *buf );
    
    (*buf)++; // skip '?'
    int no = readInt( buf );

    SKIP_SPACE( *buf );
    array->num_dim = 0;
    while ( **buf == '[' ){
        (*buf)++;
        array->dim[array->num_dim] = readInt( buf );
        array->num_dim++;
        SKIP_SPACE( *buf );
        if ( **buf != ']' ) errorAndExit( *buf );
        (*buf)++;
    }
    for ( int i=array->num_dim ; i<20 ; i++ ) array->dim[i] = 0;

    return no;
}

int *ScriptParser::decodeArray( char **buf, int offset )
{
    struct ArrayVariable array;
    int dim, i;

    SKIP_SPACE( *buf );
    int no = decodeArraySub( buf, &array );
    
    if ( array_variables[ no ].data == NULL ) errorAndExit( string_buffer + string_buffer_offset );
    if ( array_variables[ no ].dim[0] <= array.dim[0] ) errorAndExit( string_buffer + string_buffer_offset );
    dim = array.dim[0];
    for ( i=1 ; i<array_variables[ no ].num_dim ; i++ ){
        if ( array_variables[ no ].dim[i] <= array.dim[i] ) errorAndExit( string_buffer + string_buffer_offset );
        dim = dim * array_variables[ no ].dim[i] + array.dim[i];
    }
    if ( array_variables[ no ].dim[i-1] <= array.dim[i-1] + offset ) errorAndExit( string_buffer + string_buffer_offset );
    dim += offset;

    return &array_variables[ no ].data[ dim ];
}

int ScriptParser::readInt( char **buf )
{
    int no;

    end_with_comma_flag = false;
    SKIP_SPACE( *buf );

    if ( (*buf)[0] == '\0' || (*buf)[0] == ':' || (*buf)[0] == ';' ){
        no = 0;
    }
    else if ( (*buf)[0] == '%' ){
        (*buf)++;
        no = num_variables[ readInt( buf ) ];
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
    if ( **buf == ',' ){
        end_with_comma_flag = true;
        (*buf)++;
        SKIP_SPACE( *buf );
    }
    return no;
}

void ScriptParser::setInt( char *buf, int val, int offset )
{
    char *p_buf;

    SKIP_SPACE( buf );
    if ( buf[0] == '%' ){
        p_buf = buf + 1;
        setNumVariable( readInt( &p_buf ) + offset, val );
    }
    else if ( buf[0] == '?' ){
        p_buf = buf;
        *(decodeArray( &p_buf, offset )) = val;
    }
    else{
        errorAndExit( string_buffer + string_buffer_offset );
    }
}

void ScriptParser::setNumVariable( int no, int val )
{
    if ( no >= VARIABLE_RANGE ) errorAndExit( string_buffer + string_buffer_offset );
    if ( num_limit_flag[no] ){
        if ( val < num_limit_lower[no] )      val = num_limit_lower[no];
        else if ( val > num_limit_upper[no] ) val = num_limit_upper[no];
    }
    num_variables[no] = val;
}

void ScriptParser::setStr( char **dst, char *src )
{
    if ( *dst ) delete[] *dst;
    if ( src ){
        *dst = new char[ strlen( src ) + 1 ];
        memcpy( *dst, src, strlen( src ) + 1 );
    }
    else
        *dst = NULL;
}

int ScriptParser::readEffect( char **buf, struct EffectLink *effect )
{
    int num = 1;
    
    effect->effect = readInt( buf );
    if ( end_with_comma_flag ){
        num++;
        effect->duration = readInt( buf );
        if ( end_with_comma_flag ){
            num++;
            readStr( buf, tmp_string_buffer );
            setStr( &effect->image, tmp_string_buffer );
        }
        else
            effect->image = NULL;
        
    }
    else{
        effect->duration = 0;
    }

    if ( effect->effect == 1 ) effect->duration = 0;
    //printf("readEffect %d: %d %d %s\n", num, effect->effect, effect->duration, effect->image );
    return num;
}

FILE *ScriptParser::fopen(const char *path, const char *mode)
{
    char file_name[256];

    sprintf( file_name, "%s%s", archive_path, path );
    return ::fopen( file_name, mode );
}
