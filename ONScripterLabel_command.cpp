/* -*- C++ -*-
 * 
 *  ONScripterLabel_command.cpp - Command executer of ONScripter
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

#include "ONScripterLabel.h"

#define ONSCRIPTER_VERSION 198

#define DEFAULT_CURSOR_WAIT    ":l/3,160,2;cursor0.bmp"
#define DEFAULT_CURSOR_NEWPAGE ":l/3,160,2;cursor1.bmp"

#define CONTINUOUS_PLAY

int ONScripterLabel::waveCommand()
{
    if ( script_h.isName( "waveloop" ) ){
        wave_play_once_flag = true;
    }
    else{
        wave_play_once_flag = false;
    }

    wavestopCommand();

    const char *buf = script_h.readStr();
    playWave( buf, wave_play_once_flag, DEFAULT_WAVE_CHANNEL );
        
    return RET_CONTINUE;
}

int ONScripterLabel::wavestopCommand()
{
    if ( wave_sample[DEFAULT_WAVE_CHANNEL] ){
        Mix_Pause( DEFAULT_WAVE_CHANNEL );
        Mix_FreeChunk( wave_sample[DEFAULT_WAVE_CHANNEL] );
        wave_sample[DEFAULT_WAVE_CHANNEL] = NULL;
    }

    return RET_CONTINUE;
}

int ONScripterLabel::waittimerCommand()
{
    int count = script_h.readInt() + internal_timer - SDL_GetTicks();
    startTimer( count );
    
    return RET_WAIT_NEXT;
}

int ONScripterLabel::waitCommand()
{
    startTimer( script_h.readInt() );

    return RET_WAIT_NEXT;
}

int ONScripterLabel::vspCommand()
{
    int no = script_h.readInt();
    int v  = script_h.readInt();
    sprite_info[ no ].valid = (v==1)?true:false;
    
    return RET_CONTINUE;
}

int ONScripterLabel::voicevolCommand()
{
    voice_volume = script_h.readInt();

    if ( wave_sample[0] ) Mix_Volume( 0, se_volume * 128 / 100 );
    
    return RET_CONTINUE;
}

int ONScripterLabel::trapCommand()
{
    const char *buf = script_h.readStr();

    if ( buf[0] == '*' ){
        trap_flag = true;
        if ( trap_dist ) delete[] trap_dist;
        trap_dist = new char[ strlen( buf ) ];
        memcpy( trap_dist, buf+1, strlen( buf ) );
    }
    else if ( !strcmp( buf, "off" ) ){
        trap_flag = false;
    }
    else{
        printf("trapCommand: [%s] is not supported\n", buf );
    }
              
    return RET_CONTINUE;
}

int ONScripterLabel::textspeedCommand()
{
    sentence_font.wait_time = script_h.readInt();

    return RET_CONTINUE;
}

int ONScripterLabel::textonCommand()
{
    text_on_flag = true;
    if ( !(display_mode & TEXT_DISPLAY_MODE) ){
        SDL_BlitSurface( accumulation_surface, NULL, text_surface, NULL );
        restoreTextBuffer();
        flush();
        display_mode = TEXT_DISPLAY_MODE;
    }

    return RET_CONTINUE;
}

int ONScripterLabel::textoffCommand()
{
    text_on_flag = false;
    if ( display_mode & TEXT_DISPLAY_MODE ){
        SDL_BlitSurface( accumulation_surface, NULL, text_surface, NULL );
        flush();
        display_mode = NORMAL_DISPLAY_MODE;
    }

    return RET_CONTINUE;
}

int ONScripterLabel::textclearCommand()
{
    newPage( false );
    return RET_CONTINUE;
}

int ONScripterLabel::texecCommand()
{
    if ( clickstr_state == CLICK_NEWPAGE ){
        new_line_skip_flag = true;
        newPage( true );
    }

    return RET_CONTINUE;
}

int ONScripterLabel::talCommand()
{
    const char *buf = script_h.readStr();
    char loc = buf[0];
    int trans = script_h.readInt();

    if ( event_mode & EFFECT_EVENT_MODE ){
        int num = readEffect( &tmp_effect );
        if ( num > 1 ) return doEffect( TMP_EFFECT, NULL, TACHI_EFFECT_IMAGE );
        else           return doEffect( tmp_effect.effect, NULL, TACHI_EFFECT_IMAGE );
    }
    else{
        int no;
        if      ( loc == 'l' ) no = 0;
        else if ( loc == 'c' ) no = 1;
        else if ( loc == 'r' ) no = 2;

        if      ( trans > 255 ) trans = 255;
        else if ( trans < 0   ) trans = 0;

        tachi_info[ no ].trans = trans;

        readEffect( &tmp_effect );
        return setEffect( tmp_effect.effect );
   }
}

int ONScripterLabel::tablegotoCommand()
{
    int count = 0;
    int no = script_h.readInt();

    while( script_h.end_with_comma_flag ){
        const char *buf = script_h.readStr();
        if ( count++ == no ){
            current_link_label_info->label_info = script_h.lookupLabel( buf+1 );
            current_link_label_info->current_line = 0;
            script_h.setCurrent( current_link_label_info->label_info.start_address );
    
            return RET_JUMP;
        }
    }

    return RET_CONTINUE;
}

int ONScripterLabel::systemcallCommand()
{
    const char *buf = script_h.readStr();
    system_menu_mode = getSystemCallNo( buf );
    event_mode = WAIT_SLEEP_MODE;

    enterSystemCall();
    
    advancePhase();
    return RET_WAIT_NEXT;
}

int ONScripterLabel::stopCommand()
{
    wavestopCommand();
    stopBGM( false );
    
    return RET_CONTINUE;
}

int ONScripterLabel::spstrCommand()
{
    const char *buf = script_h.readStr();
    decodeExbtnControl( NULL, buf, false, true );
    
    return RET_CONTINUE;
}

int ONScripterLabel::spbtnCommand()
{
    int sprite_no = script_h.readInt();
    int no        = script_h.readInt();

    if ( sprite_info[ sprite_no ].num_of_cells == 0 ) return RET_CONTINUE;

    last_button_link->next = new ButtonLink();
    last_button_link = last_button_link->next;

    last_button_link->button_type = SPRITE_BUTTON;
    last_button_link->sprite_no   = sprite_no;
    last_button_link->no          = no;

    if ( sprite_info[ sprite_no ].image_surface || sprite_info[ sprite_no ].trans_mode == AnimationInfo::TRANS_STRING ){
        last_button_link->image_rect = last_button_link->select_rect = sprite_info[ last_button_link->sprite_no ].pos;
        sprite_info[ sprite_no ].valid = true;
    }

    return RET_CONTINUE;
}

int ONScripterLabel::skipoffCommand() 
{ 
    skip_flag = false; 
 
    return RET_CONTINUE; 
} 

int ONScripterLabel::sevolCommand()
{
    se_volume = script_h.readInt();

    for ( int i=1 ; i<MIX_CHANNELS ; i++ )
        if ( wave_sample[i] ) Mix_Volume( i, se_volume * 128 / 100 );
        
    return RET_CONTINUE;
}

int ONScripterLabel::setwindow2Command()
{
    const char *buf = script_h.readStr();
    if ( buf[0] == '#' ){
        sentence_font.is_transparent = true;
        readColor( &sentence_font.window_color, buf+1 );
    }
    else{
        sentence_font.is_transparent = false;
        sentence_font_info.setImageName( buf );
        parseTaggedString( &sentence_font_info );
        setupAnimationInfo( &sentence_font_info );
    }
    repaintCommand();

    return RET_CONTINUE;
}

int ONScripterLabel::setwindowCommand()
{
    sentence_font.top_xy[0] = script_h.readInt();
    sentence_font.top_xy[1] = script_h.readInt();
    sentence_font.num_xy[0] = script_h.readInt();
    sentence_font.num_xy[1] = script_h.readInt();
    sentence_font.font_size_xy[0] = script_h.readInt();
    sentence_font.font_size_xy[1] = script_h.readInt();
    sentence_font.pitch_xy[0] = script_h.readInt() + sentence_font.font_size_xy[0];
    sentence_font.pitch_xy[1] = script_h.readInt() + sentence_font.font_size_xy[1];
    sentence_font.wait_time = script_h.readInt();
    sentence_font.is_bold = script_h.readInt()?true:false;
    sentence_font.is_shadow = script_h.readInt()?true:false;

    sentence_font.openFont( font_file, screen_ratio1, screen_ratio2 );

    const char *buf = script_h.readStr();
    if ( buf[0] == '#' ){
        sentence_font.is_transparent = true;
        readColor( &sentence_font.window_color, buf + 1 );

        sentence_font_info.pos.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
        sentence_font_info.pos.y = script_h.readInt() * screen_ratio1 / screen_ratio2;
        sentence_font_info.pos.w = script_h.readInt() * screen_ratio1 / screen_ratio2 - sentence_font_info.pos.x + 1;
        sentence_font_info.pos.h = script_h.readInt() * screen_ratio1 / screen_ratio2 - sentence_font_info.pos.y + 1;
    }
    else{
        sentence_font.is_transparent = false;
        sentence_font_info.setImageName( buf );
        parseTaggedString( &sentence_font_info );
        setupAnimationInfo( &sentence_font_info );
        sentence_font_info.pos.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
        sentence_font_info.pos.y = script_h.readInt() * screen_ratio1 / screen_ratio2;
    }

    lookbackflushCommand();
    clearCurrentTextBuffer();
    display_mode = NORMAL_DISPLAY_MODE;
    
    return RET_CONTINUE;
}

int ONScripterLabel::setcursorCommand()
{
    bool abs_flag;

    if ( script_h.isName( "abssetcursor" ) ){
        abs_flag = true;
    }
    else{
        abs_flag = false;
    }
    
    int no = script_h.readInt();
    const char* buf = script_h.readStr();
    int x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    int y = script_h.readInt() * screen_ratio1 / screen_ratio2;

    loadCursor( no, buf, x, y, abs_flag );
    
    return RET_CONTINUE;
}

int ONScripterLabel::selectCommand()
{
    int ret = enterTextDisplayMode( RET_WAIT_NOREAD );
    if ( ret != RET_NOMATCH ) return ret;

    int xy[2];
    int select_mode;
    SelectLink *last_select_link;
    char *p_buf, *save_buf;

    if ( script_h.isName( "selnum" ) ){
        select_mode = SELECT_NUM_MODE;
        p_buf = script_h.getCurrent();
        script_h.readToken();
        save_buf = script_h.saveStringBuffer();
    }
    else if ( script_h.isName( "selgosub" ) ){
        select_mode = SELECT_GOSUB_MODE;
    }
    else if ( script_h.isName( "select" ) ){
        select_mode = SELECT_GOTO_MODE;
    }
    else if ( script_h.isName( "csel" ) ){
        select_mode = SELECT_CSEL_MODE;
    }

    bool first_token_flag = true;
    int count = 0;

    if ( event_mode & (WAIT_INPUT_MODE | WAIT_BUTTON_MODE) ){

        if ( current_button_state.button == 0 ) return RET_WAIT;
        
        if ( selectvoice_file_name[SELECTVOICE_SELECT] )
            playWave( selectvoice_file_name[SELECTVOICE_SELECT], false, DEFAULT_WAVE_CHANNEL );

        event_mode = IDLE_EVENT_MODE;

        deleteButtonLink();

        int counter = 1;
        last_select_link = root_select_link.next;
        while ( last_select_link ){
            if ( current_button_state.button == counter++ ) break;
            last_select_link = last_select_link->next;
        }

        if ( select_mode  == SELECT_GOTO_MODE ){
            current_link_label_info->label_info = script_h.lookupLabel( last_select_link->label );
            current_link_label_info->current_line = 0;
            script_h.setCurrent( current_link_label_info->label_info.start_address );
            ret = RET_JUMP;
        }
        else if ( select_mode == SELECT_GOSUB_MODE ){
            current_link_label_info->current_line = select_label_info.current_line;
            gosubReal( last_select_link->label, false, select_label_info.current_script );
            ret = RET_JUMP;
        }
        else{
            script_h.setInt( save_buf, current_button_state.button - 1 );
            ret = RET_CONTINUE;
        }
        deleteSelectLink();

        newPage( true );

        return ret;
    }
    else{
        bool comma_flag = true;
        if ( select_mode == SELECT_CSEL_MODE ){
            shelter_soveon_flag = saveon_flag;
            saveoffCommand();
        }
        SelectLink *link;
        shortcut_mouse_line = -1;
        flush();
        skip_flag = false;
        xy[0] = sentence_font.xy[0];
        xy[1] = sentence_font.xy[1];

        if ( selectvoice_file_name[SELECTVOICE_OPEN] )
            playWave( selectvoice_file_name[SELECTVOICE_OPEN], false, DEFAULT_WAVE_CHANNEL );

        last_select_link = &root_select_link;
        select_label_info.current_line = current_link_label_info->current_line;
        select_label_info.current_script = script_h.getCurrent();
        const char *buf = script_h.readStr();
        while(1){
            //printf("sel [%s] comma %d\n", buf, comma_flag  );
            if ( buf[0] != 0x0a ){
                comma_flag = script_h.end_with_comma_flag;
                first_token_flag = false;
                count++;
                if ( select_mode == SELECT_NUM_MODE || count % 2 ){
                    if ( select_mode != SELECT_NUM_MODE && !comma_flag ) errorAndExit( script_h.getStringBuffer(), "select: comma is needed here" );
                    link = new SelectLink();
                    setStr( &link->text, buf );
                    //printf("Select text %s\n", link->text);
                }
                if ( select_mode == SELECT_NUM_MODE || !(count % 2) ){
                    setStr( &link->label, buf+1 );
                    //printf("Select label %s\n", link->label );
                    last_select_link->next = link;
                    last_select_link = last_select_link->next;
                }
                select_label_info.current_script = script_h.getCurrent();
                buf = script_h.readStr();
            }
            else{
                //if ( first_token_flag ) comma_flag = true;
                if ( (count & 1) == 1 ) errorAndExit( script_h.getStringBuffer(), "select: label must be in the same line." );
                //if ( (count & 1) == 0 && !comma_flag ) errorAndExit( script_h.getStringBuffer(), "select: comma is neede here." );
                do{
                    if ( buf[0] == 0x0a ) select_label_info.current_line++;
                    select_label_info.current_script = script_h.getCurrent();
                    buf = script_h.readStr();
                    if ( buf[0] == ',' ){
                        if ( comma_flag ) errorAndExit( script_h.getStringBuffer(), "double comma" );
                        else comma_flag = true;
                    }
                } while ( buf[0] == 0x0a || buf[0] == ',' );

                if ( !comma_flag ) break;
            }
            
            if ( first_token_flag ) first_token_flag = false;
        }

        if ( select_mode != SELECT_CSEL_MODE ){
            last_select_link = root_select_link.next;
            int counter = 1;
            while( last_select_link ){
                if ( *last_select_link->text ){
                    last_button_link->next = getSelectableSentence( last_select_link->text, &sentence_font );
                    last_button_link = last_button_link->next;
                    last_button_link->no = counter;
                }
                counter++;
                last_select_link = last_select_link->next;
            }
            SDL_BlitSurface( text_surface, NULL, select_surface, NULL );
        }

        if ( select_mode == SELECT_GOTO_MODE || select_mode == SELECT_CSEL_MODE ){ /* Resume */
            //p_script_buffer = current_link_label_info->current_script;
            //script_h.setCurrent( p_script_buffer );
            //script_h.readToken();
            //readLine( &p_script_buffer );

            if ( select_mode == SELECT_CSEL_MODE ){
                current_link_label_info->label_info = script_h.lookupLabel( "customsel" );
                current_link_label_info->current_line = 0;
                script_h.setCurrent( current_link_label_info->label_info.start_address );

                return RET_JUMP;
            }
        }
        sentence_font.xy[0] = xy[0];
        sentence_font.xy[1] = xy[1];

        event_mode = WAIT_INPUT_MODE | WAIT_BUTTON_MODE;
        refreshMouseOverButton();

        return RET_WAIT;
    }
}

