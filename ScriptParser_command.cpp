/* -*- C++ -*-
 *
 *  ScriptParser_command.cpp - Define command executer of ONScripter
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

int ScriptParser::windowbackCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer(), "not in the define section" );
    windowback_flag = true;

    return RET_CONTINUE;
}

int ScriptParser::versionstrCommand()
{
    delete[] version_str;

    const char *buf = script_h.readStr();
    char *buf2 = new char[ strlen( buf ) + 1 ];
    strcpy( buf2, buf );

    buf = script_h.readStr();
    version_str = new char[ strlen( buf2 ) + strlen( buf ) + strlen("\n") * 2 + 1 ];
    sprintf( version_str, "%s\n%s\n", buf2, buf );

    delete[] buf2;

    printf("versionstr %s", version_str );
    
    return RET_CONTINUE;
}

int ScriptParser::usewheelCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer(), "not in the define section" );

    usewheel_flag = true;

    return RET_CONTINUE;
}

int ScriptParser::useescspcCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer(), "not in the define section" );

    if ( !force_button_shortcut_flag )
        useescspc_flag = true;

    return RET_CONTINUE;
}

int ScriptParser::underlineCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer(), "not in the define section" );

    underline_value = script_h.readInt() * screen_ratio1 / screen_ratio2;

    return RET_CONTINUE;
}

int ScriptParser::transmodeCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer(), "not in the define section" );

    const char *buf = script_h.readStr();
    if      ( !strcmp( buf, "leftup" ) )   trans_mode = AnimationInfo::TRANS_TOPLEFT;
    else if ( !strcmp( buf, "copy" ) )     trans_mode = AnimationInfo::TRANS_COPY;
    else if ( !strcmp( buf, "alpha" ) )    trans_mode = AnimationInfo::TRANS_ALPHA;
    else if ( !strcmp( buf, "righttup" ) ) trans_mode = AnimationInfo::TRANS_TOPRIGHT;

    return RET_CONTINUE;
}

int ScriptParser::timeCommand()
{
    time_t t = time(NULL);
    struct tm *tm = localtime( &t );

    script_h.readToken();
    script_h.setInt( script_h.getStringBuffer(), tm->tm_hour );
    
    script_h.readToken();
    script_h.setInt( script_h.getStringBuffer(), tm->tm_min );
    
    script_h.readToken();
    script_h.setInt( script_h.getStringBuffer(), tm->tm_sec );

    return RET_CONTINUE;
}

int ScriptParser::textgosubCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer(), "not in the define section" );

    const char *buf = script_h.readStr();
    setStr( &textgosub_label, buf+1 );
    
    return RET_CONTINUE;
}

int ScriptParser::subCommand()
{
    int val1 = script_h.readInt();
    char *save_buf = script_h.saveStringBuffer();

    int val2 = script_h.readInt();
    script_h.setInt( save_buf, val1 - val2 );

    return RET_CONTINUE;
}

int ScriptParser::straliasCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer(), "not in the define section" );
    
    const char *buf = script_h.readStr();
    char *buf2 = new char[ strlen( buf ) + 1 ];
    strcpy( buf2, buf );
    
    buf = script_h.readStr();
    
    script_h.addStrAlias( buf2, buf );
    delete[] buf2;
    
    return RET_CONTINUE;
}

int ScriptParser::skipCommand()
{
    int skip_num    = script_h.readInt();
    int skip_offset = current_link_label_info->current_line + skip_num;

    while ( skip_offset >= current_link_label_info->label_info.num_of_lines ){ 
        skip_offset -= current_link_label_info->label_info.num_of_lines + 1;
        current_link_label_info->label_info = script_h.lookupLabelNext( current_link_label_info->label_info.name );
    }
    if ( skip_offset == -1 ) skip_offset = 0; // -1 indicates a label line
    current_link_label_info->current_line = skip_offset;
    
    script_h.setCurrent( current_link_label_info->label_info.start_address );
    script_h.skipLine( current_link_label_info->current_line );
    string_buffer_offset = 0;

    return RET_JUMP;
}

int ScriptParser::selectvoiceCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer(), "not in the define section" );

    for ( int i=0 ; i<SELECTVOICE_NUM ; i++ ){
        const char *buf = script_h.readStr();
        if ( buf[0] != '\0' ){
            setStr( &selectvoice_file_name[i], buf );
        }
        else if ( selectvoice_file_name[i] ){
            delete selectvoice_file_name[i];
            selectvoice_file_name[i] = NULL;
        }
    }

    return RET_CONTINUE;
}

int ScriptParser::selectcolorCommand()
{
    const char *buf = script_h.readStr();
    if ( buf[0] != '#' ) errorAndExit( buf, "selectcolor: no preceeding #" );
    readColor( &sentence_font.on_color, buf+1 );

    buf = script_h.readStr();
    if ( buf[0] != '#' ) errorAndExit( buf, "selectcolor: no preceeding #" );
    readColor( &sentence_font.off_color, buf+1 );
    
    return RET_CONTINUE;
}

int ScriptParser::savenumberCommand()
{
    num_save_file = script_h.readInt();

    return RET_CONTINUE;
}

int ScriptParser::savenameCommand()
{
    const char *buf = script_h.readStr();
    setStr( &save_menu_name, buf );

    buf = script_h.readStr();
    setStr( &load_menu_name, buf );

    buf = script_h.readStr();
    setStr( &save_item_name, buf );

    return RET_CONTINUE;
}

int ScriptParser::roffCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer(), "not in the define section" );
    rmode_flag = false;

    return RET_CONTINUE;
}

int ScriptParser::rmenuCommand()
{
    MenuLink *menu, *link;
    
    /* ---------------------------------------- */
    /* Delete old MenuLink */
    link = root_menu_link.next;
    while( link ){
        menu = link->next;
        if ( link->label ) delete[] link->label;
        delete link;
        link = menu;
    }

    link = &root_menu_link;
    menu_link_num   = 0;
    menu_link_width = 0;

    bool first_flag = true;
    while ( script_h.isEndWithComma() || first_flag ){
        MenuLink *menu = new MenuLink();

        const char *buf = script_h.readStr();
        setStr( &menu->label, buf );
        if ( menu_link_width < strlen( buf ) / 2 )
            menu_link_width = strlen( buf ) / 2;

        buf = script_h.readStr();
        menu->system_call_no = getSystemCallNo( buf );

        link->next = menu;
        link = menu;
        menu_link_num++;

        first_flag = false;
    }
    
    return RET_CONTINUE;
}

