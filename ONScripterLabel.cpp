/* -*- C++ -*-
 * 
 *  ONScripterLabel.cpp - Execution block parser of ONScripter
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

extern void initSJIS2UTF16();
extern "C" void waveCallback( int channel );

#define DEFAULT_AUDIOBUF  4096

#define FONT_FILE "default.ttf"
#define REGISTRY_FILE "registry.txt"
#define DLL_FILE "dll.txt"
#define DEFAULT_ENV_FONT "ÇlÇr ÉSÉVÉbÉN"
#define DEFAULT_VOLUME 100

typedef int (ONScripterLabel::*FuncList)();
static struct FuncLUT{
    char command[40];
    FuncList method;
} func_lut[] = {
    {"wavestop",   &ONScripterLabel::wavestopCommand},
    {"waveloop",   &ONScripterLabel::waveCommand},
    {"wave",   &ONScripterLabel::waveCommand},
    {"waittimer",   &ONScripterLabel::waittimerCommand},
    {"wait",   &ONScripterLabel::waitCommand},
    {"vsp",   &ONScripterLabel::vspCommand},
    {"voicevol",   &ONScripterLabel::voicevolCommand},
    {"trap",   &ONScripterLabel::trapCommand},
    {"textspeed",   &ONScripterLabel::textspeedCommand},
    {"texton",   &ONScripterLabel::textonCommand},
    {"textoff",   &ONScripterLabel::textoffCommand},
    {"textclear",   &ONScripterLabel::textclearCommand},
    {"textbtnwait",   &ONScripterLabel::btnwaitCommand},
    {"texec",   &ONScripterLabel::texecCommand},
    {"tateyoko",   &ONScripterLabel::tateyokoCommand},
    {"tal", &ONScripterLabel::talCommand},
    {"tablegoto",   &ONScripterLabel::tablegotoCommand},
    {"systemcall",   &ONScripterLabel::systemcallCommand},
    {"stop",   &ONScripterLabel::stopCommand},
    {"spstr",   &ONScripterLabel::spstrCommand},
    {"splitstring",   &ONScripterLabel::splitstringCommand},
    {"spclclk",   &ONScripterLabel::spclclkCommand},
    {"spbtn",   &ONScripterLabel::spbtnCommand},
    {"skipoff",   &ONScripterLabel::skipoffCommand},
    {"sevol",   &ONScripterLabel::sevolCommand},
    {"setwindow2",   &ONScripterLabel::setwindow2Command},
    {"setwindow",   &ONScripterLabel::setwindowCommand},
    {"setcursor",   &ONScripterLabel::setcursorCommand},
    {"selnum",   &ONScripterLabel::selectCommand},
    {"selgosub",   &ONScripterLabel::selectCommand},
    {"selectbtnwait", &ONScripterLabel::btnwaitCommand},
    {"select",   &ONScripterLabel::selectCommand},
    {"savetime",   &ONScripterLabel::savetimeCommand},
    {"savescreenshot2",   &ONScripterLabel::savescreenshotCommand},
    {"savescreenshot",   &ONScripterLabel::savescreenshotCommand},
    {"saveon",   &ONScripterLabel::saveonCommand},
    {"saveoff",   &ONScripterLabel::saveoffCommand},
    {"savegame",   &ONScripterLabel::savegameCommand},
    {"savefileexist",   &ONScripterLabel::savefileexistCommand},
    {"rnd",   &ONScripterLabel::rndCommand},
    {"rnd2",   &ONScripterLabel::rndCommand},
    {"rmode",   &ONScripterLabel::rmodeCommand},
    {"resettimer",   &ONScripterLabel::resettimerCommand},
    {"reset",   &ONScripterLabel::resetCommand},
    {"repaint",   &ONScripterLabel::repaintCommand},
    {"quakey",   &ONScripterLabel::quakeCommand},
    {"quakex",   &ONScripterLabel::quakeCommand},
    {"quake",   &ONScripterLabel::quakeCommand},
    {"puttext",   &ONScripterLabel::puttextCommand},
    {"prnumclear",   &ONScripterLabel::prnumclearCommand},
    {"prnum",   &ONScripterLabel::prnumCommand},
    {"print",   &ONScripterLabel::printCommand},
    {"playstop",   &ONScripterLabel::playstopCommand},
    {"playonce",   &ONScripterLabel::playCommand},
    {"play",   &ONScripterLabel::playCommand},
    {"ofscpy", &ONScripterLabel::ofscpyCommand},
    {"nega", &ONScripterLabel::negaCommand},
    {"msp", &ONScripterLabel::mspCommand},
    {"mpegplay", &ONScripterLabel::mpegplayCommand},
    {"mp3vol", &ONScripterLabel::mp3volCommand},
    {"mp3stop", &ONScripterLabel::playstopCommand},
    {"mp3save", &ONScripterLabel::mp3Command},
    {"mp3loop", &ONScripterLabel::mp3Command},
    {"mp3", &ONScripterLabel::mp3Command},
    {"movemousecursor", &ONScripterLabel::movemousecursorCommand},
    {"monocro", &ONScripterLabel::monocroCommand},
    {"menu_window", &ONScripterLabel::menu_windowCommand},
    {"menu_full", &ONScripterLabel::menu_fullCommand},
    {"menu_automode", &ONScripterLabel::menu_automodeCommand},
    {"lsph", &ONScripterLabel::lspCommand},
    {"lsp", &ONScripterLabel::lspCommand},
    {"loopbgmstop", &ONScripterLabel::loopbgmstopCommand},
    {"loopbgm", &ONScripterLabel::loopbgmCommand},
    {"lookbacksp", &ONScripterLabel::lookbackspCommand},
    {"lookbackflush", &ONScripterLabel::lookbackflushCommand},
    {"lookbackbutton",      &ONScripterLabel::lookbackbuttonCommand},
    {"locate", &ONScripterLabel::locateCommand},
    {"loadgame", &ONScripterLabel::loadgameCommand},
    {"ld", &ONScripterLabel::ldCommand},
    {"jumpf", &ONScripterLabel::jumpfCommand},
    {"jumpb", &ONScripterLabel::jumpbCommand},
    {"isfull", &ONScripterLabel::isfullCommand},
    {"isskip", &ONScripterLabel::isskipCommand},
    {"ispage", &ONScripterLabel::ispageCommand},
    {"isdown", &ONScripterLabel::isdownCommand},
    {"input", &ONScripterLabel::inputCommand},
    {"humanorder", &ONScripterLabel::humanorderCommand},
    {"getzxc", &ONScripterLabel::getzxcCommand},
    {"getversion", &ONScripterLabel::getversionCommand},
    {"gettimer", &ONScripterLabel::gettimerCommand},
    {"getscreenshot", &ONScripterLabel::getscreenshotCommand},
    {"gettext", &ONScripterLabel::gettextCommand},
    {"gettab", &ONScripterLabel::gettabCommand},
    {"getret", &ONScripterLabel::getretCommand},
    {"getreg", &ONScripterLabel::getregCommand},
    {"getpageup", &ONScripterLabel::getpageupCommand},
    {"getpage", &ONScripterLabel::getpageCommand},
    {"getmousepos", &ONScripterLabel::getmouseposCommand},
    {"getinsert", &ONScripterLabel::getinsertCommand},
    {"getfunction", &ONScripterLabel::getfunctionCommand},
    {"getenter", &ONScripterLabel::getenterCommand},
    {"getcursorpos", &ONScripterLabel::getcursorposCommand},
    {"getcursor", &ONScripterLabel::getcursorCommand},
    {"getcselnum", &ONScripterLabel::getcselnumCommand},
    {"getbtntimer", &ONScripterLabel::gettimerCommand},
    {"game", &ONScripterLabel::gameCommand},
    {"fileexist", &ONScripterLabel::fileexistCommand},
    {"existspbtn", &ONScripterLabel::spbtnCommand},
    {"exec_dll", &ONScripterLabel::exec_dllCommand},
    {"exbtn_d", &ONScripterLabel::exbtnCommand},
    {"exbtn", &ONScripterLabel::exbtnCommand},
    {"erasetextwindow", &ONScripterLabel::erasetextwindowCommand},
    {"end", &ONScripterLabel::endCommand},
    {"dwavestop", &ONScripterLabel::dwavestopCommand},
    {"dwaveplayloop", &ONScripterLabel::dwaveCommand},
    {"dwaveplay", &ONScripterLabel::dwaveCommand},
    {"dwaveloop", &ONScripterLabel::dwaveCommand},
    {"dwaveload", &ONScripterLabel::dwaveCommand},
    {"dwave", &ONScripterLabel::dwaveCommand},
    {"drawtext", &ONScripterLabel::drawtextCommand},
    {"drawsp2", &ONScripterLabel::drawsp2Command},
    {"drawsp", &ONScripterLabel::drawspCommand},
    {"drawfill", &ONScripterLabel::drawfillCommand},
    {"drawclear", &ONScripterLabel::drawclearCommand},
    {"drawbg2", &ONScripterLabel::drawbg2Command},
    {"drawbg", &ONScripterLabel::drawbgCommand},
    {"draw", &ONScripterLabel::drawCommand},
    {"delay", &ONScripterLabel::delayCommand},
    {"csp", &ONScripterLabel::cspCommand},
    {"cselgoto", &ONScripterLabel::cselgotoCommand},
    {"cselbtn", &ONScripterLabel::cselbtnCommand},
    {"csel", &ONScripterLabel::selectCommand},
    {"click", &ONScripterLabel::clickCommand},
    {"cl", &ONScripterLabel::clCommand},
    {"cell", &ONScripterLabel::cellCommand},
    {"caption", &ONScripterLabel::captionCommand},
    {"btnwait2", &ONScripterLabel::btnwaitCommand},
    {"btnwait", &ONScripterLabel::btnwaitCommand},
    {"btntime2", &ONScripterLabel::btntimeCommand},
    {"btntime", &ONScripterLabel::btntimeCommand},
    {"btndown",  &ONScripterLabel::btndownCommand},
    {"btndef",  &ONScripterLabel::btndefCommand},
    {"btn",     &ONScripterLabel::btnCommand},
    {"br",      &ONScripterLabel::brCommand},
    {"blt",      &ONScripterLabel::bltCommand},
    {"bgmstop", &ONScripterLabel::playstopCommand},
    {"bgmonce", &ONScripterLabel::mp3Command}, 
    {"bgm", &ONScripterLabel::mp3Command}, 
    {"bg",      &ONScripterLabel::bgCommand},
    {"barclear",      &ONScripterLabel::barclearCommand},
    {"bar",      &ONScripterLabel::barCommand},
    {"avi",      &ONScripterLabel::aviCommand},
    {"automode_time",      &ONScripterLabel::automode_timeCommand},
    {"autoclick",      &ONScripterLabel::autoclickCommand},
    {"amsp",      &ONScripterLabel::amspCommand},
    {"allspresume",      &ONScripterLabel::allspresumeCommand},
    {"allsphide",      &ONScripterLabel::allsphideCommand},
    {"abssetcursor", &ONScripterLabel::setcursorCommand},
    {"", NULL}
};

void ONScripterLabel::initSDL( bool cdaudio_flag )
{
    /* ---------------------------------------- */
    /* Initialize SDL */

    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO ) < 0 ){
        fprintf( stderr, "Couldn't initialize SDL: %s\n", SDL_GetError() );
        exit(-1);
    }
    atexit(SDL_Quit);

    if( cdaudio_flag && SDL_InitSubSystem( SDL_INIT_CDROM ) < 0 ){
        fprintf( stderr, "Couldn't initialize CD-ROM: %s\n", SDL_GetError() );
        exit(-1);
    }

    /* ---------------------------------------- */
    /* Initialize SDL */
    if ( TTF_Init() < 0 ){
        fprintf( stderr, "can't initialize SDL TTF\n");
        exit(-1);
    }

