/* -*- C++ -*-
 *
 *  ONScripterLabel_rmenu.cpp - Right click menu handler of ONScripter
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

#if defined(LINUX) || defined(MACOSX)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#elif defined(WIN32)
#include <windows.h>
#else
#endif

#define SAVEFILE_MAGIC_NUMBER "ONS"
#define SAVEFILE_VERSION_MAJOR 1
#define SAVEFILE_VERSION_MINOR 7

#define READ_LENGTH 4096

void ONScripterLabel::searchSaveFiles( int no )
{
    unsigned int i, start, end;
    char file_name[256];

    if ( no == -1 ){
        start = 0;
        end = num_save_file;
    }
    else{
        start = no;
        end = no+1;
    }
    
    for ( i=start ; i<end ; i++ ){

#if defined(LINUX) || defined(MACOSX)
        sprintf( file_name, "%ssave%d.dat", archive_path, i+1 );
        struct stat buf;
        struct tm *tm;
        if ( stat( file_name, &buf ) != 0 ){
            save_file_info[i].valid = false;
            continue;
        }
        tm = localtime( &buf.st_mtime );
        
        save_file_info[i].month  = tm->tm_mon + 1;
        save_file_info[i].day    = tm->tm_mday;
        save_file_info[i].hour   = tm->tm_hour;
        save_file_info[i].minute = tm->tm_min;
#elif defined(WIN32)
        sprintf( file_name, "%ssave%d.dat", archive_path, i+1 );
        HANDLE  handle;
        FILETIME    tm, ltm;
        SYSTEMTIME  stm;

        handle = CreateFile( file_name, GENERIC_READ, 0, NULL,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        if ( handle == INVALID_HANDLE_VALUE ){
            save_file_info[i].valid = false;
            continue;
        }
            
        GetFileTime( handle, NULL, NULL, &tm );
        FileTimeToLocalFileTime( &tm, &ltm );
        FileTimeToSystemTime( &ltm, &stm );
        CloseHandle( handle );

        save_file_info[i].month  = stm.wMonth;
        save_file_info[i].day    = stm.wDay;
        save_file_info[i].hour   = stm.wHour;
        save_file_info[i].minute = stm.wMinute;
#else
        sprintf( file_name, "save%d.dat", i+1 );
        FILE *fp;
        if ( (fp = fopen( file_name, "rb" )) == NULL ){
            save_file_info[i].valid = false;
            continue;
        }
        fclose( fp );

        save_file_info[i].month  = 1;
        save_file_info[i].day    = 1;
        save_file_info[i].hour   = 0;
        save_file_info[i].minute = 0;
#endif
        save_file_info[i].valid = true;
        getSJISFromInteger( save_file_info[i].sjis_month,  save_file_info[i].month );
        getSJISFromInteger( save_file_info[i].sjis_day,    save_file_info[i].day );
        getSJISFromInteger( save_file_info[i].sjis_hour,   save_file_info[i].hour );
        getSJISFromInteger( save_file_info[i].sjis_minute, save_file_info[i].minute );
    }
}

int ONScripterLabel::loadSaveFile( int no )
{
    FILE *fp;
    char file_name[256], *str = NULL;
    int  i, j, address;
    int  file_version;
    
    sprintf( file_name, "save%d.dat", no );
    if ( ( fp = fopen( file_name, "rb" ) ) == NULL ){
        fprintf( stderr, "can't open save file %s\n", file_name );
        return -1;
    }

    /* ---------------------------------------- */
    /* Load magic number */
    for ( i=0 ; i<(int)strlen( SAVEFILE_MAGIC_NUMBER ) ; i++ )
        if ( fgetc( fp ) != SAVEFILE_MAGIC_NUMBER[i] ) break;
    if ( i != (int)strlen( SAVEFILE_MAGIC_NUMBER ) ){ // in the case of old file format
        file_version = 0;
        fseek( fp, 0, SEEK_SET );
    }
    else{
        file_version = (fgetc( fp ) * 100) + fgetc( fp );
    }
    printf("Save file version is %d.%d\n", file_version/100, file_version%100 );
    if ( file_version > SAVEFILE_VERSION_MAJOR*100 + SAVEFILE_VERSION_MINOR ){
        fprintf( stderr, "Save file is newer than %d.%d, please use the latest ONScripter.\n", SAVEFILE_VERSION_MAJOR, SAVEFILE_VERSION_MINOR );
        return -1;
    }
    
    deleteLabelLink();

    /* ---------------------------------------- */
    /* Load text history */
    if ( file_version >= 107 )
        loadInt( fp, &tateyoko_mode );
    else
        tateyoko_mode = 0;
    loadInt( fp, &text_history_num );
    struct TextBuffer *tb = &text_buffer[0];
    for ( i=0 ; i<text_history_num ; i++ ){
        loadInt( fp, &tb->num_xy[0] );
        loadInt( fp, &tb->num_xy[1] );
        loadInt( fp, &tb->xy[0] );
        loadInt( fp, &tb->xy[1] );
        if ( tb->buffer ) delete[] tb->buffer;
        tb->buffer = new char[ tb->num_xy[0] * tb->num_xy[1] * 2 ];
        for ( j=0 ; j<tb->num_xy[0] * tb->num_xy[1] * 2 ; j++ )
            tb->buffer[j] = fgetc( fp );
        tb = tb->next;
    }
    current_text_buffer = &text_buffer[0];

    /* ---------------------------------------- */
    /* Load sentence font */
    loadInt( fp, &j );
    //sentence_font.is_valid = (j==1)?true:false;
    loadInt( fp, &sentence_font.font_size_xy[0] );
    if ( file_version >= 100 ){
        loadInt( fp, &sentence_font.font_size_xy[1] );
    }
    else{
        sentence_font.font_size_xy[1] = sentence_font.font_size_xy[0];
    }
    loadInt( fp, &sentence_font.top_xy[0] );
    loadInt( fp, &sentence_font.top_xy[1] );
    loadInt( fp, &sentence_font.num_xy[0] );
    loadInt( fp, &sentence_font.num_xy[1] );
    loadInt( fp, &sentence_font.xy[0] );
    loadInt( fp, &sentence_font.xy[1] );
    loadInt( fp, &sentence_font.pitch_xy[0] );
    loadInt( fp, &sentence_font.pitch_xy[1] );
    loadInt( fp, &sentence_font.wait_time );
    loadInt( fp, &j );
    sentence_font.is_bold = (j==1)?true:false;
    loadInt( fp, &j );
    sentence_font.is_shadow = (j==1)?true:false;
    loadInt( fp, &j );
    sentence_font.is_transparent = (j==1)?true:false;

    /* Dummy, must be removed later !! */
    for ( i=0 ; i<8 ; i++ ){
        loadInt( fp, &j );
        //sentence_font.window_color[i] = j;
    }
    /* Should be char, not integer !! */
    for ( i=0 ; i<3 ; i++ ){
        loadInt( fp, &j );
        sentence_font.window_color[i] = j;
    }
    loadStr( fp, &sentence_font_info.image_name );

    loadInt( fp, &j ); sentence_font_info.pos.x = j * screen_ratio1 / screen_ratio2; 
    loadInt( fp, &j ); sentence_font_info.pos.y = j * screen_ratio1 / screen_ratio2;
    loadInt( fp, &j ); sentence_font_info.pos.w = j * screen_ratio1 / screen_ratio2;
    loadInt( fp, &j ); sentence_font_info.pos.h = j * screen_ratio1 / screen_ratio2;

    if ( !sentence_font.is_transparent ){
        parseTaggedString( &sentence_font_info );
        setupAnimationInfo( &sentence_font_info );
    }

    sentence_font.closeFont();

    loadInt( fp, &clickstr_state );
    loadInt( fp, &j );
    new_line_skip_flag = (j==1)?true:false;
    if ( file_version >= 103 ){
        loadInt( fp, &erase_text_window_mode );
    }
    
    /* ---------------------------------------- */
    /* Load link label info */
    label_stack_depth = 0;

    while( 1 ){
        loadStr( fp, &str );
        current_link_label_info->label_info = script_h.lookupLabel( str );
        
        loadInt( fp, &current_link_label_info->current_line );

        int offset;
        loadInt( fp, &offset );
            
        script_h.setCurrent( current_link_label_info->label_info.start_address, false );
        script_h.skipLine( current_link_label_info->current_line );
        current_link_label_info->current_script = script_h.getCurrent() + offset;

        if ( file_version <= 104 )
        {
            if ( file_version >= 102 )
                loadInt( fp, &j );

            loadInt( fp, &address );
            current_link_label_info->string_buffer_offset = 0;
        }
        else{
            loadInt( fp, &current_link_label_info->string_buffer_offset );
        }
        
        if ( fgetc( fp ) == 0 ) break;

        if ( file_version<= 105 )
            current_link_label_info->string_buffer_offset = 0;
        
        current_link_label_info->next = new LinkLabelInfo();
        current_link_label_info->next->previous = current_link_label_info;
        current_link_label_info = current_link_label_info->next;
        current_link_label_info->next = NULL;
        label_stack_depth++;
    }
    script_h.setCurrent( current_link_label_info->current_script );
    string_buffer_offset = current_link_label_info->string_buffer_offset;
    
    int tmp_event_mode = fgetc( fp );

    /* ---------------------------------------- */
    /* Load variables */
    loadVariables( fp, 0, 200 );

    /* ---------------------------------------- */
    /* Load monocro flag */
    monocro_flag = (fgetc( fp )==1)?true:false;
    if ( file_version >= 101 ){
        monocro_flag_new = (fgetc( fp )==1)?true:false;
    }
    for ( i=0 ; i<3 ; i++ ) monocro_color[i] = fgetc( fp );

    if ( file_version >= 101 ){
        for ( i=0 ; i<3 ; i++ ) monocro_color_new[i] = fgetc( fp );
        need_refresh_flag = (fgetc( fp )==1)?true:false;
    }
    else{
        need_refresh_flag = false;
    }
    
    for ( i=0 ; i<256 ; i++ ){
        monocro_color_lut[i][0] = (monocro_color[0] * i) >> 8;
        monocro_color_lut[i][1] = (monocro_color[1] * i) >> 8;
        monocro_color_lut[i][2] = (monocro_color[2] * i) >> 8;
    }
    
    /* Load nega flag */
    if ( file_version >= 104 ){
        nega_mode = (unsigned char)fgetc( fp );
    }
    /* ---------------------------------------- */
    /* Load current images */
    bg_info.remove();
    bg_info.color[0] = (unsigned char)fgetc( fp );
    bg_info.color[1] = (unsigned char)fgetc( fp );
    bg_info.color[2] = (unsigned char)fgetc( fp );
    bg_info.num_of_cells = 1;
    loadStr( fp, &bg_info.file_name );
    setupAnimationInfo( &bg_info );
    bg_effect_image = (EFFECT_IMAGE)fgetc( fp );

    if ( bg_effect_image == COLOR_EFFECT_IMAGE ){
        SDL_FillRect( background_surface, NULL, SDL_MapRGB( effect_dst_surface->format, bg_info.color[0], bg_info.color[1], bg_info.color[2]) );
    }
    else{
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

    for ( i=0 ; i<3 ; i++ ){
        tachi_info[i].remove();
    }

    for ( i=0 ; i<MAX_SPRITE_NUM ; i++ ){
        sprite_info[i].remove();
    }

    /* ---------------------------------------- */
    /* Load Tachi image and Sprite */
    for ( i=0 ; i<3 ; i++ ){
        loadStr( fp, &tachi_info[i].image_name );
        if ( tachi_info[i].image_name ){
            parseTaggedString( &tachi_info[i] );
            setupAnimationInfo( &tachi_info[ i ] );
            tachi_info[ i ].pos.x = screen_width * (i+1) / 4 - tachi_info[ i ].pos.w / 2;
            tachi_info[ i ].pos.y = underline_value - tachi_info[ i ].image_surface->h + 1;
        }
    }

    /* ---------------------------------------- */
    /* Load current sprites */
    for ( i=0 ; i<MAX_SPRITE_NUM ; i++ ){
        loadInt( fp, &j );
        sprite_info[i].valid = (j==1)?true:false;
        loadInt( fp, &j ); sprite_info[i].pos.x = j * screen_ratio1 / screen_ratio2;
        loadInt( fp, &j ); sprite_info[i].pos.y = j * screen_ratio1 / screen_ratio2;
        loadInt( fp, &sprite_info[i].trans );
        loadStr( fp, &sprite_info[i].image_name );
        if ( sprite_info[i].image_name ){
            parseTaggedString( &sprite_info[i] );
            setupAnimationInfo( &sprite_info[i] );
        }
    }

    /* ---------------------------------------- */
    /* Load current playing CD track */
    stopBGM( false );
    current_cd_track = (Sint8)fgetc( fp );
    music_play_once_flag = (fgetc( fp )==1)?true:false;
    loadStr( fp, &music_file_name );

    if ( current_cd_track >= 0 ){
        if ( cdaudio_flag ){
            if ( cdrom_info ) playCDAudio( current_cd_track );
        }
        else{
            playMP3( current_cd_track );
        }
    }
    else if ( music_file_name ){
        if ( current_cd_track == -2 )
            playMIDIFile();
        else
            if ( playWave( music_file_name, !music_play_once_flag, ONS_MIX_CHANNELS-1 ) )
                playMP3( current_cd_track );
    }

    /* ---------------------------------------- */
    /* Load rmode flag */
    rmode_flag = (fgetc( fp )==1)?true:false;
    
    /* ---------------------------------------- */
    /* Load text on flag */
    text_on_flag = (fgetc( fp )==1)?true:false;

    fclose( fp );

    refreshSurface( accumulation_surface, NULL, REFRESH_SHADOW_MODE );
    SDL_BlitSurface( accumulation_surface, NULL, text_surface, NULL );
    restoreTextBuffer();
    flush();
    display_mode = TEXT_DISPLAY_MODE;

    event_mode = tmp_event_mode;
    if ( event_mode & WAIT_BUTTON_MODE ) event_mode = WAIT_SLEEP_MODE; // Re-execute the selectCommand, etc.

    return 0;
}

