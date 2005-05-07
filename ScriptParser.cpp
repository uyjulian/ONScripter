/* -*- C++ -*-
 *
 *  ScriptParser.cpp - Define block parser of ONScripter
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

#include "ScriptParser.h"

#define VERSION_STR1 "ONScripter"
#define VERSION_STR2 "Copyright (C) 2001-2005 Studio O.G.A. All Rights Reserved."

#define DEFAULT_SAVE_MENU_NAME "＜セーブ＞"
#define DEFAULT_LOAD_MENU_NAME "＜ロード＞"
#define DEFAULT_SAVE_ITEM_NAME "しおり"

#define DEFAULT_TEXT_SPEED_LOW    40
#define DEFAULT_TEXT_SPEED_MIDDLE 20
#define DEFAULT_TEXT_SPEED_HIGHT  10

#define MAX_TEXT_BUFFER 17

typedef int (ScriptParser::*FuncList)();
static struct FuncLUT{
    char command[40];
    FuncList method;
} func_lut[] = {
    {"zenkakko", &ScriptParser::zenkakkoCommand},
    {"windoweffect", &ScriptParser::effectCommand},
    {"windowback", &ScriptParser::windowbackCommand},
    {"versionstr", &ScriptParser::versionstrCommand},
    {"usewheel", &ScriptParser::usewheelCommand},
    {"useescspc", &ScriptParser::useescspcCommand},
    {"underline", &ScriptParser::underlineCommand},
    {"transmode", &ScriptParser::transmodeCommand},
    {"time", &ScriptParser::timeCommand},
    {"textgosub", &ScriptParser::textgosubCommand},
    {"tan", &ScriptParser::tanCommand},
    {"sub", &ScriptParser::subCommand},
    {"stralias", &ScriptParser::straliasCommand},
    {"spi", &ScriptParser::soundpressplginCommand},
    {"soundpressplgin", &ScriptParser::soundpressplginCommand},
    {"skip",     &ScriptParser::skipCommand},
    {"sin", &ScriptParser::sinCommand},
    {"shadedistance",     &ScriptParser::shadedistanceCommand},
    {"selectvoice",     &ScriptParser::selectvoiceCommand},
    {"selectcolor",     &ScriptParser::selectcolorCommand},
    {"savenumber",     &ScriptParser::savenumberCommand},
    {"savename",     &ScriptParser::savenameCommand},
    {"sar",    &ScriptParser::nsaCommand},
    {"rubyon",    &ScriptParser::rubyonCommand},
    {"rubyoff",    &ScriptParser::rubyoffCommand},
    {"roff",    &ScriptParser::roffCommand},
    {"rmenu",    &ScriptParser::rmenuCommand},
    {"return",   &ScriptParser::returnCommand},
    {"pretextgosub", &ScriptParser::pretextgosubCommand},
    {"numalias", &ScriptParser::numaliasCommand},
    {"nsadir",    &ScriptParser::nsadirCommand},
    {"nsa",    &ScriptParser::nsaCommand},
    {"notif",    &ScriptParser::ifCommand},
    {"next",    &ScriptParser::nextCommand},
    {"nsa",    &ScriptParser::arcCommand},
    {"ns3",    &ScriptParser::nsaCommand},
    {"ns2",    &ScriptParser::nsaCommand},
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
    {"mode_ext", &ScriptParser::mode_extCommand},
    {"mod",      &ScriptParser::modCommand},
    {"mid",      &ScriptParser::midCommand},
    {"menusetwindow",      &ScriptParser::menusetwindowCommand},
    {"menuselectvoice",      &ScriptParser::menuselectvoiceCommand},
    {"menuselectcolor",      &ScriptParser::menuselectcolorCommand},
    {"maxkaisoupage",      &ScriptParser::maxkaisoupageCommand},
    {"lookbacksp",      &ScriptParser::lookbackspCommand},
    {"lookbackcolor",      &ScriptParser::lookbackcolorCommand},
    //{"lookbackbutton",      &ScriptParser::lookbackbuttonCommand},
    {"loadgosub",      &ScriptParser::loadgosubCommand},
    {"linepage2",    &ScriptParser::linepageCommand},
    {"linepage",    &ScriptParser::linepageCommand},
    {"len",      &ScriptParser::lenCommand},
    {"labellog",      &ScriptParser::labellogCommand},
    {"kidokuskip", &ScriptParser::kidokuskipCommand},
    {"kidokumode", &ScriptParser::kidokumodeCommand},
    {"itoa2", &ScriptParser::itoaCommand},
    {"itoa", &ScriptParser::itoaCommand},
    {"intlimit", &ScriptParser::intlimitCommand},
    {"inc",      &ScriptParser::incCommand},
    {"if",       &ScriptParser::ifCommand},
    {"humanz",       &ScriptParser::humanzCommand},
    {"goto",     &ScriptParser::gotoCommand},
    {"gosub",    &ScriptParser::gosubCommand},
    {"globalon",    &ScriptParser::globalonCommand},
    {"getparam",    &ScriptParser::getparamCommand},
    //{"game",    &ScriptParser::gameCommand},
    {"for",   &ScriptParser::forCommand},
    {"filelog",   &ScriptParser::filelogCommand},
    {"effectcut",   &ScriptParser::effectcutCommand},
    {"effectblank",   &ScriptParser::effectblankCommand},
    {"effect",   &ScriptParser::effectCommand},
    {"div",   &ScriptParser::divCommand},
    {"dim",   &ScriptParser::dimCommand},
    {"defvoicevol",   &ScriptParser::defvoicevolCommand},
    {"defsub",   &ScriptParser::defsubCommand},
    {"defsevol",   &ScriptParser::defsevolCommand},
    {"defmp3vol",   &ScriptParser::defmp3volCommand},
    {"defaultspeed", &ScriptParser::defaultspeedCommand},
    {"defaultfont", &ScriptParser::defaultfontCommand},
    {"dec",   &ScriptParser::decCommand},
    {"date",   &ScriptParser::dateCommand},
    {"cos", &ScriptParser::cosCommand},
    {"cmp",      &ScriptParser::cmpCommand},
    {"clickvoice",   &ScriptParser::clickvoiceCommand},
    {"clickstr",   &ScriptParser::clickstrCommand},
    {"break",   &ScriptParser::breakCommand},
    {"automode", &ScriptParser::mode_extCommand},
    {"atoi",      &ScriptParser::atoiCommand},
    {"arc",      &ScriptParser::arcCommand},
    {"add",      &ScriptParser::addCommand},
    {"", NULL}
};

ScriptParser::ScriptParser()
{
    debug_level = 0;
    srand( time(NULL) );

    archive_path = NULL;
    version_str = NULL;
    nsa_path = "";
    key_table = NULL;
    
    save_menu_name = NULL;
    load_menu_name = NULL;
    save_item_name = NULL;

    load_str_size = 128;
    load_str_buffer = new char[load_str_size];
    text_buffer = NULL;
    
    /* ---------------------------------------- */
    /* Sound related variables */
    int i;
    for ( i=0 ; i<     CLICKVOICE_NUM ; i++ )
             clickvoice_file_name[i] = NULL;
    for ( i=0 ; i<    SELECTVOICE_NUM ; i++ )
            selectvoice_file_name[i] = NULL;
    for ( i=0 ; i<MENUSELECTVOICE_NUM ; i++ )
        menuselectvoice_file_name[i] = NULL;
}