int ScriptParser::returnCommand()
{
    if ( --label_stack_depth < 0 ) errorAndExit( script_h.getStringBuffer(), "too many returns" );
    
    current_link_label_info = current_link_label_info->previous;

    script_h.setCurrent( current_link_label_info->current_script );
    string_buffer_offset = current_link_label_info->string_buffer_offset;

    if ( current_link_label_info->next->textgosub_flag ){
        script_h.next_text_line_flag = false;
        
        if ( script_h.getStringBuffer()[string_buffer_offset] == 0x0a )
            script_h.text_line_flag = true;
    }

    delete current_link_label_info->next;
    current_link_label_info->next = NULL;
    
    return RET_JUMP;
}

int ScriptParser::numaliasCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( "numalias: not in the define section" );

    const char *buf = script_h.readStr();
    int no = script_h.readInt();
    script_h.addNumAlias( buf, no );
    
    return RET_CONTINUE;
}

int ScriptParser::nsadirCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( "nsadir: not in the define section" );

    const char *buf = script_h.readStr();
    
    if ( strlen(nsa_path) > 0 ){
        delete[] nsa_path;
    }
    nsa_path = new char[ strlen(buf) + 2 ];
    sprintf( nsa_path, "%s%c", buf, DELIMITER );

    return RET_CONTINUE;
}

