/* -*- C++ -*-
 * 
 *  ONScripterLabel_command.cpp - Command executer of ONScripter
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

#include "ONScripterLabel.h"

#define ONSCRIPTER_VERSION 200

#define DEFAULT_CURSOR_WAIT    ":l/3,160,2;cursor0.bmp"
#define DEFAULT_CURSOR_NEWPAGE ":l/3,160,2;cursor1.bmp"

#define DEFAULT_LOOKBACK_NAME0 "uoncur.bmp"
#define DEFAULT_LOOKBACK_NAME1 "uoffcur.bmp"
#define DEFAULT_LOOKBACK_NAME2 "doncur.bmp"
#define DEFAULT_LOOKBACK_NAME3 "doffcur.bmp"

#define CONTINUOUS_PLAY

int ONScripterLabel::waveCommand()
{
    wave_play_loop_flag = false;
    
    if ( script_h.isName( "waveloop" ) ){
        wave_play_loop_flag = true;
    }

    wavestopCommand();

    const char *buf = script_h.readStr();
    setStr( &wave_file_name, buf );
    playWave( buf, wave_play_loop_flag, DEFAULT_WAVE_CHANNEL );
        
    return RET_CONTINUE;
}

int ONScripterLabel::wavestopCommand()
{
    if ( wave_sample[DEFAULT_WAVE_CHANNEL] ){
        Mix_Pause( DEFAULT_WAVE_CHANNEL );
        Mix_FreeChunk( wave_sample[DEFAULT_WAVE_CHANNEL] );
        wave_sample[DEFAULT_WAVE_CHANNEL] = NULL;
    }
    setStr( &wave_file_name, NULL );

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
    sprite_info[ no ].visible = (v==1)?true:false;
    dirty_rect.add( sprite_info[no].pos );
    
    return RET_CONTINUE;
}

int ONScripterLabel::voicevolCommand()
{
    voice_volume = script_h.readInt();

    if ( wave_sample[0] ) Mix_Volume( 0, se_volume * 128 / 100 );
    
    return RET_CONTINUE;
}

int ONScripterLabel::vCommand()
{
    char buf[256];
    
    sprintf( buf, "wav%c%s.wav", DELIMITER, script_h.getStringBuffer()+1 );
    playWave( buf, false, DEFAULT_WAVE_CHANNEL );
    
    return RET_CONTINUE;
}

int ONScripterLabel::trapCommand()
{
    script_h.readToken();
    const char *buf = script_h.getStringBuffer();
    
    if ( buf[0] == '*' ){
        trap_flag = true;
        if ( trap_dist ) delete[] trap_dist;
        trap_dist = new char[ strlen(buf) ];
        memcpy( trap_dist, buf+1, strlen(buf) );
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
        refreshSurface( accumulation_surface, NULL, REFRESH_NORMAL_MODE );
        refreshSurface( text_surface, NULL, REFRESH_SHADOW_MODE );
        dirty_rect.fill( screen_width, screen_height );
        flush();
        display_mode = next_display_mode = TEXT_DISPLAY_MODE;
    }

    return RET_CONTINUE;
}

int ONScripterLabel::textoffCommand()
{
    text_on_flag = false;
    if ( display_mode & TEXT_DISPLAY_MODE ){
        refreshSurface( accumulation_surface, NULL, REFRESH_NORMAL_MODE );
        SDL_BlitSurface( accumulation_surface, NULL, text_surface, NULL );
        dirty_rect.fill( screen_width, screen_height );
        flush();
        display_mode = next_display_mode = NORMAL_DISPLAY_MODE;
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
        clickstr_state = CLICK_NONE;
    }

    return RET_CONTINUE;
}

int ONScripterLabel::tateyokoCommand()
{
    int mode = script_h.readInt();

    if ( tateyoko_mode == 0 )
        tateyoko_mode = mode;
    
    return RET_CONTINUE;
}

int ONScripterLabel::talCommand()
{
    script_h.readToken();
    char loc = script_h.getStringBuffer()[0];
    int trans = script_h.readInt();

    if ( event_mode & EFFECT_EVENT_MODE ){
        int num = readEffect( &tmp_effect );
        if ( num > 1 ) return doEffect( TMP_EFFECT, NULL, TACHI_EFFECT_IMAGE );
        else           return doEffect( tmp_effect.effect, NULL, TACHI_EFFECT_IMAGE );
    }
    else{
        int no = 0;
        if      ( loc == 'l' ) no = 0;
        else if ( loc == 'c' ) no = 1;
        else if ( loc == 'r' ) no = 2;

        if      ( trans > 255 ) trans = 255;
        else if ( trans < 0   ) trans = 0;

        tachi_info[ no ].trans = trans;
        dirty_rect.add( tachi_info[ no ].pos );

        readEffect( &tmp_effect );
        return setEffect( tmp_effect.effect );
   }
}

int ONScripterLabel::tablegotoCommand()
{
    int count = 0;
    int no = script_h.readInt();

    while( script_h.getEndStatus() & ScriptHandler::END_COMMA ){
        script_h.readToken();
        const char *buf = script_h.getStringBuffer();
        if ( count++ == no ){
            current_link_label_info->label_info = script_h.lookupLabel( buf+1 );
            current_link_label_info->current_line = 0;
            script_h.setCurrent( current_link_label_info->label_info.start_address );
            string_buffer_offset = 0;
    
            return RET_JUMP;
        }
    }

    return RET_CONTINUE;
}

int ONScripterLabel::systemcallCommand()
{
    script_h.readToken();
    system_menu_mode = getSystemCallNo( script_h.getStringBuffer() );

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

int ONScripterLabel::splitstringCommand()
{
    script_h.readStr();
    const char *save_buf = script_h.saveStringBuffer();
    
    const char *buf = script_h.readStr();
    char delimiter = buf[0];

    char token[256];
    while( ScriptHandler::END_COMMA ){
        script_h.readToken();
        if ( script_h.current_variable.type != ScriptHandler::VAR_STR ) return RET_CONTINUE;
        int no = script_h.current_variable.var_no;

        unsigned int c=0;
        while( save_buf[c] != delimiter && save_buf[c] != '\0' ) c++;
        memcpy( token, save_buf, c );
        token[c] = '\0';
        setStr( &script_h.str_variables[no], token );
        save_buf += c+1;
    }
    
    return RET_CONTINUE;
}

int ONScripterLabel::spstrCommand()
{
    const char *buf = script_h.readStr();
    decodeExbtnControl( NULL, buf );
    
    return RET_CONTINUE;
}

int ONScripterLabel::spclclkCommand()
{
    if ( !force_button_shortcut_flag )
        spclclk_flag = true;
    return RET_CONTINUE;
}

int ONScripterLabel::spbtnCommand()
{
    int sprite_no = script_h.readInt();
    int no        = script_h.readInt();

    if ( sprite_info[ sprite_no ].num_of_cells == 0 ) return RET_CONTINUE;

    ButtonLink *button = new ButtonLink();
    button->next = root_button_link.next;
    root_button_link.next = button;

    button->button_type = ButtonLink::SPRITE_BUTTON;
    button->sprite_no   = sprite_no;
    button->no          = no;

    if ( sprite_info[ sprite_no ].image_surface ||
         sprite_info[ sprite_no ].trans_mode == AnimationInfo::TRANS_STRING )
    {
        button->image_rect = button->select_rect = sprite_info[ sprite_no ].pos;
        allocateSelectedSurface( sprite_no, button );
        sprite_info[ sprite_no ].visible = true;
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

    for ( int i=1 ; i<ONS_MIX_CHANNELS ; i++ )
        if ( wave_sample[i] ) Mix_Volume( i, se_volume * 128 / 100 );
        
    return RET_CONTINUE;
}

int ONScripterLabel::setwindow2Command()
{
    const char *buf = script_h.readStr();
    if ( buf[0] == '#' ){
        sentence_font.is_transparent = true;
        readColor( &sentence_font.window_color, buf );
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
    tateyoko_mode = 0;
    sentence_font.ttf_font  = NULL;
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

    const char *buf = script_h.readStr();
    dirty_rect.add( sentence_font_info.pos );
    if ( buf[0] == '#' ){
        sentence_font.is_transparent = true;
        readColor( &sentence_font.window_color, buf );

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
#if 0        
        if ( sentence_font_info.image_surface ){
            sentence_font_info.pos.w = sentence_font_info.image_surface->w * screen_ratio1 / screen_ratio2;
            sentence_font_info.pos.h = sentence_font_info.image_surface->h * screen_ratio1 / screen_ratio2;
        }
#endif        
        sentence_font.window_color[0] = sentence_font.window_color[1] = sentence_font.window_color[2] = 0xff;
    }

    dirty_rect.add( sentence_font_info.pos );
    lookbackflushCommand();
    clearCurrentTextBuffer();
    display_mode = next_display_mode = NORMAL_DISPLAY_MODE;
    
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
    int select_mode = SELECT_GOTO_MODE;
    SelectLink *last_select_link;
    char *p_buf;

    if ( script_h.isName( "selnum" ) ){
        select_mode = SELECT_NUM_MODE;
        p_buf = script_h.getCurrent();
        script_h.readInt();
        script_h.pushVariable();
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
            string_buffer_offset = 0;

            ret = RET_JUMP;
        }
        else if ( select_mode == SELECT_GOSUB_MODE ){
            current_link_label_info->current_line = select_label_info.current_line;
            string_buffer_offset = 0;
            gosubReal( last_select_link->label, false, select_label_info.current_script );

            ret = RET_JUMP;
        }
        else{
            script_h.setInt( &script_h.pushed_variable, current_button_state.button - 1 );
            current_link_label_info->current_line = select_label_info.current_line;
            script_h.setCurrent( select_label_info.current_script );
            ret = RET_CONTINUE_NOREAD;
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
        SelectLink *link = NULL;
        shortcut_mouse_line = -1;
        //flush();
        skip_flag = false;
        automode_flag = false;
        xy[0] = sentence_font.xy[0];
        xy[1] = sentence_font.xy[1];

        if ( selectvoice_file_name[SELECTVOICE_OPEN] )
            playWave( selectvoice_file_name[SELECTVOICE_OPEN], false, DEFAULT_WAVE_CHANNEL );

        last_select_link = &root_select_link;
        select_label_info.current_line = current_link_label_info->current_line;
        script_h.readToken();
        const char *buf = script_h.getStringBuffer();
        int count = 0;
        while(1){
            //printf("sel [%s] comma %d\n", buf, comma_flag  );
            if ( buf[0] != 0x0a && comma_flag == true ){
                comma_flag = (script_h.getEndStatus() & ScriptHandler::END_COMMA);
                count++;
                if ( select_mode == SELECT_NUM_MODE || count % 2 ){
                    buf = script_h.readStr( true );
                    if ( select_mode != SELECT_NUM_MODE && !comma_flag ) errorAndExit( "select: comma is needed here." );
                    link = new SelectLink();
                    setStr( &link->text, buf );
                    //printf("Select text %s\n", link->text);
                }
                if ( select_mode == SELECT_NUM_MODE || !(count % 2) ){
                    if ( select_mode != SELECT_NUM_MODE )
                        setStr( &link->label, buf+1 );
                    //printf("Select label %s\n", link->label );
                    last_select_link->next = link;
                    last_select_link = last_select_link->next;
                }
                script_h.readToken();
                buf = script_h.getStringBuffer();
            }
            else{
                if ( select_mode != SELECT_NUM_MODE && (count & 1) == 1 ) errorAndExit( "select: label must be in the same line." );
                do{
                    if ( buf[0] == 0x0a || buf[0] == ',' ){
                        select_label_info.current_line++;
                        script_h.setText( false );
                        script_h.readToken();
                        buf = script_h.getStringBuffer();
                    }
                    select_label_info.current_script = script_h.getCurrent();
                    if ( buf[0] == ',' ){
                        if ( comma_flag ) errorAndExit( "select: double comma." );
                        else comma_flag = true;
                    }
                } while ( buf[0] == 0x0a || buf[0] == ',' );

                if ( !comma_flag ) break;
            }
        }

        if ( select_mode != SELECT_CSEL_MODE ){
            last_select_link = root_select_link.next;
            int counter = 1;
            while( last_select_link ){
                if ( *last_select_link->text ){
                    ButtonLink *button = getSelectableSentence( last_select_link->text, &sentence_font );
                    button->next = root_button_link.next;
                    root_button_link.next = button;
                    button->no = counter;
                }
                counter++;
                last_select_link = last_select_link->next;
            }
        }

        if ( select_mode == SELECT_GOTO_MODE || select_mode == SELECT_CSEL_MODE ){ /* Resume */
            if ( select_mode == SELECT_CSEL_MODE ){
                current_link_label_info->label_info = script_h.lookupLabel( "customsel" );
                current_link_label_info->current_line = 0;
                script_h.setCurrent( current_link_label_info->label_info.start_address );
                string_buffer_offset = 0;

                return RET_JUMP;
            }
        }
        sentence_font.xy[0] = xy[0];
        sentence_font.xy[1] = xy[1];

        flushEvent();
        event_mode = WAIT_INPUT_MODE | WAIT_BUTTON_MODE | WAIT_ANIMATION_MODE;
        advancePhase();
        refreshMouseOverButton();

        return RET_WAIT;
    }
}