ScriptParser::~ScriptParser()
{
    reset();

    if (version_str) delete[] version_str;
    if (save_menu_name) delete[] save_menu_name;
    if (load_menu_name) delete[] load_menu_name;
    if (save_item_name) delete[] save_item_name;
    delete[] load_str_buffer;
}

void ScriptParser::reset()
{
    UserFuncLUT *func = root_user_func.next;
    while(func){
        UserFuncLUT *tmp = func;
        func = func->next;
        delete tmp;
    }
    root_user_func.next = NULL;
    last_user_func = &root_user_func;

    // reset misc variables
    if (strlen(nsa_path) > 0){
        delete[] nsa_path;
        nsa_path = "";
    }

    globalon_flag = false;
    labellog_flag = false;
    filelog_flag = false;
    kidokuskip_flag = false;

    rmode_flag = true;
    windowback_flag = false;
    usewheel_flag = false;
    useescspc_flag = false;
    mode_saya_flag = false;
    mode_ext_flag = false;
    rubyon_flag = false;
    zenkakko_flag = false;
    string_buffer_offset = 0;

    break_flag = false;
    trans_mode = AnimationInfo::TRANS_TOPLEFT;

    if (version_str) delete[] version_str;
    version_str = new char[strlen(VERSION_STR1)+
                           strlen("\n")+
                           strlen(VERSION_STR2)+
                           strlen("\n")+
                           +1];
    sprintf( version_str, "%s\n%s\n", VERSION_STR1, VERSION_STR2 );
    z_order = 25;

    textgosub_label = NULL;
    pretextgosub_label = NULL;
    loadgosub_label = NULL;

    /* ---------------------------------------- */
    /* Lookback related variables */
    lookback_sp[0] = lookback_sp[1] = -1;
    lookback_color[0] = 0xff;
    lookback_color[1] = 0xff;
    lookback_color[2] = 0x00;

    /* ---------------------------------------- */
    /* Save/Load related variables */
    setStr( &save_menu_name, DEFAULT_SAVE_MENU_NAME );
    setStr( &load_menu_name, DEFAULT_LOAD_MENU_NAME );
    setStr( &save_item_name, DEFAULT_SAVE_ITEM_NAME );
    num_save_file = 9;

    /* ---------------------------------------- */
    /* Text related variables */
    sentence_font.reset();
    menu_font.reset();
    ruby_font.reset();

    current_font = &sentence_font;
    shade_distance[0] = 1;
    shade_distance[1] = 1;
    
    default_text_speed[0] = DEFAULT_TEXT_SPEED_LOW;
    default_text_speed[1] = DEFAULT_TEXT_SPEED_MIDDLE;
    default_text_speed[2] = DEFAULT_TEXT_SPEED_HIGHT;
    max_text_buffer = MAX_TEXT_BUFFER;
    num_chars_in_sentence = 0;
    if (text_buffer){
        delete[] text_buffer;
        text_buffer = NULL;
    }
    current_text_buffer = start_text_buffer = NULL;
    
    clickstr_line = 0;
    clickstr_state = CLICK_NONE;
    linepage_line = -1;
    
    /* ---------------------------------------- */
    /* Sound related variables */
    int i;
    for ( i=0 ; i<     CLICKVOICE_NUM ; i++ )
        setStr(&clickvoice_file_name[i], NULL);
    for ( i=0 ; i<    SELECTVOICE_NUM ; i++ )
        setStr(&selectvoice_file_name[i], NULL);
    for ( i=0 ; i<MENUSELECTVOICE_NUM ; i++ )
        setStr(&menuselectvoice_file_name[i], NULL);

    /* ---------------------------------------- */
    /* Menu related variables */
    menu_font.font_size_xy[0] = DEFAULT_FONT_SIZE;
    menu_font.font_size_xy[1] = DEFAULT_FONT_SIZE;
    menu_font.top_xy[0] = 0;
    menu_font.top_xy[1] = 16;
    menu_font.num_xy[0] = 32;
    menu_font.num_xy[1] = 23;
    menu_font.pitch_xy[0] = menu_font.font_size_xy[0];
    menu_font.pitch_xy[1] = 2 + menu_font.font_size_xy[1];
    menu_font.window_color[0] = menu_font.window_color[1] = menu_font.window_color[2] = 0xcc;

    deleteRMenuLink();

    /* ---------------------------------------- */
    /* Effect related variables */
    effect_blank = 10;
    effect_cut_flag = false;

    window_effect.effect = 1;
    window_effect.duration = 0;
    root_effect_link.num = 0;
    root_effect_link.effect = 0;
    root_effect_link.duration = 0;

    EffectLink *link = root_effect_link.next;
    while(link){
        EffectLink *tmp = link;
        link = link->next;
        delete tmp;
    }
    last_effect_link = &root_effect_link;
    last_effect_link->next = NULL;

    script_h.loadLog( script_h.log_info[ScriptHandler::LABEL_LOG] );
    
    current_mode = DEFINE_MODE;

    script_h.setCurrent( "effect 1,1" );
    readToken();
    effectCommand();
}