int ONScripterLabel::saveSaveFile( int no )
{
    FILE *fp;
    int i, j;
    char file_name[256];

    if ( no >= 0 ){
        saveAll();
        sprintf( file_name, "save%d.dat", no );

        if ( ( fp = fopen( file_name, "wb" ) ) == NULL ){
            fprintf( stderr, "can't open save file %s\n", file_name );
            return -1;
        }

        if ( !saveon_flag ){
            int c;
            long len;
            char *buf = new char[ READ_LENGTH ];
        
            fseek( tmp_save_fp, 0, SEEK_END );
            len = ftell( tmp_save_fp );
            fseek( tmp_save_fp, 0, SEEK_SET );
            while( len > 0 ){
                if ( len > READ_LENGTH ) c = READ_LENGTH;
                else                     c = len;
                len -= c;
                fread( buf, 1, c, tmp_save_fp );
                fwrite( buf, 1, c, fp );
            }
            fclose( fp );

            delete[] buf;
            return 0;
        }
    }
    else{
        if ( tmp_save_fp ) fclose( tmp_save_fp );
        if ( (tmp_save_fp = tmpfile()) == NULL ){
            fprintf( stderr, "can't open tmp_file\n");
            return -1;
        }
        fp = tmp_save_fp;
    }

    /* ---------------------------------------- */
    /* Save magic number */
    for ( i=0 ; i<(int)strlen( SAVEFILE_MAGIC_NUMBER ) ; i++ )
        fputc( SAVEFILE_MAGIC_NUMBER[i], fp );
    fputc( SAVEFILE_VERSION_MAJOR, fp );
    fputc( SAVEFILE_VERSION_MINOR, fp );
    
    /* ---------------------------------------- */
    /* Save text history */
    saveInt( fp, tateyoko_mode );
    saveInt( fp, text_history_num );
    struct TextBuffer *tb = current_text_buffer;
    for ( i=0 ; i<text_history_num ; i++ ){
        saveInt( fp, tb->num_xy[0] );
        saveInt( fp, tb->num_xy[1] );
        saveInt( fp, tb->xy[0] );
        saveInt( fp, tb->xy[1] );
        for ( j=0 ; j<tb->num_xy[0] * tb->num_xy[1] * 2 ; j++ )
            fputc( tb->buffer[j], fp );
        tb = tb->next;
    }

    /* ---------------------------------------- */
    /* Save sentence font */
    //saveInt( fp, (sentence_font.is_valid?1:0) );
    saveInt( fp, 0 ); // dummy write, must be removed later
    saveInt( fp, sentence_font.font_size_xy[0] );
    saveInt( fp, sentence_font.font_size_xy[1] );
    saveInt( fp, sentence_font.top_xy[0] );
    saveInt( fp, sentence_font.top_xy[1] );
    saveInt( fp, sentence_font.num_xy[0] );
    saveInt( fp, sentence_font.num_xy[1] );
    saveInt( fp, sentence_font.xy[0] );
    saveInt( fp, sentence_font.xy[1] );
    saveInt( fp, sentence_font.pitch_xy[0] );
    saveInt( fp, sentence_font.pitch_xy[1] );
    saveInt( fp, sentence_font.wait_time );
    saveInt( fp, (sentence_font.is_bold?1:0) );
    saveInt( fp, (sentence_font.is_shadow?1:0) );
    saveInt( fp, (sentence_font.is_transparent?1:0) );
    /* Dummy, must be removed later !! */
    for ( i=0 ; i<8 ; i++ )
        saveInt( fp, 0 );
    /* Should be char, not integer !! */
    for ( i=0 ; i<3 ; i++ )
        saveInt( fp, sentence_font.window_color[i] );
    saveStr( fp, sentence_font_info.image_name );
    saveInt( fp, sentence_font_info.pos.x * screen_ratio2 / screen_ratio1 );
    saveInt( fp, sentence_font_info.pos.y * screen_ratio2 / screen_ratio1 );
    saveInt( fp, sentence_font_info.pos.w * screen_ratio2 / screen_ratio1 );
    saveInt( fp, sentence_font_info.pos.h * screen_ratio2 / screen_ratio1 );

    saveInt( fp, clickstr_state );
    saveInt( fp, new_line_skip_flag?1:0 );
    saveInt( fp, erase_text_window_mode );
    
    /* ---------------------------------------- */
    /* Save link label info */
    current_link_label_info->current_script = script_h.getCurrent();
    current_link_label_info->string_buffer_offset = string_buffer_offset;
    LinkLabelInfo *info = &root_link_label_info;

    while( info ){
        fprintf( fp, "%s", info->label_info.name );
        fputc( 0, fp );
        saveInt( fp, info->current_line );

        script_h.pushCurrent( info->label_info.start_address );
        script_h.skipLine( info->current_line );
        saveInt( fp, info->current_script - script_h.getCurrent() );
        script_h.popCurrent();
        saveInt( fp, info->string_buffer_offset );

        if ( info->next ) fputc( 1, fp );
        info = info->next;
    }
    fputc( 0, fp );

    fputc( shelter_event_mode, fp );
    
    /* ---------------------------------------- */
    /* Save variables */
    saveVariables( fp, 0, 200 );
    
    /* ---------------------------------------- */
    /* Save monocro flag */
    monocro_flag?fputc(1,fp):fputc(0,fp);
    monocro_flag_new?fputc(1,fp):fputc(0,fp);
    for ( i=0 ; i<3 ; i++ ) fputc( monocro_color[i], fp );
    for ( i=0 ; i<3 ; i++ ) fputc( monocro_color_new[i], fp );

    need_refresh_flag?fputc(1,fp):fputc(0,fp);
    
    /* Save nega flag */
    fputc( nega_mode, fp );

    /* ---------------------------------------- */
    /* Save current images */
    fputc( bg_info.color[0], fp );
    fputc( bg_info.color[1], fp );
    fputc( bg_info.color[2], fp );
    saveStr( fp, bg_info.file_name );
    fputc( bg_effect_image, fp );

    saveStr( fp, tachi_info[0].image_name );
    saveStr( fp, tachi_info[1].image_name );
    saveStr( fp, tachi_info[2].image_name );

    /* ---------------------------------------- */
    /* Save current sprites */
    for ( i=0 ; i<MAX_SPRITE_NUM ; i++ ){
        saveInt( fp, sprite_info[i].valid?1:0 );
        saveInt( fp, sprite_info[i].pos.x * screen_ratio2 / screen_ratio1 );
        saveInt( fp, sprite_info[i].pos.y * screen_ratio2 / screen_ratio1 );
        saveInt( fp, sprite_info[i].trans );
        saveStr( fp, sprite_info[i].image_name );
    }

    /* ---------------------------------------- */
    /* Save current playing CD track */
    fputc( (Sint8)current_cd_track, fp );
    music_play_once_flag?fputc(1,fp):fputc(0,fp);
    saveStr( fp, music_file_name );
    
    /* ---------------------------------------- */
    /* Save rmode flag */
    rmode_flag?fputc(1,fp):fputc(0,fp);

    /* ---------------------------------------- */
    /* Save text on flag */
    text_on_flag?fputc(1,fp):fputc(0,fp);

    if ( no >= 0 ) fclose( fp );

    return 0;
}