int ONScripterLabel::savetimeCommand()
{
    int no = script_h.readInt() - 1;

    script_h.readToken();
    if ( no >= MAX_SAVE_FILE ){
        script_h.setInt( script_h.getStringBuffer(), 0 );
        for ( int i=0 ; i<4 ; i++ )
            script_h.readToken();
        return RET_CONTINUE;
    }
    
    searchSaveFiles( no );

    if ( !save_file_info[no].valid ){
        script_h.setInt( script_h.getStringBuffer(), 0 );
        for ( int i=0 ; i<4 ; i++ )
            script_h.readToken();
        return RET_CONTINUE;
    }

    script_h.setInt( script_h.getStringBuffer(), save_file_info[no].month );
    script_h.readToken();
    script_h.setInt( script_h.getStringBuffer(), save_file_info[no].day );
    script_h.readToken();
    script_h.setInt( script_h.getStringBuffer(), save_file_info[no].hour );
    script_h.readToken();
    script_h.setInt( script_h.getStringBuffer(), save_file_info[no].minute );
    script_h.readToken();

    return RET_CONTINUE;
}

int ONScripterLabel::saveonCommand()
{
    saveon_flag = true;

    return RET_CONTINUE;
}

int ONScripterLabel::saveoffCommand()
{
    if ( saveon_flag )
        saveSaveFile( -1 );
    
    saveon_flag = false;

    return RET_CONTINUE;
}