int ScriptParser::open()
{
    script_h.cBR = new DirectReader( archive_path, key_table );
    script_h.cBR->open();
    
    if ( script_h.readScript( archive_path ) ) return -1;

    switch ( script_h.screen_size ){
      case ScriptHandler::SCREEN_SIZE_800x600:
#if defined(PDA)
        screen_ratio1 = 2;
        screen_ratio2 = 5;
#else
        screen_ratio1 = 1;
        screen_ratio2 = 1;
#endif
        screen_width  = 800 * screen_ratio1 / screen_ratio2;
        screen_height = 600 * screen_ratio1 / screen_ratio2;
        break;
      case ScriptHandler::SCREEN_SIZE_400x300:
#if defined(PDA)
        screen_ratio1 = 4;
        screen_ratio2 = 5;
#else
        screen_ratio1 = 1;
        screen_ratio2 = 1;
#endif
        screen_width  = 400 * screen_ratio1 / screen_ratio2;
        screen_height = 300 * screen_ratio1 / screen_ratio2;
        break;
      case ScriptHandler::SCREEN_SIZE_320x240:
        screen_ratio1 = 1;
        screen_ratio2 = 1;
        screen_width  = 320 * screen_ratio1 / screen_ratio2;
        screen_height = 240 * screen_ratio1 / screen_ratio2;
        break;
      case ScriptHandler::SCREEN_SIZE_640x480:
      default:
#if defined(PDA)
        screen_ratio1 = 1;
        screen_ratio2 = 2;
#else
        screen_ratio1 = 1;
        screen_ratio2 = 1;
#endif
        screen_width  = 640 * screen_ratio1 / screen_ratio2;
        screen_height = 480 * screen_ratio1 / screen_ratio2;
        break;
    }

    return 0;
}