void ONScripterLabel::enterSystemCall()
{
    shelter_button_link = root_button_link.next;
    last_button_link = &root_button_link;
    last_button_link->next = NULL;
    shelter_select_link = root_select_link.next;
    root_select_link.next = NULL;
    SDL_BlitSurface( text_surface, NULL, shelter_text_surface, NULL );
    shelter_event_mode = event_mode;
    shelter_mouse_state.x = last_mouse_state.x;
    shelter_mouse_state.y = last_mouse_state.y;
    shelter_text_buffer = current_text_buffer;
    event_mode = IDLE_EVENT_MODE;
    system_menu_enter_flag = true;
    yesno_caller = SYSTEM_NULL;
}

void ONScripterLabel::leaveSystemCall( bool restore_flag )
{
    if ( restore_flag ){
        SDL_BlitSurface( shelter_text_surface, NULL, text_surface, NULL );

        flush();
        root_button_link.next = shelter_button_link;
        root_select_link.next = shelter_select_link;
        event_mode = shelter_event_mode;
        if ( event_mode & WAIT_BUTTON_MODE ){
            if ( mouse_rotation_mode == MOUSE_ROTATION_NONE ||
                 mouse_rotation_mode == MOUSE_ROTATION_PDA_VGA )
                SDL_WarpMouse( shelter_mouse_state.x, shelter_mouse_state.y );
            else if ( mouse_rotation_mode == MOUSE_ROTATION_PDA )
                SDL_WarpMouse( screen_height - shelter_mouse_state.y - 1, shelter_mouse_state.x );
        }
        current_text_buffer = shelter_text_buffer;
    }

    system_menu_mode = SYSTEM_NULL;
    system_menu_enter_flag = false;
    yesno_caller = SYSTEM_NULL;
    key_pressed_flag = false;

    //printf("leaveSystemCall %d %d\n",event_mode, clickstr_state);

    if ( event_mode & WAIT_SLEEP_MODE )
        event_mode &= ~WAIT_SLEEP_MODE;
    else
        event_mode |= WAIT_ANIMATION_MODE;
    refreshMouseOverButton();
    advancePhase();
}