int ONScripterLabel::savegameCommand()
{
    int no = script_h.readInt();

    if ( no < 0 )
        errorAndExit( script_h.getStringBuffer(), "savegame: the specified number is less than 0" );
    else{
        shelter_event_mode = event_mode;
        saveSaveFile( no );
    }

    return RET_CONTINUE;
}

int ONScripterLabel::savefileexistCommand()
{
    script_h.readToken();
    char *save_buf = script_h.saveStringBuffer();
    int no = script_h.readInt();

    searchSaveFiles( no );

    int val;
    if ( save_file_info[no].valid )
        val = 1;
    else
        val = 0;

    script_h.setInt( save_buf, val );

    return RET_CONTINUE;
}

int ONScripterLabel::rndCommand()
{
    int  upper, lower;
    char *save_buf;
    
    if ( script_h.isName( "rnd2" ) ){
        script_h.readToken();
        save_buf = script_h.saveStringBuffer();
        
        lower = script_h.readInt();
        upper = script_h.readInt();
    }
    else{
        script_h.readToken();
        save_buf = script_h.saveStringBuffer();

        lower = 0;
        upper = script_h.readInt() - 1;
    }

    script_h.setInt( save_buf, lower + (int)( (double)(upper-lower+1)*rand()/(RAND_MAX+1.0)) );

    return RET_CONTINUE;
}

int ONScripterLabel::rmodeCommand()
{
    if ( script_h.readInt() == 1 ) rmode_flag = true;
    else                           rmode_flag = false;

    return RET_CONTINUE;
}

int ONScripterLabel::resettimerCommand()
{
    internal_timer = SDL_GetTicks();
    return RET_CONTINUE;
}

int ONScripterLabel::resetCommand()
{
    int i;

    for ( i=0 ; i<199 ; i++ ){
        script_h.num_variables[i] = 0;
        if ( script_h.str_variables[i] ) delete[] script_h.str_variables[i];
        script_h.str_variables[i] = NULL;
    }

    sentence_font.xy[0] = 0;
    sentence_font.xy[1] = 0;
    resetSentenceFont();
    if ( sentence_font.openFont( font_file, screen_ratio1, screen_ratio2 ) == NULL ){
        fprintf( stderr, "can't open font file: %s\n", font_file );
        SDL_Quit();
        exit(-1);
    }
    
    text_char_flag = false;
    skip_flag      = false;
    monocro_flag   = false;
    nega_mode      = 0;
    saveon_flag    = true;
    clickstr_state = CLICK_NONE;
    
    deleteLabelLink();
    current_link_label_info->label_info = script_h.lookupLabel( "start" );
    current_link_label_info->current_line = 0;
    script_h.setCurrent( current_link_label_info->label_info.start_address );
    string_buffer_offset = 0;
    
    barclearCommand();
    prnumclearCommand();
    for ( i=0 ; i<MAX_SPRITE_NUM ; i++ ){
        sprite_info[i].remove();
    }

    deleteButtonLink();
    deleteSelectLink();

    wavestopCommand();
    stopBGM( false );

    SDL_FillRect( background_surface, NULL, SDL_MapRGBA( background_surface->format, 0, 0, 0, 0 ) );
    SDL_BlitSurface( background_surface, NULL, accumulation_surface, NULL );
    SDL_BlitSurface( accumulation_surface, NULL, text_surface, NULL );

    return RET_JUMP;
}

int ONScripterLabel::repaintCommand()
{
    refreshSurface( accumulation_surface, NULL, (display_mode&TEXT_DISPLAY_MODE)?REFRESH_SHADOW_MODE:REFRESH_NORMAL_MODE );
    SDL_BlitSurface( accumulation_surface, NULL, text_surface, NULL );
    restoreTextBuffer();
    flush();
    
    return RET_CONTINUE;
}

int ONScripterLabel::quakeCommand()
{
    int quake_type;

    if      ( script_h.isName( "quakey" ) ){
        quake_type = 0;
    }
    else if ( script_h.isName( "quakex" ) ){
        quake_type = 1;
    }
    else{
        quake_type = 2;
    }

    tmp_effect.num      = script_h.readInt();
    tmp_effect.duration = script_h.readInt();
    if ( tmp_effect.duration < tmp_effect.num * 4 ) tmp_effect.duration = tmp_effect.num * 4;
    
    if ( event_mode & EFFECT_EVENT_MODE ){
        if ( effect_counter == 0 ){
            SDL_BlitSurface( text_surface, NULL, effect_src_surface, NULL );
            SDL_BlitSurface( text_surface, NULL, effect_dst_surface, NULL );
        }
        tmp_effect.effect = CUSTOM_EFFECT_NO + quake_type;
        return doEffect( TMP_EFFECT, NULL, DIRECT_EFFECT_IMAGE );
    }
    else{
        setEffect( 2 ); // 2 is dummy value
        return RET_WAIT; // RET_WAIT de yoi?
        //return RET_WAIT_NEXT;
    }
}