unsigned char ScriptParser::convHexToDec( char ch )
{
    if      ( '0' <= ch && ch <= '9' ) return ch - '0';
    else if ( 'a' <= ch && ch <= 'f' ) return ch - 'a' + 10;
    else if ( 'A' <= ch && ch <= 'F' ) return ch - 'A' + 10;
    else errorAndExit("convHexToDec: not valid character for color.");

    return 0;
}

void ScriptParser::readColor( uchar3 *color, const char *buf ){
    if ( buf[0] != '#' ) errorAndExit("readColor: no preceeding #.");
    (*color)[0] = convHexToDec( buf[1] ) << 4 | convHexToDec( buf[2] );
    (*color)[1] = convHexToDec( buf[3] ) << 4 | convHexToDec( buf[4] );
    (*color)[2] = convHexToDec( buf[5] ) << 4 | convHexToDec( buf[6] );
}

int ScriptParser::parseLine()
{
    if ( debug_level > 0 ) printf("ScriptParser::Parseline %s\n", script_h.getStringBuffer() );

    if ( script_h.getStringBuffer()[0] == ';' ) return RET_CONTINUE;
    else if ( script_h.getStringBuffer()[0] == '*' ) return RET_CONTINUE;
    else if ( script_h.getStringBuffer()[0] == ':' ) return RET_CONTINUE;
    else if ( script_h.isText() ) return RET_NOMATCH;

    const char *cmd = script_h.getStringBuffer();
    if (cmd[0] != '_'){
        UserFuncLUT *uf = root_user_func.next;
        while( uf ){
            if ( !strcmp( uf->command, cmd ) ){
                gosubReal( cmd, script_h.getNext() );
                return RET_CONTINUE;
            }
            uf = uf->next;
        }
    }
    else{
        cmd++;
    }

    int lut_counter = 0;
    while( func_lut[ lut_counter ].method ){
        if ( !strcmp( func_lut[ lut_counter ].command, cmd ) ){
            return (this->*func_lut[ lut_counter ].method)();
        }
        lut_counter++;
    }

    return RET_NOMATCH;
}