void ONScripterLabel::executeSystemCall()
{
    //printf("*****  executeSystemCall %d %d %d*****\n", system_menu_enter_flag, volatile_button_state.button, system_menu_mode );
    
    if ( !system_menu_enter_flag ){
        enterSystemCall();
    }
    
    switch( system_menu_mode ){
      case SYSTEM_SKIP:
        executeSystemSkip();
        break;
      case SYSTEM_RESET:
        executeSystemReset();
        break;
      case SYSTEM_SAVE:
        executeSystemSave();
        break;
      case SYSTEM_YESNO:
        executeSystemYesNo();
        break;
      case SYSTEM_LOAD:
        executeSystemLoad();
        break;
      case SYSTEM_LOOKBACK:
        executeSystemLookback();
        break;
      case SYSTEM_WINDOWERASE:
        executeWindowErase();
        break;
      case SYSTEM_MENU:
        executeSystemMenu();
        break;
      default:
        leaveSystemCall();
    }
}

void ONScripterLabel::executeSystemMenu()
{
    MenuLink *link;
    int counter = 1;

    if ( event_mode & (WAIT_INPUT_MODE | WAIT_BUTTON_MODE) ){

        if ( current_button_state.button == 0 ) return;
        event_mode = IDLE_EVENT_MODE;

        deleteButtonLink();

        if ( current_button_state.button == -1 ){
            if ( menuselectvoice_file_name[MENUSELECTVOICE_CANCEL] )
                playWave( menuselectvoice_file_name[MENUSELECTVOICE_CANCEL], false, DEFAULT_WAVE_CHANNEL );
            leaveSystemCall();
            return;
        }
    
        if ( menuselectvoice_file_name[MENUSELECTVOICE_CLICK] )
            playWave( menuselectvoice_file_name[MENUSELECTVOICE_CLICK], false, DEFAULT_WAVE_CHANNEL );
        
        link = root_menu_link.next;
        while ( link ){
            if ( current_button_state.button == counter++ ){
                system_menu_mode = link->system_call_no;
                break;
            }
            link = link->next;
        }

        advancePhase();
    }
    else{
        if ( menuselectvoice_file_name[MENUSELECTVOICE_OPEN] )
            playWave( menuselectvoice_file_name[MENUSELECTVOICE_OPEN], false, DEFAULT_WAVE_CHANNEL );
        refreshSurface( text_surface, NULL );
        shadowTextDisplay( text_surface, text_surface, NULL, &menu_font );
        flush();

        menu_font.num_xy[0] = menu_link_width;
        menu_font.num_xy[1] = menu_link_num;
        menu_font.top_xy[0] = (screen_width * screen_ratio2 / screen_ratio1 - menu_font.num_xy[0] * menu_font.pitch_xy[0]) / 2;
        menu_font.top_xy[1] = (screen_height * screen_ratio2 / screen_ratio1  - menu_font.num_xy[1] * menu_font.pitch_xy[1]) / 2;

        menu_font.xy[0] = (menu_font.num_xy[0] - menu_link_width) / 2;
        menu_font.xy[1] = (menu_font.num_xy[1] - menu_link_num) / 2;

        link = root_menu_link.next;
        while( link ){
            last_button_link->next = getSelectableSentence( link->label, &menu_font, false );
            last_button_link = last_button_link->next;
            last_button_link->no = counter++;

            link = link->next;
            flush( &last_button_link->image_rect );
        }

        event_mode = WAIT_INPUT_MODE | WAIT_BUTTON_MODE;
        refreshMouseOverButton();
        system_menu_mode = SYSTEM_MENU;
        yesno_caller = SYSTEM_MENU;
    }
}