int ONScripterLabel::puttextCommand()
{
    int ret = enterTextDisplayMode();
    if ( ret != RET_NOMATCH ) return ret;

    script_h.readToken();
    string_buffer_offset = 0;
    script_h.text_line_flag = true;
    script_h.next_text_line_flag = true;

    return RET_CONTINUE_NOREAD;
}

int ONScripterLabel::prnumclearCommand()
{
    for ( int i=0 ; i<MAX_PARAM_NUM ; i++ ) {
        if ( prnum_info[i] ) {
            delete prnum_info[i];
            prnum_info[i] = NULL;
        }
    }
    return RET_CONTINUE;
}

int ONScripterLabel::prnumCommand()
{
    int no = script_h.readInt();
    if ( prnum_info[no] ) delete prnum_info[no];
    prnum_info[no] = new AnimationInfo();
    prnum_info[no]->trans_mode = AnimationInfo::TRANS_STRING;
    prnum_info[no]->abs_flag = true;
    prnum_info[no]->num_of_cells = 1;
    prnum_info[no]->current_cell = 0;
    prnum_info[no]->color_list = new uchar3[ prnum_info[no]->num_of_cells ];
    
    int param = script_h.readInt();
    prnum_info[no]->pos.x = script_h.readInt();
    prnum_info[no]->pos.y = script_h.readInt();
    prnum_info[no]->font_size_xy[0] = script_h.readInt();
    prnum_info[no]->font_size_xy[1] = script_h.readInt();

    const char *buf = script_h.readStr();
    if ( buf[0] != '#' ) errorAndExit( script_h.getStringBuffer(), "prnum: Color is not specified." );
    readColor( &prnum_info[no]->color_list[0], buf+1 );

    char num_buf[12], buf2[7];
    int ptr = 0;

    sprintf( num_buf, "%3d", param );
    if ( param<0 ){
        if ( param>-10 ) {
            buf2[ptr++] = "�@"[0];
            buf2[ptr++] = "�@"[1];
        }
        buf2[ptr++] = "�|"[0];
        buf2[ptr++] = "�|"[1];
        sprintf( num_buf, "%d", -param );
    }
    for ( int i=0 ; i<(int)strlen( num_buf ) ; i++ ){
        if ( num_buf[i] == ' ' ) {
            buf2[ptr++] = "�@"[0];
            buf2[ptr++] = "�@"[1];
            continue;
        }
        getSJISFromInteger( &buf2[ptr], num_buf[i] - '0', false );
        ptr += 2;
        if ( ptr >= 6 ) break; // up to 3 columns (NScripter's restriction)
    }
    setStr( &prnum_info[no]->file_name, buf2 );

    setupAnimationInfo( prnum_info[no] );

    return RET_CONTINUE;
}

int ONScripterLabel::printCommand()
{
    if ( event_mode & EFFECT_EVENT_MODE )
    {
        int num = readEffect( &tmp_effect );
        if ( num > 1 ) return doEffect( TMP_EFFECT, NULL, TACHI_EFFECT_IMAGE );
        else           return doEffect( tmp_effect.effect, NULL, TACHI_EFFECT_IMAGE );
    }
    else{
        readEffect( &tmp_effect );
        return setEffect( tmp_effect.effect );
    }
}

int ONScripterLabel::playstopCommand()
{
    stopBGM( false );
    return RET_CONTINUE;
}

int ONScripterLabel::playCommand()
{
    if ( script_h.isName( "playonce" ) )
        music_play_once_flag = true;
    else
        music_play_once_flag = false;

    const char *buf = script_h.readStr();

    if ( buf[0] == '*' ){
        int new_cd_track = atoi( buf + 1 );
#ifdef CONTINUOUS_PLAY        
        if ( current_cd_track != new_cd_track ) {
#endif        
            stopBGM( false );
            current_cd_track = new_cd_track;

            if ( cdaudio_flag ){
                if ( cdrom_info ) playCDAudio( current_cd_track );
            }
            else{
                playMP3( current_cd_track );
            }
#ifdef CONTINUOUS_PLAY        
        }
#endif
    }
    else{ // play MIDI
        stopBGM( false );
        
        setStr( &music_file_name, buf );
        playMIDIFile();
    }

    return RET_CONTINUE;
}

int ONScripterLabel::ofscpyCommand()
{
    SDL_BlitSurface( screen_surface, NULL, text_surface, NULL );

    return RET_CONTINUE;
}

int ONScripterLabel::negaCommand()
{
    nega_mode = script_h.readInt();
    need_refresh_flag = true;

    return RET_CONTINUE;
}

int ONScripterLabel::mspCommand()
{
    int no = script_h.readInt();
    sprite_info[ no ].pos.x += script_h.readInt() * screen_ratio1 / screen_ratio2;
    sprite_info[ no ].pos.y += script_h.readInt() * screen_ratio1 / screen_ratio2;
    sprite_info[ no ].trans += script_h.readInt();
    if ( sprite_info[ no ].trans > 255 ) sprite_info[ no ].trans = 255;
    else if ( sprite_info[ no ].trans < 0 ) sprite_info[ no ].trans = 0;

    return RET_CONTINUE;
}

int ONScripterLabel::mp3volCommand()
{
    mp3_volume = script_h.readInt();

    if ( mp3_sample ) SMPEG_setvolume( mp3_sample, mp3_volume );

    return RET_CONTINUE;
}

int ONScripterLabel::mp3Command()
{
    if      ( script_h.isName( "mp3save" ) ){
        music_play_once_flag = true;
    }
    else if ( script_h.isName( "mp3loop" ) ){
        music_play_once_flag = false;
    }
    else{
        music_play_once_flag = true;
    }

    stopBGM( false );
    
    const char *buf = script_h.readStr();
    setStr( &music_file_name, buf );

    playMP3( 0 );
        
    return RET_CONTINUE;
}

int ONScripterLabel::monocroCommand()
{
    const char *buf = script_h.readStr();

    if ( !strcmp( buf, "off" ) ){
        monocro_flag_new = false;
    }
    else if ( buf[0] != '#' ){
        errorAndExit( script_h.getStringBuffer() );
    }
    else{
        monocro_flag_new = true;
        readColor( &monocro_color_new, buf + 1 );
    }
    need_refresh_flag = true;

    return RET_CONTINUE;
}

int ONScripterLabel::menu_windowCommand()
{
    if ( fullscreen_mode ){
        if ( SDL_WM_ToggleFullScreen( screen_surface ) ) fullscreen_mode = false;
    }

    return RET_CONTINUE;
}

int ONScripterLabel::menu_fullCommand()
{
    if ( !fullscreen_mode ){
        if ( SDL_WM_ToggleFullScreen( screen_surface ) ) fullscreen_mode = true;
    }

    return RET_CONTINUE;
}

int ONScripterLabel::lspCommand()
{
    bool v;

    if ( script_h.isName( "lsph" ) )
        v = false;
    else
        v = true;

    int no = script_h.readInt();
    sprite_info[ no ].valid = v;

    const char *buf = script_h.readStr();
    sprite_info[ no ].setImageName( buf );

    sprite_info[ no ].pos.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    sprite_info[ no ].pos.y = script_h.readInt() * screen_ratio1 / screen_ratio2;

    if ( script_h.end_with_comma_flag )
        sprite_info[ no ].trans = script_h.readInt();
    else
        sprite_info[ no ].trans = 255;

    parseTaggedString( &sprite_info[ no ] );
    setupAnimationInfo( &sprite_info[ no ] );

    return RET_CONTINUE;
}