#if defined(PDA)
    int bpp = 16;
#if defined(PDA_VGA)
    screen_width  *= 2; // for checking VGA screen
    screen_height *= 2;
#endif
#else
    int bpp = 32;
#endif

    mouse_rotation_mode = MOUSE_ROTATION_NONE;
#ifndef SCREEN_ROTATION
    screen_surface = SDL_SetVideoMode( screen_width, screen_height, bpp, DEFAULT_SURFACE_FLAG );
#else    
    screen_surface = SDL_SetVideoMode( screen_height, screen_width, bpp, DEFAULT_SURFACE_FLAG );
#endif
#if defined(USE_OVERLAY)    
    screen_overlay = SDL_CreateYUVOverlay( screen_width, screen_height, SDL_YV12_OVERLAY, screen_surface );
    printf("overlay %d\n", screen_overlay->hw_overlay );
#endif
    
    /* ---------------------------------------- */
    /* Check if VGA screen is available. */
#if defined(PDA) && defined(PDA_VGA)
    if ( screen_surface == NULL ){
        screen_width  /= 2;
        screen_height /= 2;
#ifndef SCREEN_ROTATION
        screen_surface = SDL_SetVideoMode( screen_width, screen_height, bpp, DEFAULT_SURFACE_FLAG );
#else
        screen_surface = SDL_SetVideoMode( screen_height, screen_width, bpp, DEFAULT_SURFACE_FLAG );
#endif
        mouse_rotation_mode = MOUSE_ROTATION_PDA;
    }
    else{
        screen_ratio1 *= 2;
        mouse_rotation_mode = MOUSE_ROTATION_PDA_VGA;
    }
