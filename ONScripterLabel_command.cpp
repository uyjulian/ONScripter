/* -*- C++ -*-
 * 
 *  ONScripterLabel_command.cpp - Command executer of ONScripter
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

    setStr( &wave_file_name, script_h.readStr() );
    playWave( wave_file_name, wave_play_loop_flag, DEFAULT_WAVE_CHANNEL );
        
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
    
    return RET_WAIT;
}

int ONScripterLabel::waitCommand()
{
    startTimer( script_h.readInt() );

    return RET_WAIT;
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
    
    sprintf( buf, RELATIVEPATH "wav%c%s.wav", DELIMITER, script_h.getStringBuffer()+1 );
    playWave( buf, false, DEFAULT_WAVE_CHANNEL );
    
    return RET_CONTINUE;
}

int ONScripterLabel::trapCommand()
{
    if      ( script_h.isName( "lr_trap" ) ){
        trap_mode = TRAP_LEFT_CLICK | TRAP_RIGHT_CLICK;
    }
    else if ( script_h.isName( "trap" ) ){
        trap_mode = TRAP_LEFT_CLICK;
    }

    if ( script_h.compareString("off") ){
        script_h.readLabel();
        trap_mode = TRAP_NONE;
        return RET_CONTINUE;
    }

    const char *buf = script_h.readLabel();
    if ( buf[0] == '*' ){
        if ( trap_dist ) delete[] trap_dist;
        trap_dist = new char[ strlen(buf) ];
        memcpy( trap_dist, buf+1, strlen(buf) );
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
        dirty_rect.fill( screen_width, screen_height );
        flush(REFRESH_SHADOW_TEXT_MODE);
        display_mode = next_display_mode = TEXT_DISPLAY_MODE;
    }

    return RET_CONTINUE;
}

int ONScripterLabel::textoffCommand()
{
    text_on_flag = false;
    if ( display_mode & TEXT_DISPLAY_MODE ){
        dirty_rect.fill( screen_width, screen_height );
        flush(REFRESH_NORMAL_MODE);
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
    if ( textgosub_clickstr_state == CLICK_NEWPAGE ){
        newPage( true );
        clickstr_state = CLICK_NONE;
    }
    else if ( textgosub_clickstr_state == (CLICK_WAIT | CLICK_EOL) ){
        if ( !sentence_font.isLineEmpty() &&
             !new_line_skip_flag ){
            current_text_buffer->addBuffer( 0x0a );
            sentence_font.newLine();
        }
    }
    
    return RET_CONTINUE;
}

int ONScripterLabel::tateyokoCommand()
{
    sentence_font.setTateyokoMode( script_h.readInt() );
    
    return RET_CONTINUE;
}

int ONScripterLabel::talCommand()
{
    int ret = leaveTextDisplayMode();
    if ( ret != RET_NOMATCH ) return ret;
    
    char loc = script_h.readLabel()[0];
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

        if      ( trans > 256 ) trans = 256;
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
        const char *buf = script_h.readLabel();
        if ( count++ == no ){
            setCurrentLinkLabel( buf+1 );
            break;
        }
    }

    return RET_CONTINUE;
}

int ONScripterLabel::systemcallCommand()
{
    system_menu_mode = getSystemCallNo( script_h.readLabel() );
    enterSystemCall();
    advancePhase();
    
    return RET_WAIT;
}

int ONScripterLabel::stopCommand()
{
    wavestopCommand();
    stopBGM( false );
    
    return RET_CONTINUE;
}

int ONScripterLabel::splitCommand()
{
    script_h.readStr();
    const char *save_buf = script_h.saveStringBuffer();
    
    char delimiter = script_h.readStr()[0];

    char token[256];
    while( script_h.getEndStatus() & ScriptHandler::END_COMMA ){

        unsigned int c=0;
        while( save_buf[c] != delimiter && save_buf[c] != '\0' ) c++;
        memcpy( token, save_buf, c );
        token[c] = '\0';
        
        script_h.readVariable();
        if ( script_h.current_variable.type & ScriptHandler::VAR_INT ||
             script_h.current_variable.type & ScriptHandler::VAR_ARRAY ){
            script_h.setInt( &script_h.current_variable, atoi(token) );
        }
        else if ( script_h.current_variable.type & ScriptHandler::VAR_STR ){
            setStr( &script_h.str_variables[ script_h.current_variable.var_no ], token );
        }

        if (save_buf[c] == '\0') return RET_CONTINUE;
        save_buf += c+1;
    }
    
    return RET_CONTINUE;
}

int ONScripterLabel::spstrCommand()
{
    decodeExbtnControl( accumulation_surface, script_h.readStr() );
    
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
    root_button_link.insert( button );

    button->button_type = ButtonLink::SPRITE_BUTTON;
    button->sprite_no   = sprite_no;
    button->no          = no;

    if ( sprite_info[ sprite_no ].image_surface ||
         sprite_info[ sprite_no ].trans_mode == AnimationInfo::TRANS_STRING )
    {
        button->image_rect = button->select_rect = sprite_info[ sprite_no ].pos;
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

    for ( int i=1 ; i<ONS_MIX_CHANNELS+ONS_MIX_EXTRA_CHANNELS ; i++ )
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
    script_h.readStr();
    const char* buf = script_h.saveStringBuffer();
    int x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    int y = script_h.readInt() * screen_ratio1 / screen_ratio2;

    loadCursor( no, buf, x, y, abs_flag );
    
    return RET_CONTINUE;
}

int ONScripterLabel::selectCommand()
{
    int ret = enterTextDisplayMode();
    if ( ret != RET_NOMATCH ) return ret;

    int select_mode = SELECT_GOTO_MODE;
    SelectLink *last_select_link;

    if ( script_h.isName( "selnum" ) ){
        select_mode = SELECT_NUM_MODE;
        script_h.readVariable();
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
        
        if ( current_button_state.button == 0 ) return RET_WAIT | RET_REREAD;
        
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
            setCurrentLinkLabel( last_select_link->label );
            ret = RET_CONTINUE;
        }
        else if ( select_mode == SELECT_GOSUB_MODE ){
            current_link_label_info->current_line = select_label_info.current_line;
            gosubReal( last_select_link->label, select_label_info.next_script );

            ret = RET_CONTINUE;
        }
        else{ // selnum
            script_h.setInt( &script_h.pushed_variable, current_button_state.button - 1 );
            current_link_label_info->current_line = select_label_info.current_line;
            script_h.setCurrent( select_label_info.next_script );
            ret = RET_CONTINUE;
        }
        deleteSelectLink();

        newPage( true );

        return ret;
    }
    else{
        bool comma_flag = true;
        if ( select_mode == SELECT_CSEL_MODE ){
            saveoffCommand();
        }
        shortcut_mouse_line = -1;

        skip_flag = false;
        automode_flag = false;
        int xy[2];
        xy[0] = sentence_font.xy[0];
        xy[1] = sentence_font.xy[1];

        if ( selectvoice_file_name[SELECTVOICE_OPEN] )
            playWave( selectvoice_file_name[SELECTVOICE_OPEN], false, DEFAULT_WAVE_CHANNEL );

        last_select_link = &root_select_link;
        select_label_info.current_line = current_link_label_info->current_line;

        while(1){
            if ( script_h.getNext()[0] != 0x0a && comma_flag == true ){

                const char *buf = script_h.readStr();
                comma_flag = (script_h.getEndStatus() & ScriptHandler::END_COMMA);
                if ( select_mode != SELECT_NUM_MODE && !comma_flag ) errorAndExit( "select: comma is needed here." );

                // Text part
                SelectLink *slink = new SelectLink();
                setStr( &slink->text, buf );
                //printf("Select text %s\n", slink->text);

                // Label part
                if (select_mode != SELECT_NUM_MODE){
                    script_h.readLabel();
                    setStr( &slink->label, script_h.getStringBuffer()+1 );
                    //printf("Select label %s\n", slink->label );
                }
                last_select_link->next = slink;
                last_select_link = last_select_link->next;

                comma_flag = (script_h.getEndStatus() & ScriptHandler::END_COMMA);
                //printf("2 comma %d %c %x\n", comma_flag, script_h.getCurrent()[0], script_h.getCurrent()[0]);
            }
            else if (script_h.getNext()[0] == 0x0a){
                //printf("comma %d\n", comma_flag);
                char *buf = script_h.getNext() + 1; // consume eol
                while ( *buf == ' ' || *buf == '\t' ) buf++;
                
                if (comma_flag && *buf == ',')
                    errorAndExit( "select: double comma." );

                bool comma2_flag = false;
                if (*buf == ','){
                    comma2_flag = true;
                    buf++;
                    while ( *buf == ' ' || *buf == '\t' ) buf++;
                }
                script_h.setCurrent(buf);
                
                if (*buf == 0x0a){
                    comma_flag |= comma2_flag;
                    continue;
                }
                
                if (!comma_flag && !comma2_flag){
                    select_label_info.next_script = buf;
                    //printf("select: stop at the end of line\n");
                    break;
                }

                //printf("continue\n");
                comma_flag = true;
            }
            else{ // if select ends at the middle of the line
                select_label_info.next_script = script_h.getNext();
                //printf("select: stop at the middle of the line\n");
                break;
            }
        }

        if ( select_mode != SELECT_CSEL_MODE ){
            last_select_link = root_select_link.next;
            int counter = 1;
            while( last_select_link ){
                if ( *last_select_link->text ){
                    ButtonLink *button = getSelectableSentence( last_select_link->text, &sentence_font );
                    root_button_link.insert( button );
                    button->no = counter;
                }
                counter++;
                last_select_link = last_select_link->next;
            }
        }

        if ( select_mode == SELECT_CSEL_MODE ){
            setCurrentLinkLabel( "customsel" );
            return RET_CONTINUE;
        }
        sentence_font.setXY( xy[0], xy[1] );

        flush( refreshMode() );
        
        flushEvent();
        event_mode = WAIT_INPUT_MODE | WAIT_BUTTON_MODE | WAIT_TIMER_MODE;
        advancePhase();
        refreshMouseOverButton();

        return RET_WAIT | RET_REREAD;
    }
}

int ONScripterLabel::savetimeCommand()
{
    int no = script_h.readInt();

    SaveFileInfo info;
    searchSaveFile( info, no );

    script_h.readVariable();
    if ( !info.valid ){
        script_h.setInt( &script_h.current_variable, 0 );
        for ( int i=0 ; i<3 ; i++ )
            script_h.readVariable();
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

int ONScripterLabel::savescreenshotCommand()
{
    if      ( script_h.isName( "savescreenshot" ) ){
    }
    else if ( script_h.isName( "savescreenshot2" ) ){
    }

    const char *buf = script_h.readStr();
    char filename[256];
    
    char *ext = strrchr( buf, '.' );
    if ( ext && (!strcmp( ext+1, "BMP" ) || !strcmp( ext+1, "bmp" ) ) ){
        sprintf( filename, "%s%s", archive_path, buf );
        for ( unsigned int i=0 ; i<strlen( filename ) ; i++ )
            if ( filename[i] == '/' || filename[i] == '\\' )
                filename[i] = DELIMITER;
        SDL_SaveBMP( screenshot_surface, filename );
    }
    else
        printf("savescreenshot: file %s is not supported.\n", buf );

    return RET_CONTINUE;
}

int ONScripterLabel::saveonCommand()
{
    saveon_flag = true;

    return RET_CONTINUE;
}

int ONScripterLabel::saveoffCommand()
{
    if (saveon_flag && internal_saveon_flag) saveSaveFile( -1 ); 
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
        //script_h.readLabel(); // save point is the next token to no
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
    int upper, lower;
    
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
    resetSentenceFont();

    skip_flag      = false;
    monocro_flag   = false;
    nega_mode      = 0;
    saveon_flag    = true;
    clickstr_state = CLICK_NONE;
    rubyon_flag    = false;
    
    deleteLabelLink();
    setCurrentLinkLabel( "start" );
    
    for ( i=0 ; i<3 ; i++ ) human_order[i] = 2-i; // "rcl"
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

    dirty_rect.fill( screen_width, screen_height );

    return RET_CONTINUE;
}

int ONScripterLabel::repaintCommand()
{
    dirty_rect.fill( screen_width, screen_height );
    flush( refreshMode() );
    
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
            //SDL_BlitSurface( accumulation_surface, NULL, effect_src_surface, NULL );
            blitSurface( accumulation_surface, NULL, effect_dst_surface, NULL );
        }
        tmp_effect.effect = CUSTOM_EFFECT_NO + quake_type;
        return doEffect( TMP_EFFECT, NULL, DIRECT_EFFECT_IMAGE );
    }
    else{
        dirty_rect.fill( screen_width, screen_height );
        setEffect( 2 ); // 2 is dummy value
        return RET_WAIT | RET_REREAD; // RET_WAIT de yoi?
        //return RET_WAIT_NEXT;
    }
}

int ONScripterLabel::puttextCommand()
{
    int ret = enterTextDisplayMode();
    if ( ret != RET_NOMATCH ) return ret;

    script_h.readStr();
    script_h.addStringBuffer(0x0a);
    string_buffer_offset = 0;
    if ( script_h.getEndStatus() & ScriptHandler::END_1BYTE_CHAR )
        string_buffer_offset = 1; // skip the heading `
    script_h.setText( true );

    return RET_CONTINUE | RET_NOREAD;
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
    prnum_info[no]->setCell(0);
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
            buf2[ptr++] = "�@"[0];
            buf2[ptr++] = "�@"[1];
        }
        buf2[ptr++] = "�|"[0];
        buf2[ptr++] = "�|"[1];
        sprintf( num_buf, "%d", -prnum_info[no]->param );
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
    dirty_rect.add( prnum_info[no]->pos );
    
    return RET_CONTINUE;
}

int ONScripterLabel::printCommand()
{
    int ret = leaveTextDisplayMode();
    if ( ret != RET_NOMATCH ) return ret;

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
        internal_midi_play_loop_flag = loop_flag;
        playMIDIFile(midi_file_name);
    }

    return RET_CONTINUE;
}

int ONScripterLabel::ofscpyCommand()
{
#ifdef USE_OPENGL
    if (texture_buffer_size < screen_width*screen_height*4){
        if (texture_buffer) delete[] texture_buffer;
        texture_buffer_size = screen_width*screen_height*4;
        texture_buffer = new unsigned char[texture_buffer_size];
    }
    glReadPixels(0, 0, screen_width, screen_height, GL_RGBA, GL_UNSIGNED_BYTE, texture_buffer);
    
    SDL_LockSurface(accumulation_surface);
    for (int i=0 ; i<screen_height ; i++)
        memcpy( (Uint32*)accumulation_surface->pixels + i*screen_width,
                texture_buffer + (screen_height - 1 - i)*screen_width*4,
                screen_width * 4);
    SDL_UnlockSurface(accumulation_surface);
#else    
    SDL_BlitSurface( screen_surface, NULL, accumulation_surface, NULL );
#endif    

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
    if ( sprite_info[ no ].trans > 256 ) sprite_info[ no ].trans = 256;
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
    bool loop_flag = false;
    if      ( script_h.isName( "mp3save" ) ){
        mp3save_flag = true;
    }
    else if ( script_h.isName( "bgmonce" ) ){
        mp3save_flag = false;
    }
    else if ( script_h.isName( "mp3loop" ) ||
              script_h.isName( "bgm" ) ){
        mp3save_flag = true;
        loop_flag = true;
    }
    else{
        mp3save_flag = false;
    }

    stopBGM( false );
    music_play_loop_flag = loop_flag;

    const char *buf = script_h.readStr();
    if ( buf[0] != '\0' ){
        setStr( &music_file_name, buf );
        if ( playWave( music_file_name, music_play_loop_flag, MIX_BGM_CHANNEL ) )
#if defined(EXTERNAL_MUSIC_PLAYER)
            if (playMusicFile()){
#else
            if (playMP3( 0 )){
#endif
                internal_midi_play_loop_flag = music_play_loop_flag;
                playMIDIFile(music_file_name);
            }
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
    if ( script_h.compareString( "off" ) ){
        script_h.readLabel();
        monocro_flag_new = false;
    }
    else{
        monocro_flag_new = true;
        readColor( &monocro_color_new, script_h.readStr() );
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
    if ( sprite_info[no].visible )
        dirty_rect.add( sprite_info[no].pos );
    sprite_info[ no ].visible = v;
    
    const char *buf = script_h.readStr();
    sprite_info[ no ].setImageName( buf );

    sprite_info[ no ].pos.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    sprite_info[ no ].pos.y = script_h.readInt() * screen_ratio1 / screen_ratio2;
    if ( script_h.getEndStatus() & ScriptHandler::END_COMMA )
        sprite_info[ no ].trans = script_h.readInt();
    else
        sprite_info[ no ].trans = 256;

    parseTaggedString( &sprite_info[ no ] );
    setupAnimationInfo( &sprite_info[ no ] );
    if ( sprite_info[no].visible )
        dirty_rect.add( sprite_info[no].pos );

    return RET_CONTINUE;
}

int ONScripterLabel::loopbgmstopCommand()
{
    if ( wave_sample[MIX_LOOPBGM_CHANNEL0] ){
        Mix_Pause(MIX_LOOPBGM_CHANNEL0);
        Mix_FreeChunk( wave_sample[MIX_LOOPBGM_CHANNEL0] );
        wave_sample[MIX_LOOPBGM_CHANNEL0] = NULL;
    }
    if ( wave_sample[MIX_LOOPBGM_CHANNEL1] ){
        Mix_Pause(MIX_LOOPBGM_CHANNEL1);
        Mix_FreeChunk( wave_sample[MIX_LOOPBGM_CHANNEL1] );
        wave_sample[MIX_LOOPBGM_CHANNEL1] = NULL;
    }
    setStr(&loop_bgm_name[0], NULL);
    
    return RET_CONTINUE;
}

int ONScripterLabel::loopbgmCommand()
{
    const char *buf = script_h.readStr();
    setStr( &loop_bgm_name[0], buf );
    buf = script_h.readStr();
    setStr( &loop_bgm_name[1], buf );

    playWave( loop_bgm_name[1], false, MIX_LOOPBGM_CHANNEL1, WAVE_PRELOAD );
    playWave( loop_bgm_name[0], false, MIX_LOOPBGM_CHANNEL0 );
    
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
    clearCurrentTextBuffer();
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
    int x = script_h.readInt();
    int y = script_h.readInt();
    sentence_font.setXY( x, y );

    return RET_CONTINUE;
}

int ONScripterLabel::loadgameCommand()
{
    int no = script_h.readInt();

    if ( no < 0 )
        errorAndExit( "loadgame: no < 0." );

    if ( loadSaveFile( no ) ) return RET_CONTINUE;
    else {
        saveon_flag = true;
        internal_saveon_flag = true;
        skip_flag = false;
        automode_flag = false;
        deleteButtonLink();
        deleteSelectLink();
        key_pressed_flag = false;
        saveon_flag = true;
        
        script_h.readToken();
        string_buffer_offset = 0;
        
        if ( event_mode & WAIT_INPUT_MODE ) return RET_WAIT | RET_NOREAD;
        return RET_CONTINUE | RET_NOREAD;
    }
}

int ONScripterLabel::ldCommand()
{
    int ret = leaveTextDisplayMode();
    if ( ret != RET_NOMATCH ) return ret;

    char loc = script_h.readLabel()[0];
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
    char *buf = script_h.getNext();
    while(*buf != '\0' && *buf != '~') buf++;
    if (*buf == '~') buf++;
    
    script_h.setCurrent(buf);
    current_link_label_info->label_info = script_h.getLabelByAddress(buf);
    current_link_label_info->current_line = script_h.getLineByAddress(buf);
    
    return RET_CONTINUE;
}

int ONScripterLabel::jumpbCommand()
{
    current_link_label_info->label_info = last_tilde.label_info;
    current_link_label_info->current_line = last_tilde.current_line;

    script_h.setCurrent( last_tilde.next_script );

    return RET_CONTINUE;
}

int ONScripterLabel::ispageCommand()
{
    script_h.readInt();

    if ( textgosub_clickstr_state == CLICK_NEWPAGE )
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

int ONScripterLabel::humanorderCommand()
{
    int ret = leaveTextDisplayMode();
    if ( ret != RET_NOMATCH ) return ret;
    
    const char *buf = script_h.readStr();
    int i;
    for ( i=0 ; i<3 ; i++ ){
        if      ( buf[i] == 'l' ) human_order[i] = 0;
        else if ( buf[i] == 'c' ) human_order[i] = 1;
        else if ( buf[i] == 'r' ) human_order[i] = 2;
    }

    if ( event_mode & EFFECT_EVENT_MODE ){
        int num = readEffect( &tmp_effect );
        if ( num > 1 ) return doEffect( TMP_EFFECT, &bg_info, bg_effect_image );
        else           return doEffect( tmp_effect.effect, &bg_info, bg_effect_image );
    }
    else{
        for ( i=0 ; i<3 ; i++ )
            dirty_rect.add( tachi_info[i].pos );

        readEffect( &tmp_effect );
        return setEffect( tmp_effect.effect );
    }
}

int ONScripterLabel::getzxcCommand()
{
    getzxc_flag = true;

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
    int i, j;
    for ( i=0, j=0 ; i<current_text_buffer->buffer2_count ; i++ ){
        if ( current_text_buffer->buffer2[i] != 0x0a )
            buf[j++] = current_text_buffer->buffer2[i];
    }
    buf[j] = '\0';

    setStr( &script_h.str_variables[no], buf );
    delete[] buf;
    
    return RET_CONTINUE;
}

int ONScripterLabel::gettabCommand()
{
    gettab_flag = true;
    
    return RET_CONTINUE;
}

int ONScripterLabel::getscreenshotCommand()
{
    int w = script_h.readInt();
    int h = script_h.readInt();
    if ( w == 0 ) w = 1;
    if ( h == 0 ) h = 1;

    if ( screenshot_surface &&
         screenshot_surface->w != w &&
         screenshot_surface->h != h ){
        SDL_FreeSurface( screenshot_surface );
        screenshot_surface = NULL;
    }

    if ( screenshot_surface == NULL )
        screenshot_surface = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG, w, h, 32, rmask, gmask, bmask, amask );

#ifdef USE_OPENGL
    if (texture_buffer_size < screen_width*screen_height*4){
        if (texture_buffer) delete[] texture_buffer;
        texture_buffer_size = screen_width*screen_height*4;
        texture_buffer = new unsigned char[texture_buffer_size];
    }
    glReadPixels(0, 0, screen_width, screen_height, GL_RGBA, GL_UNSIGNED_BYTE, texture_buffer);
    
    SDL_LockSurface(effect_dst_surface);
    for (int i=0 ; i<screen_height ; i++)
        memcpy( (Uint32*)effect_dst_surface->pixels + i*screen_width,
                texture_buffer + (screen_height - 1 - i)*screen_width*4,
                screen_width * 4);
    SDL_UnlockSurface(effect_dst_surface);
#else
    SDL_BlitSurface( screen_surface, NULL, effect_dst_surface, NULL ); // Bucause screen_surface may be in 16bit depth
#endif    
    resizeSurface( effect_dst_surface, NULL, screenshot_surface, NULL );

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
    script_h.readVariable();

    if ( script_h.current_variable.type == ScriptHandler::VAR_INT ||
         script_h.current_variable.type == ScriptHandler::VAR_ARRAY ){
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
    script_h.readVariable();
    
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
                    
                    if ( !script_h.compareString("=") ){
                        script_h.popCurrent();
                        continue;
                    }
                    script_h.setCurrent(script_h.getNext()+1);

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

int ONScripterLabel::getinsertCommand()
{
    getinsert_flag = true;

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
    script_h.setInt( &script_h.current_variable, sentence_font.x() );
    
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, sentence_font.y() );
    
    return RET_CONTINUE;
}

int ONScripterLabel::getcursorCommand()
{
    if ( !force_button_shortcut_flag )
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

    setCurrentLinkLabel( "start" );
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
    }
    text_buffer[0].previous = &text_buffer[max_text_buffer-1];
    text_buffer[max_text_buffer-1].next = &text_buffer[0];
    start_text_buffer = current_text_buffer = &text_buffer[0];

    clearCurrentTextBuffer();

    /* ---------------------------------------- */
    /* Initialize local variables */
    for ( i=0 ; i<script_h.global_variable_border ; i++ ){
        script_h.num_variables[i] = 0;
        delete[] script_h.str_variables[i];
        script_h.str_variables[i] = NULL;
    }

    return RET_CONTINUE;
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
        root_button_link.insert( button );
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
    
    sprintf( buf, RELATIVEPATH "voice%c%s.wav", DELIMITER, script_h.getStringBuffer()+2 );
    playWave( buf, false, 0 );
    
    return RET_CONTINUE;
}