int ONScripterLabel::lookbackflushCommand()
{
    current_text_buffer = current_text_buffer->next;
    for ( int i=0 ; i<MAX_TEXT_BUFFER-1 ; i++ ){
        current_text_buffer->xy[1] = -1;
        current_text_buffer = current_text_buffer->next;
    }

    return RET_CONTINUE;
}

int ONScripterLabel::locateCommand()
{
    sentence_font.xy[0] = script_h.readInt();
    sentence_font.xy[1] = script_h.readInt();

    return RET_CONTINUE;
}

int ONScripterLabel::loadgameCommand()
{
    int no = script_h.readInt();

    if ( no < 0 )
        errorAndExit( "loadgame: no < 0." );

    if ( loadSaveFile( no ) ) return RET_CONTINUE;
    else {
        skip_flag = false;
        deleteButtonLink();
        deleteSelectLink();
        key_pressed_flag = false;
        saveon_flag = true;
        if ( event_mode & WAIT_INPUT_MODE ) return RET_WAIT;
        return RET_JUMP;
    }
}

int ONScripterLabel::ldCommand()
{
    const char *buf = script_h.readStr();
    char loc = buf[0];

    buf = script_h.readStr();
    
    if ( event_mode & EFFECT_EVENT_MODE ){
        int num = readEffect( &tmp_effect );
        if ( num > 1 ) return doEffect( TMP_EFFECT, NULL, TACHI_EFFECT_IMAGE );
        else           return doEffect( tmp_effect.effect, NULL, TACHI_EFFECT_IMAGE );
    }
    else{
        int no;

        if      ( loc == 'l' ) no = 0;
        else if ( loc == 'c' ) no = 1;
        else if ( loc == 'r' ) no = 2;
        
        tachi_info[ no ].setImageName( buf );
        parseTaggedString( &tachi_info[ no ] );
        setupAnimationInfo( &tachi_info[ no ] );
        if ( tachi_info[ no ].image_surface )
            tachi_info[ no ].valid = true;
        
        tachi_info[ no ].pos.x = screen_width * (no+1) / 4 - tachi_info[ no ].pos.w / 2;
        tachi_info[ no ].pos.y = underline_value - tachi_info[ no ].image_surface->h + 1;

        readEffect( &tmp_effect );
        return setEffect( tmp_effect.effect );
    }
}

int ONScripterLabel::jumpfCommand()
{
    jumpf_flag = true;

    return RET_CONTINUE;
}

int ONScripterLabel::jumpbCommand()
{
    current_link_label_info->label_info = last_tilde.label_info;
    current_link_label_info->current_line = last_tilde.current_line;

    script_h.setCurrent( last_tilde.current_script );

    return RET_JUMP;
}

int ONScripterLabel::ispageCommand()
{
    script_h.readToken();

    if ( clickstr_state == CLICK_NEWPAGE )
        script_h.setInt( script_h.getStringBuffer(), 1 );
    else
        script_h.setInt( script_h.getStringBuffer(), 0 );
    
    return RET_CONTINUE;
}

int ONScripterLabel::isdownCommand()
{
    script_h.readToken();

    if ( current_button_state.down_flag )
        script_h.setInt( script_h.getStringBuffer(), 1 );
    else
        script_h.setInt( script_h.getStringBuffer(), 0 );
    
    return RET_CONTINUE;
}

int ONScripterLabel::getversionCommand()
{
    script_h.readToken();
    
    script_h.setInt( script_h.getStringBuffer(), ONSCRIPTER_VERSION );

    return RET_CONTINUE;
}

int ONScripterLabel::gettimerCommand()
{
    bool gettimer_flag;
    
    if      ( script_h.isName( "gettimer" ) ){
        gettimer_flag = true;
    }
    else if ( script_h.isName( "getbtntimer" ) ){
        gettimer_flag = false;
    }

    script_h.readToken();

    if ( gettimer_flag ){
        script_h.setInt( script_h.getStringBuffer(), SDL_GetTicks() - internal_timer );
    }
    else{
        script_h.setInt( script_h.getStringBuffer(), btnwait_time );
    }
        
    return RET_CONTINUE; 
}

int ONScripterLabel::getregCommand()
{
    char *path = NULL, *key = NULL;
    script_h.readToken();
    
    if ( script_h.getStringBuffer()[0] != '$') errorAndExit( "getreg: no string variable." );
    char *p_buf = script_h.getStringBuffer()+1;
    int no = script_h.parseInt( &p_buf );

    const char *buf = script_h.readStr();
    setStr( &path, buf );
    buf = script_h.readStr();
    setStr( &key, buf );

    printf("  reading Registry file for [%s] %s\n", path, key );
        
    char reg_buf[256], reg_buf2[256];
    FILE *fp;
    bool found_flag = false;

    if ( ( fp = fopen( registry_file, "r" ) ) == NULL ){
        fprintf( stderr, "Cannot open file [%s]\n", registry_file );
        return RET_CONTINUE;
    }
    while( fgets( reg_buf, 256, fp) && !found_flag ){
        if ( reg_buf[0] == '[' ){
            unsigned int c=0;
            while ( reg_buf[c] != ']' && reg_buf[c] != '\0' ) c++;
            if ( !strncmp( reg_buf + 1, path, (c-1>strlen(path))?(c-1):strlen(path) ) ){
                while( fgets( reg_buf2, 256, fp) ){

                    script_h.pushCurrent( reg_buf2 );
                    buf = script_h.readStr( NULL, true );
                    if ( strncmp( buf,
                                  key,
                                  (strlen(buf)>strlen(key))?strlen(buf):strlen(key) ) ){
                        script_h.popCurrent();
                        continue;
                    }
                    
                    buf = script_h.readStr();
                    if ( buf[0] != '=' ){
                        script_h.popCurrent();
                        continue;
                    }

                    buf = script_h.readStr();
                    script_h.popCurrent();
                    setStr( &script_h.str_variables[ no ], buf );
                    printf("  $%d = %s\n", no, script_h.str_variables[ no ] );
                    found_flag = true;
                    break;
                }
            }
        }
    }

    if ( !found_flag ) fprintf( stderr, "  The key is not found.\n" );
    fclose(fp);

    return RET_CONTINUE;
}

int ONScripterLabel::getmouseposCommand()
{
    script_h.readToken();
    script_h.setInt( script_h.getStringBuffer(), current_button_state.x * screen_ratio2 / screen_ratio1 );
    
    script_h.readToken();
    script_h.setInt( script_h.getStringBuffer(), current_button_state.y * screen_ratio2 / screen_ratio1 );
    
    return RET_CONTINUE;
}

int ONScripterLabel::getcursorposCommand()
{
    script_h.readToken();
    script_h.setInt( script_h.getStringBuffer(), sentence_font.x() );
    
    script_h.readToken();
    script_h.setInt( script_h.getStringBuffer(), sentence_font.y() );
    
    return RET_CONTINUE;
}

int ONScripterLabel::getcselnumCommand()
{
    int count = 0;

    SelectLink *link = root_select_link.next;
    while ( link ) {
        count++;
        link = link->next;
    }
    script_h.readToken();
    script_h.setInt( script_h.getStringBuffer(), count );

    return RET_CONTINUE;
}