int ONScripterLabel::savetimeCommand()
{
    int no = script_h.readInt();

    script_h.readInt();

    SaveFileInfo info;
    searchSaveFile( info, no );

    if ( !info.valid ){
        script_h.setInt( &script_h.current_variable, 0 );
        for ( int i=0 ; i<3 ; i++ )
            script_h.readToken();
        return RET_CONTINUE;
    }

    script_h.setInt( &script_h.current_variable, info.month );
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, info.day );
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, info.hour );
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, info.minute );

    return RET_CONTINUE;
}

int ONScripterLabel::saveonCommand()
{
    saveon_flag = true;

    return RET_CONTINUE;
}

int ONScripterLabel::saveoffCommand()
{
    if ( saveon_flag && internal_saveon_flag )
        saveSaveFile( -1 );
    
    saveon_flag = false;

    return RET_CONTINUE;
}

int ONScripterLabel::savegameCommand()
{
    int no = script_h.readInt();

    if ( no < 0 )
        errorAndExit("savegame: the specified number is less than 0.");
    else{
        shelter_event_mode = event_mode;
        //char *p_buf = script_h.getNext();
        //script_h.readToken(); // save point is the next token to no
        saveSaveFile( no ); 
        //script_h.setCurrent( p_buf, false );
    }

    return RET_CONTINUE;
}

int ONScripterLabel::savefileexistCommand()
{
    script_h.readInt();
    script_h.pushVariable();
    int no = script_h.readInt();

    SaveFileInfo info;
    searchSaveFile( info, no );

    script_h.setInt( &script_h.pushed_variable, (info.valid==true)?1:0 );

    return RET_CONTINUE;
}

