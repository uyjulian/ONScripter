/* -*- C++ -*-
 * 
 *  ONScripterLabel.h - Execution block parser of ONScripter
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

#ifndef __ONSCRIPTER_LABEL_H__
#define __ONSCRIPTER_LABEL_H__

#include "ScriptParser.h"
#include "DirtyRect.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#if defined(MP3_MAD)
#include "MadWrapper.h"
#else
#include <smpeg.h>
#endif

#define DEFAULT_SURFACE_FLAG (SDL_SWSURFACE)
//#define DEFAULT_SURFACE_FLAG (SDL_HWSURFACE)

#define DEFAULT_BLIT_FLAG (0)
//#define DEFAULT_BLIT_FLAG (SDL_RLEACCEL)

#define MAX_SPRITE_NUM 1000
#define MAX_PARAM_NUM 100
#define CUSTOM_EFFECT_NO 100
#define ONS_MIX_CHANNELS 50

#define DEFAULT_WM_TITLE "ONScripter"
#define DEFAULT_WM_ICON  "ONScripter"

#define DEFAULT_WAVE_CHANNEL 1

#define DEFAULT_FONT_SIZE 26

//#define USE_OVERLAY

class ONScripterLabel : public ScriptParser
{
public:
    ONScripterLabel( bool cdaudio_flag, char *default_font, char *default_registry, char *default_dll, char *default_archive_path, bool force_button_shortcut_flag, bool disable_rescale_flag, bool edit_flag );
    ~ONScripterLabel();

    bool skip_flag;
    bool draw_one_page_flag;
    bool key_pressed_flag;
    int shift_pressed_status;
    int ctrl_pressed_status;
    
    int  eventLoop();

    /* ---------------------------------------- */
    /* Commands */
    int wavestopCommand();
    int waveCommand();
    int waittimerCommand();
    int waitCommand();
    int vspCommand();
    int voicevolCommand();
    int vCommand();
    int trapCommand();
    int textspeedCommand();
    int textonCommand();
    int textoffCommand();
    int textclearCommand();
    int texecCommand();
    int tateyokoCommand();
    int talCommand();
    int tablegotoCommand();
    int systemcallCommand();
    int stopCommand();
    int spstrCommand();
    int splitstringCommand();
    int spclclkCommand();
    int spbtnCommand();
    int skipoffCommand();
    int sevolCommand();
    int setwindow2Command();
    int setwindowCommand();
    int setcursorCommand();
    int selectCommand();
    int savetimeCommand();
    int saveonCommand();
    int saveoffCommand();
    int savegameCommand();
    int savefileexistCommand();
    int savescreenshotCommand();
    int resettimerCommand();
    int resetCommand();
    int repaintCommand();
    int rndCommand();
    int rmodeCommand();
    int quakeCommand();
    int puttextCommand();
    int prnumclearCommand();
    int prnumCommand();
    int printCommand();
    int playstopCommand();
    int playonceCommand();
    int playCommand();
    int ofscpyCommand();
    int negaCommand();
    int mspCommand();
    int mpegplayCommand();
    int mp3volCommand();
    int mp3Command();
    int movemousecursorCommand();
    int monocroCommand();
    int menu_windowCommand();
    int menu_fullCommand();
    int menu_automodeCommand();
    int lspCommand();
    int lookbackspCommand();
    int lookbackflushCommand();
    int lookbackbuttonCommand();
    int locateCommand();
    int loadgameCommand();
    int ldCommand();
    int jumpfCommand();
    int jumpbCommand();
    int ispageCommand();
    int isfullCommand();
    int isskipCommand();
    int isdownCommand();
    int inputCommand();
    int getzxcCommand();
    int getversionCommand();
    int gettimerCommand();
    int gettextCommand();
    int gettabCommand();
    int getscreenshotCommand();
    int getretCommand();
    int getregCommand();
    int getpageupCommand();
    int getpageCommand();
    int getmouseposCommand();
    int getinsertCommand();
    int getfunctionCommand();
    int getenterCommand();
    int getcursorposCommand();
    int getcursorCommand();
    int getcselnumCommand();
    int gameCommand();
    int fileexistCommand();
    int exec_dllCommand();
    int exbtnCommand();
    int erasetextwindowCommand();
    int endCommand();
    int dwavestopCommand();
    int dwaveCommand();
    int dvCommand();
    int delayCommand();
    int cspCommand();
    int cselgotoCommand();
    int cselbtnCommand();
    int clickCommand();
    int clCommand();
    int cellCommand();
    int captionCommand();
    int btnwait2Command();
    int btnwaitCommand();
    int btntime2Command();
    int btntimeCommand();
    int btndownCommand();
    int btndefCommand();
    int btnCommand();
    int brCommand();
    int bltCommand();
    int bgCommand();
    int barclearCommand();
    int barCommand();
    int aviCommand();
    int automode_timeCommand();
    int autoclickCommand();
    int allspresumeCommand();
    int allsphideCommand();
    int amspCommand();
    