int ScriptParser::nsaCommand()
{
    int archive_type = NsaReader::ARCHIVE_TYPE_NSA;
    
    if ( script_h.isName("ns2") ){
        archive_type = NsaReader::ARCHIVE_TYPE_NS2;
    }
    
    delete script_h.cBR;
    script_h.cBR = new NsaReader( archive_path );
    if ( script_h.cBR->open( nsa_path, archive_type ) ){
        fprintf( stderr, " *** failed to open Nsa archive, exitting ...  ***\n");
        exit(-1);
    }

    return RET_CONTINUE;
}

int ScriptParser::nextCommand()
{
    char *p_buf;
    int val;
    
    if ( !break_flag ){
        p_buf = current_for_link->p_int;
        val   = script_h.parseInt( &p_buf );
        script_h.setInt( current_for_link->p_int, val + current_for_link->step );
    }

    p_buf = current_for_link->p_int;
    val   = script_h.parseInt( &p_buf );
    
    if ( break_flag ||
         current_for_link->step >= 0 && val > current_for_link->to ||
         current_for_link->step < 0  && val < current_for_link->to ){
        break_flag = false;
        for_stack_depth--;
        current_for_link = current_for_link->previous;

        delete current_for_link->next;
        current_for_link->next = NULL;

        return RET_CONTINUE;
    }
    else{
        current_link_label_info->label_info   = current_for_link->label_info;
        current_link_label_info->current_line = current_for_link->current_line;
        script_h.setCurrent( current_for_link->current_script );
        string_buffer_offset = 0;
        
        return RET_JUMP;
    }
}

int ScriptParser::mulCommand()
{
    int val1 = script_h.readInt();
    char *save_buf = script_h.saveStringBuffer();

    int val2 = script_h.readInt();
    script_h.setInt( save_buf, val1 * val2 );

    return RET_CONTINUE;
}

int ScriptParser::movCommand()
{
    int count, no;
    
    if ( script_h.isName( "mov10" ) ){
        count = 10;
    }
    else if ( script_h.isName( "movl" ) ){
        count = -1; // infinite
    }
    else if ( script_h.getStringBuffer()[3] >= '3' && script_h.getStringBuffer()[3] <= '9' ){
        count = script_h.getStringBuffer()[3] - '0';
    }
    else{
        count = 1;
    }

    script_h.readToken();
    
    if ( script_h.getStringBuffer()[0] == '%' || script_h.getStringBuffer()[0] == '?' ){
        char *save_buf = script_h.saveStringBuffer();
        bool loop_flag = script_h.isEndWithComma();
        int i=0;
        while ( (count==-1 || i<count) && loop_flag ){
            no = script_h.readInt();
            loop_flag = script_h.isEndWithComma();
            script_h.setInt( save_buf, no, i++ );
        }
    }
    else if ( script_h.getStringBuffer()[0] == '$'){
        char *p_buf = script_h.getStringBuffer()+1;
        no = script_h.parseInt( &p_buf );

        const char *buf = script_h.readStr();
        setStr( &script_h.str_variables[ no ], buf );
    }
    else errorAndExit( script_h.getStringBuffer() );
    
    return RET_CONTINUE;
}

int ScriptParser::mode_sayaCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer(), "not in the define section" );
    mode_saya_flag = true;

    return RET_CONTINUE;
}

int ScriptParser::modCommand()
{
    script_h.readToken();
    char *p_buf = script_h.saveStringBuffer(), *p_buf2 = p_buf;

    int val1 = script_h.parseInt( &p_buf2 );
    int val2 = script_h.readInt();
    script_h.setInt( p_buf, val1 % val2 );

    return RET_CONTINUE;
}