int ONScripterLabel::rndCommand()
{
    int  upper, lower;
    
    if ( script_h.isName( "rnd2" ) ){
        script_h.readInt();
        script_h.pushVariable();
        
        lower = script_h.readInt();
        upper = script_h.readInt();
    }
    else{
        script_h.readInt();
        script_h.pushVariable();

        lower = 0;
        upper = script_h.readInt() - 1;
    }

    script_h.setInt( &script_h.pushed_variable, lower + (int)( (double)(upper-lower+1)*rand()/(RAND_MAX+1.0)) );

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

    for ( i=0 ; i<script_h.global_variable_border ; i++ ){
        script_h.num_variables[i] = 0;
        if ( script_h.str_variables[i] ) delete[] script_h.str_variables[i];
        script_h.str_variables[i] = NULL;
    }

    erase_text_window_mode = 1;
    clearCurrentTextBuffer();
    tateyoko_mode = 0;
    sentence_font.xy[0] = 0;
    sentence_font.xy[1] = 0;
    resetSentenceFont();
    
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
    for ( i=0 ; i<3 ; i++ ){
        tachi_info[i].remove();
    }
    bg_info.remove();
    setStr( &bg_info.file_name, "black");
    
    deleteButtonLink();
    deleteSelectLink();

    wavestopCommand();
    stopBGM( false );

    setBackground( accumulation_surface );
    SDL_BlitSurface( accumulation_surface, NULL, text_surface, NULL );
    dirty_rect.fill( screen_width, screen_height );

    return RET_JUMP;
}

int ONScripterLabel::repaintCommand()
{
    refreshSurface( accumulation_surface, NULL, REFRESH_NORMAL_MODE );
    refreshSurface( text_surface, NULL, isTextVisible()?REFRESH_SHADOW_MODE:REFRESH_NORMAL_MODE );
    dirty_rect.fill( screen_width, screen_height );
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
            //SDL_BlitSurface( text_surface, NULL, effect_src_surface, NULL );
            SDL_BlitSurface( text_surface, NULL, effect_dst_surface, NULL );
        }
        tmp_effect.effect = CUSTOM_EFFECT_NO + quake_type;
        return doEffect( TMP_EFFECT, NULL, DIRECT_EFFECT_IMAGE );
    }
    else{
        dirty_rect.fill( screen_width, screen_height );
        setEffect( 2 ); // 2 is dummy value
        return RET_WAIT; // RET_WAIT de yoi?
        //return RET_WAIT_NEXT;
    }
}

int ONScripterLabel::puttextCommand()
{
    int ret = enterTextDisplayMode();
    if ( ret != RET_NOMATCH ) return ret;

    script_h.readStr();
    string_buffer_offset = 0;
    script_h.setText( true );
    /*line_head_flag = true*/

    return RET_CONTINUE_NOREAD;
}

int ONScripterLabel::prnumclearCommand()
{
    for ( int i=0 ; i<MAX_PARAM_NUM ; i++ ) {
        if ( prnum_info[i] ) {
            dirty_rect.add( prnum_info[i]->pos );
            delete prnum_info[i];
            prnum_info[i] = NULL;
        }
    }
    return RET_CONTINUE;
}