protected:
    /* ---------------------------------------- */
    /* Event related variables */
    enum { NOT_EDIT_MODE            = 0,
           EDIT_SELECT_MODE         = 1,
           EDIT_VARIABLE_INDEX_MODE = 2,
           EDIT_VARIABLE_NUM_MODE   = 3,
           EDIT_MP3_VOLUME_MODE     = 4,
           EDIT_VOICE_VOLUME_MODE   = 5,
           EDIT_SE_VOLUME_MODE      = 6
    };
    
    int variable_edit_mode;
    int variable_edit_index;
    int variable_edit_num;
    int variable_edit_sign;
    
    void variableEditMode( SDL_KeyboardEvent *event );
    void keyDownEvent( SDL_KeyboardEvent *event );
    void keyUpEvent( SDL_KeyboardEvent *event );
    void keyPressEvent( SDL_KeyboardEvent *event );
    void mousePressEvent( SDL_MouseButtonEvent *event );
    void mouseMoveEvent( SDL_MouseMotionEvent *event );
    void timerEvent();
    void flushEventSub( SDL_Event &event );
    void flushEvent();
    void startTimer( int count );
    void advancePhase( int count=0 );
    void trapHandler();
    void initSDL( bool cdaudio_flag );
    void openAudio();
    
private:
    enum { NORMAL_DISPLAY_MODE = 0, TEXT_DISPLAY_MODE = 1 };
    enum { IDLE_EVENT_MODE      = 0,
           EFFECT_EVENT_MODE    = 1,
           WAIT_BUTTON_MODE     = 2, // For select and btnwait.
           WAIT_INPUT_MODE      = (4|8),  // For select and text wait. It allows the right click menu.
           WAIT_SLEEP_MODE      = 16,
           WAIT_TIMER_MODE      = 32,
           WAIT_TEXTBTN_MODE    = 64,
           WAIT_VOICE_MODE      = 128
    };
    typedef enum { COLOR_EFFECT_IMAGE  = 0,
                   DIRECT_EFFECT_IMAGE = 1,
                   BG_EFFECT_IMAGE     = 2,
                   TACHI_EFFECT_IMAGE  = 3
    } EFFECT_IMAGE;

    /* ---------------------------------------- */
    /* Global definitions */
    bool edit_flag;
    bool disable_rescale_flag;
    enum { MOUSE_ROTATION_NONE    = 0,
           MOUSE_ROTATION_PDA     = 1,
           MOUSE_ROTATION_PDA_VGA = 2
    };
    int mouse_rotation_mode;
    
    long internal_timer;
    bool automode_flag;
    long automode_time;
    long autoclick_time;
    long remaining_time;

    FILE *tmp_save_fp;
    bool saveon_flag;
    bool internal_saveon_flag;
    bool shelter_soveon_flag; // used by csel
    int yesno_caller;
    int yesno_selected_file_no;

    bool monocro_flag, monocro_flag_new;
    uchar3 monocro_color, monocro_color_new;
    uchar3 monocro_color_lut[256];
    bool need_refresh_flag;
    int  nega_mode;

    bool trap_flag;
    char *trap_dist;
    char *wm_title_string;
    char *wm_icon_string;
    char wm_edit_string[256];
    bool fullscreen_mode;

    Uint32 rmask, gmask, bmask, amask;

    bool btntime2_flag;
    long btntime_value;
    long internal_button_timer;
    long btnwait_time;
    bool btndown_flag;

    void quit();

    /* ---------------------------------------- */
    /* Script related variables */
    enum { REFRESH_NORMAL_MODE = 0,
           REFRESH_SAYA_MODE   = 1,
           REFRESH_SHADOW_MODE = 2,
           REFRESH_CURSOR_MODE = 4
    };
    
    int display_mode, next_display_mode;
    int event_mode;
    SDL_Surface *accumulation_surface; // Text window + Sprite + Tachi image + background
    SDL_Surface *text_surface; // Text + Select_image + Tachi image + background
    SDL_Surface *screen_surface; // Text + Select_image + Tachi image + background
    SDL_Surface *effect_dst_surface; // Intermediate source buffer for effect
    SDL_Surface *effect_src_surface; // Intermediate destnation buffer for effect
    SDL_Surface *shelter_text_surface; // Intermediate buffer to store text_surface when entering system menu
    SDL_Surface *screenshot_surface; // Screenshot