#endif
    mouse_rotation_mode = MOUSE_ROTATION_NONE;
    underline_value = screen_height - 1;

    if ( screen_surface == NULL ) {
        fprintf( stderr, "Couldn't set %dx%dx%d video mode: %s\n",
                 screen_width, screen_height, bpp, SDL_GetError() );
        exit(-1);
    }

    initSJIS2UTF16();
    
    wm_title_string = new char[ strlen(DEFAULT_WM_TITLE) + 1 ];
    memcpy( wm_title_string, DEFAULT_WM_TITLE, strlen(DEFAULT_WM_TITLE) + 1 );
    wm_icon_string = new char[ strlen(DEFAULT_WM_ICON) + 1 ];
    memcpy( wm_icon_string, DEFAULT_WM_TITLE, strlen(DEFAULT_WM_ICON) + 1 );
    SDL_WM_SetCaption( wm_title_string, wm_icon_string );

    openAudio();
}

void ONScripterLabel::openAudio()
{
#if defined(PDA)    
    if ( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, DEFAULT_AUDIOBUF ) < 0 ){
#else        
    if ( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, DEFAULT_AUDIOBUF ) < 0 ){
#endif        
        fprintf(stderr, "Couldn't open audio device!\n"
                "  reason: [%s].\n", SDL_GetError());
        audio_open_flag = false;
    }
    else{
        int freq;
        Uint16 format;
        int channels;

        Mix_QuerySpec( &freq, &format, &channels);
        printf("Opened audio at %d Hz %d bit %s\n", freq,
               (format&0xFF),
               (channels > 1) ? "stereo" : "mono");
        audio_format.format = format;
        audio_format.freq = freq;
        audio_format.channels = channels;

        audio_open_flag = true;

        Mix_AllocateChannels( ONS_MIX_CHANNELS+ONS_MIX_EXTRA_CHANNELS );
        Mix_ChannelFinished( waveCallback );
    }
}

ONScripterLabel::ONScripterLabel( bool cdaudio_flag, char *default_font, char *default_registry, char *default_dll, char *default_archive_path, bool force_button_shortcut_flag, bool disable_rescale_flag, bool edit_flag )
        :ScriptParser( default_archive_path )
{
    int i;

    printf("ONScripter\n");

    initSDL( cdaudio_flag );
    
    amask = 0xff000000;
    rmask = 0x00ff0000;
    gmask = 0x0000ff00;
    bmask = 0x000000ff;

    picture_surface = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG, screen_width, screen_height, 32, rmask, gmask, bmask, amask );
    SDL_SetAlpha( picture_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
    text_surface = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG, screen_width, screen_height, 32, rmask, gmask, bmask, amask );
    SDL_SetAlpha( text_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
    accumulation_surface = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG, screen_width, screen_height, 32, rmask, gmask, bmask, amask );
    SDL_SetAlpha( accumulation_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
    effect_src_surface   = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG, screen_width, screen_height, 32, rmask, gmask, bmask, amask );
    SDL_SetAlpha( effect_src_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
    effect_dst_surface   = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG, screen_width, screen_height, 32, rmask, gmask, bmask, amask );
    SDL_SetAlpha( effect_dst_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
    shelter_accumulation_surface = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG, screen_width, screen_height, 32, rmask, gmask, bmask, amask );
    SDL_SetAlpha( shelter_accumulation_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
    screenshot_surface = NULL;

    internal_timer = SDL_GetTicks();
    automode_flag = false;
    automode_time = 3000;
    autoclick_time = 0;
    remaining_time = -1;
    btntime2_flag = false;
    btntime_value = 0;
    btnwait_time = 0;

    this->force_button_shortcut_flag = force_button_shortcut_flag;
    disableGetButtonFlag();
    
    tmp_save_fp = NULL;
    saveon_flag = true; // false while saveoff
    internal_saveon_flag = true; // false within a sentence

    for ( i=0 ; i<3 ; i++ ) human_order[i] = 2-i; // "rcl"
    monocro_flag = monocro_flag_new = false;
    nega_mode = 0;
    trap_flag = false;
    trap_dist = NULL;
    
    system_menu_enter_flag = false;
    system_menu_mode = SYSTEM_NULL;
    skip_flag = false;
    draw_one_page_flag = false;
    key_pressed_flag = false;
    shift_pressed_status = 0;
    ctrl_pressed_status = 0;
    display_mode = next_display_mode = NORMAL_DISPLAY_MODE;
    event_mode = IDLE_EVENT_MODE;
    all_sprite_hide_flag = false;
    
    current_over_button = 0;

    variable_edit_mode = NOT_EDIT_MODE;
    this->disable_rescale_flag = disable_rescale_flag;
    this->edit_flag = edit_flag;

    /* ---------------------------------------- */
    /* Sound related variables */
    this->cdaudio_flag = cdaudio_flag;
    cdrom_info = NULL;
    if ( cdaudio_flag ){
        if ( SDL_CDNumDrives() > 0 ) cdrom_info = SDL_CDOpen( 0 );
        if ( !cdrom_info ){
            fprintf(stderr, "Couldn't open default CD-ROM: %s\n", SDL_GetError());
        }
        else if ( cdrom_info && !CD_INDRIVE( SDL_CDStatus( cdrom_info ) ) ) {
            fprintf( stderr, "no CD-ROM in the drive\n" );
            SDL_CDClose( cdrom_info );
            cdrom_info = NULL;
        }
    }

    wave_play_loop_flag = false;
    wave_file_name = NULL;
    
    midi_play_loop_flag = false;
    internal_midi_play_loop_flag = false;
    midi_file_name = NULL;
    midi_info  = NULL;

    music_play_loop_flag = false;
    cd_play_loop_flag = false;
    mp3save_flag = false;
    mp3_sample = NULL;
    music_file_name = NULL;
    mp3_buffer = NULL;
#if defined(EXTERNAL_MUSIC_PLAYER)
    music_info = NULL;
#endif
    current_cd_track = -1;
    loop_bgm_name[0] = NULL;
    loop_bgm_name[1] = NULL;

    for ( i=0 ; i<ONS_MIX_CHANNELS+ONS_MIX_EXTRA_CHANNELS ; i++ ) wave_sample[i] = NULL;

    for ( i=0 ; i<MAX_PARAM_NUM ; i++ ) bar_info[i] = prnum_info[i] = NULL;
    
    /* ---------------------------------------- */
    /* Initialize registry */
    registry_file = NULL;
    if ( default_registry ) setStr( &registry_file, default_registry );
    else                    setStr( &registry_file, REGISTRY_FILE );

    /* ---------------------------------------- */
    /* Initialize dll */
    dll_file = NULL;
    if ( default_dll ) setStr( &dll_file, default_dll );
    else               setStr( &dll_file, DLL_FILE );
    dll_str = NULL;
    dll_ret = 0;

    /* ---------------------------------------- */
    /* Initialize font */
    if ( default_font ){
        font_file = new char[ strlen(default_font) + 1 ];
        sprintf( font_file, "%s", default_font );
    }
    else{
        font_file = new char[ strlen(archive_path) + strlen(FONT_FILE) + 1 ];
        sprintf( font_file, "%s%s", archive_path, FONT_FILE );
    }
    
    new_line_skip_flag = false;
    erase_text_window_mode = 1;
    text_on_flag = true;

    resetSentenceFont();
    if ( sentence_font.openFont( font_file, screen_ratio1, screen_ratio2 ) == NULL ){
        fprintf( stderr, "can't open font file: %s\n", font_file );
        exit(-1);
    }

    /* ---------------------------------------- */
    /* Effect related variables */
    setStr( &bg_info.file_name, "black" );
    createBackground();

    /* ---------------------------------------- */
    /* Load global variables if available */
    loadEnvData();
    
    FILE *fp;

    if ( ( fp = fopen( "gloval.sav", "rb" ) ) != NULL ||
         ( fp = fopen( "global.sav", "rb" ) ) != NULL ){
        loadVariables( fp, script_h.global_variable_border, VARIABLE_RANGE );
        fclose( fp );
    }
}