int ONScripterLabel::drawtextCommand()
{
#ifdef USE_OPENGL
    refreshText( accumulation_surface, NULL, REFRESH_TEXT_MODE | REFRESH_OPENGL_MODE);
#else    
    refreshText( accumulation_surface, NULL, REFRESH_TEXT_MODE );
#endif    
    
    return RET_CONTINUE;
}

int ONScripterLabel::drawsp2Command()
{
    int sprite_no = script_h.readInt();
    int cell_no = script_h.readInt();
    int alpha = script_h.readInt();
    int x = script_h.readInt();
    int y = script_h.readInt();
    int scale_x = script_h.readInt();
    int scale_y = script_h.readInt();
    int rot = script_h.readInt();

    AnimationInfo &si = sprite_info[sprite_no];
    int old_cell_no = si.current_cell;
    si.setCell(cell_no);
#ifdef USE_OPENGL
    glMatrixMode(GL_MODELVIEW) ;
    glPushMatrix();
    glLoadIdentity() ;

    glTranslatef( x, screen_height-y, 0.0 );
    glRotatef( (float)rot, 0.0, 0.0, 1.0 );
    glScalef( scale_x * 0.01, scale_y * 0.01, 1.0 );

    SDL_Rect poly_rect = si.pos;
    poly_rect.x = -si.pos.w/2;
    poly_rect.y = screen_height-si.pos.h/2;
    SDL_Rect tex_rect = si.pos;
    tex_rect.x = si.pos.w*si.current_cell;
    tex_rect.y = 0;
    drawTexture( si.tex_id, poly_rect, tex_rect, alpha, &si );

    glPopMatrix();
#else    
    si.blendOnSurface( accumulation_surface, x-si.pos.w/2, y-si.pos.h/2,
                       NULL, alpha, scale_x, scale_y, rot );
#endif    
    si.setCell(old_cell_no);

    return RET_CONTINUE;
}