void ONScripterLabel::executeSystemSkip()
{
    skip_flag = true;
    if ( !(shelter_event_mode & WAIT_BUTTON_MODE) )
        shelter_event_mode |= WAIT_SLEEP_MODE;
    leaveSystemCall();
}

void ONScripterLabel::executeSystemReset()
{
    if ( yesno_caller == SYSTEM_RESET ){
        leaveSystemCall();
    }
    else{
        if ( yesno_caller == SYSTEM_NULL )
            yesno_caller = SYSTEM_RESET;
        system_menu_mode = SYSTEM_YESNO;
        advancePhase();
    }
}

void ONScripterLabel::executeWindowErase()
{
    if ( event_mode & (WAIT_INPUT_MODE | WAIT_BUTTON_MODE) ){
        event_mode = IDLE_EVENT_MODE;

        leaveSystemCall();
    }
    else{
        refreshSurface( text_surface, NULL, mode_saya_flag ? REFRESH_SAYA_MODE : REFRESH_NORMAL_MODE );
        flush();

        event_mode = WAIT_INPUT_MODE;
        system_menu_mode = SYSTEM_WINDOWERASE;
    }
}

void ONScripterLabel::executeSystemLoad()
{
    if ( event_mode & (WAIT_INPUT_MODE | WAIT_BUTTON_MODE) ){

        if ( current_button_state.button == 0 ) return;
        event_mode = IDLE_EVENT_MODE;

        if ( current_button_state.button > 0 ){
            if ( !save_file_info[current_button_state.button-1].valid ){
                event_mode  = WAIT_INPUT_MODE | WAIT_BUTTON_MODE;
                refreshMouseOverButton();
                return;
            }
            deleteButtonLink();
            yesno_selected_file_no = current_button_state.button;
            yesno_caller = SYSTEM_LOAD;
            system_menu_mode = SYSTEM_YESNO;
            advancePhase();
        }
        else{
            deleteButtonLink();
            leaveSystemCall();
        }
    }
    else{
        searchSaveFiles();
    
        refreshSurface( text_surface, NULL );
        shadowTextDisplay( text_surface, text_surface, NULL, &menu_font );
        flush();
        
        system_font.xy[0] = (system_font.num_xy[0] - strlen( load_menu_name ) / 2) / 2;
        system_font.xy[1] = 0;
        drawString( load_menu_name, system_font.color, &system_font, true, text_surface );
        system_font.xy[1] += 2;
        
        int counter = 1;
        bool nofile_flag;
        char *buffer = new char[ strlen( save_item_name ) + 30 + 1 ];

        for ( unsigned int i=0 ; i<num_save_file ; i++ ){
            system_font.xy[0] = (system_font.num_xy[0] - (strlen( save_item_name ) / 2 + 15) ) / 2;

            if ( save_file_info[i].valid ){
                sprintf( buffer, "%s%s@%sŒŽ%s“ú%sŽž%s•ª",
                         save_item_name,
                         save_file_info[i].sjis_no,
                         save_file_info[i].sjis_month,
                         save_file_info[i].sjis_day,
                         save_file_info[i].sjis_hour,
                         save_file_info[i].sjis_minute );
                nofile_flag = false;
            }
            else{
                sprintf( buffer, "%s%s@||||||||||||",
                         save_item_name,
                         save_file_info[i].sjis_no );
                nofile_flag = true;
            }
            last_button_link->next = getSelectableSentence( buffer, &system_font, false, nofile_flag );
            last_button_link = last_button_link->next;
            last_button_link->no = counter++;
            flush( &last_button_link->image_rect );
        }
        delete[] buffer;

        event_mode = WAIT_INPUT_MODE | WAIT_BUTTON_MODE;
        refreshMouseOverButton();
        system_menu_mode = SYSTEM_LOAD;
    }
}