ONScripterLabel::~ONScripterLabel()
{
}

void ONScripterLabel::resetSentenceFont()
{
    sentence_font.reset();
    sentence_font.font_size_xy[0] = DEFAULT_FONT_SIZE;
    sentence_font.font_size_xy[1] = DEFAULT_FONT_SIZE;
    sentence_font.top_xy[0] = 8;
    sentence_font.top_xy[1] = 16;// + sentence_font.font_size;
    sentence_font.num_xy[0] = 23;
    sentence_font.num_xy[1] = 16;
    sentence_font.pitch_xy[0] = sentence_font.font_size_xy[0];
    sentence_font.pitch_xy[1] = 2 + sentence_font.font_size_xy[1];
    sentence_font.wait_time = 20;
    sentence_font.window_color[0] = sentence_font.window_color[1] = sentence_font.window_color[2] = 0x99;
    sentence_font_info.pos.x = 0;
    sentence_font_info.pos.y = 0;
    sentence_font_info.pos.w = screen_width+1;
    sentence_font_info.pos.h = screen_height+1;
}

void ONScripterLabel::flush( SDL_Rect *rect, bool clear_dirty_flag, bool direct_flag )
{
#if defined(USE_OVERLAY)
    SDL_LockYUVOverlay( screen_overlay );
    SDL_LockSurface( accumulation_surface );
#endif    
    if ( direct_flag ){
        flushSub( *rect );
    }
    else{
        if ( rect ) dirty_rect.add( *rect );

        if ( dirty_rect.area > 0 ){
            if ( dirty_rect.area >= dirty_rect.bounding_box.w * dirty_rect.bounding_box.h )
                flushSub( dirty_rect.bounding_box );
            else{
                for ( int i=0 ; i<dirty_rect.num_history ; i++ ){
                    //printf("%d: ", i );
                    flushSub( dirty_rect.history[i] );
                }
            }
        }
    }
#if defined(USE_OVERLAY)
    SDL_UnlockSurface( accumulation_surface );
    SDL_UnlockYUVOverlay( screen_overlay );
    
    SDL_Rect screen_rect;
    screen_rect.x = screen_rect.y = 0;
    screen_rect.w = screen_width;
    screen_rect.h = screen_height;
    SDL_DisplayYUVOverlay( screen_overlay, &screen_rect );
#endif    
    
    if ( clear_dirty_flag ) dirty_rect.clear();
}

void ONScripterLabel::flushSub( SDL_Rect &rect )
{
    //printf("flush %d %d %d %d\n", rect.x, rect.y, rect.w, rect.h );

#if defined(USE_OVERLAY)
    Uint32 *src = (Uint32*)accumulation_surface->pixels + accumulation_surface->w * rect.y + rect.x;
    Uint8  *dst_y = screen_overlay->pixels[0] + screen_overlay->w * rect.y + rect.x;
    Uint8  *dst_u = screen_overlay->pixels[1] + screen_overlay->w * rect.y + rect.x;
    Uint8  *dst_v = screen_overlay->pixels[2] + screen_overlay->w * rect.y + rect.x;
    for ( int i=rect.y ; i<rect.y+rect.h ; i++ ){
        for ( int j=rect.x ; j<rect.x+rect.w ; j++, src++ ){
            *dst_y++ = *src & 0xff;
            *dst_u++ = 0x80;
            *dst_v++ = 0x80;
        }
        src += accumulation_surface->w - rect.w;
        dst_y += screen_overlay->w - rect.w;
        dst_u += (screen_overlay->w - rect.w)/2;
        dst_v += (screen_overlay->w - rect.w)/2;
    }
#else    
#ifndef SCREEN_ROTATION
    SDL_BlitSurface( accumulation_surface, &rect, screen_surface, &rect );
    SDL_UpdateRect( screen_surface, rect.x, rect.y, rect.w, rect.h );
#else
    blitRotation( accumulation_surface, &rect, screen_surface, &rect );
    SDL_UpdateRect( screen_surface, screen_height - (rect.y +rect.h), rect.x, rect.h, rect.w );
#endif
#endif    
}