int ONScripterLabel::gameCommand()
{
    int i;
    
    current_link_label_info->label_info = script_h.lookupLabel( "start" );
    current_link_label_info->current_line = 0;
    script_h.setCurrent( current_link_label_info->label_info.start_address );
    current_mode = NORMAL_MODE;

    sentence_font.wait_time = default_text_speed[text_speed_no];

    /* ---------------------------------------- */
    /* Load default cursor */
    loadCursor( CURSOR_WAIT_NO, DEFAULT_CURSOR_WAIT, 0, 0 );
    loadCursor( CURSOR_NEWPAGE_NO, DEFAULT_CURSOR_NEWPAGE, 0, 0 );
    
    /* ---------------------------------------- */
    /* Load log files */
    if ( filelog_flag ) loadFileLog();

    /* ---------------------------------------- */
    /* Lookback related variables */
    for ( i=0 ; i<4 ; i++ ){
        setStr( &lookback_info[i].image_name, lookback_image_name[i] );
        parseTaggedString( &lookback_info[i] );
        setupAnimationInfo( &lookback_info[i] );
    }
    
    /* ---------------------------------------- */
    /* Initialize local variables */
    for ( i=0 ; i<200 ; i++ ){
        script_h.num_variables[i] = 0;
        delete[] script_h.str_variables[i];
        script_h.str_variables[i] = new char[1];
        script_h.str_variables[i][0] = '\0';
    }

    return RET_JUMP;
}

int ONScripterLabel::exbtnCommand()
{
    int sprite_no = -1, no;
    ButtonLink *button;
    
    if ( script_h.isName( "exbtn_d" ) ){
        button = &exbtn_d_button_link;
        if ( button->exbtn_ctl ) delete[] button->exbtn_ctl;
    }
    else{
        sprite_no = script_h.readInt();
        no = script_h.readInt();

        if ( sprite_info[ sprite_no ].num_of_cells == 0 ) return RET_CONTINUE;
        
        button = new ButtonLink();
        last_button_link->next = button;
        last_button_link = last_button_link->next;
    }
    //printf("exbtnCommand %s\n",string_buffer + string_buffer_offset);
    const char *buf = script_h.readStr();

    //if ( !sprite_info[ sprite_no ].valid ) return RET_CONTINUE;

    button->button_type = EX_SPRITE_BUTTON;
    button->sprite_no   = sprite_no;
    button->no          = no;
    button->exbtn_ctl   = new char[ strlen( buf ) + 1 ];
    strcpy( button->exbtn_ctl, buf );
    
    if ( sprite_no >= 0 &&
         ( sprite_info[ sprite_no ].image_surface || sprite_info[ sprite_no ].trans_mode == AnimationInfo::TRANS_STRING ) ){
        button->image_rect = button->select_rect = sprite_info[ button->sprite_no ].pos;
        sprite_info[ sprite_no ].valid = true;
    }

    return RET_CONTINUE;
}

int ONScripterLabel::erasetextwindowCommand()
{
    erase_text_window_mode = script_h.readInt();

    return RET_CONTINUE;
}

int ONScripterLabel::endCommand()
{
    saveGlovalData();
    saveFileLog();
    if ( labellog_flag ) script_h.saveLabelLog();
    if ( kidokuskip_flag ) script_h.saveKidokuData();
    if ( cdrom_info ){
        SDL_CDStop( cdrom_info );
        SDL_CDClose( cdrom_info );
    }
    if ( midi_info ){
        Mix_HaltMusic();
        Mix_FreeMusic( midi_info );
    }
    SDL_Quit();
    exit(0);
}

int ONScripterLabel::dwavestopCommand()
{
    int ch = script_h.readInt();

    if ( wave_sample[ch] ){
        Mix_Pause( ch );
        Mix_FreeChunk( wave_sample[ch] );
        wave_sample[ch] = NULL;
    }

    return RET_CONTINUE;
}

int ONScripterLabel::dwaveCommand()
{
    if ( script_h.isName( "dwaveloop" ) ){
        wave_play_once_flag = true;
    }
    else{
        wave_play_once_flag = false;
    }

    int ch = script_h.readInt();

    if ( wave_sample[ch] ){
        Mix_Pause( ch );
        Mix_FreeChunk( wave_sample[ch] );
        wave_sample[ch] = NULL;
    }

    const char *buf = script_h.readStr();
    playWave( buf, wave_play_once_flag, ch );
        
    return RET_CONTINUE;
}

int ONScripterLabel::delayCommand()
{
    int t = script_h.readInt();

    if ( event_mode & (WAIT_SLEEP_MODE | WAIT_INPUT_MODE ) ){
        event_mode = IDLE_EVENT_MODE;
        return RET_CONTINUE;
    }
    else{
        event_mode = WAIT_SLEEP_MODE | WAIT_INPUT_MODE;
        key_pressed_flag = false;
        startTimer( t );
        return RET_WAIT;
    }
}

int ONScripterLabel::cspCommand()
{
    int no = script_h.readInt();

    if ( no == -1 )
        for ( int i=0 ; i<MAX_SPRITE_NUM ; i++ ){
            sprite_info[i].remove();
        }
    else{
        sprite_info[no].remove();
    }

    return RET_CONTINUE;
}

int ONScripterLabel::cselgotoCommand()
{
    int csel_no = script_h.readInt();

    int counter = 0;
    SelectLink *link = root_select_link.next;
    while( link ){
        if ( csel_no == counter++ ) break;
        link = link->next;
    }
    if ( !link ) errorAndExit( "cselgoto: no select link" );

    current_link_label_info->label_info   = script_h.lookupLabel( link->label );
    current_link_label_info->current_line = 0;
    script_h.setCurrent( current_link_label_info->label_info.start_address );
    
    saveon_flag = shelter_soveon_flag;

    deleteSelectLink();
    newPage( true );
    
    return RET_JUMP;
}

int ONScripterLabel::cselbtnCommand()
{
    int csel_no   = script_h.readInt();
    int button_no = script_h.readInt();

    FontInfo csel_info = sentence_font;
    csel_info.top_xy[0] = script_h.readInt();
    csel_info.top_xy[1] = script_h.readInt();
    csel_info.xy[0] = csel_info.xy[1] = 0;

    int counter = 0;
    SelectLink *link = root_select_link.next;
    while ( link ){
        if ( csel_no == counter++ ) break;
        link = link->next;
    }
    if ( link == NULL || link->text == NULL || *link->text == '\0' )
        errorAndExit( "cselbtn: no select text" );

    last_button_link->next = getSelectableSentence( link->text, &csel_info );
    last_button_link = last_button_link->next;
    last_button_link->button_type = CUSTOM_SELECT_BUTTON;
    last_button_link->no          = button_no;
    last_button_link->sprite_no   = csel_no;

    sentence_font.is_valid = csel_info.is_valid;
    sentence_font.ttf_font = csel_info.ttf_font;

    SDL_BlitSurface( text_surface, &last_button_link->select_rect, select_surface, NULL );

    return RET_CONTINUE;
}

int ONScripterLabel::clickCommand()
{
    if ( event_mode & WAIT_INPUT_MODE ){
        event_mode = IDLE_EVENT_MODE;
        return RET_CONTINUE;
    }
    else{
        skip_flag = false;
        event_mode = WAIT_INPUT_MODE;
        key_pressed_flag = false;
        return RET_WAIT;
    }
}