void ONScripterLabel::executeSystemSave()
{
    if ( event_mode & (WAIT_INPUT_MODE | WAIT_BUTTON_MODE) ){

        if ( current_button_state.button == 0 ) return;
        event_mode = IDLE_EVENT_MODE;

        deleteButtonLink();

        if ( current_button_state.button > 0 ){
            yesno_selected_file_no = current_button_state.button;
            yesno_caller = SYSTEM_SAVE;
            system_menu_mode = SYSTEM_YESNO;
            advancePhase();
            return;
        }
        leaveSystemCall();
    }
    else{
        searchSaveFiles();

        refreshSurface( text_surface, NULL );
        shadowTextDisplay( text_surface, text_surface, NULL, &menu_font );
        flush();

        system_font.xy[0] = (system_font.num_xy[0] - strlen( save_menu_name ) / 2 ) / 2;
        system_font.xy[1] = 0;
        drawString( save_menu_name, system_font.color, &system_font, true, text_surface );
        system_font.xy[1] += 2;
        
        int counter = 1;
        bool nofile_flag;
        char *buffer = new char[ strlen( save_item_name ) + 30 + 1 ];
    
        for ( unsigned int i=0 ; i<num_save_file ; i++ ){
            system_font.xy[0] = (system_font.num_xy[0] - (strlen( save_item_name ) / 2 + 15) ) / 2;

            if ( save_file_info[i].valid ){
                sprintf( buffer, "%s%s@%sŒŽ%s“ú%sŽž%s•ª",
                         save_item_name,
                         save_file_info[i].sjis_no,
                         save_file_info[i].sjis_month,
                         save_file_info[i].sjis_day,
                         save_file_info[i].sjis_hour,
                         save_file_info[i].sjis_minute );
                nofile_flag = false;
            }
            else{
                sprintf( buffer, "%s%s@||||||||||||",
                         save_item_name,
                         save_file_info[i].sjis_no );
                nofile_flag = true;
            }
            last_button_link->next = getSelectableSentence( buffer, &system_font, false, nofile_flag );
            last_button_link = last_button_link->next;
            last_button_link->no = counter++;
            flush( &last_button_link->image_rect );
        }
        delete[] buffer;

        event_mode = WAIT_INPUT_MODE | WAIT_BUTTON_MODE;
        refreshMouseOverButton();
        system_menu_mode = SYSTEM_SAVE;
    }
}