int ScriptParser::midCommand()
{
    script_h.readToken();

    if ( script_h.getStringBuffer()[0] == '$'){
        char *p_buf = script_h.getStringBuffer()+1;
        int no = script_h.parseInt( &p_buf );
        
        const char *buf = script_h.readStr();
        int start = script_h.readInt();
        int len   = script_h.readInt();

        delete[] script_h.str_variables[ no ];
        script_h.str_variables[ no ] = new char[ len + 1 ];
        memcpy( script_h.str_variables[ no ], buf + start, len );
        script_h.str_variables[ no ][ len ] = '\0';
    }
    else errorAndExit( script_h.getStringBuffer(), "no string variable" );

    return RET_CONTINUE;
}

int ScriptParser::menusetwindowCommand()
{
    menu_font.closeFont();
    menu_font.font_size_xy[0] = script_h.readInt();
    menu_font.font_size_xy[1] = script_h.readInt();
    menu_font.pitch_xy[0]     = script_h.readInt() + menu_font.font_size_xy[0];
    menu_font.pitch_xy[1]     = script_h.readInt() + menu_font.font_size_xy[1];
    menu_font.is_bold         = script_h.readInt()?true:false;
    menu_font.is_shadow       = script_h.readInt()?true:false;

    const char *buf = script_h.readStr();
    if ( strlen( buf ) != 7 ) errorAndExit( buf, "menusetwindow: no color" );
    readColor( &menu_font.window_color, buf+1 );

    return RET_CONTINUE;
}

int ScriptParser::menuselectvoiceCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer(), "not in the define section" );

    for ( int i=0 ; i<MENUSELECTVOICE_NUM ; i++ ){
        const char *buf = script_h.readStr();
        if ( buf [0] != '\0' ){
            setStr( &menuselectvoice_file_name[i], buf );
        }
        else if ( menuselectvoice_file_name[i] ){
            delete menuselectvoice_file_name[i];
            menuselectvoice_file_name[i] = NULL;
        }
    }

    return RET_CONTINUE;
}

int ScriptParser::menuselectcolorCommand()
{
    int i;

    const char *buf = script_h.readStr();
    if ( buf[0] != '#' ) errorAndExit( script_h.getStringBuffer(), "no preceeding #" );
    readColor( &menu_font.on_color, buf+1 );
    for ( i=0 ; i<3 ; i++ ) system_font.on_color[i] = menu_font.on_color[i];

    buf = script_h.readStr();
    if ( buf[0] != '#' ) errorAndExit( script_h.getStringBuffer(), "no preceeding #" );
    readColor( &menu_font.off_color, buf+1 );
    for ( i=0 ; i<3 ; i++ ) system_font.off_color[i] = menu_font.off_color[i];
    
    buf = script_h.readStr();
    if ( buf[0] != '#' ) errorAndExit( script_h.getStringBuffer(), "no preceeding #" );
    readColor( &menu_font.nofile_color, buf+1 );
    for ( i=0 ; i<3 ; i++ ) system_font.nofile_color[i] = menu_font.nofile_color[i];
    
    return RET_CONTINUE;
}

int ScriptParser::lookbackspCommand()
{
    for ( int i=0 ; i<2 ; i++ )
        lookback_sp[i] = script_h.readInt();

    return RET_CONTINUE;
}

int ScriptParser::lookbackcolorCommand()
{
    const char *buf = script_h.readStr();
    if ( buf[0] != '#' ) errorAndExit( script_h.getStringBuffer(), "no preceeding #" );
    readColor( &lookback_color, buf+1 );

    return RET_CONTINUE;
}

int ScriptParser::lookbackbuttonCommand()
{
    for ( int i=0 ; i<4 ; i++ ){
        const char *buf = script_h.readStr();
        setStr( &lookback_image_name[i], buf );
    }
    return RET_CONTINUE;
}

int ScriptParser::linepageCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer(), "not in the define section" );
    script_h.setLinepage( true );

    return RET_CONTINUE;
}