ScriptParser::EffectLink *ScriptParser::getEffect( int effect_no )
{
    if      ( effect_no == WINDOW_EFFECT ) return &window_effect;
    else if ( effect_no == TMP_EFFECT )    return &tmp_effect;
    
    EffectLink *link = &root_effect_link;
    while( link ){
        if ( link->num == effect_no ) return link;
        link = link->next;
    }

    fprintf( stderr, "no effect was found [%d]\n", effect_no );
    exit(-1);
}

void ScriptParser::deleteRMenuLink()
{
    RMenuLink *link = root_rmenu_link.next;
    while(link){
        RMenuLink *tmp = link;
        link = link->next;
        delete tmp;
    }
    root_rmenu_link.next = NULL;

    rmenu_link_num   = 0;
    rmenu_link_width = 0;
}

int ScriptParser::getSystemCallNo( const char *buffer )
{
    if      ( !strcmp( buffer, "skip" ) )        return SYSTEM_SKIP;
    else if ( !strcmp( buffer, "reset" ) )       return SYSTEM_RESET;
    else if ( !strcmp( buffer, "save" ) )        return SYSTEM_SAVE;
    else if ( !strcmp( buffer, "load" ) )        return SYSTEM_LOAD;
    else if ( !strcmp( buffer, "lookback" ) )    return SYSTEM_LOOKBACK;
    else if ( !strcmp( buffer, "windowerase" ) ) return SYSTEM_WINDOWERASE;
    else if ( !strcmp( buffer, "rmenu" ) )       return SYSTEM_MENU;
    else if ( !strcmp( buffer, "automode" ) )    return SYSTEM_AUTOMODE;
    else if ( !strcmp( buffer, "end" ) )         return SYSTEM_END;
    else{
        printf("Unsupported system call %s\n", buffer );
        return -1;
    }
}