void ONScripterLabel::executeSystemYesNo()
{
    char name[64] = {'\0'};
	
    if ( event_mode & (WAIT_INPUT_MODE | WAIT_BUTTON_MODE) ){

        if ( current_button_state.button == 0 ) return;
        event_mode = IDLE_EVENT_MODE;

        deleteButtonLink();

        if ( current_button_state.button == 1 ){ // yes is selected
            if ( menuselectvoice_file_name[MENUSELECTVOICE_YES] )
                playWave( menuselectvoice_file_name[MENUSELECTVOICE_YES], false, DEFAULT_WAVE_CHANNEL );
            if ( yesno_caller == SYSTEM_SAVE ){
                saveSaveFile( yesno_selected_file_no );
                leaveSystemCall();
            }
            else if ( yesno_caller == SYSTEM_LOAD ){

                if ( loadSaveFile( yesno_selected_file_no ) ){
                    system_menu_mode = yesno_caller;
                    advancePhase();
                    return;
                }
                leaveSystemCall( false );
                saveon_flag = true;
            }
            else if ( yesno_caller == SYSTEM_RESET || 
                      yesno_caller == SYSTEM_MENU ){

                resetCommand();
                //line_cache = -1;
                event_mode = WAIT_SLEEP_MODE;
                leaveSystemCall( false );
            }
        }
        else{
            if ( menuselectvoice_file_name[MENUSELECTVOICE_NO] )
                playWave( menuselectvoice_file_name[MENUSELECTVOICE_NO], false, DEFAULT_WAVE_CHANNEL );
            system_menu_mode = yesno_caller;
            advancePhase();
        }
    }
    else{
        refreshSurface( text_surface, NULL );
        shadowTextDisplay( text_surface, text_surface, NULL, &menu_font );
        flush();

        if ( yesno_caller == SYSTEM_SAVE || yesno_caller == SYSTEM_LOAD )
            sprintf( name, "%s%s", 
                     save_item_name,
                     save_file_info[yesno_selected_file_no-1].sjis_no );
            
        if ( yesno_caller == SYSTEM_SAVE )
            strcat( name, "‚ÉƒZ[ƒu‚µ‚Ü‚·B‚æ‚ë‚µ‚¢‚Å‚·‚©H" );
        else if ( yesno_caller == SYSTEM_LOAD )
            strcat( name, "‚ðƒ[ƒh‚µ‚Ü‚·B‚æ‚ë‚µ‚¢‚Å‚·‚©H" );
        else if ( yesno_caller == SYSTEM_RESET || 
                  yesno_caller == SYSTEM_MENU )
            strcpy( name, "I—¹‚µ‚Ü‚·B‚æ‚ë‚µ‚¢‚Å‚·‚©H" );
        system_font.xy[0] = (system_font.num_xy[0] - strlen( name ) / 2 ) / 2;
        system_font.xy[1] = 10;
        drawString( name, system_font.color, &system_font, true, text_surface );

        strcpy( name, "‚Í‚¢" );
        system_font.xy[0] = 12;
        system_font.xy[1] = 12;
        last_button_link->next = getSelectableSentence( name, &system_font, false );
        last_button_link = last_button_link->next;
        last_button_link->no = 1;
        flush( &last_button_link->image_rect );

        strcpy( name, "‚¢‚¢‚¦" );
        system_font.xy[0] = 18;
        system_font.xy[1] = 12;
        last_button_link->next = getSelectableSentence( name, &system_font, false );
        last_button_link = last_button_link->next;
        last_button_link->no = 2;
        flush( &last_button_link->image_rect );
        
        event_mode = WAIT_INPUT_MODE | WAIT_BUTTON_MODE;
        refreshMouseOverButton();
    }
}