int ONScripterLabel::clCommand()
{
    const char *buf = script_h.readStr();
    char loc = buf[0];
    
    if ( event_mode & EFFECT_EVENT_MODE ){
        int num = readEffect( &tmp_effect );
        if ( num > 1 ) return doEffect( TMP_EFFECT, NULL, TACHI_EFFECT_IMAGE );
        else           return doEffect( tmp_effect.effect, NULL, TACHI_EFFECT_IMAGE );
    }
    else{
        if ( loc == 'l' || loc == 'a' ){
            tachi_info[0].remove();
        }
        if ( loc == 'c' || loc == 'a' ){
            tachi_info[1].remove();
        }
        if ( loc == 'r' || loc == 'a' ){
            tachi_info[2].remove();
        }

        readEffect( &tmp_effect );
        return setEffect( tmp_effect.effect );
    }
}

int ONScripterLabel::cellCommand()
{
    int sprite_no = script_h.readInt();
    int no        = script_h.readInt();

    if ( sprite_info[ sprite_no ].num_of_cells > 0 )
        sprite_info[ sprite_no ].current_cell = no;
        
    return RET_CONTINUE;
}

int ONScripterLabel::captionCommand()
{
    const char* buf = script_h.readStr();
    char *buf2 = new char[ strlen( buf ) + 1 ];
    strcpy( buf2, buf );
    
#if defined(LINUX) /* convert sjis to euc */
    int i = 0;
    while ( buf2[i] ) {
        if ( (unsigned char)buf2[i] > 0x80 ) {
            unsigned char c1, c2;
            c1 = buf2[i];
            c2 = buf2[i+1];

            c1 -= (c1 <= 0x9f) ? 0x71 : 0xb1;
            c1 = c1 * 2 + 1;
            if (c2 >= 0x9e) {
                c2 -= 0x7e;
                c1++;
            }
            else if (c2 >= 0x80) {
                c2 -= 0x20;
            }
            else {
                c2 -= 0x1f;
            }

            buf2[i]   = c1 | 0x80;
            buf2[i+1] = c2 | 0x80;
            i++;
        }
        i++;
    }
#endif

    setStr( &wm_title_string, buf2 );
    setStr( &wm_icon_string,  buf2 );
    delete[] buf2;
    
    SDL_WM_SetCaption( wm_title_string, wm_icon_string );

    return RET_CONTINUE;
}

int ONScripterLabel::btnwaitCommand()
{
    bool del_flag, textbtn_flag = false, selectbtn_flag = false;

    if ( script_h.isName( "btnwait2" ) ){
        del_flag = false;
    }
    else if ( script_h.isName( "btnwait" ) ){
        del_flag = true;
    }
    else if ( script_h.isName( "textbtnwait" ) ){
        del_flag = false;
        textbtn_flag = true;
    }
    else if ( script_h.isName( "selectbtnwait" ) ){
        del_flag = false;
        selectbtn_flag = true;
    }

    script_h.readToken();
    
    if ( event_mode & WAIT_BUTTON_MODE )
    {
        btnwait_time = SDL_GetTicks() - internal_button_timer;
        btntime_value = 0;

        if ( textbtn_flag && skip_flag ) current_button_state.button = 0;
        script_h.setInt( script_h.getStringBuffer(), current_button_state.button );

        if ( del_flag ){
            if ( current_button_state.button > 0 ) deleteButtonLink();
            if ( exbtn_d_button_link.exbtn_ctl ){
                delete[] exbtn_d_button_link.exbtn_ctl;
                exbtn_d_button_link.exbtn_ctl = NULL;
            }
        }

        event_mode = IDLE_EVENT_MODE;
        btndown_flag = false;

        return RET_CONTINUE;
    }
    else{
        shortcut_mouse_line = 0;
        skip_flag = false;
        event_mode = WAIT_BUTTON_MODE;
        if ( textbtn_flag ) event_mode |= WAIT_TEXTBTN_MODE;

        ButtonLink *p_button_link = root_button_link.next;
        while( p_button_link ){
            if ( current_button_link.button_type == SPRITE_BUTTON ||
                 current_button_link.button_type == EX_SPRITE_BUTTON )
                sprite_info[ current_button_link.sprite_no ].current_cell = 0;
            p_button_link = p_button_link->next;
        }

        refreshSurface( accumulation_surface,
                        NULL,
                        ( erase_text_window_mode == 0 && text_on_flag)?REFRESH_SHADOW_MODE:REFRESH_NORMAL_MODE );
        SDL_BlitSurface( accumulation_surface, NULL, text_surface, NULL );

        if ( erase_text_window_mode == 0 && text_on_flag ){
            restoreTextBuffer();
            display_mode = TEXT_DISPLAY_MODE;
        }
        else{
            display_mode = NORMAL_DISPLAY_MODE;
        }
        
        /* ---------------------------------------- */
        /* Resotre csel button */
        FontInfo f_info = sentence_font;
        f_info.xy[0] = 0;
        f_info.xy[1] = 0;
        
        p_button_link = root_button_link.next;
        while ( p_button_link ){
            if ( p_button_link->button_type == CUSTOM_SELECT_BUTTON ){
            
                f_info.xy[0] = f_info.xy[1] = 0;
                f_info.top_xy[0] = p_button_link->image_rect.x * screen_ratio2 / screen_ratio1;
                f_info.top_xy[1] = p_button_link->image_rect.y * screen_ratio2 / screen_ratio1;

                int counter = 0;
                SelectLink *s_link = root_select_link.next;
                while ( s_link ){
                    if ( p_button_link->sprite_no == counter++ ) break;
                    s_link = s_link->next;
                }
            
                drawString( s_link->text, f_info.off_color, &f_info, false, text_surface );
            
            }
            p_button_link = p_button_link->next;
        }
    
        sentence_font.is_valid = f_info.is_valid;
        sentence_font.ttf_font = f_info.ttf_font;
        
        SDL_BlitSurface( text_surface, NULL, select_surface, NULL );

        flush();

        refreshMouseOverButton();

        if ( btntime_value > 0 ){
            event_mode |= WAIT_SLEEP_MODE;
            startTimer( btntime_value );
            if ( usewheel_flag ) current_button_state.button = -5;
            else                 current_button_state.button = -2;
        }
        internal_button_timer = SDL_GetTicks();

        if ( textbtn_flag ){
            event_mode |= WAIT_ANIMATION_MODE;
            advancePhase();
        }
        
        return RET_WAIT;
    }
}

int ONScripterLabel::btntimeCommand()
{
    btntime_value = script_h.readInt();
    
    return RET_CONTINUE;
}

int ONScripterLabel::btndownCommand()
{
    btndown_flag = (script_h.readInt()==1)?true:false;

    return RET_CONTINUE;
}

int ONScripterLabel::btndefCommand()
{
    const char *buf = script_h.readStr();

    if ( strcmp( buf, "clear" ) ){
        btndef_info.remove();
        if ( buf[0] != '\0' ){
            btndef_info.setImageName( buf );
            parseTaggedString( &btndef_info );
            setupAnimationInfo( &btndef_info );
            SDL_SetAlpha( btndef_info.image_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
        }
    }
    
    deleteButtonLink();
    
    return RET_CONTINUE;
}

int ONScripterLabel::btnCommand()
{
    SDL_Rect src_rect;

    ButtonLink *b_link = new ButtonLink();
    
    b_link->button_type  = NORMAL_BUTTON;
    b_link->no           = script_h.readInt();
    b_link->image_rect.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    b_link->image_rect.y = script_h.readInt() * screen_ratio1 / screen_ratio2;
    b_link->image_rect.w = script_h.readInt() * screen_ratio1 / screen_ratio2;
    b_link->image_rect.h = script_h.readInt() * screen_ratio1 / screen_ratio2;
    b_link->select_rect = b_link->image_rect;

    src_rect.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    src_rect.y = script_h.readInt() * screen_ratio1 / screen_ratio2;
    src_rect.w = b_link->image_rect.w;
    src_rect.h = b_link->image_rect.h;

    b_link->image_surface = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG, b_link->image_rect.w, b_link->image_rect.h, 32, rmask, gmask, bmask, amask );

    SDL_BlitSurface( btndef_info.image_surface, &src_rect, b_link->image_surface, NULL );

    last_button_link->next = b_link;
    last_button_link = last_button_link->next;
    
    return RET_CONTINUE;
}