int ONScripterLabel::drawspCommand()
{
    int sprite_no = script_h.readInt();
    int cell_no = script_h.readInt();
    int alpha = script_h.readInt();
    int x = script_h.readInt();
    int y = script_h.readInt();

    AnimationInfo &si = sprite_info[sprite_no];
    int old_cell_no = si.current_cell;
    si.setCell(cell_no);
#ifdef USE_OPENGL
    glMatrixMode(GL_MODELVIEW) ;
    glPushMatrix();
    glLoadIdentity() ;

    SDL_Rect poly_rect = si.pos;
    poly_rect.x = x;
    poly_rect.y = y;
    SDL_Rect tex_rect = si.pos;
    tex_rect.x = si.pos.w*si.current_cell;
    tex_rect.y = 0;
    drawTexture( si.tex_id, poly_rect, tex_rect, alpha, &si );

    glPopMatrix();
#else    
    si.blendOnSurface( accumulation_surface, x, y,
                                           NULL, alpha );
#endif    
    si.setCell(old_cell_no);

    return RET_CONTINUE;
}

int ONScripterLabel::drawfillCommand()
{
    int r = script_h.readInt();
    int g = script_h.readInt();
    int b = script_h.readInt();

#ifdef USE_OPENGL
    glColor4f(r/256.0, g/256.0, b/256.0, 1.0);
    glDisable(GL_TEXTURE_2D);
    SDL_Rect rect = {0, 0, screen_width, screen_height};
    drawTexture( effect_src_id, rect, rect, -1 );
    glEnable(GL_TEXTURE_2D);
#else    
    SDL_FillRect( accumulation_surface, NULL, SDL_MapRGBA( accumulation_surface->format, r, g, b, 0xff) );
#endif    
    
    return RET_CONTINUE;
}