int ScriptParser::lenCommand()
{
    script_h.readToken();
    char *p_buf = script_h.saveStringBuffer();
    
    const char *buf = script_h.readStr();

    script_h.setInt( p_buf, strlen( buf ) );

    return RET_CONTINUE;
}

int ScriptParser::labellogCommand()
{
    script_h.loadLabelLog();

    labellog_flag = true;
    return RET_CONTINUE;
}

int ScriptParser::kidokuskipCommand()
{
    kidokuskip_flag = true;
    kidokumode_flag = true;
    script_h.loadKidokuData();
    
    return RET_CONTINUE;
}

int ScriptParser::kidokumodeCommand()
{
    if ( script_h.readInt() == 1 )
        kidokumode_flag = true;
    else
        kidokumode_flag = false;

    return RET_CONTINUE;
}

int ScriptParser::itoaCommand()
{
    script_h.readToken();
    if ( script_h.getStringBuffer()[0] != '$' ) errorAndExit( script_h.getStringBuffer(), "no string variable" );

    char *p_buf = script_h.getStringBuffer() + 1;
    int no  = script_h.parseInt( &p_buf );
    int val = script_h.readInt();

    char val_str[20];
    sprintf( val_str, "%d", val );
    setStr( &script_h.str_variables[ no ], val_str );
    
    return RET_CONTINUE;
}

int ScriptParser::intlimitCommand()
{
    int no = script_h.readInt();

    script_h.num_limit_flag[ no ]  = true;
    script_h.num_limit_lower[ no ] = script_h.readInt();
    script_h.num_limit_upper[ no ] = script_h.readInt();

    return RET_CONTINUE;
}

int ScriptParser::incCommand()
{
    int val = script_h.readInt();
    script_h.setInt( script_h.getStringBuffer(), val + 1 );

    return RET_CONTINUE;
}

int ScriptParser::ifCommand()
{
    int left_value, right_value;
    bool if_flag, condition_flag = true, f = false;
    char *tmp_buffer;

    if ( script_h.isName( "notif" ) )
        if_flag = false;
    else
        if_flag = true;

    script_h.readToken();

    while(1){
        if ( script_h.getStringBuffer()[0] == 'f' ){ // fchk
            const char *buf = script_h.readStr();
            f = script_h.cBR->getAccessFlag( buf );
            //printf("fchk %s(%d,%d) ", tmp_string_buffer, cBR->getAccessFlag( tmp_string_buffer ), condition_flag );
        }
        else if ( script_h.getStringBuffer()[0] == 'l' ){ // lchk
            const char *buf = script_h.readStr();
            f = script_h.getLabelAccessFlag( buf+1 );
            //printf("lchk %s(%d,%d) ", tmp_string_buffer, getLabelAccessFlag( tmp_string_buffer+1 ), condition_flag );
        }
        else if ( script_h.getStringBuffer()[0] == '$' || script_h.isQuat() ){
            const char *buf = script_h.readStr( NULL, true ); // reread
            tmp_buffer = new char[ strlen( buf ) + 1 ];
            memcpy( tmp_buffer, buf, strlen( buf ) + 1 );
            
            script_h.readToken();
            const char *save_buf = script_h.saveStringBuffer();

            buf = script_h.readStr();

            int val = strcmp( tmp_buffer, buf );
            if      ( !strcmp( save_buf, ">=" ) ) f = (val >= 0);
            else if ( !strcmp( save_buf, "<=" ) ) f = (val <= 0);
            else if ( !strcmp( save_buf, "==" ) ) f = (val == 0);
            else if ( !strcmp( save_buf, "!=" ) ) f = (val != 0);
            else if ( !strcmp( save_buf, "<>" ) ) f = (val != 0);
            else if ( !strcmp( save_buf, "<" ) )  f = (val <  0);
            else if ( !strcmp( save_buf, ">" ) )  f = (val >  0);
            else if ( !strcmp( save_buf, "=" ) )  f = (val == 0);

            delete[] tmp_buffer;
        }
        else{
            left_value = script_h.readInt( true );
            //printf("left (%d) ", left_value );

            script_h.readToken();
            const char *save_buf = script_h.saveStringBuffer();
            //printf("op %s ", save_buf );

            right_value = script_h.readInt();
            //printf("right (%d) ", right_value );

            if      ( !strcmp( save_buf, ">=" ) ) f = (left_value >= right_value);
            else if ( !strcmp( save_buf, "<=" ) ) f = (left_value <= right_value);
            else if ( !strcmp( save_buf, "==" ) ) f = (left_value == right_value);
            else if ( !strcmp( save_buf, "!=" ) ) f = (left_value != right_value);
            else if ( !strcmp( save_buf, "<>" ) ) f = (left_value != right_value);
            else if ( !strcmp( save_buf, "<" ) )  f = (left_value <  right_value);
            else if ( !strcmp( save_buf, ">" ) )  f = (left_value >  right_value);
            else if ( !strcmp( save_buf, "=" ) )  f = (left_value == right_value);
        }
        condition_flag &= (if_flag)?(f):(!f);

        script_h.readToken();
        if ( script_h.getStringBuffer()[0] == '&' ){
            script_h.readToken();
            continue;
        }
        break;
    };

    /* Execute command */
    if ( condition_flag )
        return RET_CONTINUE_NOREAD;
    else
        return RET_SKIP_LINE;
}