void ONScripterLabel::mouseOverCheck( int x, int y )
{
    int c = 0;

    last_mouse_state.x = x;
    last_mouse_state.y = y;

    /* ---------------------------------------- */
    /* Check button */
    int button = 0;
    ButtonLink *p_button_link = root_button_link.next;
    while( p_button_link ){
        if ( x >= p_button_link->select_rect.x && x < p_button_link->select_rect.x + p_button_link->select_rect.w &&
             y >= p_button_link->select_rect.y && y < p_button_link->select_rect.y + p_button_link->select_rect.h ){
            button = p_button_link->no;
            break;
        }
        p_button_link = p_button_link->next;
        c++;
    }

    if ( current_over_button != button ){
        DirtyRect dirty = dirty_rect;
        dirty_rect.clear();

        SDL_Rect check_src_rect = {0, 0, 0, 0};
        SDL_Rect check_dst_rect = {0, 0, 0, 0};
        if ( event_mode & WAIT_BUTTON_MODE && current_over_button != 0 ){
            if ( current_button_link->no_selected_surface ){
                SDL_BlitSurface( current_button_link->no_selected_surface, NULL, accumulation_surface, &current_button_link->image_rect );
                check_src_rect = current_button_link->image_rect;
            }
            if ( current_button_link->button_type == ButtonLink::SPRITE_BUTTON || 
                 current_button_link->button_type == ButtonLink::EX_SPRITE_BUTTON ){
                sprite_info[ current_button_link->sprite_no ].visible = true;
                sprite_info[ current_button_link->sprite_no ].setCell(0);
            }
            dirty_rect.add( current_button_link->image_rect );
        }

        if ( p_button_link ){
            if ( event_mode & WAIT_BUTTON_MODE ){
                if ( system_menu_mode != SYSTEM_NULL ){
                    if ( menuselectvoice_file_name[MENUSELECTVOICE_OVER] )
                        playWave( menuselectvoice_file_name[MENUSELECTVOICE_OVER], false, DEFAULT_WAVE_CHANNEL );
                }
                else{
                    if ( selectvoice_file_name[SELECTVOICE_OVER] )
                        playWave( selectvoice_file_name[SELECTVOICE_OVER], false, DEFAULT_WAVE_CHANNEL );
                }

                if ( p_button_link->selected_surface ){
                    SDL_BlitSurface( p_button_link->selected_surface, NULL, accumulation_surface, &p_button_link->image_rect );
                    check_dst_rect = p_button_link->image_rect;
                }
                if ( p_button_link->button_type == ButtonLink::SPRITE_BUTTON || 
                     p_button_link->button_type == ButtonLink::EX_SPRITE_BUTTON ){
                    sprite_info[ p_button_link->sprite_no ].setCell(1);
                    if ( p_button_link->button_type == ButtonLink::EX_SPRITE_BUTTON ){
                        decodeExbtnControl( accumulation_surface, p_button_link->exbtn_ctl, &check_src_rect, &check_dst_rect );
                    }
                    sprite_info[ p_button_link->sprite_no ].visible = true;
                }
                if ( nega_mode == 1 && !(event_mode & WAIT_INPUT_MODE) ) makeNegaSurface( accumulation_surface, &p_button_link->image_rect );
                if ( monocro_flag && !(event_mode & WAIT_INPUT_MODE) ) makeMonochromeSurface( accumulation_surface, &p_button_link->image_rect );
                if ( nega_mode == 2 && !(event_mode & WAIT_INPUT_MODE) ) makeNegaSurface( accumulation_surface, &p_button_link->image_rect );
                dirty_rect.add( p_button_link->image_rect );
            }
            current_button_link = p_button_link;
            shortcut_mouse_line = c;
        }
        else{
            if ( exbtn_d_button_link.exbtn_ctl ){
                decodeExbtnControl( accumulation_surface, exbtn_d_button_link.exbtn_ctl, &check_src_rect, &check_dst_rect );
            }
        }
        flush();
        dirty_rect = dirty;
    }
    current_over_button = button;
}

void ONScripterLabel::executeLabel()
{
  executeLabelTop:    

    if ( debug_level > 0 )
        printf("*****  executeLabel %s:%d:%d *****\n",
               current_link_label_info->label_info.name,
               current_link_label_info->current_line,
               string_buffer_offset );
    int ret;

    while ( current_link_label_info->current_line<current_link_label_info->label_info.num_of_lines ){
        const char *s_buf = script_h.getStringBuffer();
        if ( s_buf[string_buffer_offset] == '~' )
        {
            last_tilde.label_info = current_link_label_info->label_info;
            last_tilde.current_line = current_link_label_info->current_line;
            last_tilde.current_script = script_h.getCurrent();

            if ( jumpf_flag ) jumpf_flag = false;
            script_h.setKidokuskip( kidokuskip_flag );

            script_h.readToken(); string_buffer_offset = 0;
            continue;
        }
        if ( jumpf_flag || break_flag && strncmp( &s_buf[string_buffer_offset], "next", 4 ) )
        {
            if ( s_buf[string_buffer_offset] == 0x0a ){
                current_link_label_info->current_line++;
            }
            script_h.readToken(); string_buffer_offset = 0;
            continue;
        }

        if ( kidokuskip_flag ){
            if ( skip_flag && kidokumode_flag && !script_h.isKidoku() ) skip_flag = false;
            //script_h.markAsKidoku();
        }

        char *current = script_h.getCurrent();
        ret = ScriptParser::parseLine();
        if ( ret == RET_NOMATCH ) ret = this->parseLine();

        s_buf = script_h.getStringBuffer();
        if ( ret == RET_COMMENT ){
            script_h.readToken(); string_buffer_offset = 0;
            continue;
        }
        else if ( ret == RET_JUMP ){
            goto executeLabelTop;
        }
        else if ( ret == RET_CONTINUE || ret == RET_CONTINUE_NOREAD ){
            if ( ret == RET_CONTINUE || s_buf[ string_buffer_offset ] == 0x0a )
            {
                if ( s_buf[ string_buffer_offset ] == 0x0a )
                    current_link_label_info->current_line++;
                script_h.readToken(); // skip tailing \0 and mark kidoku
                string_buffer_offset = 0;
            }
        }
        else if ( ret == RET_SKIP_LINE ){
            current_link_label_info->current_line++;
            script_h.skipLine();
            string_buffer_offset = 0;
        }
        else if ( ret == RET_WAIT || ret == RET_WAIT_NOREAD ){
            if ( ret == RET_WAIT )
                script_h.setCurrent( current );
            return;
        }
        else if ( ret == RET_WAIT_NEXT ){
            if ( s_buf[ string_buffer_offset ] == 0x0a ){
                current_link_label_info->current_line++;
            }
            script_h.readToken();
            string_buffer_offset = 0;
            return;
        }
    }

    current_link_label_info->label_info = script_h.lookupLabelNext( current_link_label_info->label_info.name );
    current_link_label_info->current_line = 0;

    if ( current_link_label_info->label_info.start_address != NULL ){
        if ( kidokuskip_flag ){
            char *buf = current_link_label_info->label_info.label_header;
            script_h.markAsKidoku( buf );
            while( *buf != 0x0a ) buf++;
            script_h.markAsKidoku( buf );
        }
        script_h.setCurrent( current_link_label_info->label_info.start_address );
        string_buffer_offset = 0;
        goto executeLabelTop;
    }
    else fprintf( stderr, " ***** End *****\n");
}