int ONScripterLabel::drawclearCommand()
{
#ifdef USE_OPENGL
    glClear(GL_COLOR_BUFFER_BIT);
#else    
    SDL_FillRect( accumulation_surface, NULL, SDL_MapRGBA( accumulation_surface->format, 0, 0, 0, 0xff) );
#endif    
    
    return RET_CONTINUE;
}

int ONScripterLabel::drawbgCommand()
{
#ifdef USE_OPENGL
    glMatrixMode(GL_MODELVIEW) ;
    glPushMatrix();
    glLoadIdentity() ;

    drawTexture( bg_info.tex_id, bg_info.pos, bg_info.pos, 256, &bg_info );

    glPopMatrix();
#else    
    bg_info.blendOnSurface( accumulation_surface, bg_info.pos.x, bg_info.pos.y );
#endif    
    
    return RET_CONTINUE;
}

int ONScripterLabel::drawbg2Command()
{
    int x = script_h.readInt();
    int y = script_h.readInt();
    int scale_x = script_h.readInt();
    int scale_y = script_h.readInt();
    int rot = script_h.readInt();

#ifdef USE_OPENGL
    glMatrixMode(GL_MODELVIEW) ;
    glPushMatrix();
    glLoadIdentity() ;

    glTranslatef( x, screen_height-y, 0.0 );
    glRotatef( (float)rot, 0.0, 0.0, 1.0 );
    glScalef( scale_x * 0.01, scale_y * 0.01, 1.0 );
    
    SDL_Rect poly_rect = bg_info.pos;
    poly_rect.x = -bg_info.pos.w/2;
    poly_rect.y = screen_height-bg_info.pos.h/2;
    SDL_Rect tex_rect = bg_info.pos;
    tex_rect.x = 0;
    tex_rect.y = 0;
    drawTexture( bg_info.tex_id, poly_rect, tex_rect, 256, &bg_info );

    glPopMatrix();
#else    
    bg_info.blendOnSurface( accumulation_surface, x-bg_info.pos.w/2, y-bg_info.pos.h/2,
                            NULL, 256, scale_x, scale_y, rot );
#endif    

    return RET_CONTINUE;
}

