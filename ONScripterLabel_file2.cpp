/* -*- C++ -*-
 *
 *  ONScripterLabel_file2.cpp - FILE I/O of ONScripter
 *
 *  Copyright (c) 2001-2005 Ogapee. All rights reserved.
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

int ONScripterLabel::loadSaveFile2( FILE *fp, int file_version )
{
    deleteNestInfo();
    
    int i, j;
    
    loadInt( fp, &i ); // 1
    loadInt( fp, &i );
    if ( i==1 ) sentence_font.is_bold = true;
    else        sentence_font.is_bold = false;
    loadInt( fp, &i );
    if ( i==1 ) sentence_font.is_shadow = true;
    else        sentence_font.is_shadow = false;

    loadInt( fp, &i ); // 0
    loadInt( fp, &i );
    rmode_flag = (i==1)?true:false;
    loadInt( fp, &i );
    sentence_font.color[0] = i;
    loadInt( fp, &i );
    sentence_font.color[1] = i;
    loadInt( fp, &i );
    sentence_font.color[2] = i;
    cursor_info[0].remove();
    loadStr( fp, &cursor_info[0].image_name );
    if ( cursor_info[0].image_name ){
        parseTaggedString( &cursor_info[0] );
        setupAnimationInfo( &cursor_info[0] );
        if ( cursor_info[0].image_surface )
            cursor_info[0 ].visible = true;
    }
    cursor_info[1].remove();
    loadStr( fp, &cursor_info[1].image_name );
    if ( cursor_info[1].image_name ){
        parseTaggedString( &cursor_info[1] );
        setupAnimationInfo( &cursor_info[1] );
        if ( cursor_info[1].image_surface )
            cursor_info[1 ].visible = true;
    }

    loadInt( fp, &window_effect.effect );
    loadInt( fp, &window_effect.duration );
    loadStr( fp, &window_effect.anim.image_name ); // probably

    sentence_font.clear();
    sentence_font.ttf_font  = NULL;
    loadInt( fp, &sentence_font.top_xy[0] );
    loadInt( fp, &sentence_font.top_xy[1] );
    loadInt( fp, &sentence_font.num_xy[0] );
    loadInt( fp, &sentence_font.num_xy[1] );
    loadInt( fp, &sentence_font.font_size_xy[0] );
    loadInt( fp, &sentence_font.font_size_xy[1] );
    loadInt( fp, &sentence_font.pitch_xy[0] );
    loadInt( fp, &sentence_font.pitch_xy[1] );
    for ( i=0 ; i<3 ; i++ )
        sentence_font.window_color[2-i] = fgetc( fp );
    if ( fgetc( fp ) == 0x00 ) sentence_font.is_transparent = true;
    else                       sentence_font.is_transparent = false;
    loadInt( fp, &sentence_font.wait_time );
    loadInt( fp, &i );
    sentence_font_info.pos.x = i * screen_ratio1 / screen_ratio2;
    loadInt( fp, &i );
    sentence_font_info.pos.y = i * screen_ratio1 / screen_ratio2;
    loadInt( fp, &i );
    sentence_font_info.pos.w = (i + 1 - sentence_font_info.pos.x * screen_ratio1 / screen_ratio2) * screen_ratio1 / screen_ratio2;
    loadInt( fp, &i );
    sentence_font_info.pos.h = (i + 1 - sentence_font_info.pos.y * screen_ratio1 / screen_ratio2) * screen_ratio1 / screen_ratio2;
    loadStr( fp, &sentence_font_info.image_name );
    if ( !sentence_font.is_transparent && sentence_font_info.image_name ){
        parseTaggedString( &sentence_font_info );
        setupAnimationInfo( &sentence_font_info );
    }

    loadInt( fp, &i );
    if ( i==1 ) cursor_info[0].abs_flag = false;
    else        cursor_info[0].abs_flag = true;
    loadInt( fp, &i );
    if ( i==1 ) cursor_info[1].abs_flag = false;
    else        cursor_info[1].abs_flag = true;
    loadInt( fp, &i );
    cursor_info[0].pos.x = i * screen_ratio1 / screen_ratio2;
    loadInt( fp, &i );
    cursor_info[1].pos.x = i * screen_ratio1 / screen_ratio2;
    loadInt( fp, &i );
    cursor_info[0].pos.y = i * screen_ratio1 / screen_ratio2;
    loadInt( fp, &i );
    cursor_info[1].pos.y = i * screen_ratio1 / screen_ratio2;

    // load background surface
    bg_info.remove();
    loadStr( fp, &bg_info.file_name );
    createBackground();

    for ( i=0 ; i<3 ; i++ ){
        tachi_info[i].remove();
        loadStr( fp, &tachi_info[i].image_name );
        if ( tachi_info[i].image_name ){
            parseTaggedString( &tachi_info[i] );
            setupAnimationInfo( &tachi_info[i] );
        }
    }

    for ( i=0 ; i<3 ; i++ ){
        loadInt( fp, &j );
        tachi_info[i].pos.x = j * screen_ratio1 / screen_ratio2;
    }

    for ( i=0 ; i<3 ; i++ ){
        loadInt( fp, &j );
        tachi_info[i].pos.y = j * screen_ratio1 / screen_ratio2;
    }

    loadInt( fp, &i ); // 0
    loadInt( fp, &i ); // 0
    loadInt( fp, &i ); // 0
    
    for ( i=0 ; i<MAX_SPRITE_NUM ; i++ ){
        sprite_info[i].remove();
        loadStr( fp, &sprite_info[i].image_name );
        if ( sprite_info[i].image_name ){
            parseTaggedString( &sprite_info[i] );
            setupAnimationInfo( &sprite_info[i] );
        }
        loadInt( fp, &j );
        sprite_info[i].pos.x = j * screen_ratio1 / screen_ratio2;
        loadInt( fp, &j );
        sprite_info[i].pos.y = j * screen_ratio1 / screen_ratio2;
        loadInt( fp, &j );
        if ( j==1 ) sprite_info[i].visible = true;
        else        sprite_info[i].visible = false;
        loadInt( fp, &sprite_info[i].current_cell );
    }

    loadVariables( fp, 0, script_h.global_variable_border );

    // nested info
    int num_nest = 0;
    loadInt( fp, &num_nest );
    last_nest_info = &root_nest_info;
    if (num_nest > 0){
        fseek( fp, (num_nest-1)*4, SEEK_CUR );
        while( num_nest > 0 ){
            NestInfo *info = new NestInfo();
            if (last_nest_info == &root_nest_info) last_nest_info = info;
        
            loadInt( fp, &i );
            if (i > 0){
                info->nest_mode = NestInfo::LABEL;
                info->next_script = script_h.getAddress( i );
                fseek( fp, -8, SEEK_CUR );
                num_nest--;
            }
            else{
                info->nest_mode = NestInfo::FOR;
                info->next_script = script_h.getAddress( -i );
                fseek( fp, -16, SEEK_CUR );
                loadInt( fp, &info->var_no );
                loadInt( fp, &info->to );
                loadInt( fp, &info->step );
                fseek( fp, -16, SEEK_CUR );
                num_nest -= 4;
            }
            info->next = root_nest_info.next;
            if (root_nest_info.next) root_nest_info.next->previous = info;
            root_nest_info.next = info;
            info->previous = &root_nest_info;
        }
        loadInt( fp, &num_nest );
        fseek(fp, num_nest*4, SEEK_CUR);
    }

    loadInt( fp, &i );
    if (i == 1) monocro_flag = true;
    else        monocro_flag = false;
    for ( i=0 ; i<3 ; i++ ){
        loadInt( fp, &j );
        monocro_color[2-i] = j;
    }
    for ( i=0 ; i<256 ; i++ ){
        monocro_color_lut[i][0] = (monocro_color[0] * i) >> 8;
        monocro_color_lut[i][1] = (monocro_color[1] * i) >> 8;
        monocro_color_lut[i][2] = (monocro_color[2] * i) >> 8;
    }
    loadInt( fp, &nega_mode );
    
    // ----------------------------------------
    // Sound
    stopCommand();
    loopbgmstopCommand();

    loadStr( fp, &midi_file_name ); // MIDI file
    loadStr( fp, &wave_file_name ); // wave, waveloop
    loadInt( fp, &i );
    if ( i >= 0 ) current_cd_track = i;

    loadInt( fp, &i ); // play, playonce MIDI
    if ( i==1 ){
        midi_play_loop_flag = true;
        internal_midi_play_loop_flag = true;
        current_cd_track = -2;
        playMIDIFile(midi_file_name);
    }
    else
        midi_play_loop_flag = false;
    
    loadInt( fp, &i ); // wave, waveloop
    if ( i==1 ) wave_play_loop_flag = true;
    else        wave_play_loop_flag = false;
    if ( wave_file_name && wave_play_loop_flag )
        playWave( wave_file_name, wave_play_loop_flag, MIX_WAVE_CHANNEL );

    loadInt( fp, &i ); // play, playonce
    if ( i==1 ) cd_play_loop_flag = true;
    else        cd_play_loop_flag = false;
    if ( current_cd_track >= 0 ){
        if ( cdaudio_flag ){
            if ( cdrom_info ) playCDAudio( current_cd_track );
        }
        else{
            playMP3( current_cd_track );
        }
    }

    loadInt( fp, &i ); // bgm, mp3, mp3loop
    if ( i==1 ) music_play_loop_flag = true;
    else        music_play_loop_flag = false;
    loadInt( fp, &i );
    if ( i==1 ) mp3save_flag = true;
    else        mp3save_flag = false;
    loadStr( fp, &music_file_name );
    if ( music_file_name ){
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

    loadInt( fp, &erase_text_window_mode );
    loadInt( fp, &i ); // 1

    barclearCommand();
    for ( i=0 ; i<MAX_PARAM_NUM ; i++ ){
        loadInt( fp, &j );
        if ( j != 0 ){
            bar_info[i] = new AnimationInfo();
            bar_info[i]->param = j;
            loadInt( fp, &j );
            bar_info[i]->pos.x = j * screen_ratio1 / screen_ratio2;
            loadInt( fp, &j );
            bar_info[i]->pos.y = j * screen_ratio1 / screen_ratio2;
            loadInt( fp, &j );
            bar_info[i]->max_width = j * screen_ratio1 / screen_ratio2;
            loadInt( fp, &j );
            bar_info[i]->pos.h = j * screen_ratio1 / screen_ratio2;
            loadInt( fp, &j );
            bar_info[i]->max_param = j;
            for ( j=0 ; j<3 ; j++ )
                bar_info[i]->color[2-j] = fgetc( fp );
            fgetc( fp ); // 0x00

            int w = bar_info[i]->max_width * bar_info[i]->param / bar_info[i]->max_param;
            if ( bar_info[i]->max_width > 0 && w > 0 ){
                bar_info[i]->pos.w = w;
                bar_info[i]->allocImage( bar_info[i]->pos.w, bar_info[i]->pos.h );
                bar_info[i]->fill( bar_info[i]->color[0], bar_info[i]->color[1], bar_info[i]->color[2], 0xff );
            }
        }
        else{
            loadInt( fp, &j ); // -1
            loadInt( fp, &j ); // 0
            loadInt( fp, &j ); // 0
            loadInt( fp, &j ); // 0
            loadInt( fp, &j ); // 0
            loadInt( fp, &j ); // 0
        }
    }

    prnumclearCommand();
    for ( i=0 ; i<MAX_PARAM_NUM ; i++ ){
        loadInt( fp, &j );
        if ( prnum_info[i] ){
            prnum_info[i] = new AnimationInfo();
            prnum_info[i]->param = j;
            loadInt( fp, &j );
            prnum_info[i]->pos.x = j * screen_ratio1 / screen_ratio2;
            loadInt( fp, &j );
            prnum_info[i]->pos.y = j * screen_ratio1 / screen_ratio2;
            loadInt( fp, &prnum_info[i]->font_size_xy[0] );
            loadInt( fp, &prnum_info[i]->font_size_xy[1] );
            for ( j=0 ; j<3 ; j++ )
                prnum_info[i]->color_list[0][2-j] = fgetc( fp );
            fgetc( fp ); // 0x00

            char num_buf[7];
            script_h.getStringFromInteger( num_buf, prnum_info[i]->param, 3 );
            setStr( &prnum_info[i]->file_name, num_buf );

            setupAnimationInfo( prnum_info[i] );
        }
        else{
            loadInt( fp, &j ); // -1
            loadInt( fp, &j ); // 0
            loadInt( fp, &j ); // 0
            loadInt( fp, &j ); // 0
            loadInt( fp, &j ); // 0
        }
    }

    loadInt( fp, &j ); // 1
    loadInt( fp, &j ); // 0
    loadInt( fp, &j ); // 1
    btndef_info.remove();
    loadStr( fp, &btndef_info.image_name );
    if ( btndef_info.image_name && btndef_info.image_name[0] != '\0' ){
        parseTaggedString( &btndef_info );
        setupAnimationInfo( &btndef_info );
        SDL_SetAlpha( btndef_info.image_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
    }

    if ( file_version >= 202 )
        script_h.loadArrayVariable(fp);
    
    loadInt( fp, &j ); // 0
    if ( fgetc( fp ) == 1 ) erase_text_window_mode = 2;
    fgetc( fp ); // 0
    fgetc( fp ); // 0
    fgetc( fp ); // 0
    loadStr( fp, &loop_bgm_name[0] );
    loadStr( fp, &loop_bgm_name[1] );
    if ( loop_bgm_name[0] ) {
        if ( loop_bgm_name[1] ) playWave( loop_bgm_name[1], false, MIX_LOOPBGM_CHANNEL1, WAVE_PRELOAD );
        playWave( loop_bgm_name[0], false, MIX_LOOPBGM_CHANNEL0 );
    }

    if ( file_version >= 201 ){
        loadInt( fp, &j );
        if ( j==1 ) rubyon_flag = true;
        else        rubyon_flag = false;
        loadInt( fp, &ruby_struct.font_size_xy[0] );
        loadInt( fp, &ruby_struct.font_size_xy[1] );
        loadStr( fp, &ruby_struct.font_name );
    }
    
    int text_num = 0;
    loadInt( fp, &text_num );
    start_text_buffer = current_text_buffer;
    for ( i=0 ; i<text_num ; i++ ){
        clearCurrentTextBuffer();
        do{
            current_text_buffer->buffer2[current_text_buffer->buffer2_count] = fgetc( fp );
        }
        while( current_text_buffer->buffer2[current_text_buffer->buffer2_count++] );
        current_text_buffer->buffer2_count--;
        current_text_buffer = current_text_buffer->next;
    }
    clearCurrentTextBuffer();

    loadInt( fp, &i );
    current_label_info = script_h.getLabelByLine( i );
    current_line = i - current_label_info.start_line;
    //printf("load %d:%d(%d-%d)\n", current_label_info.start_line, current_line, i, current_label_info.start_line);
    char *buf = script_h.getAddressByLine( i );
    
    loadInt( fp, &j );
    for ( i=0 ; i<j ; i++ ){
        while( *buf != ':' ) buf++;
        buf++;
    }
    script_h.setCurrent( buf );

    fclose(fp);

    dirty_rect.fill( screen_width, screen_height );
    display_mode = next_display_mode = NORMAL_DISPLAY_MODE;
    flush( refreshMode() );

    clickstr_state = CLICK_NONE;
    event_mode = 0;//WAIT_SLEEP_MODE;
    draw_cursor_flag = false;
    
    return 0;
}

void ONScripterLabel::saveSaveFile2( FILE *fp )
{
    int i, j;
    
    saveInt( fp, 1 );
    saveInt( fp, (sentence_font.is_bold?1:0) );
    saveInt( fp, (sentence_font.is_shadow?1:0) );

    saveInt( fp, 0 );
    saveInt( fp, (rmode_flag)?1:0 );
    saveInt( fp, sentence_font.color[0] );
    saveInt( fp, sentence_font.color[1] );
    saveInt( fp, sentence_font.color[2] );
    saveStr( fp, cursor_info[0].image_name );
    saveStr( fp, cursor_info[1].image_name );

    saveInt( fp, window_effect.effect );
    saveInt( fp, window_effect.duration );
    saveStr( fp, window_effect.anim.image_name ); // probably
    
    saveInt( fp, sentence_font.top_xy[0] );
    saveInt( fp, sentence_font.top_xy[1] );
    saveInt( fp, sentence_font.num_xy[0] );
    saveInt( fp, sentence_font.num_xy[1] );
    saveInt( fp, sentence_font.font_size_xy[0] );
    saveInt( fp, sentence_font.font_size_xy[1] );
    saveInt( fp, sentence_font.pitch_xy[0] );
    saveInt( fp, sentence_font.pitch_xy[1] );
    for ( i=0 ; i<3 ; i++ )
        fputc( sentence_font.window_color[2-i], fp );
    fputc( ( sentence_font.is_transparent )?0x00:0xff, fp ); 
    saveInt( fp, sentence_font.wait_time );
    saveInt( fp, sentence_font_info.pos.x * screen_ratio2 / screen_ratio1 );
    saveInt( fp, sentence_font_info.pos.y * screen_ratio2 / screen_ratio1 );
    saveInt( fp, sentence_font_info.pos.w * screen_ratio2 / screen_ratio1 + sentence_font_info.pos.x * screen_ratio2 / screen_ratio1 - 1 );
    saveInt( fp, sentence_font_info.pos.h * screen_ratio2 / screen_ratio1 + sentence_font_info.pos.y * screen_ratio2 / screen_ratio1 - 1 );
    saveStr( fp, sentence_font_info.image_name );

    saveInt( fp, (cursor_info[0].abs_flag)?0:1 );
    saveInt( fp, (cursor_info[1].abs_flag)?0:1 );
    saveInt( fp, cursor_info[0].pos.x * screen_ratio2 / screen_ratio1 );
    saveInt( fp, cursor_info[1].pos.x * screen_ratio2 / screen_ratio1 );
    saveInt( fp, cursor_info[0].pos.y * screen_ratio2 / screen_ratio1 );
    saveInt( fp, cursor_info[1].pos.y * screen_ratio2 / screen_ratio1 );
    
    saveStr( fp, bg_info.file_name );
    for ( i=0 ; i<3 ; i++ )
        saveStr( fp, tachi_info[i].image_name );

    for ( i=0 ; i<3 ; i++ )
        saveInt( fp, tachi_info[i].pos.x * screen_ratio2 / screen_ratio1 );

    for ( i=0 ; i<3 ; i++ )
        saveInt( fp, tachi_info[i].pos.y * screen_ratio2 / screen_ratio1 );

    saveInt( fp, 0 );
    saveInt( fp, 0 );
    saveInt( fp, 0 );
    
    for ( i=0 ; i<MAX_SPRITE_NUM ; i++ ){
        saveStr( fp, sprite_info[i].image_name );
        saveInt( fp, sprite_info[i].pos.x * screen_ratio2 / screen_ratio1 );
        saveInt( fp, sprite_info[i].pos.y * screen_ratio2 / screen_ratio1 );
        saveInt( fp, sprite_info[i].visible?1:0 );
        saveInt( fp, sprite_info[i].current_cell );
    }

    saveVariables( fp, 0, script_h.global_variable_border );

    // nested info
    int num_nest = 0;
    NestInfo *info = root_nest_info.next;
    while( info ){
        if      (info->nest_mode == NestInfo::LABEL) num_nest++;
        else if (info->nest_mode == NestInfo::FOR)   num_nest+=4;
        info = info->next;
    }
    saveInt( fp, num_nest );
    info = root_nest_info.next;
    while( info ){
        if  (info->nest_mode == NestInfo::LABEL){
            saveInt( fp, script_h.getOffset( info->next_script ) );
        }
        else if (info->nest_mode == NestInfo::FOR){
            saveInt( fp, info->var_no );
            saveInt( fp, info->to );
            saveInt( fp, info->step );
            saveInt( fp, -script_h.getOffset( info->next_script ) );
        }
        info = info->next;
    }
    
    saveInt( fp, (monocro_flag)?1:0 );
    for ( i=0 ; i<3 ; i++ )
        saveInt( fp, monocro_color[2-i] );
    saveInt( fp, nega_mode );

    // sound
    saveStr( fp, midi_file_name ); // MIDI file

    saveStr( fp, wave_file_name ); // wave, waveloop

    if ( current_cd_track >= 0 ) // play CD
        saveInt( fp, current_cd_track );
    else
        saveInt( fp, -1 );

    saveInt( fp, (midi_play_loop_flag)?1:0 ); // play, playonce MIDI
    saveInt( fp, (wave_play_loop_flag)?1:0 ); // wave, waveloop
    saveInt( fp, (cd_play_loop_flag)?1:0 ); // play, playonce
    saveInt( fp, (music_play_loop_flag)?1:0 ); // bgm, mp3, mp3loop
    saveInt( fp, (mp3save_flag)?1:0 );
    if (mp3save_flag)
        saveStr( fp, music_file_name );
    else
        saveStr( fp, NULL );
    
    saveInt( fp, (erase_text_window_mode>0)?1:0 );
    saveInt( fp, 1 );
    
    for ( i=0 ; i<MAX_PARAM_NUM ; i++ ){
        if ( bar_info[i] ){
            saveInt( fp, bar_info[i]->param );
            saveInt( fp, bar_info[i]->pos.x * screen_ratio2 / screen_ratio1 );
            saveInt( fp, bar_info[i]->pos.y * screen_ratio2 / screen_ratio1 );
            saveInt( fp, bar_info[i]->max_width * screen_ratio2 / screen_ratio1 );
            saveInt( fp, bar_info[i]->pos.h * screen_ratio2 / screen_ratio1 );
            saveInt( fp, bar_info[i]->max_param );
            for ( j=0 ; j<3 ; j++ )
                fputc( bar_info[i]->color[2-j], fp );
            fputc( 0x00, fp );
        }
        else{
            saveInt( fp, 0 );
            saveInt( fp, -1 );
            saveInt( fp, 0 );
            saveInt( fp, 0 );
            saveInt( fp, 0 );
            saveInt( fp, 0 );
            saveInt( fp, 0 );
        }
    }
    
    for ( i=0 ; i<MAX_PARAM_NUM ; i++ ){
        if ( prnum_info[i] ){
            saveInt( fp, prnum_info[i]->param );
            saveInt( fp, prnum_info[i]->pos.x * screen_ratio2 / screen_ratio1 );
            saveInt( fp, prnum_info[i]->pos.y * screen_ratio2 / screen_ratio1 );
            saveInt( fp, prnum_info[i]->font_size_xy[0] );
            saveInt( fp, prnum_info[i]->font_size_xy[1] );
            for ( j=0 ; j<3 ; j++ )
                fputc( prnum_info[i]->color_list[0][2-j], fp );
            fputc( 0x00, fp );
        }
        else{
            saveInt( fp, 0 );
            saveInt( fp, -1 );
            saveInt( fp, 0 );
            saveInt( fp, 0 );
            saveInt( fp, 0 );
            saveInt( fp, 0 );
        }
    }

    saveInt( fp, 1 );//
    saveInt( fp, 0 );
    saveInt( fp, 1 );
    saveStr( fp, btndef_info.image_name );

    script_h.saveArrayVariable(fp);
    
    saveInt( fp, 0 );
    fputc( (erase_text_window_mode==2)?1:0, fp );
    fputc( 0, fp );
    fputc( 0, fp );
    fputc( 0, fp );
    saveStr( fp, loop_bgm_name[0] );
    saveStr( fp, loop_bgm_name[1] );

    saveInt( fp, (rubyon_flag)?1:0 );
    saveInt( fp, ruby_struct.font_size_xy[0] );
    saveInt( fp, ruby_struct.font_size_xy[1] );
    saveStr( fp, ruby_struct.font_name );
    
    TextBuffer *tb = current_text_buffer;
    int text_num = 0;
    while( tb != start_text_buffer ){
        tb = tb->previous;
        text_num++;
    }
    saveInt( fp, text_num );

    for ( i=0 ; i<text_num ; i++ ){
        for ( j=0 ; j<tb->buffer2_count ; j++ )
            fputc( tb->buffer2[j], fp );
        fputc( 0, fp );
        tb = tb->next;
    }

    saveInt( fp, current_label_info.start_line + current_line );
    char *buf = script_h.getAddressByLine( current_label_info.start_line + current_line );
    //printf("save %d:%d\n", current_label_info.start_line, current_line);

    i = 0;
    if (!script_h.isText()){
        while( buf != script_h.getCurrent() ){
            if ( *buf == ':' ) i++;
            buf++;
        }
    }
    saveInt( fp, i );
}