int ONScripterLabel::parseLine( )
{
    int ret, lut_counter = 0;
    const char *s_buf = script_h.getStringBuffer();

    if ( !script_h.isText() ){
        while( func_lut[ lut_counter ].method ){
            if ( !strcmp( func_lut[ lut_counter ].command, s_buf ) ){
                return (this->*func_lut[ lut_counter ].method)();
            }
            lut_counter++;
        }
        
        if ( s_buf[0] == 'v' && s_buf[0] >= '0' && s_buf[1] <= '9' )
            return vCommand();
        else if ( s_buf[0] == 'd' && s_buf[1] == 'v' && s_buf[2] >= '0' && s_buf[2] <= '9' )
            return dvCommand();

        if ( s_buf[0] != 0x0a && s_buf[0] != '@' && s_buf[0] != '\\' &&
             s_buf[0] != '/'  && s_buf[0] != '!' && s_buf[0] != '#'  &&
             s_buf[0] != '_'  && s_buf[0] != '%' && !(s_buf[0] & 0x80 ) ){
            fprintf( stderr, " command [%s] is not supported yet!!\n", s_buf );

            string_buffer_offset = 0;
            script_h.skipToken();

            return RET_CONTINUE;
        }
    }

    /* Text */
    if ( s_buf[string_buffer_offset] != 0x0a ){
        if ( current_mode == DEFINE_MODE ) errorAndExit( "text cannot be displayed in define section." );
        ret = textCommand();
    }
    else{
        ret = RET_CONTINUE;
    }

    if ( script_h.getStringBuffer()[string_buffer_offset] == 0x0a ||
         // for puttext
         ( script_h.getStringBuffer()[string_buffer_offset] == '\0' && 
           script_h.getEndStatus() & ScriptHandler::END_QUAT ) ){
        ret = RET_CONTINUE;
        if ( !new_line_skip_flag && script_h.isText() ){
            if (!sentence_font.isEndOfLine()) // otherwise already done in drawChar
                current_text_buffer->addBuffer( 0x0a );
            sentence_font.newLine();
            if ( internal_saveon_flag ){
                internal_saveon_flag = false;
                saveSaveFile(-1);
            }
        }

        event_mode = IDLE_EVENT_MODE;
        new_line_skip_flag = false;
    }

    return ret;
}

SDL_Surface *ONScripterLabel::loadImage( char *file_name )
{
    unsigned long length;
    unsigned char *buffer;
    SDL_Surface *ret = NULL, *tmp;

    if ( !file_name ) return NULL;
    length = script_h.cBR->getFileLength( file_name );
    if ( length == 0 ){
        fprintf( stderr, " *** can't find file [%s] ***\n", file_name );
        return NULL;
    }
    if ( filelog_flag )
        script_h.findAndAddLog( ScriptHandler::FILE_LOG, file_name, true );
    //printf(" ... loading %s length %ld\n", file_name, length );
    buffer = new unsigned char[length];
    int location;
    script_h.cBR->getFile( file_name, buffer, &location );
    tmp = IMG_Load_RW(SDL_RWFromMem( buffer, length ),1);

    char *ext = strrchr(file_name, '.');
    if ( !tmp && ext && (!strcmp( ext+1, "JPG" ) || !strcmp( ext+1, "jpg" ) ) ){
        fprintf( stderr, " *** force-loading a JPG image [%s]\n", file_name );
        SDL_RWops *src = SDL_RWFromMem( buffer, length );
        tmp = IMG_LoadJPG_RW(src);
        SDL_RWclose(src);
    }
    
    delete[] buffer;
    if ( !tmp ){
        fprintf( stderr, " *** can't load file [%s] ***\n", file_name );
        return NULL;
    }

    ret = SDL_ConvertSurface( tmp, accumulation_surface->format, DEFAULT_SURFACE_FLAG );
    if ( ret &&
         screen_ratio2 != screen_ratio1 &&
         (!disable_rescale_flag || location == BaseReader::ARCHIVE_TYPE_NONE) )
    {
        SDL_Surface *ret2 = ret;

        SDL_Rect src_rect, dst_rect;
        src_rect.x = src_rect.y = dst_rect.x = dst_rect.y = 0;
        src_rect.w = ret2->w;
        src_rect.h = ret2->h;
        if ( (dst_rect.w = ret2->w * screen_ratio1 / screen_ratio2) == 0 ) dst_rect.w = 1;
        if ( (dst_rect.h = ret2->h * screen_ratio1 / screen_ratio2) == 0 ) dst_rect.h = 1;
        ret = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG, dst_rect.w, dst_rect.h, 32, rmask, gmask, bmask, amask );
        
        resizeSurface( ret2, &src_rect, ret, &dst_rect );
        SDL_FreeSurface( ret2 );
    }
    SDL_FreeSurface( tmp );

    return ret;
}

/* ---------------------------------------- */
/* Delete label link */
void ONScripterLabel::deleteLabelLink()
{
    LinkLabelInfo *info;
    
    current_link_label_info = root_link_label_info.next;
    while ( current_link_label_info ){
        info = current_link_label_info;
        current_link_label_info = current_link_label_info->next;
        delete info;
    }
    root_link_label_info.next = NULL;
    current_link_label_info = &root_link_label_info;
}

/* ---------------------------------------- */
void ONScripterLabel::deleteButtonLink()
{
    ButtonLink *b1 = root_button_link.next;

    while( b1 ){
        ButtonLink *b2 = b1;
        b1 = b1->next;
        delete b2;
    }
    root_button_link.next = NULL;
}

void ONScripterLabel::refreshMouseOverButton()
{
    int mx, my;
    current_over_button = 0;
    current_button_link = root_button_link.next;
    SDL_GetMouseState( &mx, &my );

    if      ( mouse_rotation_mode == MOUSE_ROTATION_NONE ||
              mouse_rotation_mode == MOUSE_ROTATION_PDA_VGA )
        mouseOverCheck( mx, my );
    else if ( mouse_rotation_mode == MOUSE_ROTATION_PDA )
        mouseOverCheck( my, screen_height - mx - 1 );
}

/* ---------------------------------------- */
/* Delete select link */
void ONScripterLabel::deleteSelectLink()
{
    SelectLink *link, *last_select_link = root_select_link.next;

    while ( last_select_link ){
        link = last_select_link;
        last_select_link = last_select_link->next;
        delete link;
    }
    root_select_link.next = NULL;
}

int ONScripterLabel::enterTextDisplayMode( int ret_wait )
{
    if ( !(display_mode & TEXT_DISPLAY_MODE) ){
        if ( event_mode & EFFECT_EVENT_MODE ){
            if ( doEffect( WINDOW_EFFECT, NULL, DIRECT_EFFECT_IMAGE ) == RET_CONTINUE ){
                display_mode = TEXT_DISPLAY_MODE;
                text_on_flag = true;
                return RET_CONTINUE_NOREAD;
            }
            return ret_wait;
        }
        else{
            next_display_mode = TEXT_DISPLAY_MODE;
            refreshSurface( picture_surface,    NULL, REFRESH_NORMAL_MODE );
            refreshSurface( effect_dst_surface, NULL, REFRESH_SHADOW_MODE );
            dirty_rect.add( sentence_font_info.pos );

            int ret = setEffect( window_effect.effect );
            if ( ret == RET_WAIT_NEXT ) return ret_wait;
            return ret;
        }
    }
    
    return RET_NOMATCH;
}