int ONScripterLabel::brCommand()
{
    int ret = enterTextDisplayMode();
    if ( ret != RET_NOMATCH ) return ret;

    sentence_font.xy[0] = 0;
    sentence_font.xy[1]++;
    text_char_flag = false;

    return RET_CONTINUE;
}

int ONScripterLabel::bltCommand()
{
    SDL_Rect src_rect, dst_rect, clip, clipped;

    dst_rect.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    dst_rect.y = script_h.readInt() * screen_ratio1 / screen_ratio2;
    dst_rect.w = script_h.readInt() * screen_ratio1 / screen_ratio2;
    dst_rect.h = script_h.readInt() * screen_ratio1 / screen_ratio2;
    src_rect.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    src_rect.y = script_h.readInt() * screen_ratio1 / screen_ratio2;
    src_rect.w = script_h.readInt() * screen_ratio1 / screen_ratio2;
    src_rect.h = script_h.readInt() * screen_ratio1 / screen_ratio2;

    if ( src_rect.w == dst_rect.w && src_rect.h == dst_rect.h ){

        clip.x = clip.y = 0;
        clip.w = screen_width;
        clip.h = screen_height;
        doClipping( &dst_rect, &clip, &clipped );
        src_rect.x += clipped.x;
        src_rect.y += clipped.y;
        src_rect.w -= clipped.x;
        src_rect.h -= clipped.y;
        
        SDL_BlitSurface( btndef_info.image_surface, &src_rect, screen_surface, &dst_rect );
        SDL_UpdateRect( screen_surface, dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h );
    }
    else{
        resizeSurface( btndef_info.image_surface, &src_rect, text_surface, &dst_rect );
        flush( &dst_rect );
    }
    
    return RET_CONTINUE;
}

int ONScripterLabel::bgCommand()
{
    const char *buf = script_h.readStr();

    if ( event_mode & EFFECT_EVENT_MODE ){
        int num = readEffect( &tmp_effect );
        if ( num > 1 ) return doEffect( TMP_EFFECT, &bg_info, bg_effect_image );
        else           return doEffect( tmp_effect.effect, &bg_info, bg_effect_image );
    }
    else{
        for ( int i=0 ; i<3 ; i++ ){
            tachi_info[i].remove();
        }
        bg_info.remove();

        bg_effect_image = COLOR_EFFECT_IMAGE;

        if ( !strcmp( (const char*)buf, "white" ) ){
            bg_info.color[0] = bg_info.color[1] = bg_info.color[2] = 0xff;
        }
        else if ( !strcmp( (const char*)buf, "black" ) ){
            bg_info.color[0] = bg_info.color[1] = bg_info.color[2] = 0x00;
        }
        else if ( buf[0] == '#' ){
            readColor( &bg_info.color, buf+1 );
        }
        else{
            setStr( &bg_info.image_name, buf );
            parseTaggedString( &bg_info );
            setupAnimationInfo( &bg_info );
            bg_effect_image = BG_EFFECT_IMAGE;
            if ( bg_info.image_surface ){
                SDL_Rect src_rect, dst_rect;
                src_rect.x = 0;
                src_rect.y = 0;
                src_rect.w = bg_info.image_surface->w;
                src_rect.h = bg_info.image_surface->h;
                dst_rect.x = (screen_width - bg_info.image_surface->w) / 2;
                dst_rect.y = (screen_height - bg_info.image_surface->h) / 2;

                SDL_BlitSurface( bg_info.image_surface, &src_rect, background_surface, &dst_rect );
            }
        }

        if ( bg_effect_image == COLOR_EFFECT_IMAGE ){
            SDL_FillRect( background_surface, NULL, SDL_MapRGB( effect_dst_surface->format, bg_info.color[0], bg_info.color[1], bg_info.color[2]) );
        }

        readEffect( &tmp_effect );
        return setEffect( tmp_effect.effect );
    }
}

int ONScripterLabel::barclearCommand()
{
    for ( int i=0 ; i<MAX_PARAM_NUM ; i++ ) {
        if ( bar_info[i] ) {
            delete bar_info[i];
            bar_info[i] = NULL;
        }
    }
    return RET_CONTINUE;
}

int ONScripterLabel::barCommand()
{
    int no = script_h.readInt();
    if ( bar_info[no] ) delete bar_info[no];
    bar_info[no] = new AnimationInfo();
    bar_info[no]->trans_mode = AnimationInfo::TRANS_COPY;
    bar_info[no]->abs_flag = true;
    bar_info[no]->num_of_cells = 1;
    bar_info[no]->current_cell = 0;
    bar_info[no]->alpha_offset = 0;

    int param           = script_h.readInt();
    bar_info[no]->pos.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    bar_info[no]->pos.y = script_h.readInt() * screen_ratio1 / screen_ratio2;
                          
    bar_info[no]->pos.w = script_h.readInt() * screen_ratio1 / screen_ratio2;
    bar_info[no]->pos.h = script_h.readInt() * screen_ratio1 / screen_ratio2;
    int max             = script_h.readInt();
    if ( max == 0 ) errorAndExit( "bar: max = 0." );
    bar_info[no]->pos.w = bar_info[no]->pos.w * param / max;

    const char *buf = script_h.readStr();
    if ( buf[0] != '#' ) errorAndExit( "bar: Color is not specified." );
    readColor( &bar_info[no]->color, buf+1 );
    
    bar_info[no]->image_surface = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG, bar_info[no]->pos.w, bar_info[no]->pos.h, 32, rmask, gmask, bmask, amask );
    SDL_SetAlpha( bar_info[no]->image_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
    SDL_FillRect( bar_info[no]->image_surface, NULL, SDL_MapRGB( bar_info[no]->image_surface->format, bar_info[no]->color[0], bar_info[no]->color[1], bar_info[no]->color[2] ) );

    return RET_CONTINUE;
}

int ONScripterLabel::autoclickCommand()
{
    autoclick_timer = script_h.readInt();

    return RET_CONTINUE;
}

int ONScripterLabel::amspCommand()
{
    int no = script_h.readInt();
    sprite_info[ no ].pos.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    sprite_info[ no ].pos.y = script_h.readInt() * screen_ratio1 / screen_ratio2;

    if ( script_h.end_with_comma_flag )
        sprite_info[ no ].trans = script_h.readInt();

    if ( sprite_info[ no ].trans > 255 ) sprite_info[ no ].trans = 255;
    else if ( sprite_info[ no ].trans < 0 ) sprite_info[ no ].trans = 0;
    
    return RET_CONTINUE;
}

int ONScripterLabel::allspresumeCommand()
{
    all_sprite_hide_flag = false;
    return RET_CONTINUE;
}

int ONScripterLabel::allsphideCommand()
{
    all_sprite_hide_flag = true;
    return RET_CONTINUE;
}