int ONScripterLabel::drawCommand()
{
#ifdef USE_OPENGL
    SDL_GL_SwapBuffers();
#else    
    SDL_Rect rect = {0, 0, screen_width, screen_height};
    flushDirect( rect, REFRESH_NONE_MODE );
    dirty_rect.clear();
#endif
    
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
        return RET_WAIT | RET_REREAD;
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
            root_button_link.removeSprite(i);
            sprite_info[i].remove();
        }
    else{
        if ( sprite_info[no].visible )
            dirty_rect.add( sprite_info[no].pos );
        root_button_link.removeSprite(no);
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

    setCurrentLinkLabel( link->label );
    
    deleteSelectLink();
    newPage( true );
    
    return RET_CONTINUE;
}

int ONScripterLabel::cselbtnCommand()
{
    int csel_no   = script_h.readInt();
    int button_no = script_h.readInt();

    FontInfo csel_info = sentence_font;
    csel_info.top_xy[0] = script_h.readInt();
    csel_info.top_xy[1] = script_h.readInt();

    int counter = 0;
    SelectLink *link = root_select_link.next;
    while ( link ){
        if ( csel_no == counter++ ) break;
        link = link->next;
    }
    if ( link == NULL || link->text == NULL || *link->text == '\0' )
        errorAndExit( "cselbtn: no select text" );

    csel_info.setLineArea( strlen(link->text)/2+1 );
    csel_info.clear();
    ButtonLink *button = getSelectableSentence( link->text, &csel_info );
    root_button_link.insert( button );
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
        return RET_WAIT | RET_REREAD;
    }
}