int ScriptParser::humanzCommand()
{
    z_order = script_h.readInt();
    
    return RET_CONTINUE;
}

int ScriptParser::gotoCommand()
{
    const char *buf = script_h.readStr();

    current_link_label_info->label_info = script_h.lookupLabel( buf+1 );
    current_link_label_info->current_line = 0;
    script_h.setCurrent( current_link_label_info->label_info.start_address );
    string_buffer_offset = 0;
    
    return RET_JUMP;
}

void ScriptParser::gosubReal( const char *label, bool textgosub_flag, char *current )
{
    label_stack_depth++;
    
    current_link_label_info->current_script = current;
    current_link_label_info->string_buffer_offset = string_buffer_offset;

    LinkLabelInfo *info = new LinkLabelInfo();
    info->previous = current_link_label_info;
    info->next = NULL;
    info->label_info = script_h.lookupLabel( label );
    info->current_line = 0;
    info->textgosub_flag = textgosub_flag;

    script_h.setCurrent( info->label_info.start_address );
    string_buffer_offset = 0;
    
    current_link_label_info->next = info;
    current_link_label_info = current_link_label_info->next;
}

int ScriptParser::gosubCommand()
{
    const char *buf = script_h.readStr();
    string_buffer_offset = 0;
    gosubReal( buf+1, false, script_h.getNext() );

    return RET_JUMP;
}

int ScriptParser::globalonCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer(), "not in the define section" );

    globalon_flag = true;
    return RET_CONTINUE;
}

int ScriptParser::forCommand()
{
    for_stack_depth++;
    ForInfo *info = new ForInfo();

    script_h.readToken();
    setStr( &info->p_int, script_h.getStringBuffer() );
    
    script_h.readToken();
    if ( script_h.getStringBuffer()[0] != '=' ) errorAndExit( script_h.getStringBuffer(), "for: no =" );

    script_h.setInt( info->p_int, script_h.readInt() );
    
    const char *buf = script_h.readStr();
    if ( strcmp( buf, "to" ) ) errorAndExit( script_h.getStringBuffer(), "for: no to" );
    
    info->to = script_h.readInt();

    script_h.readToken();
    if ( !strncmp( script_h.getStringBuffer(), "step", 4 ) ){
        info->step = script_h.readInt();
        script_h.readToken();
    }
    else{
        info->step = 1;
    }
    
    /* ---------------------------------------- */
    /* Step forward callee's label info */
    info->label_info = current_link_label_info->label_info;
    info->current_line = current_link_label_info->current_line;
    info->current_script = script_h.getCurrent();

    info->previous = current_for_link;
    current_for_link->next = info;
    current_for_link = current_for_link->next;

    //printf("stack %d forCommand %d = %d to %d step %d\n", for_stack_depth,
    //info->var_no, *info->p_var, info->to, info->step );

    return RET_CONTINUE_NOREAD;
}