void ScriptParser::saveGlovalData()
{
    if ( !globalon_flag ) return;

    FILE *fp;

    if ( ( fp = fopen( "gloval.sav", "wb" ) ) == NULL ){
        fprintf( stderr, "can't write gloval.sav\n" );
        exit( -1 );
    }

    saveVariables( fp, script_h.global_variable_border, VARIABLE_RANGE );
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

void ScriptParser::loadStr( FILE *fp, char **str )
{
    int counter = 0, ch;

    while ((ch = fgetc(fp)) != EOF){
        if (counter >= load_str_size){
            char *buf = load_str_buffer;
            load_str_buffer = new char[load_str_size*2];
            memcpy(load_str_buffer, buf, load_str_size);
            delete[] buf;
            load_str_size *= 2;
        }
        load_str_buffer[counter++] = ch;
        if (ch == 0) break;
    }

    if (ch == EOF || counter == 1)
        *str = NULL;
    else
        setStr(str, load_str_buffer);
}

void ScriptParser::saveVariables( FILE *fp, int from, int to )
{
    for ( int i=from ; i<to ; i++ ){
        saveInt( fp, script_h.variable_data[i].num );
        saveStr( fp, script_h.variable_data[i].str );
    }
}

void ScriptParser::loadVariables( FILE *fp, int from, int to )
{
    for ( int i=from ; i<to ; i++ ){
        loadInt( fp, &script_h.variable_data[i].num );
        loadStr( fp, &script_h.variable_data[i].str );
    }
}

void ScriptParser::errorAndExit( const char *str, const char *reason )
{
    if ( reason )
        fprintf( stderr, " *** Parse error at %s:%d [%s]; %s ***\n",
                 current_label_info.name,
                 current_line,
                 str, reason );
    else
        fprintf( stderr, " *** Parse error at %s:%d [%s] ***\n",
                 current_label_info.name,
                 current_line,
                 str );
    exit(-1);
}

void ScriptParser::deleteNestInfo()
{
    NestInfo *info = root_nest_info.next;
    while(info){
        NestInfo *tmp = info;
        info = info->next;
        delete tmp;
    }
    root_nest_info.next = NULL;
    last_nest_info = &root_nest_info;
}

void ScriptParser::setStr( char **dst, const char *src, int num )
{
    if ( *dst ) delete[] *dst;
    if ( src ){
        if (num >= 0){
            *dst = new char[ num + 1 ];
            memcpy( *dst, src, num );
            (*dst)[num] = '\0';
        }
        else{
            *dst = new char[ strlen( src ) + 1 ];
            strcpy( *dst, src );
        }
    }
    else
        *dst = NULL;
}

void ScriptParser::setCurrentLabel( const char *label )
{
    current_label_info = script_h.lookupLabel( label );
    current_line = script_h.getLineByAddress( current_label_info.start_address );
    script_h.setCurrent( current_label_info.start_address );
}

void ScriptParser::readToken()
{
    script_h.readToken();
    if (script_h.isText()) string_buffer_offset = 0;

    if (script_h.isText() && linepage_line >= 0){
        char ch = '@'; // click wait
        if (sentence_font.getRemainingLine() <= linepage_line)
            ch = '\\'; // newline

        // ugly work around
        unsigned int len = strlen(script_h.getStringBuffer());
        if (script_h.getStringBuffer()[len-1] == 0x0a){
            script_h.getStringBuffer()[len-1] = ch;
            script_h.addStringBuffer(0x0a);
        }
        else{
            script_h.addStringBuffer(ch);
        }
    }
}

int ScriptParser::readEffect( EffectLink *effect )
{
    int num = 1;
    
    effect->effect = script_h.readInt();
    if ( script_h.getEndStatus() & ScriptHandler::END_COMMA ){
        num++;
        effect->duration = script_h.readInt();
        if ( script_h.getEndStatus() & ScriptHandler::END_COMMA ){
            num++;
            const char *buf = script_h.readStr();
            effect->anim.setImageName( buf );
        }
        else
            effect->anim.remove();
    }

    //printf("readEffect %d: %d %d %s\n", num, effect->effect, effect->duration, effect->anim.image_name );
    return num;
}

FILE *ScriptParser::fopen(const char *path, const char *mode)
{
    char *file_name = new char[strlen(archive_path)+strlen(path)+1];
    sprintf( file_name, "%s%s", archive_path, path );

    FILE *fp = ::fopen( file_name, mode );
    delete[] file_name;

    return fp;
}

void ScriptParser::createKeyTable( const char *key_exe )
{
    if (!key_exe) return;
    
    FILE *fp = ::fopen(key_exe, "rb");
    if (fp == NULL){
        fprintf(stderr, "createKeyTable: can't open EXE file %s\n", key_exe);
        return;
    }

    key_table = new unsigned char[256];

    int i;
    for (i=0 ; i<256 ; i++) key_table[i] = i;

    unsigned char ring_buffer[256];
    int ring_start = 0, ring_last = 0;
    
    int ch, count;
    while((ch = fgetc(fp)) != EOF){
        i = ring_start;
        count = 0;
        while (i != ring_last &&
               ring_buffer[i] != ch ){
            count++;
            i = (i+1)%256;
        }
        if (i == ring_last && count == 255) break;
        if (i != ring_last)
            ring_start = (i+1)%256;
        ring_buffer[ring_last] = ch;
        ring_last = (ring_last+1)%256;
    }
    fclose(fp);

    if (ch == EOF)
        errorAndExit( "createKeyTable: can't find a key table." );

    // Key table creation
    ring_buffer[ring_last] = ch;
    for (i=0 ; i<256 ; i++)
        key_table[ring_buffer[(ring_start+i)%256]] = i;
}