void ONScripterLabel::clearCurrentTextBuffer()
{
    sentence_font.clear();

    if ( current_text_buffer->buffer2 &&
         (current_text_buffer->num_xy[0] != sentence_font.num_xy[0] ||
          current_text_buffer->num_xy[1] != sentence_font.num_xy[1] ) ){
        delete[] current_text_buffer->buffer2;
        current_text_buffer->buffer2 = NULL;
    }
    if ( !current_text_buffer->buffer2 ){
        current_text_buffer->buffer2 = new char[ (sentence_font.num_xy[0]*2+1) * sentence_font.num_xy[1] + 1 ];
        current_text_buffer->num_xy[0] = sentence_font.num_xy[0];
        current_text_buffer->num_xy[1] = sentence_font.num_xy[1];
    }

    current_text_buffer->buffer2_count = 0;
    num_chars_in_sentence = 0;

    SDL_FillRect( text_surface, NULL, SDL_MapRGBA( text_surface->format, 0, 0, 0, 0 ) );
    cached_text_buffer = current_text_buffer;
}

void ONScripterLabel::shadowTextDisplay( SDL_Surface *dst_surface, SDL_Surface *src_surface, SDL_Rect *clip, FontInfo *info )
{
    if ( !info ) info = &sentence_font;
    
    if ( info->is_transparent ){
        if ( src_surface != dst_surface )
            SDL_BlitSurface( src_surface, clip, dst_surface, clip );

        SDL_Rect rect;
        if ( info == &sentence_font ){
            rect = sentence_font_info.pos;
        
            if ( clip && doClipping( &rect, clip ) ) return;
        }
        else{
            rect.x = rect.y = 0;
            rect.w = screen_width;
            rect.h = screen_height;
        }
        makeMonochromeSurface( dst_surface, &rect, info );
    }
    else if ( sentence_font_info.image_surface ){
        /* drawTaggedSurface must be used instead !! */
        sentence_font_info.blendOnSurface( dst_surface, sentence_font_info.pos.x, sentence_font_info.pos.y,
                                           src_surface, sentence_font_info.pos.x, sentence_font_info.pos.y,
                                           clip );
    }
}

void ONScripterLabel::newPage( bool next_flag )
{
    /* ---------------------------------------- */
    /* Set forward the text buffer */
    if ( next_flag ){
        if ( current_text_buffer->buffer2_count != 0 ){
            current_text_buffer = current_text_buffer->next;
            if ( start_text_buffer == current_text_buffer )
                start_text_buffer = start_text_buffer->next;
        }
    }

    internal_saveon_flag = true;
    clearCurrentTextBuffer();
    if ( need_refresh_flag ){
        refreshSurfaceParameters();
    }
    refreshSurface( picture_surface,      &sentence_font_info.pos, REFRESH_NORMAL_MODE );
    refreshSurface( accumulation_surface, &sentence_font_info.pos, REFRESH_SHADOW_MODE );

    flush();
}

struct ONScripterLabel::ButtonLink *ONScripterLabel::getSelectableSentence( char *buffer, FontInfo *info, bool flush_flag, bool nofile_flag )
{
    ButtonLink *button_link = new ButtonLink();

    int current_text_xy[2];
    current_text_xy[0] = info->xy[0];
    current_text_xy[1] = info->xy[1];
    
    /* ---------------------------------------- */
    /* Draw selected characters */
    drawString( buffer, info->on_color, info, false, accumulation_surface, &button_link->image_rect );

    button_link->select_rect = button_link->image_rect;

    button_link->selected_surface = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG, button_link->image_rect.w, button_link->image_rect.h, 32, rmask, gmask, bmask, amask );
    SDL_SetAlpha( button_link->selected_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
    SDL_BlitSurface( accumulation_surface, &button_link->image_rect, button_link->selected_surface, NULL );

    /* ---------------------------------------- */
    /* Draw shadowed characters */
    info->setXY( current_text_xy[0], current_text_xy[1] );
    if ( nofile_flag )
        drawString( buffer, info->nofile_color, info, flush_flag, accumulation_surface, NULL );
    else
        drawString( buffer, info->off_color, info, flush_flag, accumulation_surface, NULL );

    button_link->no_selected_surface = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG, button_link->image_rect.w, button_link->image_rect.h, 32, rmask, gmask, bmask, amask );
    SDL_SetAlpha( button_link->no_selected_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
    SDL_BlitSurface( accumulation_surface, &button_link->image_rect, button_link->no_selected_surface, NULL );

    info->newLine();
    if (info->getTateyokoMode() == FontInfo::YOKO_MODE)
        info->setXY( current_text_xy[0] );
    else
        info->setXY( -1, current_text_xy[1] );

    dirty_rect.add( button_link->image_rect );
    
    return button_link;
}

void ONScripterLabel::decodeExbtnControl( SDL_Surface *surface, const char *ctl_str, SDL_Rect *check_src_rect, SDL_Rect *check_dst_rect )
{
    char sound_name[256];
    int sprite_no, cell_no;

    while( char com = *ctl_str++ ){
        if ( com == 'C' ){
            sprite_no = getNumberFromBuffer( &ctl_str );
            if ( *ctl_str == ',' ){
                ctl_str++;
                cell_no = getNumberFromBuffer( &ctl_str );
            }
            else
                cell_no = -1;
            refreshSprite( surface, sprite_no, false, cell_no, NULL, NULL );
        }
        else if ( com == 'P' ){
            sprite_no = getNumberFromBuffer( &ctl_str );
            if ( *ctl_str == ',' ){
                ctl_str++;
                cell_no = getNumberFromBuffer( &ctl_str );
            }
            else
                cell_no = -1;
            refreshSprite( surface, sprite_no, true, cell_no, check_src_rect, check_dst_rect );
        }
        else if ( com == 'S' ){
            sprite_no = getNumberFromBuffer( &ctl_str );
            if ( *ctl_str != ',' ) continue;
            ctl_str++;
            if ( *ctl_str != '(' ) continue;
            ctl_str++;
            char *buf = sound_name;
            while (*ctl_str != ')' && *ctl_str != '\0' ) *buf++ = *ctl_str++;
            *buf++ = '\0';
            playWave( sound_name, false, sprite_no );
            if ( *ctl_str == ')' ) ctl_str++;
        }
        else if ( com == 'M' ){
            sprite_no = getNumberFromBuffer( &ctl_str );
            SDL_Rect rect = sprite_info[ sprite_no ].pos;
            if ( *ctl_str != ',' ) continue;
            ctl_str++; // skip ','
            sprite_info[ sprite_no ].pos.x = getNumberFromBuffer( &ctl_str ) * screen_ratio1 / screen_ratio2;
            if ( *ctl_str != ',' ) continue;
            ctl_str++; // skip ','
            sprite_info[ sprite_no ].pos.y = getNumberFromBuffer( &ctl_str ) * screen_ratio1 / screen_ratio2;
            if ( surface ){
                refreshSurface( surface, &rect );
                refreshSurface( surface, &sprite_info[ sprite_no ].pos );
            }
        }
    }
}