int ScriptParser::filelogCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer() );

    filelog_flag = true;
    return RET_CONTINUE;
}

int ScriptParser::effectcutCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer() );
    
    effect_cut_flag = true;

    return RET_CONTINUE;
}

int ScriptParser::effectblankCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer() );
    
    effect_blank = script_h.readInt();

    return RET_CONTINUE;
}

int ScriptParser::effectCommand()
{
    EffectLink *elink;

    if ( script_h.isName( "windoweffect") )
    {
        elink = &window_effect;
    }
    else{
        if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer(), "not in the define section" );

        elink = new EffectLink();
        elink->num = script_h.readInt();

        last_effect_link->next = elink;
        last_effect_link = last_effect_link->next;
    }
    
    readEffect( elink );

    return RET_CONTINUE;
}

int ScriptParser::divCommand()
{
    int val1 = script_h.readInt();
    char *save_buf = script_h.saveStringBuffer();

    int val2 = script_h.readInt();
    script_h.setInt( save_buf, val1 / val2 );

    return RET_CONTINUE;
}

int ScriptParser::dimCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer(), "not in the define section" );

    struct ScriptHandler::ArrayVariable array;
    int dim = 1;

    script_h.readToken();
    char *p_buf = script_h.getCurrent();
    int no = script_h.decodeArraySub( &p_buf, &array );

    script_h.array_variables[ no ].num_dim = array.num_dim;
    //printf("dimCommand no=%d dim=%d\n",no,array.num_dim);
    for ( int i=0 ; i<array.num_dim ; i++ ){
        //printf("%d ",array.dim[i]);
        script_h.array_variables[ no ].dim[i] = array.dim[i]+1;
        dim *= (array.dim[i]+1);
        script_h.array_variables[ no ].data = new int[ dim ];
        memset( script_h.array_variables[ no ].data, 0, sizeof(int) * dim );
    }
    skipToken();
    
    return RET_CONTINUE;
}

int ScriptParser::defvoicevolCommand()
{
    voice_volume = script_h.readInt();

    return RET_CONTINUE;
}

int ScriptParser::defsevolCommand()
{
    se_volume = script_h.readInt();

    return RET_CONTINUE;
}

int ScriptParser::defmp3volCommand()
{
    mp3_volume = script_h.readInt();

    return RET_CONTINUE;
}

int ScriptParser::defaultspeedCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer(), "not in the define section" );

    for ( int i=0 ; i<3 ; i++ ) default_text_speed[i] = script_h.readInt();

    return RET_CONTINUE;
}

int ScriptParser::decCommand()
{
    int val = script_h.readInt();
    script_h.setInt( script_h.getStringBuffer(), val - 1 );

    return RET_CONTINUE;
}

int ScriptParser::dateCommand()
{
    time_t t = time(NULL);
    struct tm *tm = localtime( &t );

    script_h.readToken();
    script_h.setInt( script_h.getStringBuffer(), tm->tm_year + 1900 );

    script_h.readToken();
    script_h.setInt( script_h.getStringBuffer(), tm->tm_mon + 1 );

    script_h.readToken();
    script_h.setInt( script_h.getStringBuffer(), tm->tm_mday );

    return RET_CONTINUE;
}