#if defined(USE_OVERLAY)    
    SDL_Overlay *screen_overlay;
#endif
    
    /* ---------------------------------------- */
    /* Button related variables */
    AnimationInfo btndef_info;

    struct ButtonState{
        int x, y, button;
        bool down_flag;
    } current_button_state, volatile_button_state, last_mouse_state, shelter_mouse_state;

    struct ButtonLink{
        typedef enum { NORMAL_BUTTON        = 0,
                       SPRITE_BUTTON        = 1,
                       EX_SPRITE_BUTTON     = 2,
                       CUSTOM_SELECT_BUTTON = 3
        } BUTTON_TYPE;

        struct ButtonLink *next;
        BUTTON_TYPE button_type;
        int no;
        int sprite_no;
        char *exbtn_ctl;
        SDL_Rect select_rect;
        SDL_Rect image_rect;
        SDL_Surface *selected_surface;
        SDL_Surface *no_selected_surface;

        ButtonLink(){
            button_type = NORMAL_BUTTON;
            next = NULL;
            exbtn_ctl = NULL;
            selected_surface = NULL;
            no_selected_surface = NULL;
        };
        ~ButtonLink(){
            if ( button_type != SPRITE_BUTTON &&
                 button_type != EX_SPRITE_BUTTON &&
                 selected_surface )
                SDL_FreeSurface( selected_surface );
            if ( button_type != SPRITE_BUTTON &&
                 button_type != EX_SPRITE_BUTTON &&
                 no_selected_surface )
                SDL_FreeSurface( no_selected_surface );
            if ( exbtn_ctl ) delete[] exbtn_ctl;
        };
        void insert( ButtonLink *button ){
            button->next = this->next;
            this->next = button;
        };
    } root_button_link, *current_button_link, *shelter_button_link, exbtn_d_button_link;

    int current_over_button;

    bool getzxc_flag;
    bool gettab_flag;
    bool getpageup_flag;
    bool getpagedown_flag;
    bool getinsert_flag;
    bool getfunction_flag;
    bool getenter_flag;
    bool getcursor_flag;
    bool spclclk_flag;

    void resetSentenceFont();
    void deleteButtonLink();
    void refreshMouseOverButton();
    void refreshSprite( SDL_Surface *surface, int sprite_no, bool active_flag, int cell_no );
    void decodeExbtnControl( SDL_Surface *surface, const char *ctl_str );
    void disableGetButtonFlag();
    int getNumberFromBuffer( const char **buf );
    
    /* ---------------------------------------- */
    /* Background related variables */
    AnimationInfo bg_info;
    EFFECT_IMAGE bg_effect_image; // This is no longer used. Remove it later.

    void setBackground( SDL_Surface *surface, SDL_Rect *clip=NULL );

    /* ---------------------------------------- */
    /* Tachi-e related variables */
    /* 0 ... left, 1 ... center, 2 ... right */
    AnimationInfo tachi_info[3];

    /* ---------------------------------------- */
    /* Sprite related variables */
    AnimationInfo sprite_info[MAX_SPRITE_NUM];
    bool all_sprite_hide_flag;
    
    void allocateSelectedSurface( int sprite_no, ButtonLink *button );
    
    /* ---------------------------------------- */
    /* Parameter related variables */
    AnimationInfo *bar_info[MAX_PARAM_NUM], *prnum_info[MAX_PARAM_NUM];

    /* ---------------------------------------- */
    /* Cursor related variables */
    enum { CURSOR_WAIT_NO    = 0,
           CURSOR_NEWPAGE_NO = 1
    };
    AnimationInfo cursor_info[2];

    int proceedAnimation();
    int estimateNextDuration( AnimationInfo *anim, SDL_Rect &rect, int minimum );
    void resetRemainingTime( int t );
    void stopAnimation( int click );
    void loadCursor( int no, const char *str, int x, int y, bool abs_flag = false );
    void saveAll();
    void loadEnvData();
    void saveEnvData();
    
    /* ---------------------------------------- */
    /* Lookback related variables */
    AnimationInfo lookback_info[4];
    
    /* ---------------------------------------- */
    /* Registry related variables */
    char *registry_file;
    
    /* ---------------------------------------- */
    /* DLL related variables */
    char *dll_file;
    char *dll_str;
    int dll_ret;
    
    /* ---------------------------------------- */
    /* Text related variables */
    AnimationInfo sentence_font_info;
    char *font_file;
    int erase_text_window_mode;
    bool text_on_flag; // suppress the effect of erase_text_window_mode
    int  tateyoko_mode;

    void drawGlyph( SDL_Surface *dst_surface, char *text, FontInfo *info, SDL_Color &color, unsigned short unicode, int xy[2], int minx, int maxy, int shadow_offset, bool flush_flag, SDL_Rect *clip );
    void drawChar( char* text, FontInfo *info, bool flush_flag, bool lookback_flag, SDL_Surface *surface, bool buffering_flag = true, SDL_Rect *clip=NULL );
    void drawString( const char *str, uchar3 color, FontInfo *info, bool flush_flag, SDL_Surface *surface, SDL_Rect *rect = NULL, bool buffering_flag = false, SDL_Rect *clip=NULL );
    void restoreTextBuffer( SDL_Surface *surface, SDL_Rect *clip=NULL );
    int clickWait( char *out_text );
    int clickNewPage( char *out_text );
    int textCommand();
    bool isTextVisible();
    
    /* ---------------------------------------- */
    /* Effect related variables */
    DirtyRect dirty_rect, dirty_rect_tmp; // only this region is updated
    int effect_counter; // counter in each effect
    int effect_timer_resolution;
    int effect_start_time;
    int effect_start_time_old;
    
    int  setEffect( int effect_no );
    int  doEffect( int effect_no, AnimationInfo *anim, int effect_image );
    void generateMosaic( SDL_Surface *dst_surface, SDL_Surface *src_surface, int level );
    
    /* ---------------------------------------- */
    /* Select related variables */
    enum { SELECT_GOTO_MODE=0, SELECT_GOSUB_MODE=1, SELECT_NUM_MODE=2, SELECT_CSEL_MODE=3 };
    struct SelectLink{
        struct SelectLink *next;
        char *text;
        char *label;

        SelectLink(){
            next = NULL;
            text = label = NULL;
        };
        ~SelectLink(){
            if ( text )  delete[] text;
            if ( label ) delete[] label;
        };
    } root_select_link, *shelter_select_link;
    struct LinkLabelInfo select_label_info;
    int shortcut_mouse_line;

    void deleteSelectLink();
    struct ONScripterLabel::ButtonLink *getSelectableSentence( char *buffer, FontInfo *info, bool flush_flag = true, bool nofile_flag = false );
    
    /* ---------------------------------------- */
    /* Sound related variables */
    char *default_cdrom_drive;
    bool cdaudio_on_flag; // false if mute
    bool volume_on_flag; // false if mute
    SDL_AudioSpec audio_format;
    bool audio_open_flag;
    bool cdaudio_flag;
    
    bool wave_play_loop_flag;
    char *wave_file_name;
    
    bool midi_play_loop_flag;
    char *midi_file_name;
    Mix_Music *midi_info;
    
    SDL_CD *cdrom_info;
    int current_cd_track;
    bool cd_play_loop_flag;
    bool music_play_loop_flag;
    bool mp3save_flag;
    char *music_file_name;
    unsigned char *mp3_buffer;
    SMPEG *mp3_sample;