int ONScripterLabel::clCommand()
{
    int ret = leaveTextDisplayMode();
    if ( ret != RET_NOMATCH ) return ret;
    
    char loc = script_h.readLabel()[0];
    
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

int ONScripterLabel::chvolCommand()
{
    int ch  = script_h.readInt();
    int vol = script_h.readInt();

    if ( wave_sample[ch] ){
        Mix_Volume( ch, vol * 128 / 100 );
    }
    
    return RET_CONTINUE;
}

int ONScripterLabel::cellCommand()
{
    int sprite_no = script_h.readInt();
    int no        = script_h.readInt();

    sprite_info[sprite_no].setCell(no);
    dirty_rect.add( sprite_info[sprite_no].pos );
        
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

    if ( event_mode & WAIT_BUTTON_MODE ||
         (textbtn_flag && (skip_flag || (draw_one_page_flag && clickstr_state == CLICK_WAIT) || ctrl_pressed_status)) )
    {
        btnwait_time = SDL_GetTicks() - internal_button_timer;
        btntime_value = 0;
        num_chars_in_sentence = 0;

        if ( textbtn_flag && (skip_flag || (draw_one_page_flag && clickstr_state == CLICK_WAIT) || ctrl_pressed_status))
            current_button_state.button = 0;
        script_h.setInt( &script_h.current_variable, current_button_state.button );

        if ( current_button_state.button >= 1 && del_flag ){
            deleteButtonLink();
            if ( exbtn_d_button_link.exbtn_ctl ){
                delete[] exbtn_d_button_link.exbtn_ctl;
                exbtn_d_button_link.exbtn_ctl = NULL;
            }
        }

        event_mode = IDLE_EVENT_MODE;
        disableGetButtonFlag();
        
        ButtonLink *p_button_link = root_button_link.next;
        while( p_button_link ){
            p_button_link->show_flag = 0;
            p_button_link = p_button_link->next;
        }
            
        return RET_CONTINUE;
    }
    else{
        shortcut_mouse_line = 0;
        skip_flag = false;

        /* ---------------------------------------- */
        /* Resotre csel button */
        if ( refreshMode() & REFRESH_TEXT_MODE ){
            display_mode = next_display_mode = TEXT_DISPLAY_MODE;
        }

        if ( exbtn_d_button_link.exbtn_ctl ){
            SDL_Rect check_src_rect = {0, 0, screen_width, screen_height};
            decodeExbtnControl( accumulation_surface, exbtn_d_button_link.exbtn_ctl, &check_src_rect );
        }

        ButtonLink *p_button_link = root_button_link.next;
        while( p_button_link ){
            p_button_link->show_flag = 0;
            if ( p_button_link->button_type == ButtonLink::SPRITE_BUTTON || 
                 p_button_link->button_type == ButtonLink::EX_SPRITE_BUTTON ){
            }
            else if ( p_button_link->button_type == ButtonLink::TMP_SPRITE_BUTTON ){
                p_button_link->show_flag = 1;
            }
            else if ( p_button_link->anim[1] != NULL ){
                p_button_link->show_flag = 2;
            }
            p_button_link = p_button_link->next;
        }

        flush( refreshMode() );

        flushEvent();
        event_mode = WAIT_BUTTON_MODE | WAIT_TIMER_MODE;
        refreshMouseOverButton();

        if ( btntime_value > 0 ){
            if ( btntime2_flag )
                event_mode |= WAIT_VOICE_MODE;
            startTimer( btntime_value );
            //if ( usewheel_flag ) current_button_state.button = -5;
            //else                 current_button_state.button = -2;
        }
        internal_button_timer = SDL_GetTicks();

        if ( textbtn_flag ){
            event_mode |= WAIT_TEXTBTN_MODE;
            if ( btntime_value == 0 ){
                if ( automode_flag ){
                    event_mode |= WAIT_VOICE_MODE;
                    if ( automode_time < 0 )
                        startTimer( -automode_time * num_chars_in_sentence );
                    else
                        startTimer( automode_time );
                    //current_button_state.button = 0;
                }
                else{
                    event_mode |= WAIT_TIMER_MODE;
                    advancePhase();
                }
            }
        }
        return RET_WAIT | RET_REREAD;
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
    if (script_h.compareString("clear")){
        script_h.readLabel();
    }
    else{
        const char *buf = script_h.readStr();

        btndef_info.remove();

        if ( buf[0] != '\0' ){
            btndef_info.setImageName( buf );
            parseTaggedString( &btndef_info );
            btndef_info.trans_mode = AnimationInfo::TRANS_COPY;
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

    button->anim[0] = new AnimationInfo();
    button->anim[0]->num_of_cells = 1;
    button->anim[0]->trans_mode = AnimationInfo::TRANS_COPY;

    src_rect.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    src_rect.y = script_h.readInt() * screen_ratio1 / screen_ratio2;
    if (btndef_info.image_surface &&
        src_rect.x + button->image_rect.w > btndef_info.image_surface->w){
        button->image_rect.w = btndef_info.image_surface->w - src_rect.x;
    }
    if (btndef_info.image_surface &&
        src_rect.y + button->image_rect.h > btndef_info.image_surface->h){
        button->image_rect.h = btndef_info.image_surface->h - src_rect.y;
    }
    src_rect.w = button->image_rect.w;
    src_rect.h = button->image_rect.h;

    button->anim[0]->pos = button->image_rect;

    SDL_Surface *surface = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG,
                                                 button->image_rect.w,
                                                 button->image_rect.h,
                                                 32, rmask, gmask, bmask, amask );
    blitSurface( btndef_info.image_surface, &src_rect, surface, NULL );
    button->anim[0]->trans_mode = AnimationInfo::TRANS_COPY;
    setupAnimationInfo( button->anim[0], NULL, surface );

    root_button_link.insert( button );
    
    return RET_CONTINUE;
}

int ONScripterLabel::brCommand()
{
    int ret = enterTextDisplayMode();
    if ( ret != RET_NOMATCH ) return ret;

    sentence_font.newLine();
    current_text_buffer->addBuffer( 0x0a );

    return RET_CONTINUE;
}

int ONScripterLabel::bltCommand()
{
    SDL_Rect src_rect, dst_rect;

    dst_rect.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    dst_rect.y = script_h.readInt() * screen_ratio1 / screen_ratio2;
    dst_rect.w = script_h.readInt() * screen_ratio1 / screen_ratio2;
    dst_rect.h = script_h.readInt() * screen_ratio1 / screen_ratio2;
    src_rect.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    src_rect.y = script_h.readInt() * screen_ratio1 / screen_ratio2;
    src_rect.w = script_h.readInt() * screen_ratio1 / screen_ratio2;
    src_rect.h = script_h.readInt() * screen_ratio1 / screen_ratio2;

#ifdef USE_OPENGL
    glMatrixMode(GL_MODELVIEW) ;
    glPushMatrix();
    glLoadIdentity() ;

    drawTexture( btndef_info.tex_id, dst_rect, src_rect, 256, &btndef_info );
        
    glPopMatrix();
    SDL_GL_SwapBuffers();
#else        
    if ( src_rect.w == dst_rect.w && src_rect.h == dst_rect.h ){

        SDL_Rect clip = {0, 0, screen_width, screen_height}, clipped;
        doClipping( &dst_rect, &clip, &clipped );
        shiftRect( src_rect, clipped );

        SDL_BlitSurface( btndef_info.image_surface, &src_rect, screen_surface, &dst_rect );
        SDL_UpdateRect( screen_surface, dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h );
        dirty_rect.clear();
    }
    else{
        resizeSurface( btndef_info.image_surface, &src_rect, accumulation_surface, &dst_rect );
        //dirty_rect.add( dst_rect );
        flushDirect( dst_rect, REFRESH_NONE_MODE );
    }
#endif    
    return RET_CONTINUE;
}

int ONScripterLabel::bgCommand()
{
    int ret = leaveTextDisplayMode();
    if ( ret != RET_NOMATCH ) return ret;

    const char *buf;
    if (script_h.compareString("white")){
        buf = "white";
        script_h.readLabel();
    }
    else if (script_h.compareString("black")){
        buf = "black";
        script_h.readLabel();
    }
    else{
        buf = script_h.readStr();
        setStr( &bg_info.file_name, buf );
    }

    if ( event_mode & EFFECT_EVENT_MODE ){
        int num = readEffect( &tmp_effect );
        if ( num > 1 ) return doEffect( TMP_EFFECT, &bg_info, bg_effect_image );
        else           return doEffect( tmp_effect.effect, &bg_info, bg_effect_image );
    }
    else{
        for ( int i=0 ; i<3 ; i++ )
            tachi_info[i].remove();

        bg_info.remove();
        setStr( &bg_info.file_name, buf );

        createBackground();
        dirty_rect.fill( screen_width, screen_height );

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
    bar_info[no]->setCell(0);

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
        SDL_FillRect( bar_info[no]->image_surface, NULL, SDL_MapRGBA( bar_info[no]->image_surface->format, bar_info[no]->color[0], bar_info[no]->color[1], bar_info[no]->color[2], 0xff ) );
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

    if ( sprite_info[ no ].trans > 256 ) sprite_info[ no ].trans = 256;
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

