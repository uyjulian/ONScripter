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
#include "resize_image.h"

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

        Mix_AllocateChannels( ONS_MIX_CHANNELS );
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

    for ( i=0 ; i<ONS_MIX_CHANNELS ; i++ ) wave_sample[i] = NULL;

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
    setStr( &bg_info.file_name, "black");
    bg_effect_image = COLOR_EFFECT_IMAGE;

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

void ONScripterLabel::blitRotation( SDL_Surface *src_surface, SDL_Rect *src_rect, SDL_Surface *dst_surface, SDL_Rect *dst_rect )
{
    SDL_Rect s_rect, d_rect;
        
    if ( src_rect ) s_rect = *src_rect;
    else{
        s_rect.x = s_rect.y = 0;
        s_rect.w = src_surface->w;
        s_rect.h = src_surface->h;
    }

    if ( dst_rect ) d_rect = *dst_rect;
    else{
        d_rect.x = d_rect.y = 0;
        d_rect.w = dst_surface->h;
        d_rect.h = dst_surface->w;
    }

    SDL_LockSurface( src_surface );
    SDL_LockSurface( dst_surface );

    Uint32 *src_buffer = (Uint32 *)src_surface->pixels;
    src_buffer += src_surface->w * s_rect.y + s_rect.x;
        
    for ( int i=0 ; i<s_rect.h ; i++ ){
        Uint16 *dst_buffer = (Uint16 *)dst_surface->pixels + dst_surface->w * d_rect.x + dst_surface->w - d_rect.y - i - 1;
        for ( int j=0 ; j<s_rect.w ; j++ ){
            *dst_buffer =  ((*src_buffer >> (src_surface->format->Rshift+3)) & 0x1f) << dst_surface->format->Rshift;
            *dst_buffer |= ((*src_buffer >> (src_surface->format->Gshift+2)) & 0x3f) << dst_surface->format->Gshift;
            *dst_buffer |= ((*src_buffer >> (src_surface->format->Bshift+3)) & 0x1f) << dst_surface->format->Bshift;

            src_buffer++;
            dst_buffer += dst_surface->w;
        }
        src_buffer += src_surface->w - s_rect.w;
    }
        
    SDL_UnlockSurface( dst_surface );
    SDL_UnlockSurface( src_surface );
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
        if ( event_mode & WAIT_BUTTON_MODE && current_over_button != 0 ){
            if ( current_button_link->no_selected_surface ){
                SDL_BlitSurface( current_button_link->no_selected_surface, NULL, accumulation_surface, &current_button_link->image_rect );
            }
            if ( current_button_link->button_type == ButtonLink::SPRITE_BUTTON || 
                 current_button_link->button_type == ButtonLink::EX_SPRITE_BUTTON ){
                sprite_info[ current_button_link->sprite_no ].visible = true;
                sprite_info[ current_button_link->sprite_no ].current_cell = 0;
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
                }
                if ( p_button_link->button_type == ButtonLink::SPRITE_BUTTON || 
                     p_button_link->button_type == ButtonLink::EX_SPRITE_BUTTON ){
                    if ( sprite_info[ p_button_link->sprite_no ].num_of_cells >= 1 )
                        sprite_info[ p_button_link->sprite_no ].current_cell = 1;
                    if ( p_button_link->button_type == ButtonLink::EX_SPRITE_BUTTON ){
                        decodeExbtnControl( accumulation_surface, p_button_link->exbtn_ctl );
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
                decodeExbtnControl( accumulation_surface, exbtn_d_button_link.exbtn_ctl  );
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
            if ( sentence_font.xy[0] < sentence_font.num_xy[0] ) // otherwise already done in drawChar
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

int ONScripterLabel::resizeSurface( SDL_Surface *src, SDL_Rect *src_rect, SDL_Surface *dst, SDL_Rect *dst_rect )
{
    SDL_Rect src_rect2, dst_rect2;

    if ( src_rect ){
        src_rect2 = *src_rect;
    }
    else{
        src_rect2.x = src_rect2.y = 0;
        src_rect2.w = src->w;
        src_rect2.h = src->h;
    }
    
    if ( dst_rect ){
        dst_rect2 = *dst_rect;
    }
    else{
        dst_rect2.x = dst_rect2.y = 0;
        dst_rect2.w = dst->w;
        dst_rect2.h = dst->h;
    }
    
    SDL_LockSurface( dst );
    SDL_LockSurface( src );
    Uint32 *src_buffer = (Uint32 *)src->pixels + src->w * src_rect2.y + src_rect2.x;
    Uint32 *dst_buffer = (Uint32 *)dst->pixels + dst->w * dst_rect2.y + dst_rect2.x;

    /* size of tmp_buffer must be larger than 16 bytes */
    size_t len = src_rect2.w * (src_rect2.h+1) * 4 + 4;
    unsigned char *tmp_buffer = new unsigned char[ (len<16)?16:len ];
    resizeImage( (unsigned char*)dst_buffer, dst_rect2.w, dst_rect2.h, dst->w * 4,
                 (unsigned char*)src_buffer, src_rect2.w, src_rect2.h, src->w * 4,
                 4, tmp_buffer, src_rect2.w * 4 );
    delete[] tmp_buffer;

    SDL_UnlockSurface( src );
    SDL_UnlockSurface( dst );

    return 0;
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

int ONScripterLabel::doClipping( SDL_Rect *dst, SDL_Rect *clip, SDL_Rect *clipped )
{
    if ( clipped ) clipped->x = clipped->y = 0;

    if ( dst->x >= clip->x + clip->w || dst->x + dst->w <= clip->x ||
         dst->y >= clip->y + clip->h || dst->y + dst->h <= clip->y )
        return -1;
    
    if ( dst->x < clip->x ){
        dst->w -= clip->x - dst->x;
        if ( clipped ) clipped->x = clip->x - dst->x;
        dst->x = clip->x;
    }
    if ( clip->x + clip->w < dst->x + dst->w ){
        dst->w = clip->x + clip->w - dst->x;
    }
    
    if ( dst->y < clip->y ){
        dst->h -= clip->y - dst->y;
        if ( clipped ) clipped->y = clip->y - dst->y;
        dst->y = clip->y;
    }
    if ( clip->y + clip->h < dst->y + dst->h ){
        dst->h = clip->y + clip->h - dst->y;
    }
    if ( clipped ){
        clipped->w = dst->w;
        clipped->h = dst->h;
    }

    return 0;
}

int ONScripterLabel::shiftRect( SDL_Rect &dst, SDL_Rect &clip )
{
    dst.x += clip.x;
    dst.y += clip.y;
    dst.w = clip.w;
    dst.h = clip.h;

    return 0;
}

void ONScripterLabel::alphaBlend( SDL_Surface *dst_surface, SDL_Rect dst_rect,
                                  SDL_Surface *src1_surface, int x1, int y1,
                                  SDL_Surface *src2_surface, int x2, int y2,
                                  SDL_Surface *mask_surface, int x3,
                                  int trans_mode, Uint32 mask_value, SDL_Rect *clip, uchar3 *direct_color )
{
    int i, j;
    SDL_Rect src1_rect, src2_rect, clip_rect, clipped_rect;
    Uint32 mask, mask1, mask2, maskrb;
    Uint32 *src1_buffer, *src2_buffer, *dst_buffer;

    /* ---------------------------------------- */
    /* 1st clipping */
    if ( clip ){
        if ( doClipping( &dst_rect, clip, &clipped_rect ) ) return;

        x1 += clipped_rect.x;
        x2 += clipped_rect.x;
        x3 += clipped_rect.x;

        y1 += clipped_rect.y;
        y2 += clipped_rect.y;
    }

    /* ---------------------------------------- */
    /* 2nd clipping */
    clip_rect.x = 0;
    clip_rect.y = 0;
    clip_rect.w = dst_surface->w;
    clip_rect.h = dst_surface->h;

    if ( doClipping( &dst_rect, &clip_rect, &clipped_rect ) ) return;
    
    x1 += clipped_rect.x;
    x2 += clipped_rect.x;
    x3 += clipped_rect.x;

    y1 += clipped_rect.y;
    y2 += clipped_rect.y;

    /* ---------------------------------------- */
    src1_rect.x = x1;
    src1_rect.y = y1;
    src2_rect.x = x2;
    src2_rect.y = y2;
    src1_rect.w = src2_rect.w = dst_rect.w;
    src1_rect.h = src2_rect.h = dst_rect.h;

    SDL_LockSurface( src1_surface );
    SDL_LockSurface( src2_surface );
    if ( dst_surface != src1_surface ) SDL_LockSurface( dst_surface );
    
    src1_buffer = (Uint32 *)src1_surface->pixels + src1_surface->w * y1 + x1;
    src2_buffer = (Uint32 *)src2_surface->pixels + src2_surface->w * y2 + x2;
    dst_buffer  = (Uint32 *)dst_surface->pixels  + dst_surface->w * dst_rect.y + dst_rect.x;
    
    if ( mask_surface && src2_surface != mask_surface ){
        SDL_LockSurface( mask_surface );
    }
    else{
        mask_surface = src2_surface;
    }

    if ( trans_mode == AnimationInfo::TRANS_ALPHA ||
         trans_mode == AnimationInfo::TRANS_MASK ){

        for ( i=0; i<dst_rect.h ; i++ ) {
            for ( j=0 ; j<dst_rect.w ; j++, src1_buffer++, src2_buffer++, dst_buffer++ ){
                mask = ~*((Uint32 *)mask_surface->pixels + mask_surface->w * ((y2+i)%mask_surface->h) + (x3+j)%mask_surface->w) & 0xff;
                mask2 = (mask * (mask_value+1)) >> 8;
                mask1 = ~mask2 & 0xff;

                maskrb = (((*src1_buffer & 0xff00ff) * mask1 +
                           (*src2_buffer & 0xff00ff) * mask2) >> 8) & 0xff00ff; // red and blue pixel
                mask = (((*src1_buffer & 0x00ff00) * mask1 +
                         (*src2_buffer & 0x00ff00) * mask2) >> 8) & 0x00ff00; // green pixel
                *dst_buffer = maskrb | mask;
            }
            src1_buffer += src1_surface->w - dst_rect.w;
            src2_buffer += src2_surface->w - dst_rect.w;
            dst_buffer  += dst_surface->w  - dst_rect.w;
        }
    }
    else if ( trans_mode == AnimationInfo::TRANS_ALPHA_PRESERVE ){

        for ( i=0; i<dst_rect.h ; i++ ) {
            for ( j=0 ; j<dst_rect.w ; j++, src1_buffer++, src2_buffer++, dst_buffer++ ){
                mask = *((Uint32 *)mask_surface->pixels + mask_surface->w * ((y2+i)%mask_surface->h) + (x3+j)%mask_surface->w) & 0xff000000;
                mask2 = mask >> 24;
                mask1 = ~mask2 & 0xff;

                maskrb = (((*src1_buffer & 0xff00ff) * mask1 +
                           (*src2_buffer & 0xff00ff) * mask2) >> 8) & 0xff00ff; // red and blue pixel
                mask |= (((*src1_buffer & 0x00ff00) * mask1 +
                          (*src2_buffer & 0x00ff00) * mask2) >> 8) & 0x00ff00; // green pixel
                *dst_buffer = maskrb | mask;
            }
            src1_buffer += src1_surface->w - dst_rect.w;
            src2_buffer += src2_surface->w - dst_rect.w;
            dst_buffer  += dst_surface->w  - dst_rect.w;
        }
    }
    else if ( trans_mode == AnimationInfo::TRANS_ALPHA_MULTIPLE ){

        Uint32 an_1, an;
        for ( i=0; i<dst_rect.h ; i++ ) {
            for ( j=0 ; j<dst_rect.w ; j++, src1_buffer++, src2_buffer++, dst_buffer++ ){
                mask = *((Uint32 *)mask_surface->pixels + mask_surface->w * ((y2+i)%mask_surface->h) + (x3+j)%mask_surface->w) & 0xff000000;
                mask2 = mask >> 24;
                mask1 = ~mask2 & 0xff;

                an_1 = *src1_buffer >> 24;
                an = 0xff-((0xff-an_1)*(0xff-mask2) >> 8);

                if ( an != 0 ){
                    maskrb = (((((*src1_buffer & 0xff00ff) * mask1) >> 8) * an_1 +
                               (*src2_buffer & 0xff00ff) * mask2) / an ) & 0xff00ff; // red and blue pixel
                    mask = (((((*src1_buffer & 0x00ff00) * mask1) >> 8) * an_1 +
                             (*src2_buffer & 0x00ff00) * mask2) / an ) & 0x00ff00; // green pixel

                    *dst_buffer = (an << 24) | maskrb | mask;
                }
                else
                    *dst_buffer = (an << 24);
            }
            src1_buffer += src1_surface->w - dst_rect.w;
            src2_buffer += src2_surface->w - dst_rect.w;
            dst_buffer  += dst_surface->w  - dst_rect.w;
        }
    }
    else if ( trans_mode == AnimationInfo::TRANS_TOPLEFT ||
              trans_mode == AnimationInfo::TRANS_TOPRIGHT ||
              trans_mode == AnimationInfo::TRANS_DIRECT ){

        Uint32 ref=0;
        if ( trans_mode == AnimationInfo::TRANS_TOPLEFT ){
            ref = *((Uint32*)src2_surface->pixels);
        }
        else if ( trans_mode == AnimationInfo::TRANS_TOPRIGHT ){
            ref = *((Uint32*)src2_surface->pixels + src2_surface->w - 1);
        }
        else if ( direct_color ) {
            ref = (*direct_color)[0] << src2_surface->format->Rshift |
                (*direct_color)[1] << src2_surface->format->Gshift |
                (*direct_color)[2] << src2_surface->format->Bshift;
        }
        ref &= 0xffffff;
        
        for ( i=0; i<dst_rect.h ; i++ ) {
            for ( j=0 ; j<dst_rect.w ; j++, src1_buffer++, src2_buffer++, dst_buffer++ ){
                if ( (*src2_buffer & 0xffffff) == ref )
                    mask2 = 0;
                else
                    mask2 = mask_value;
                mask1 = ~mask2 & 0xff;

                maskrb = (((*src1_buffer & 0xff00ff) * mask1 +
                           (*src2_buffer & 0xff00ff) * mask2) >> 8) & 0xff00ff; // red and blue pixel
                mask = (((*src1_buffer & 0x00ff00) * mask1 +
                         (*src2_buffer & 0x00ff00) * mask2) >> 8) & 0x00ff00; // green pixel
                *dst_buffer = maskrb | mask;
            }
            src1_buffer += src1_surface->w - dst_rect.w;
            src2_buffer += src2_surface->w - dst_rect.w;
            dst_buffer  += dst_surface->w  - dst_rect.w;
        }
    }
    else if ( trans_mode == AnimationInfo::TRANS_FADE_MASK ||
              trans_mode == AnimationInfo::TRANS_CROSSFADE_MASK )
    {
        Uint32 overflow_mask;
        if ( trans_mode == AnimationInfo::TRANS_FADE_MASK )
            overflow_mask = 0xffffffff;
        else
            overflow_mask = 0xffffff00;
        
        for ( i=0; i<dst_rect.h ; i++ ){
            for ( j=0 ; j<dst_rect.w ; j++, src1_buffer++, src2_buffer++, dst_buffer++ ){
                mask = *((Uint32 *)mask_surface->pixels + mask_surface->w * ((y2+i)%mask_surface->h) + (x3+j)%mask_surface->w) & 0xff;
                if ( mask_value > mask ){
                    mask2 = mask_value - mask;
                    if ( mask2 & overflow_mask ) mask2 = 0xff;
                }
                else{
                    mask2 = 0;
                }
                mask1 = ~mask2 & 0xff;

                maskrb = (((*src1_buffer & 0xff00ff) * mask1 +
                           (*src2_buffer & 0xff00ff) * mask2) >> 8) & 0xff00ff; // red and blue pixel
                mask = (((*src1_buffer & 0x00ff00) * mask1 +
                         (*src2_buffer & 0x00ff00) * mask2) >> 8) & 0x00ff00; // green pixel
                *dst_buffer = maskrb | mask;
            }
            src1_buffer += src1_surface->w - dst_rect.w;
            src2_buffer += src2_surface->w - dst_rect.w;
            dst_buffer  += dst_surface->w  - dst_rect.w;
        }
    }
    else if ( trans_mode == AnimationInfo::TRANS_COPY ){
        mask2 = mask_value;
        mask1 = ~mask2 & 0xff;

        for ( i=0; i<dst_rect.h ; i++ ) {
            for ( j=0 ; j<dst_rect.w ; j++, src1_buffer++, src2_buffer++, dst_buffer++ )
            {
                maskrb = (((*src1_buffer & 0xff00ff) * mask1 +
                           (*src2_buffer & 0xff00ff) * mask2) >> 8) & 0xff00ff; // red and blue pixel
                mask = (((*src1_buffer & 0x00ff00) * mask1 +
                         (*src2_buffer & 0x00ff00) * mask2) >> 8) & 0x00ff00; // green pixel
                *dst_buffer = maskrb | mask;
            }
            src1_buffer += src1_surface->w - dst_rect.w;
            src2_buffer += src2_surface->w - dst_rect.w;
            dst_buffer  += dst_surface->w  - dst_rect.w;
        }
    }
    
    if ( mask_surface && src2_surface != mask_surface ) SDL_UnlockSurface( mask_surface );
    if ( dst_surface != src1_surface ) SDL_UnlockSurface( dst_surface );
    SDL_UnlockSurface( src2_surface );
    SDL_UnlockSurface( src1_surface );
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
    
    SDL_Rect rect;
    if ( info->is_transparent ){
        if ( src_surface != dst_surface )
            SDL_BlitSurface( src_surface, clip, dst_surface, clip );

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
        alphaBlend( dst_surface, sentence_font_info.pos,
                    src_surface, sentence_font_info.pos.x, sentence_font_info.pos.y,
                    sentence_font_info.image_surface, 0, 0,
                    sentence_font_info.mask_surface, sentence_font_info.alpha_offset,
                    sentence_font_info.trans_mode, 255, clip, &sentence_font_info.direct_color );
        rect = sentence_font_info.pos;
        if ( clip ) doClipping( &rect, clip );
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
    info->setXY( current_text_xy[0] );

    dirty_rect.add( button_link->image_rect );
    
    return button_link;
}

void ONScripterLabel::makeNegaSurface( SDL_Surface *surface, SDL_Rect *dst_rect )
{
    int i, j;
    SDL_Rect rect;
    Uint32 *buf, cr, cg, cb;

    if ( dst_rect ){
        rect.x = dst_rect->x;
        rect.y = dst_rect->y;
        rect.w = dst_rect->w;
        rect.h = dst_rect->h;
        if ( rect.x + rect.w > surface->w ) rect.w = surface->w - rect.x;
        if ( rect.y + rect.h > surface->h ) rect.h = surface->h - rect.y;
    }
    else{
        rect.x = rect.y = 0;
        rect.w = surface->w;
        rect.h = surface->h;
    }

    SDL_LockSurface( surface );
    buf = (Uint32 *)surface->pixels + rect.y * surface->w + rect.x;
    
    for ( i=rect.y ; i<rect.y + rect.h ; i++ ){
        for ( j=rect.x ; j<rect.x + rect.w ; j++, buf++ ){
            cr = ((*buf >> surface->format->Rshift) & 0xff) ^ 0xff;
            cg = ((*buf >> surface->format->Gshift) & 0xff) ^ 0xff;
            cb = ((*buf >> surface->format->Bshift) & 0xff) ^ 0xff;
            *buf = cr << surface->format->Rshift |
                   cg << surface->format->Gshift |
                   cb << surface->format->Bshift;
        }
        buf += surface->w - rect.w;
    }

    SDL_UnlockSurface( surface );
}

void ONScripterLabel::makeMonochromeSurface( SDL_Surface *surface, SDL_Rect *dst_rect, FontInfo *info )
{
    int i, j;
    SDL_Rect rect;
    Uint32 *buf, c;

    if ( dst_rect ){
        rect.x = dst_rect->x;
        rect.y = dst_rect->y;
        rect.w = dst_rect->w;
        rect.h = dst_rect->h;
        if ( rect.x + rect.w > surface->w ) rect.w = surface->w - rect.x;
        if ( rect.y + rect.h > surface->h ) rect.h = surface->h - rect.y;
    }
    else{
        rect.x = rect.y = 0;
        rect.w = surface->w;
        rect.h = surface->h;
    }

    SDL_LockSurface( surface );
    buf = (Uint32 *)surface->pixels + rect.y * surface->w + rect.x;
    
    for ( i=rect.y ; i<rect.y + rect.h ; i++ ){
        for ( j=rect.x ; j<rect.x + rect.w ; j++, buf++ ){
            if ( !info ){
                c = (((*buf >> surface->format->Rshift) & 0xff) * 77 +
                     ((*buf >> surface->format->Gshift) & 0xff) * 151 +
                     ((*buf >> surface->format->Bshift) & 0xff) * 28 ) >> 8; 
                *buf = monocro_color_lut[c][0] << surface->format->Rshift |
                    monocro_color_lut[c][1] << surface->format->Gshift |
                    monocro_color_lut[c][2] << surface->format->Bshift;
            }
            else{
                c = ((((*buf >> surface->format->Rshift) & 0xff) * info->window_color[0] >> 8) << surface->format->Rshift |
                     (((*buf >> surface->format->Gshift) & 0xff) * info->window_color[1] >> 8) << surface->format->Gshift |
                     (((*buf >> surface->format->Bshift) & 0xff) * info->window_color[2] >> 8) << surface->format->Bshift );
                *buf = c;
            }
        }
        buf += surface->w - rect.w;
    }

    SDL_UnlockSurface( surface );
}

void ONScripterLabel::refreshSurfaceParameters()
{
    int i;
    
    monocro_flag = monocro_flag_new;
    for ( i=0 ; i<3 ; i++ )
        monocro_color[i] = monocro_color_new[i];
    for ( i=0 ; i<256 ; i++ ){
        monocro_color_lut[i][0] = (monocro_color[0] * i) >> 8;
        monocro_color_lut[i][1] = (monocro_color[1] * i) >> 8;
        monocro_color_lut[i][2] = (monocro_color[2] * i) >> 8;
    }
    need_refresh_flag = false;
}

void ONScripterLabel::refreshSurface( SDL_Surface *surface, SDL_Rect *clip, int refresh_mode )
{
    int i, top;
    
    setBackground( surface, clip );
    
    if ( !all_sprite_hide_flag ){
        if ( z_order < 10 && refresh_mode & REFRESH_SAYA_MODE )
            top = 9;
        else
            top = z_order;
        for ( i=MAX_SPRITE_NUM-1 ; i>top ; i-- ){
            if ( sprite_info[i].image_name && sprite_info[i].visible ){
                drawTaggedSurface( surface, &sprite_info[i], clip );
            }
        }
    }

    for ( i=0 ; i<3 ; i++ ){
        if ( tachi_info[human_order[2-i]].image_surface ){
            drawTaggedSurface( surface, &tachi_info[human_order[2-i]], clip );
        }
    }

    if ( refresh_mode & REFRESH_SHADOW_MODE && windowback_flag ){
        shadowTextDisplay( surface, surface, clip );
        restoreTextBuffer( surface, clip, true );
    }

    if ( !all_sprite_hide_flag ){
        if ( refresh_mode & REFRESH_SAYA_MODE )
            top = 10;
        else
            top = 0;
        for ( i=z_order ; i>=top ; i-- ){
            if ( sprite_info[i].image_name && sprite_info[i].visible ){
                drawTaggedSurface( surface, &sprite_info[i], clip );
            }
        }
    }

    if ( !( refresh_mode & REFRESH_SAYA_MODE ) ){
        for ( i=0 ; i<MAX_PARAM_NUM ; i++ ){
            if ( bar_info[i] ) {
                drawTaggedSurface( surface, bar_info[i], clip );
            }
        }
        for ( i=0 ; i<MAX_PARAM_NUM ; i++ ){
            if ( prnum_info[i] ){
                drawTaggedSurface( surface, prnum_info[i], clip );
            }
        }
    }

    if ( refresh_mode & REFRESH_SHADOW_MODE && !windowback_flag ){
        shadowTextDisplay( surface, surface, clip );
        restoreTextBuffer( surface, clip, true );
    }
    
    if ( nega_mode == 1 ) makeNegaSurface( surface, clip );
    if ( monocro_flag ) makeMonochromeSurface( surface, clip );
    if ( nega_mode == 2 ) makeNegaSurface( surface, clip );

    if ( refresh_mode & REFRESH_CURSOR_MODE && !textgosub_label ){
        if ( clickstr_state == CLICK_WAIT )
            drawTaggedSurface( surface, &cursor_info[CURSOR_WAIT_NO], clip );
        else if ( clickstr_state == CLICK_NEWPAGE )
            drawTaggedSurface( surface, &cursor_info[CURSOR_NEWPAGE_NO], clip );
    }

    if ( surface == accumulation_surface ){
        if ( clip ) dirty_rect.add( *clip );
        else        dirty_rect.fill( screen_width, screen_height );
    }
}

void ONScripterLabel::refreshSprite( SDL_Surface *surface, int sprite_no, bool active_flag, int cell_no )
{
    if ( sprite_info[sprite_no].image_name && 
         ( sprite_info[ sprite_no ].visible != active_flag ||
           (cell_no >= 0 && sprite_info[ sprite_no ].current_cell != cell_no ) ) )
    {
        if ( cell_no >= 0 )
            sprite_info[ sprite_no ].current_cell = cell_no;

        sprite_info[ sprite_no ].visible = active_flag;

        if ( surface )
            refreshSurface( surface, &sprite_info[ sprite_no ].pos );
    }
}

void ONScripterLabel::decodeExbtnControl( SDL_Surface *surface, const char *ctl_str )
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
            refreshSprite( surface, sprite_no, false, cell_no );
        }
        else if ( com == 'P' ){
            sprite_no = getNumberFromBuffer( &ctl_str );
            if ( *ctl_str == ',' ){
                ctl_str++;
                cell_no = getNumberFromBuffer( &ctl_str );
            }
            else
                cell_no = -1;
            refreshSprite( surface, sprite_no, true, cell_no );
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

void ONScripterLabel::setBackground( SDL_Surface *surface, SDL_Rect *clip )
{
    if ( !bg_info.image_surface ||
         bg_info.image_surface->w != surface->w ||
         bg_info.image_surface->h != surface->h ){
        if ( clip ){
            SDL_Rect rect = *clip;
            SDL_FillRect( surface, &rect, SDL_MapRGB( surface->format, bg_info.color[0], bg_info.color[1], bg_info.color[2]) );
        }
        else{
            SDL_FillRect( surface, NULL, SDL_MapRGB( surface->format, bg_info.color[0], bg_info.color[1], bg_info.color[2]) );
        }
    }

    if ( bg_info.image_surface ){
        SDL_Rect src_rect, dst_rect;
        src_rect.x = 0;
        src_rect.y = 0;
        src_rect.w = dst_rect.w = bg_info.image_surface->w;
        src_rect.h = dst_rect.h = bg_info.image_surface->h;
        dst_rect.x = (screen_width - bg_info.image_surface->w) / 2;
        dst_rect.y = (screen_height - bg_info.image_surface->h) / 2;

        if ( clip ){
            SDL_Rect clipped_rect;
            if ( doClipping( &dst_rect, clip, &clipped_rect ) )
                return;
            else if ( shiftRect( src_rect, clipped_rect ) )
                return;
        }

        SDL_BlitSurface( bg_info.image_surface, &src_rect, surface, &dst_rect );
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
    
    if ( sp->no_selected_surface &&
         ( sp->no_selected_surface->w != sp->pos.w ||
           sp->no_selected_surface->h != sp->pos.h ) ){
        SDL_FreeSurface( sp->no_selected_surface );
        sp->no_selected_surface = NULL;
    }
    if ( !sp->no_selected_surface ){
        sp->no_selected_surface = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG,
                                                        sp->pos.w, sp->pos.h,
                                                        32, rmask, gmask, bmask, amask );
        SDL_SetAlpha( sp->no_selected_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
    }
    button->no_selected_surface = sp->no_selected_surface;

    if ( sp->selected_surface &&
         ( sp->selected_surface->w != sp->pos.w ||
           sp->selected_surface->h != sp->pos.h ) ){
        SDL_FreeSurface( sp->selected_surface );
        sp->selected_surface = NULL;
    }
    if ( !sp->selected_surface && sp->num_of_cells >= 2 ){
        sp->selected_surface = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG,
                                                     sp->pos.w, sp->pos.h,
                                                     32, rmask, gmask, bmask, amask );
        SDL_SetAlpha( sp->selected_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
    }
    button->selected_surface = sp->selected_surface;
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