#if defined(EXTERNAL_MUSIC_PLAYER)
    Mix_Music *music_info;
#endif
    
    Mix_Chunk *wave_sample[ONS_MIX_CHANNELS];

#if defined(EXTERNAL_MUSIC_PLAYER)
    int playMusic();
    int playMusicFile();
#endif
    int playMIDIFile();
    int playMIDI();
    int playMP3( int cd_no );
    void playMPEG( const char *filename, bool click_flag );
    void playAVI( const char *filename, bool click_flag );
    int playCDAudio( int cd_no );
    enum { WAVE_PLAY        = 0,
           WAVE_PRELOAD     = 1,
           WAVE_PLAY_LOADED = 2
    };
    int playWave( const char *file_name, bool loop_flag, int channel, int play_mode=WAVE_PLAY );
    void stopBGM( bool continue_flag );
    void playClickVoice();
    void setupWaveHeader( unsigned char *buffer, int channels, int rate, unsigned long data_length );
    unsigned long decodeOggVorbis( unsigned char *buffer_in, unsigned char *buffer_out, unsigned long length, int &channels, int &rate );
    
    /* ---------------------------------------- */
    /* Text event related variables */
    TTF_Font *text_font;
    bool new_line_skip_flag;
    int text_speed_no;

    void shadowTextDisplay( SDL_Surface *dst_surface, SDL_Surface *src_surface, SDL_Rect *clip=NULL, FontInfo *info=NULL );
    void clearCurrentTextBuffer();
    void newPage( bool next_flag );
    
    void deleteLabelLink();
    void blitRotation( SDL_Surface *src_surface, SDL_Rect *src_rect, SDL_Surface *dst_surface, SDL_Rect *dst_rect );
    void flush( SDL_Rect *rect=NULL, bool clear_dirty_flag=true, bool direct_flag=false );
    void flushSub( SDL_Rect &rect );
    void executeLabel();
    int parseLine();
    void parseTaggedString( AnimationInfo *anim );
    void setupAnimationInfo( AnimationInfo *anim );
    int doClipping( SDL_Rect *dst, SDL_Rect *clip, SDL_Rect *clipped=NULL );
    int shiftRect( SDL_Rect &dst, SDL_Rect &clip );
    void alphaBlend( SDL_Surface *dst_surface, SDL_Rect dst_rect,
                     SDL_Surface *src1_surface, int x1, int y1,
                     SDL_Surface *src2_surface, int x2, int y2,
                     SDL_Surface *mask_surface, int x3,
                     int trans_mode, Uint32 mask_value = 255, SDL_Rect *clip=NULL, uchar3 *direct_color=NULL );
    int enterTextDisplayMode( int ret_wait = RET_WAIT );
    int resizeSurface( SDL_Surface *src, SDL_Rect *src_rect, SDL_Surface *dst, SDL_Rect *dst_rect );
    SDL_Surface *loadImage( char *file_name );

    void drawTaggedSurface( SDL_Surface *dst_surface, AnimationInfo *anim, SDL_Rect *clip );
    void makeNegaSurface( SDL_Surface *surface, SDL_Rect *dst_rect=NULL );
    void makeMonochromeSurface( SDL_Surface *surface, SDL_Rect *dst_rect=NULL, FontInfo *info=NULL );
    void refreshSurfaceParameters();
    void refreshSurface( SDL_Surface *surface, SDL_Rect *clip=NULL, int refresh_mode = REFRESH_NORMAL_MODE );
    void mouseOverCheck( int x, int y );
    void shiftCursorOnButton( int diff );
    
    /* ---------------------------------------- */
    /* System call related method */
    bool system_menu_enter_flag;
    int  system_menu_mode;

    int shelter_event_mode;
    struct TextBuffer *shelter_text_buffer;
    
    void searchSaveFile( SaveFileInfo &info, int no );
    int loadSaveFile( int no );
    void saveMagicNumber( FILE *fp );
    void saveSaveFileFromTmpFile( FILE *fp );
    int saveSaveFile( int no );
    int loadSaveFile2( FILE *fp );
    void saveSaveFile2( FILE *fp );
    void setupLookbackButton();

    void enterSystemCall();
    void leaveSystemCall( bool restore_flag = true );
    void executeSystemCall();
    void executeSystemMenu();
    void executeSystemSkip();
    void executeSystemReset();
    void executeWindowErase();
    void executeSystemLoad();
    void executeSystemSave();
    void executeSystemYesNo();
    void executeSystemLookback();
};

#endif // __ONSCRIPTER_LABEL_H__