void ONScripterLabel::setupLookbackButton()
{
    AnimationInfo *info[2];
    SDL_Rect rect;
    int offset;

    deleteButtonLink();
    
    /* ---------------------------------------- */
    /* Previous button check */
    if ( (current_text_buffer->previous->xy[1] != -1 ) &&
         current_text_buffer->previous != shelter_text_buffer ){
        last_button_link->next = new ButtonLink();
        last_button_link = last_button_link->next;
    
        last_button_link->button_type = NORMAL_BUTTON;
        last_button_link->no = 1;
        last_button_link->select_rect.x = sentence_font_info.pos.x;
        last_button_link->select_rect.y = sentence_font_info.pos.y;
        last_button_link->select_rect.w = sentence_font_info.pos.w;
        last_button_link->select_rect.h = sentence_font_info.pos.h/3;

        if ( lookback_sp[0] >= 0 ){
            info[0] = &sprite_info[ lookback_sp[0] ];
            info[0]->current_cell = 0;
            info[1] = &sprite_info[ lookback_sp[0] ];
            last_button_link->image_rect.x = sprite_info[ lookback_sp[0] ].pos.x;
            last_button_link->image_rect.y = sprite_info[ lookback_sp[0] ].pos.y;
        }
        else{
            info[0] = &lookback_info[0];
            info[1] = &lookback_info[1];
            last_button_link->image_rect.x = sentence_font_info.pos.x + sentence_font_info.pos.w - info[0]->pos.w;
            last_button_link->image_rect.y = sentence_font_info.pos.y;
        }
            
        if ( info[0]->image_surface ){
            last_button_link->image_rect.w = info[0]->pos.w;
            last_button_link->image_rect.h = info[0]->pos.h;
            
            last_button_link->selected_surface =
                SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG,
                                      info[0]->pos.w, info[0]->pos.h,
                                      32, rmask, gmask, bmask, amask );
            SDL_SetAlpha( last_button_link->selected_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );

            offset = (info[0]->pos.w + info[0]->alpha_offset) * info[0]->current_cell;
            rect.x = rect.y = 0;
            rect.w = info[0]->pos.w;
            rect.h = info[0]->pos.h;
            alphaBlend( last_button_link->selected_surface, rect,
                        text_surface, last_button_link->image_rect.x, last_button_link->image_rect.y,
                        info[0]->image_surface, offset, 0,
                        info[0]->mask_surface, info[0]->alpha_offset,
                        info[0]->trans_mode );
        }

        if ( lookback_sp[0] >= 0 ) info[1]->current_cell = 1;

        if ( info[1]->image_surface ){
            last_button_link->no_selected_surface =
                SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG,
                                      info[1]->pos.w, info[1]->pos.h,
                                      32, rmask, gmask, bmask, amask );
            SDL_SetAlpha( last_button_link->no_selected_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
        
            offset = (info[1]->pos.w + info[1]->alpha_offset) * info[1]->current_cell;
            rect.x = rect.y = 0;
            rect.w = info[1]->pos.w;
            rect.h = info[1]->pos.h;
            alphaBlend( last_button_link->no_selected_surface, rect,
                        text_surface, last_button_link->image_rect.x, last_button_link->image_rect.y,
                        info[1]->image_surface, offset, 0,
                        info[1]->mask_surface, info[1]->alpha_offset,
                        info[1]->trans_mode );
            SDL_BlitSurface( last_button_link->no_selected_surface, NULL, text_surface, &last_button_link->image_rect );
        }
    }

    /* ---------------------------------------- */
    /* Next button check */
    if ( current_text_buffer->next != shelter_text_buffer ){
        last_button_link->next = new ButtonLink();
        last_button_link = last_button_link->next;
    
        last_button_link->button_type = NORMAL_BUTTON;
        last_button_link->no = 2;
        last_button_link->select_rect.x = sentence_font_info.pos.x;
        last_button_link->select_rect.y = sentence_font_info.pos.y + sentence_font_info.pos.h*2/3;
        last_button_link->select_rect.w = sentence_font_info.pos.w;
        last_button_link->select_rect.h = sentence_font_info.pos.h/3;

        if ( lookback_sp[1] >= 0 ){
            info[0] = &sprite_info[ lookback_sp[1] ];
            info[0]->current_cell = 0;
            info[1] = &sprite_info[ lookback_sp[1] ];
            last_button_link->image_rect.x = sprite_info[ lookback_sp[1] ].pos.x;
            last_button_link->image_rect.y = sprite_info[ lookback_sp[1] ].pos.y;
        }
        else{
            info[0] = &lookback_info[2];
            info[1] = &lookback_info[3];
            last_button_link->image_rect.x = sentence_font_info.pos.x + sentence_font_info.pos.w - info[0]->pos.w;
            last_button_link->image_rect.y = sentence_font_info.pos.y + sentence_font_info.pos.h - info[0]->pos.h;
        }
            
        if ( info[0]->image_surface ){
            last_button_link->image_rect.w = info[0]->pos.w;
            last_button_link->image_rect.h = info[0]->pos.h;
            
            last_button_link->selected_surface =
                SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG,
                                      info[0]->pos.w, info[0]->pos.h,
                                      32, rmask, gmask, bmask, amask );
            SDL_SetAlpha( last_button_link->selected_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );

            offset = (info[0]->pos.w + info[0]->alpha_offset) * info[0]->current_cell;
            rect.x = rect.y = 0;
            rect.w = info[0]->pos.w;
            rect.h = info[0]->pos.h;
            alphaBlend( last_button_link->selected_surface, rect,
                        text_surface, last_button_link->image_rect.x, last_button_link->image_rect.y,
                        info[0]->image_surface, offset, 0,
                        info[0]->mask_surface, info[0]->alpha_offset,
                        info[0]->trans_mode );
        }

        if ( lookback_sp[1] >= 0 ) info[1]->current_cell = 1;

        if ( info[1]->image_surface ){
            last_button_link->no_selected_surface =
                SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG,
                                      info[1]->pos.w, info[1]->pos.h,
                                      32, rmask, gmask, bmask, amask );
            SDL_SetAlpha( last_button_link->no_selected_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
        
            offset = (info[1]->pos.w + info[1]->alpha_offset) * info[1]->current_cell;
            rect.x = rect.y = 0;
            rect.w = info[1]->pos.w;
            rect.h = info[1]->pos.h;
            alphaBlend( last_button_link->no_selected_surface, rect,
                        text_surface, last_button_link->image_rect.x, last_button_link->image_rect.y,
                        info[1]->image_surface, offset, 0,
                        info[1]->mask_surface, info[1]->alpha_offset,
                        info[1]->trans_mode );
            SDL_BlitSurface( last_button_link->no_selected_surface, NULL, text_surface, &last_button_link->image_rect );
        }
    }
}

void ONScripterLabel::executeSystemLookback()
{
    int i;
    uchar3 color;
    
    if ( event_mode & (WAIT_INPUT_MODE | WAIT_BUTTON_MODE) ){
        if ( current_button_state.button == 0 ) return;
        if ( current_button_state.button < 0 ){
            event_mode = IDLE_EVENT_MODE;
            deleteButtonLink();
            leaveSystemCall();
            return;
        }
        
        if ( current_button_state.button == 1 )
            current_text_buffer = current_text_buffer->previous;
        else
            current_text_buffer = current_text_buffer->next;
    }
    else{
        current_text_buffer = current_text_buffer->previous;
        if ( current_text_buffer->xy[1] == -1 ){
            leaveSystemCall();
            return;
        }

        event_mode = WAIT_INPUT_MODE | WAIT_BUTTON_MODE;
        system_menu_mode = SYSTEM_LOOKBACK;
    }

    shadowTextDisplay( text_surface, accumulation_surface, NULL, &menu_font );
    for ( i=0 ; i<3 ; i++ ){
        color[i] = sentence_font.color[i];
        sentence_font.color[i] = lookback_color[i];
    }
    restoreTextBuffer();
    for ( i=0 ; i<3 ; i++ ) sentence_font.color[i] = color[i];
    setupLookbackButton();
    refreshMouseOverButton();
    flush();
}