void ONScripterLabel::loadCursor( int no, const char *str, int x, int y, bool abs_flag )
{
    cursor_info[ no ].setImageName( str );
    cursor_info[ no ].pos.x = x;
    cursor_info[ no ].pos.y = y;

    parseTaggedString( &cursor_info[ no ] );
    setupAnimationInfo( &cursor_info[ no ] );
    if ( filelog_flag )
        script_h.findAndAddLog( ScriptHandler::FILE_LOG, cursor_info[ no ].file_name, true ); // a trick for save file
    cursor_info[ no ].abs_flag = abs_flag;
    if ( cursor_info[ no ].image_surface )
        cursor_info[ no ].visible = true;
    else
        cursor_info[ no ].remove();
}

void ONScripterLabel::saveAll()
{
    saveEnvData();
    saveGlovalData();
    if ( filelog_flag )  script_h.saveLog( ScriptHandler::FILE_LOG );
    if ( labellog_flag ) script_h.saveLog( ScriptHandler::LABEL_LOG );
    if ( kidokuskip_flag ) script_h.saveKidokuData();
}

void ONScripterLabel::loadEnvData()
{
    fullscreen_mode = false;
    volume_on_flag = true;
    text_speed_no = 1;
    draw_one_page_flag = false;
    default_env_font = NULL;
    cdaudio_on_flag = true;
    default_cdrom_drive = NULL;
    kidokumode_flag = true;
    
    FILE *fp;
    int i;
    
    if ( (fp = fopen( "envdata", "r" )) != NULL ){
        loadInt( fp, &i );
        if (i == 1) menu_fullCommand();
        loadInt( fp, &i );
        if (i == 0) volume_on_flag = false;
        loadInt( fp, &text_speed_no );
        loadInt( fp, &i );
        if (i == 1) draw_one_page_flag = true;
        loadStr( fp, &default_env_font );
        loadInt( fp, &i );
        if (i == 0) cdaudio_on_flag = false;
        loadStr( fp, &default_cdrom_drive );
        loadInt( fp, &voice_volume );
        voice_volume = DEFAULT_VOLUME - voice_volume;
        loadInt( fp, &se_volume );
        se_volume = DEFAULT_VOLUME - se_volume;
        loadInt( fp, &mp3_volume );
        mp3_volume = DEFAULT_VOLUME - mp3_volume;
        loadInt( fp, &i );
        if (i == 0) kidokumode_flag = false;
        fclose( fp );
    }
    else{
        setStr( &default_env_font, DEFAULT_ENV_FONT );
        voice_volume = se_volume = mp3_volume = DEFAULT_VOLUME;
    }
}

void ONScripterLabel::saveEnvData()
{
    FILE *fp;

    if ( (fp = fopen( "envdata", "w" )) != NULL ){
        saveInt( fp, fullscreen_mode?1:0 );
        saveInt( fp, volume_on_flag?1:0 );
        saveInt( fp, text_speed_no );
        saveInt( fp, draw_one_page_flag?1:0 );
        saveStr( fp, default_env_font );
        saveInt( fp, cdaudio_on_flag?1:0 );
        saveStr( fp, default_cdrom_drive );
        saveInt( fp, DEFAULT_VOLUME - voice_volume );
        saveInt( fp, DEFAULT_VOLUME - se_volume );
        saveInt( fp, DEFAULT_VOLUME - mp3_volume );
        saveInt( fp, kidokumode_flag?1:0 );
        saveInt( fp, 0 ); // ?
        fclose( fp );
    }
}

bool ONScripterLabel::isTextVisible()
{
    return ( next_display_mode == TEXT_DISPLAY_MODE ||
             erase_text_window_mode == 0 && text_on_flag );
}

void ONScripterLabel::quit()
{
    saveAll();
    
    if ( cdrom_info ){
        SDL_CDStop( cdrom_info );
        SDL_CDClose( cdrom_info );
    }
    if ( midi_info ){
        Mix_HaltMusic();
        Mix_FreeMusic( midi_info );
    }
#if defined(EXTERNAL_MUSIC_PLAYER)
    if ( music_info ){
        Mix_HaltMusic();
        Mix_FreeMusic( music_info );
    }
#endif
}

void ONScripterLabel::allocateSelectedSurface( int sprite_no, ButtonLink *button )
{
    AnimationInfo *sp = &sprite_info[ sprite_no ];

    if ( button->no_selected_surface &&
         ( sp->num_of_cells == 1 ||
           button->no_selected_surface->w != sp->pos.w ||
           button->no_selected_surface->h != sp->pos.h ) ){
        SDL_FreeSurface( button->no_selected_surface );
        button->no_selected_surface = NULL;
    }
    if ( !button->no_selected_surface && sp->num_of_cells != 1 ){
        button->no_selected_surface = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG,
                                                            sp->pos.w, sp->pos.h,
                                                            32, rmask, gmask, bmask, amask );
        SDL_SetAlpha( button->no_selected_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
    }

    if ( button->selected_surface &&
         ( button->selected_surface->w != sp->pos.w ||
           button->selected_surface->h != sp->pos.h ) ){
        SDL_FreeSurface( button->selected_surface );
        button->selected_surface = NULL;
    }
    if ( !button->selected_surface ){
        button->selected_surface = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG,
                                                         sp->pos.w, sp->pos.h,
                                                         32, rmask, gmask, bmask, amask );
        SDL_SetAlpha( button->selected_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
    }
    
    button->image_rect.w = sp->pos.w;
    button->image_rect.h = sp->pos.h;
}

void ONScripterLabel::disableGetButtonFlag()
{
    btndown_flag = false;

    getzxc_flag = false;
    gettab_flag = false;
    getpageup_flag = false;
    getpagedown_flag = false;
    getinsert_flag = false;
    getfunction_flag = false;
    getenter_flag = false;
    getcursor_flag = false;
    spclclk_flag = false;
}

int ONScripterLabel::getNumberFromBuffer( const char **buf )
{
    int ret = 0;
    while ( **buf >= '0' && **buf <= '9' )
        ret = ret*10 + *(*buf)++ - '0';

    return ret;
}