int ONScripterLabel::prnumCommand()
{
    int no = script_h.readInt();
    if ( prnum_info[no] ){
        dirty_rect.add( prnum_info[no]->pos );
        delete prnum_info[no];
    }
    prnum_info[no] = new AnimationInfo();
    prnum_info[no]->trans_mode = AnimationInfo::TRANS_STRING;
    prnum_info[no]->abs_flag = true;
    prnum_info[no]->num_of_cells = 1;
    prnum_info[no]->current_cell = 0;
    prnum_info[no]->color_list = new uchar3[ prnum_info[no]->num_of_cells ];
    
    prnum_info[no]->param = script_h.readInt();
    prnum_info[no]->pos.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    prnum_info[no]->pos.y = script_h.readInt() * screen_ratio1 / screen_ratio2;
    prnum_info[no]->font_size_xy[0] = script_h.readInt();
    prnum_info[no]->font_size_xy[1] = script_h.readInt();

    const char *buf = script_h.readStr();
    readColor( &prnum_info[no]->color_list[0], buf );

    char num_buf[12], buf2[7];
    int ptr = 0;

    sprintf( num_buf, "%3d", prnum_info[no]->param );
    if ( prnum_info[no]->param<0 ){
        if ( prnum_info[no]->param>-10 ) {
            buf2[ptr++] = "@"[0];
            buf2[ptr++] = "@"[1];
        }
        buf2[ptr++] = "|"[0];
        buf2[ptr++] = "|"[1];
        sprintf( num_buf, "%d", -prnum_info[no]->param );
    }
    for ( int i=0 ; i<(int)strlen( num_buf ) ; i++ ){
        if ( num_buf[i] == ' ' ) {
            buf2[ptr++] = "@"[0];
            buf2[ptr++] = "@"[1];
            continue;
        }
        getSJISFromInteger( &buf2[ptr], num_buf[i] - '0', false );
        ptr += 2;
        if ( ptr >= 6 ) break; // up to 3 columns (NScripter's restriction)
    }
    setStr( &prnum_info[no]->file_name, buf2 );

    setupAnimationInfo( prnum_info[no] );
    dirty_rect.add( prnum_info[no]->pos );
    
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
    bool loop_flag = true;
    if ( script_h.isName( "playonce" ) )
        loop_flag = false;

    const char *buf = script_h.readStr();
    if ( buf[0] == '*' ){
        cd_play_loop_flag = loop_flag;
        int new_cd_track = atoi( buf + 1 );
#ifdef CONTINUOUS_PLAY        
        if ( current_cd_track != new_cd_track ) {
#endif        
            stopBGM( false );
            cd_play_loop_flag = loop_flag;
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
        
        setStr( &midi_file_name, buf );
        midi_play_loop_flag = loop_flag;
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
    dirty_rect.add( sprite_info[ no ].pos );
    sprite_info[ no ].pos.x += script_h.readInt() * screen_ratio1 / screen_ratio2;
    sprite_info[ no ].pos.y += script_h.readInt() * screen_ratio1 / screen_ratio2;
    dirty_rect.add( sprite_info[ no ].pos );
    if ( script_h.getEndStatus() & ScriptHandler::END_COMMA )
        sprite_info[ no ].trans += script_h.readInt();
    if ( sprite_info[ no ].trans > 255 ) sprite_info[ no ].trans = 255;
    else if ( sprite_info[ no ].trans < 0 ) sprite_info[ no ].trans = 0;

    return RET_CONTINUE;
}

int ONScripterLabel::mpegplayCommand()
{
    script_h.readStr();
    const char *save_buf = script_h.saveStringBuffer();
    
    bool click_flag = (script_h.readInt()==1)?true:false;

    stopBGM( false );
    playMPEG( save_buf, click_flag );

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
        mp3save_flag = true;
        music_play_loop_flag = false;
    }
    else if ( script_h.isName( "mp3loop" ) ||
              script_h.isName( "bgm" ) ){
        mp3save_flag = false;
        music_play_loop_flag = true;
    }
    else{
        mp3save_flag = false;
        music_play_loop_flag = false;
    }

    stopBGM( false );
    
    const char *buf = script_h.readStr();
    if ( buf[0] != '\0' ){
        setStr( &music_file_name, buf );
        if ( playWave( music_file_name, music_play_loop_flag, ONS_MIX_CHANNELS-1 ) )
#if defined(EXTERNAL_MUSIC_PLAYER)
            playMusicFile();
#else
            playMP3( 0 );
#endif
    }
        
    return RET_CONTINUE;
}

int ONScripterLabel::movemousecursorCommand()
{
    int x = script_h.readInt();
    int y = script_h.readInt();

    if ( mouse_rotation_mode == MOUSE_ROTATION_NONE ||
         mouse_rotation_mode == MOUSE_ROTATION_PDA_VGA )
        SDL_WarpMouse( x, y );
    else if ( mouse_rotation_mode == MOUSE_ROTATION_PDA )
        SDL_WarpMouse( screen_height - y - 1, x );
    
    return RET_CONTINUE;
}

int ONScripterLabel::monocroCommand()
{
    script_h.readToken();
    const char *buf = script_h.getStringBuffer();

    if ( !strcmp( buf, "off" ) ){
        monocro_flag_new = false;
    }
    else{
        monocro_flag_new = true;
        readColor( &monocro_color_new, buf );
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

int ONScripterLabel::menu_automodeCommand()
{
    automode_flag = true;
    skip_flag = false;
    printf("menu_automode: change to automode\n");
    
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
    if ( sprite_info[no].visible ) dirty_rect.add( sprite_info[no].pos );
    sprite_info[ no ].visible = v;
    
    const char *buf = script_h.readStr();
    sprite_info[ no ].setImageName( buf );

    sprite_info[ no ].pos.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    sprite_info[ no ].pos.y = script_h.readInt() * screen_ratio1 / screen_ratio2;
    if ( script_h.getEndStatus() & ScriptHandler::END_COMMA )
        sprite_info[ no ].trans = script_h.readInt();
    else
        sprite_info[ no ].trans = 255;

    parseTaggedString( &sprite_info[ no ] );
    setupAnimationInfo( &sprite_info[ no ] );
    if ( sprite_info[no].visible ) dirty_rect.add( sprite_info[no].pos );

    return RET_CONTINUE;
}

int ONScripterLabel::lookbackspCommand()
{
    for ( int i=0 ; i<2 ; i++ )
        lookback_sp[i] = script_h.readInt();

    if ( filelog_flag ){
        script_h.findAndAddLog( ScriptHandler::FILE_LOG, DEFAULT_LOOKBACK_NAME0, true );
        script_h.findAndAddLog( ScriptHandler::FILE_LOG, DEFAULT_LOOKBACK_NAME1, true );
        script_h.findAndAddLog( ScriptHandler::FILE_LOG, DEFAULT_LOOKBACK_NAME2, true );
        script_h.findAndAddLog( ScriptHandler::FILE_LOG, DEFAULT_LOOKBACK_NAME3, true );
    }

    return RET_CONTINUE;
}

int ONScripterLabel::lookbackflushCommand()
{
    current_text_buffer = current_text_buffer->next;
    for ( int i=0 ; i<max_text_buffer-1 ; i++ ){
        current_text_buffer->buffer2_count = 0;
        current_text_buffer = current_text_buffer->next;
    }
    start_text_buffer = current_text_buffer;
    
    return RET_CONTINUE;
}

int ONScripterLabel::lookbackbuttonCommand()
{
    for ( int i=0 ; i<4 ; i++ ){
        const char *buf = script_h.readStr();
        setStr( &lookback_info[i].image_name, buf );
        parseTaggedString( &lookback_info[i] );
        setupAnimationInfo( &lookback_info[i] );
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
        automode_flag = false;
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
    script_h.readToken();
    char loc = script_h.getStringBuffer()[0];

    const char *buf = script_h.readStr();
    
    if ( event_mode & EFFECT_EVENT_MODE ){
        int num = readEffect( &tmp_effect );
        if ( num > 1 ) return doEffect( TMP_EFFECT, NULL, TACHI_EFFECT_IMAGE );
        else           return doEffect( tmp_effect.effect, NULL, TACHI_EFFECT_IMAGE );
    }
    else{
        int no = 0;
        if      ( loc == 'l' ) no = 0;
        else if ( loc == 'c' ) no = 1;
        else if ( loc == 'r' ) no = 2;
        
        dirty_rect.add( tachi_info[ no ].pos );
        tachi_info[ no ].setImageName( buf );
        parseTaggedString( &tachi_info[ no ] );
        setupAnimationInfo( &tachi_info[ no ] );
        if ( tachi_info[ no ].image_surface ){
            tachi_info[ no ].visible = true;
            tachi_info[ no ].pos.x = screen_width * (no+1) / 4 - tachi_info[ no ].pos.w / 2;
            tachi_info[ no ].pos.y = underline_value - tachi_info[ no ].image_surface->h + 1;
            dirty_rect.add( tachi_info[ no ].pos );
        }

        readEffect( &tmp_effect );
        return setEffect( tmp_effect.effect );
    }
}

int ONScripterLabel::jumpfCommand()
{
    jumpf_flag = true;
    script_h.setKidokuskip( false );

    return RET_CONTINUE;
}

int ONScripterLabel::jumpbCommand()
{
    current_link_label_info->label_info = last_tilde.label_info;
    current_link_label_info->current_line = last_tilde.current_line;

    script_h.setCurrent( last_tilde.current_script );
    string_buffer_offset = 0;

    return RET_JUMP;
}

int ONScripterLabel::ispageCommand()
{
    script_h.readInt();

    if ( clickstr_state == CLICK_NEWPAGE )
        script_h.setInt( &script_h.current_variable, 1 );
    else
        script_h.setInt( &script_h.current_variable, 0 );
    
    return RET_CONTINUE;
}

int ONScripterLabel::isfullCommand()
{
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, fullscreen_mode?1:0 );
    
    return RET_CONTINUE;
}

int ONScripterLabel::isskipCommand()
{
    script_h.readInt();

    if ( automode_flag )
        script_h.setInt( &script_h.current_variable, 2 );
    else if ( skip_flag )
        script_h.setInt( &script_h.current_variable, 1 );
    else
        script_h.setInt( &script_h.current_variable, 0 );
    
    return RET_CONTINUE;
}

int ONScripterLabel::isdownCommand()
{
    script_h.readInt();

    if ( current_button_state.down_flag )
        script_h.setInt( &script_h.current_variable, 1 );
    else
        script_h.setInt( &script_h.current_variable, 0 );
    
    return RET_CONTINUE;
}

int ONScripterLabel::inputCommand()
{
    script_h.readStr();
    
    if ( script_h.current_variable.type != ScriptHandler::VAR_STR ) 
        errorAndExit( "input: no string variable." );
    int no = script_h.current_variable.var_no;

    script_h.readStr(); // description
    const char *buf = script_h.readStr(); // default value
    setStr( &script_h.str_variables[no], buf );

    printf( "*** inputCommand(): $%d is set to the default value: %s\n",
            no, buf );
    script_h.readInt(); // maxlen
    script_h.readInt(); // widechar flag
    if ( script_h.getEndStatus() & ScriptHandler::END_COMMA ){
        script_h.readInt(); // window width
        script_h.readInt(); // window height
        script_h.readInt(); // text box width
        script_h.readInt(); // text box height
    }

    return RET_CONTINUE;
}

int ONScripterLabel::getversionCommand()
{
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, ONSCRIPTER_VERSION );

    return RET_CONTINUE;
}

int ONScripterLabel::gettimerCommand()
{
    bool gettimer_flag=false;
    
    if      ( script_h.isName( "gettimer" ) ){
        gettimer_flag = true;
    }
    else if ( script_h.isName( "getbtntimer" ) ){
    }

    script_h.readInt();

    if ( gettimer_flag ){
        script_h.setInt( &script_h.current_variable, SDL_GetTicks() - internal_timer );
    }
    else{
        script_h.setInt( &script_h.current_variable, btnwait_time );
    }
        
    return RET_CONTINUE; 
}

int ONScripterLabel::gettextCommand()
{
    script_h.readStr();
    int no = script_h.current_variable.var_no;

    char *buf = new char[ current_text_buffer->buffer2_count + 1 ];
    int i=0;
    for ( i=0 ; i<current_text_buffer->buffer2_count ; i++ ){
        buf[i] = current_text_buffer->buffer2[i];
    }
    buf[i] = '\0';

    setStr( &script_h.str_variables[no], buf );
    delete[] buf;
    
    return RET_CONTINUE;
}

int ONScripterLabel::gettabCommand()
{
    gettab_flag = true;
    
    return RET_CONTINUE;
}

int ONScripterLabel::getpageupCommand()
{
    getpageup_flag = true;
    
    return RET_CONTINUE;
}

int ONScripterLabel::getpageCommand()
{
    getpageup_flag = true;
    getpagedown_flag = true;
    
    return RET_CONTINUE;
}

int ONScripterLabel::getretCommand()
{
    script_h.readToken();

    if ( script_h.current_variable.type == ScriptHandler::VAR_INT ||
         script_h.current_variable.type == ScriptHandler::VAR_PTR ){
        script_h.setInt( &script_h.current_variable, dll_ret );
    }
    else if ( script_h.current_variable.type == ScriptHandler::VAR_STR ){
        int no = script_h.current_variable.var_no;
        setStr( &script_h.str_variables[no], dll_str );
    }
    else errorAndExit( "getret: no variable." );
    
    return RET_CONTINUE;
}

int ONScripterLabel::getregCommand()
{
    script_h.readStr();
    
    if ( script_h.current_variable.type != ScriptHandler::VAR_STR ) 
        errorAndExit( "getreg: no string variable." );
    int no = script_h.current_variable.var_no;

    const char *buf = script_h.readStr();
    char path[256], key[256];
    strcpy( path, buf );
    buf = script_h.readStr();
    strcpy( key, buf );

    printf("  reading Registry file for [%s] %s\n", path, key );
        
    FILE *fp;
    if ( ( fp = fopen( registry_file, "r" ) ) == NULL ){
        fprintf( stderr, "Cannot open file [%s]\n", registry_file );
        return RET_CONTINUE;
    }

    char reg_buf[256], reg_buf2[256];
    bool found_flag = false;
    while( fgets( reg_buf, 256, fp) && !found_flag ){
        if ( reg_buf[0] == '[' ){
            unsigned int c=0;
            while ( reg_buf[c] != ']' && reg_buf[c] != '\0' ) c++;
            if ( !strncmp( reg_buf + 1, path, (c-1>strlen(path))?(c-1):strlen(path) ) ){
                while( fgets( reg_buf2, 256, fp) ){

                    script_h.pushCurrent( reg_buf2 );
                    buf = script_h.readStr();
                    if ( strncmp( buf,
                                  key,
                                  (strlen(buf)>strlen(key))?strlen(buf):strlen(key) ) ){
                        script_h.popCurrent();
                        continue;
                    }
                    
                    script_h.readToken();
                    if ( script_h.getStringBuffer()[0] != '=' ){
                        script_h.popCurrent();
                        continue;
                    }

                    buf = script_h.readStr();
                    setStr( &script_h.str_variables[no], buf );
                    script_h.popCurrent();
                    printf("  $%d = %s\n", no, script_h.str_variables[no] );
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
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, current_button_state.x * screen_ratio2 / screen_ratio1 );
    
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, current_button_state.y * screen_ratio2 / screen_ratio1 );
    
    return RET_CONTINUE;
}

int ONScripterLabel::getfunctionCommand()
{
    getfunction_flag = true;
    
    return RET_CONTINUE;
}

int ONScripterLabel::getenterCommand()
{
    if ( !force_button_shortcut_flag )
        getenter_flag = true;
    
    return RET_CONTINUE;
}

int ONScripterLabel::getcursorposCommand()
{
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, sentence_font.x( tateyoko_mode ) );
    
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, sentence_font.y( tateyoko_mode ) );
    
    return RET_CONTINUE;
}

int ONScripterLabel::getcursorCommand()
{
    getcursor_flag = true;
    
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
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, count );

    return RET_CONTINUE;
}

int ONScripterLabel::gameCommand()
{
    int i;
    
    current_link_label_info->label_info = script_h.lookupLabel( "start" );
    current_link_label_info->current_line = 0;
    script_h.setCurrent( current_link_label_info->label_info.start_address );
    string_buffer_offset = 0;
    current_mode = NORMAL_MODE;

    //text_speed_no = 1;
    //sentence_font.wait_time = -1;

    /* ---------------------------------------- */
    if ( !lookback_info[0].image_surface ){
        setStr( &lookback_info[0].image_name, DEFAULT_LOOKBACK_NAME0 );
        parseTaggedString( &lookback_info[0] );
        setupAnimationInfo( &lookback_info[0] );
    }
    if ( !lookback_info[1].image_surface ){
        setStr( &lookback_info[1].image_name, DEFAULT_LOOKBACK_NAME1 );
        parseTaggedString( &lookback_info[1] );
        setupAnimationInfo( &lookback_info[1] );
    }
    if ( !lookback_info[2].image_surface ){
        setStr( &lookback_info[2].image_name, DEFAULT_LOOKBACK_NAME2 );
        parseTaggedString( &lookback_info[2] );
        setupAnimationInfo( &lookback_info[2] );
    }
    if ( !lookback_info[3].image_surface ){
        setStr( &lookback_info[3].image_name, DEFAULT_LOOKBACK_NAME3 );
        parseTaggedString( &lookback_info[3] );
        setupAnimationInfo( &lookback_info[3] );
    }
    
    /* ---------------------------------------- */
    /* Load default cursor */
    loadCursor( CURSOR_WAIT_NO, DEFAULT_CURSOR_WAIT, 0, 0 );
    //cursor_info[ CURSOR_WAIT_NO ].deleteImageName(); // a trick for save file
    loadCursor( CURSOR_NEWPAGE_NO, DEFAULT_CURSOR_NEWPAGE, 0, 0 );
    //cursor_info[ CURSOR_NEWPAGE_NO ].deleteImageName(); // a trick for save file

    /* ---------------------------------------- */
    /* Initialize text buffer */
    text_buffer = new TextBuffer[max_text_buffer];
    for ( i=0 ; i<max_text_buffer-1 ; i++ ){
        text_buffer[i].next = &text_buffer[i+1];
        text_buffer[i+1].previous = &text_buffer[i];
        text_buffer[i].buffer2 = NULL;
        text_buffer[i].buffer2_count = 0;
    }
    text_buffer[0].previous = &text_buffer[max_text_buffer-1];
    text_buffer[max_text_buffer-1].next = &text_buffer[0];
    text_buffer[max_text_buffer-1].buffer2 = NULL;
    text_buffer[max_text_buffer-1].buffer2_count = 0;
    start_text_buffer = current_text_buffer = &text_buffer[0];

    clearCurrentTextBuffer();

    /* ---------------------------------------- */
    /* Initialize local variables */
    for ( i=0 ; i<script_h.global_variable_border ; i++ ){
        script_h.num_variables[i] = 0;
        delete[] script_h.str_variables[i];
        script_h.str_variables[i] = NULL;
    }

    return RET_JUMP;
}

int ONScripterLabel::fileexistCommand()
{
    script_h.readInt();
    script_h.pushVariable();
    const char *buf = script_h.readStr();

    script_h.setInt( &script_h.pushed_variable, (script_h.cBR->getFileLength(buf)>0)?1:0 );

    return RET_CONTINUE;
}

int ONScripterLabel::exec_dllCommand()
{
    const char *buf = script_h.readStr();
    char dll_name[256];
    unsigned int c=0;
    while( buf[c] != '/' ) dll_name[c] = buf[c++];
    dll_name[c] = '\0';

    printf("  reading %s for %s\n", dll_file, dll_name );

    FILE *fp;
    if ( ( fp = fopen( dll_file, "r" ) ) == NULL ){
        fprintf( stderr, "Cannot open file [%s]\n", dll_file );
        return RET_CONTINUE;
    }

    char dll_buf[256], dll_buf2[256];
    bool found_flag = false;
    while( fgets( dll_buf, 256, fp) && !found_flag ){
        if ( dll_buf[0] == '[' ){
            c=0;
            while ( dll_buf[c] != ']' && dll_buf[c] != '\0' ) c++;
            if ( !strncmp( dll_buf + 1, dll_name, (c-1>strlen(dll_name))?(c-1):strlen(dll_name) ) ){
                found_flag = true;
                while( fgets( dll_buf2, 256, fp) ){
                    c=0;
                    while ( dll_buf2[c] == ' ' || dll_buf2[c] == '\t' ) c++;
                    if ( !strncmp( &dll_buf2[c], "str", 3 ) ){
                        c+=3;
                        while ( dll_buf2[c] == ' ' || dll_buf2[c] == '\t' ) c++;
                        if ( dll_buf2[c] != '=' ) continue;
                        c++;
                        while ( dll_buf2[c] != '"' ) c++;
                        unsigned int c2 = ++c;
                        while ( dll_buf2[c2] != '"' && dll_buf2[c2] != '\0' ) c2++;
                        dll_buf2[c2] = '\0';
                        setStr( &dll_str, &dll_buf2[c] );
                        printf("  dll_str = %s\n", dll_str );
                    }
                    else if ( !strncmp( &dll_buf2[c], "ret", 3 ) ){
                        c+=3;
                        while ( dll_buf2[c] == ' ' || dll_buf2[c] == '\t' ) c++;
                        if ( dll_buf2[c] != '=' ) continue;
                        c++;
                        while ( dll_buf2[c] == ' ' || dll_buf2[c] == '\t' ) c++;
                        dll_ret = atoi( &dll_buf2[c] );
                        printf("  dll_ret = %d\n", dll_ret );
                    }
                    else if ( dll_buf2[c] == '[' )
                        break;
                }
            }
        }
    }

    if ( !found_flag ) fprintf( stderr, "  The DLL is not found in %s.\n", dll_file );
    fclose( fp );
    
    return RET_CONTINUE;
}

int ONScripterLabel::exbtnCommand()
{
    int sprite_no=-1, no=0;
    ButtonLink *button;
    
    if ( script_h.isName( "exbtn_d" ) ){
        button = &exbtn_d_button_link;
        if ( button->exbtn_ctl ) delete[] button->exbtn_ctl;
    }
    else{
        sprite_no = script_h.readInt();
        no = script_h.readInt();

        if ( sprite_info[ sprite_no ].num_of_cells == 0 ){
            script_h.readStr();
            return RET_CONTINUE;
        }
        
        button = new ButtonLink();
        button->next = root_button_link.next;
        root_button_link.next = button;
    }

    const char *buf = script_h.readStr();
    
    button->button_type = ButtonLink::EX_SPRITE_BUTTON;
    button->sprite_no   = sprite_no;
    button->no          = no;
    button->exbtn_ctl   = new char[ strlen( buf ) + 1 ];
    strcpy( button->exbtn_ctl, buf );
    
    if ( sprite_no >= 0 &&
         ( sprite_info[ sprite_no ].image_surface ||
           sprite_info[ sprite_no ].trans_mode == AnimationInfo::TRANS_STRING ) )
    {
        button->image_rect = button->select_rect = sprite_info[ sprite_no ].pos;
        allocateSelectedSurface( sprite_no, button );
        sprite_info[ sprite_no ].visible = true;
    }

    return RET_CONTINUE;
}

int ONScripterLabel::erasetextwindowCommand()
{
    erase_text_window_mode = script_h.readInt();
    dirty_rect.add( sentence_font_info.pos );

    return RET_CONTINUE;
}

int ONScripterLabel::endCommand()
{
    quit();
    exit(0);
    return RET_CONTINUE; // dummy
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
    int play_mode = WAVE_PLAY;
    bool loop_flag = false;
    
    if ( script_h.isName( "dwaveloop" ) ){
        loop_flag = true;
    }
    else if ( script_h.isName( "dwaveload" ) ){
        play_mode = WAVE_PRELOAD;
    }
    else if ( script_h.isName( "dwaveplayloop" ) ){
        play_mode = WAVE_PLAY_LOADED;
        loop_flag = true;
    }
    else if ( script_h.isName( "dwaveplay" ) ){
        play_mode = WAVE_PLAY_LOADED;
        loop_flag = false;
    }

    int ch = script_h.readInt();
    const char *buf = NULL;
    if ( play_mode != WAVE_PLAY_LOADED ){
        buf = script_h.readStr();
    }
    playWave( buf, loop_flag, ch, play_mode );
        
    return RET_CONTINUE;
}

int ONScripterLabel::dvCommand()
{
    char buf[256];
    
    sprintf( buf, "voice%c%s.wav", DELIMITER, script_h.getStringBuffer()+2 );
    playWave( buf, false, 0 );
    
    return RET_CONTINUE;
}

int ONScripterLabel::delayCommand()
{
    int t = script_h.readInt();

    if ( event_mode & WAIT_INPUT_MODE ){
        event_mode = IDLE_EVENT_MODE;
        return RET_CONTINUE;
    }
    else{
        event_mode = WAIT_INPUT_MODE;
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
            if ( sprite_info[i].visible )
                dirty_rect.add( sprite_info[i].pos );
            if ( sprite_info[i].image_name ){
                sprite_info[i].pos.x = -1000 * screen_ratio1 / screen_ratio2;
                sprite_info[i].pos.y = -1000 * screen_ratio1 / screen_ratio2;
            }
            sprite_info[i].remove();
        }
    else{
        if ( sprite_info[no].visible )
            dirty_rect.add( sprite_info[no].pos );
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
    string_buffer_offset = 0;
    
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
    csel_info.num_xy[1] = 1;

    int counter = 0;
    SelectLink *link = root_select_link.next;
    while ( link ){
        if ( csel_no == counter++ ) break;
        link = link->next;
    }
    if ( link == NULL || link->text == NULL || *link->text == '\0' )
        errorAndExit( "cselbtn: no select text" );

    ButtonLink *button = getSelectableSentence( link->text, &csel_info );
    button->next = root_button_link.next;
    root_button_link.next = button;
    button->button_type = ButtonLink::CUSTOM_SELECT_BUTTON;
    button->no          = button_no;
    button->sprite_no   = csel_no;

    sentence_font.ttf_font = csel_info.ttf_font;

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
    script_h.readToken();
    char loc = script_h.getStringBuffer()[0];
    
    if ( event_mode & EFFECT_EVENT_MODE ){
        int num = readEffect( &tmp_effect );
        if ( num > 1 ) return doEffect( TMP_EFFECT, NULL, TACHI_EFFECT_IMAGE );
        else           return doEffect( tmp_effect.effect, NULL, TACHI_EFFECT_IMAGE );
    }
    else{
        if ( loc == 'l' || loc == 'a' ){
            dirty_rect.add( tachi_info[0].pos );
            tachi_info[0].remove();
        }
        if ( loc == 'c' || loc == 'a' ){
            dirty_rect.add( tachi_info[1].pos );
            tachi_info[1].remove();
        }
        if ( loc == 'r' || loc == 'a' ){
            dirty_rect.add( tachi_info[2].pos );
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

    if ( sprite_info[ sprite_no ].num_of_cells > 0 ){
        if ( no >= sprite_info[ sprite_no ].num_of_cells )
            no = sprite_info[ sprite_no ].num_of_cells-1;
        sprite_info[ sprite_no ].current_cell = no;
        dirty_rect.add( sprite_info[ sprite_no ].pos );
    }
        
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
            if (c2 > 0x9e) {
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
    bool del_flag=false, textbtn_flag=false, selectbtn_flag=false;

    if ( script_h.isName( "btnwait2" ) ){
        display_mode = next_display_mode = NORMAL_DISPLAY_MODE;
    }
    else if ( script_h.isName( "btnwait" ) ){
        del_flag = true;
        display_mode = next_display_mode = NORMAL_DISPLAY_MODE;
    }
    else if ( script_h.isName( "textbtnwait" ) ){
        textbtn_flag = true;
    }
    else if ( script_h.isName( "selectbtnwait" ) ){
        selectbtn_flag = true;
    }

    script_h.readInt();
    
    if ( event_mode & WAIT_BUTTON_MODE )
    {
        btnwait_time = SDL_GetTicks() - internal_button_timer;
        btntime_value = 0;

        if ( textbtn_flag && skip_flag ) current_button_state.button = 0;
        script_h.setInt( &script_h.current_variable, current_button_state.button );

        if ( del_flag ){
            if ( current_button_state.button > 0 ) deleteButtonLink();
            if ( exbtn_d_button_link.exbtn_ctl ){
                delete[] exbtn_d_button_link.exbtn_ctl;
                exbtn_d_button_link.exbtn_ctl = NULL;
            }
        }

        event_mode = IDLE_EVENT_MODE;
        disableGetButtonFlag();
            
        return RET_CONTINUE;
    }
    else{
        shortcut_mouse_line = 0;
        skip_flag = false;

        /* ---------------------------------------- */
        /* Resotre csel button */
        FontInfo f_info = sentence_font;
        ButtonLink *p_button_link = root_button_link.next;
        while ( p_button_link ){
            if ( p_button_link->button_type == ButtonLink::CUSTOM_SELECT_BUTTON ){
                
                f_info.xy[0] = f_info.xy[1] = 0;
                f_info.num_xy[1] = 1;
                f_info.top_xy[0] = p_button_link->image_rect.x * screen_ratio2 / screen_ratio1;
                f_info.top_xy[1] = p_button_link->image_rect.y * screen_ratio2 / screen_ratio1;

                int counter = 0;
                SelectLink *s_link = root_select_link.next;
                while ( s_link ){
                    if ( p_button_link->sprite_no == counter++ ) break;
                    s_link = s_link->next;
                }

                drawString( s_link->text, f_info.off_color, &f_info, false, text_surface );
                dirty_rect.add( p_button_link->image_rect );
                if ( p_button_link->no_selected_surface )
                    SDL_BlitSurface( text_surface, &p_button_link->image_rect, p_button_link->no_selected_surface, NULL );
            }
            else if ( p_button_link->button_type == ButtonLink::SPRITE_BUTTON ||
                      p_button_link->button_type == ButtonLink::EX_SPRITE_BUTTON ){
                bool visible = sprite_info[p_button_link->sprite_no].visible;
                sprite_info[p_button_link->sprite_no].visible = true;
                if ( p_button_link->selected_surface ){
                    sprite_info[p_button_link->sprite_no].current_cell = 1;
                    refreshSurface( effect_dst_surface, &p_button_link->image_rect, isTextVisible()?REFRESH_SHADOW_MODE:REFRESH_NORMAL_MODE );
                    SDL_BlitSurface( effect_dst_surface, &p_button_link->image_rect, p_button_link->selected_surface, NULL );
                }
                sprite_info[p_button_link->sprite_no].current_cell = 0;
                if ( p_button_link->no_selected_surface ){
                    refreshSurface( effect_dst_surface, &p_button_link->image_rect, isTextVisible()?REFRESH_SHADOW_MODE:REFRESH_NORMAL_MODE );
                    SDL_BlitSurface( effect_dst_surface, &p_button_link->image_rect, p_button_link->no_selected_surface, NULL );
                }
                sprite_info[p_button_link->sprite_no].visible = visible;
            }
            else{
                if ( p_button_link->no_selected_surface )
                    SDL_BlitSurface( text_surface, &p_button_link->image_rect, p_button_link->no_selected_surface, NULL );
            }

            p_button_link = p_button_link->next;
        }
        refreshSurface( accumulation_surface, NULL, REFRESH_NORMAL_MODE );
        //refreshSurface( text_surface, NULL, isTextVisible()?REFRESH_SHADOW_MODE:REFRESH_NORMAL_MODE );

        if ( isTextVisible() ){
            display_mode = next_display_mode = TEXT_DISPLAY_MODE;
        }

        if ( exbtn_d_button_link.exbtn_ctl ){
            decodeExbtnControl( text_surface, exbtn_d_button_link.exbtn_ctl  );
        }
        flush();

        flushEvent();
        event_mode = WAIT_BUTTON_MODE;
        refreshMouseOverButton();
        
        if ( btntime_value > 0 ){
            startTimer( btntime_value );
            if ( usewheel_flag ) current_button_state.button = -5;
            else                 current_button_state.button = -2;
        }
        internal_button_timer = SDL_GetTicks();

        if ( textbtn_flag ){
            event_mode |= WAIT_TEXTBTN_MODE;
            if ( btntime_value == 0 ){
                if ( !wave_sample[0] && automode_flag ){
                    if ( automode_time < 0 )
                        startTimer( -automode_time * current_text_buffer->buffer2_count / 2 );
                    else
                        startTimer( automode_time );
                    current_button_state.button = 0;
                }
                else{
                    event_mode |= WAIT_ANIMATION_MODE;
                    advancePhase();
                }
            }
        }
        return RET_WAIT;
    }
}

int ONScripterLabel::btntimeCommand()
{
    if ( script_h.isName( "btntime2" ) )
        btntime2_flag = true;
    else
        btntime2_flag = false;
    
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
    script_h.readToken();
    
    if ( strcmp( script_h.getStringBuffer(), "clear" ) ){
        const char *buf = script_h.readStr( true );
        
        btndef_info.remove();
        if ( buf[0] != '\0' ){
            btndef_info.setImageName( buf );
            parseTaggedString( &btndef_info );
            setupAnimationInfo( &btndef_info );
            SDL_SetAlpha( btndef_info.image_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
        }
    }
    
    deleteButtonLink();

    disableGetButtonFlag();
    
    return RET_CONTINUE;
}

int ONScripterLabel::btnCommand()
{
    SDL_Rect src_rect;

    ButtonLink *button = new ButtonLink();
    
    button->no           = script_h.readInt();
    button->image_rect.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    button->image_rect.y = script_h.readInt() * screen_ratio1 / screen_ratio2;
    button->image_rect.w = script_h.readInt() * screen_ratio1 / screen_ratio2;
    button->image_rect.h = script_h.readInt() * screen_ratio1 / screen_ratio2;
    button->select_rect = button->image_rect;

    src_rect.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    src_rect.y = script_h.readInt() * screen_ratio1 / screen_ratio2;
    src_rect.w = button->image_rect.w;
    src_rect.h = button->image_rect.h;

    button->selected_surface    = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG,
                                                        button->image_rect.w,
                                                        button->image_rect.h,
                                                        32, rmask, gmask, bmask, amask );
    SDL_SetAlpha( button->selected_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
    button->no_selected_surface = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG,
                                                        button->image_rect.w,
                                                        button->image_rect.h,
                                                        32, rmask, gmask, bmask, amask );
    SDL_SetAlpha( button->no_selected_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
    SDL_BlitSurface( btndef_info.image_surface, &src_rect, button->selected_surface, NULL );

    button->next = root_button_link.next;
    root_button_link.next = button;
    
    return RET_CONTINUE;
}

int ONScripterLabel::brCommand()
{
    int ret = enterTextDisplayMode();
    if ( ret != RET_NOMATCH ) return ret;

    sentence_font.xy[0] = 0;
    sentence_font.xy[1]++;
    current_text_buffer->buffer2[current_text_buffer->buffer2_count++] = 0x0a;
    if ( internal_saveon_flag ){
        internal_saveon_flag = false;
        saveSaveFile(-1);
    }

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
        shiftRect( src_rect, clipped );

#if defined(USE_OVERLAY)
        SDL_LockYUVOverlay( screen_overlay );
        SDL_LockSurface( btndef_info.image_surface );
        Uint32 *src = (Uint32*)btndef_info.image_surface->pixels + btndef_info.image_surface->w * src_rect.y + src_rect.x;
        Uint8  *dst_y = screen_overlay->pixels[0] + screen_overlay->w * dst_rect.y + dst_rect.x;
        Uint8  *dst_u = screen_overlay->pixels[1] + screen_overlay->w * dst_rect.y + dst_rect.x;
        Uint8  *dst_v = screen_overlay->pixels[2] + screen_overlay->w * dst_rect.y + dst_rect.x;
        for ( int i=dst_rect.y ; i<dst_rect.y+dst_rect.h ; i++ ){
            for ( int j=dst_rect.x ; j<dst_rect.x+dst_rect.w ; j++, src++ ){
                *dst_y++ = *src & 0xff;
                *dst_u++ = 0x80;
                *dst_v++ = 0x80;
            }
            src += btndef_info.image_surface->w - dst_rect.w;
            dst_y += screen_overlay->w - dst_rect.w;
            dst_u += (screen_overlay->w - dst_rect.w)/2;
            dst_v += (screen_overlay->w - dst_rect.w)/2;
        }
        SDL_UnlockSurface( btndef_info.image_surface );
        SDL_UnlockYUVOverlay( screen_overlay );

        SDL_Rect screen_rect;
        screen_rect.x = screen_rect.y = 0;
        screen_rect.w = screen_width;
        screen_rect.h = screen_height;
        SDL_DisplayYUVOverlay( screen_overlay, &screen_rect );
#else    
#ifndef SCREEN_ROTATION    
        SDL_BlitSurface( btndef_info.image_surface, &src_rect, screen_surface, &dst_rect );
        SDL_UpdateRect( screen_surface, dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h );
#else    
        blitRotation( btndef_info.image_surface, &src_rect, screen_surface, &dst_rect );
        SDL_UpdateRect( screen_surface, dst_rect.y, screen_width - dst_rect.x - dst_rect.w, dst_rect.h, dst_rect.w );
#endif
#endif        
        dirty_rect.clear();
    }
    else{
        resizeSurface( btndef_info.image_surface, &src_rect, text_surface, &dst_rect );
        dirty_rect.add( dst_rect );
        flush( &dst_rect );
    }
    
    return RET_CONTINUE;
}

int ONScripterLabel::bgCommand()
{
    script_h.readToken();

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
        dirty_rect.fill( screen_width, screen_height );

        if ( !strcmp( script_h.getStringBuffer(), "white" ) ){
            setStr( &bg_info.file_name, "white" );
            bg_info.color[0] = bg_info.color[1] = bg_info.color[2] = 0xff;
        }
        else if ( !strcmp( script_h.getStringBuffer(), "black" ) ){
            setStr( &bg_info.file_name, "black" );
            bg_info.color[0] = bg_info.color[1] = bg_info.color[2] = 0x00;
        }
        else{
            const char *buf = script_h.readStr( true );
            
            if ( buf[0] == '#' ){
                setStr( &bg_info.file_name, buf );
                readColor( &bg_info.color, buf );
            }
            else{
                bg_info.color[0] = bg_info.color[1] = bg_info.color[2] = 0x00;
                setStr( &bg_info.image_name, buf );
                parseTaggedString( &bg_info );
                setupAnimationInfo( &bg_info );
                bg_effect_image = BG_EFFECT_IMAGE;
            }
        }

        readEffect( &tmp_effect );
        return setEffect( tmp_effect.effect );
    }
}

int ONScripterLabel::barclearCommand()
{
    for ( int i=0 ; i<MAX_PARAM_NUM ; i++ ) {
        if ( bar_info[i] ) {
            dirty_rect.add( bar_info[i]->pos );
            delete bar_info[i];
            bar_info[i] = NULL;
        }
    }
    return RET_CONTINUE;
}

int ONScripterLabel::barCommand()
{
    int no = script_h.readInt();
    if ( bar_info[no] ){
        dirty_rect.add( bar_info[no]->pos );
        bar_info[no]->remove();
    }
    else{
        bar_info[no] = new AnimationInfo();
    }
    bar_info[no]->trans_mode = AnimationInfo::TRANS_COPY;
    bar_info[no]->abs_flag = true;
    bar_info[no]->num_of_cells = 1;
    bar_info[no]->current_cell = 0;
    bar_info[no]->alpha_offset = 0;

    bar_info[no]->param = script_h.readInt();
    bar_info[no]->pos.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    bar_info[no]->pos.y = script_h.readInt() * screen_ratio1 / screen_ratio2;
                          
    bar_info[no]->pos.w = script_h.readInt() * screen_ratio1 / screen_ratio2;
    bar_info[no]->pos.h = script_h.readInt() * screen_ratio1 / screen_ratio2;
    bar_info[no]->max_param = script_h.readInt();
    if ( bar_info[no]->max_param == 0 ) errorAndExit( "bar: max = 0." );

    const char *buf = script_h.readStr();
    readColor( &bar_info[no]->color, buf );

    dirty_rect.add( bar_info[no]->pos );

    bar_info[no]->max_width = bar_info[no]->pos.w;
    bar_info[no]->pos.w = bar_info[no]->pos.w * bar_info[no]->param / bar_info[no]->max_param;
    if ( bar_info[no]->pos.w > 0 ){
        bar_info[no]->image_surface = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG, bar_info[no]->pos.w, bar_info[no]->pos.h, 32, rmask, gmask, bmask, amask );
        SDL_SetAlpha( bar_info[no]->image_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
        SDL_FillRect( bar_info[no]->image_surface, NULL, SDL_MapRGB( bar_info[no]->image_surface->format, bar_info[no]->color[0], bar_info[no]->color[1], bar_info[no]->color[2] ) );
    }

    return RET_CONTINUE;
}

int ONScripterLabel::aviCommand()
{
    script_h.readStr();
    const char *save_buf = script_h.saveStringBuffer();
    
    bool click_flag = (script_h.readInt()==1)?true:false;

    stopBGM( false );
    playAVI( save_buf, click_flag );

    return RET_CONTINUE;
}

int ONScripterLabel::automode_timeCommand()
{
    automode_time = script_h.readInt();
    
    return RET_CONTINUE;
}

int ONScripterLabel::autoclickCommand()
{
    autoclick_time = script_h.readInt();

    return RET_CONTINUE;
}

int ONScripterLabel::amspCommand()
{
    int no = script_h.readInt();
    dirty_rect.add( sprite_info[ no ].pos );
    sprite_info[ no ].pos.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    sprite_info[ no ].pos.y = script_h.readInt() * screen_ratio1 / screen_ratio2;

    if ( script_h.getEndStatus() & ScriptHandler::END_COMMA )
        sprite_info[ no ].trans = script_h.readInt();

    if ( sprite_info[ no ].trans > 255 ) sprite_info[ no ].trans = 255;
    else if ( sprite_info[ no ].trans < 0 ) sprite_info[ no ].trans = 0;
    dirty_rect.add( sprite_info[ no ].pos );
    
    return RET_CONTINUE;
}

int ONScripterLabel::allspresumeCommand()
{
    all_sprite_hide_flag = false;
    for ( int i=0 ; i<MAX_SPRITE_NUM ; i++ ){
        if ( sprite_info[i].visible )
            dirty_rect.add( sprite_info[i].pos );
    }
    return RET_CONTINUE;
}

int ONScripterLabel::allsphideCommand()
{
    all_sprite_hide_flag = true;
    for ( int i=0 ; i<MAX_SPRITE_NUM ; i++ ){
        if ( sprite_info[i].visible )
            dirty_rect.add( sprite_info[i].pos );
    }
    return RET_CONTINUE;
}