int ScriptParser::cmpCommand()
{
    script_h.readToken();
    char *save_buf = script_h.saveStringBuffer();

    const char *buf = script_h.readStr();
    char *tmp_buffer = new char[ strlen( buf ) + 1 ];
    strcpy( tmp_buffer, buf );

    buf = script_h.readStr();

    int val = strcmp( tmp_buffer, buf );
    if      ( val > 0 ) val = 1;
    else if ( val < 0 ) val = -1;
    script_h.setInt( save_buf, val );

    delete[] tmp_buffer;
    
    return RET_CONTINUE;
}

int ScriptParser::clickvoiceCommand()
{
    if ( current_mode != DEFINE_MODE ) errorAndExit( script_h.getStringBuffer(), "not in the define section" );

    for ( int i=0 ; i<CLICKVOICE_NUM ; i++ ){
        const char *buf = script_h.readStr();
        if ( buf[0] != '\0' ){
            setStr( &clickvoice_file_name[i], buf );
        }
        else if ( clickvoice_file_name[i] ){
            delete clickvoice_file_name[i];
            clickvoice_file_name[i] = NULL;
        }
    }

    return RET_CONTINUE;
}

int ScriptParser::clickstrCommand()
{
    const char *buf = script_h.readStr();

    clickstr_num = strlen( buf ) / 2;
    clickstr_list = new char[clickstr_num * 2];
    for ( int i=0 ; i<clickstr_num*2 ; i++ ) clickstr_list[i] = buf[i];

    clickstr_line = script_h.readInt();
           
    return RET_CONTINUE;
}

int ScriptParser::breakCommand()
{
    const char *buf = script_h.readStr();
    if ( buf[0] == '*' ){
        current_link_label_info->label_info = script_h.lookupLabel( buf+1 );
        current_link_label_info->current_line = 0;
        script_h.setCurrent( current_link_label_info->label_info.start_address );
        string_buffer_offset = 0;
        
        return RET_JUMP;
    }
    else{
        break_flag = true;
        return RET_CONTINUE;
    }
}

int ScriptParser::atoiCommand()
{
    script_h.readToken();
    char *save_buf = script_h.saveStringBuffer();
    
    const char *buf = script_h.readStr();
        
    script_h.setInt( save_buf, atoi( buf ) );
    
    return RET_CONTINUE;
}

int ScriptParser::arcCommand()
{
    const char *buf = script_h.readStr();
    char *buf2 = new char[ strlen( buf ) + 1 ];
    strcpy( buf2, buf );

    int i = 0;
    while ( buf2[i] != '|' && buf2[i] != '\0' ) i++;
    buf2[i] = '\0';

    if ( strcmp( script_h.cBR->getArchiveName(), "sar" ) ){
        delete script_h.cBR;
        script_h.cBR = new SarReader( archive_path );
    }
    if ( script_h.cBR->open( buf2 ) ){
        fprintf( stderr, " *** failed to open archive %s ...  ***\n", buf2 );
        exit(-1);
    }
    return RET_CONTINUE;
}

int ScriptParser::addCommand()
{
    char *tmp_buffer;
    int no;

    script_h.readToken();
    char *save_buf = script_h.saveStringBuffer();
    
    if ( script_h.getStringBuffer()[0] == '%' || script_h.getStringBuffer()[0] == '?' ){
        char *p_buf = script_h.getStringBuffer();
        no = script_h.parseInt( &p_buf );

        script_h.setInt( save_buf, no + script_h.readInt() );
    }
    else if ( script_h.getStringBuffer()[0] == '$'){
        char *p_buf = script_h.getStringBuffer()+1;
        no = script_h.parseInt( &p_buf );

        const char *buf = script_h.readStr();
        tmp_buffer = script_h.str_variables[ no ];

        script_h.str_variables[ no ] = new char[ strlen( tmp_buffer ) + strlen( buf ) + 1 ];
        strcpy( script_h.str_variables[ no ], tmp_buffer );
        strcat( script_h.str_variables[ no ], buf );

        delete[] tmp_buffer;
    }
    else errorAndExit( "add: no variable." );

    return RET_CONTINUE;
}
