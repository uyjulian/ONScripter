//$Id:$ -*- C++ -*-
/*
 *  DirectReader.h - Reader from independent files
 *
 *  Copyright (c) 2001-2003 Ogapee
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

#ifndef __DIRECT_READER_H__
#define __DIRECT_READER_H__

#include "BaseReader.h"
#include <string.h>

#define MAX_FILE_NAME_LENGTH 512

class DirectReader : public BaseReader
{
public:
    DirectReader( char *path=NULL );
    ~DirectReader();

    int open( char *name=NULL, int archive_type = ARCHIVE_TYPE_NONE );
    int close();

    char *getArchiveName() const;
    int getNumFiles();
    int getNumAccessed();
    void registerCompressionType( const char *ext, int type );

    struct FileInfo getFileByIndex( int index );
    size_t getFileLength( const char *file_name );
    size_t getFile( const char *file_name, unsigned char *buffer );

protected:
    char *archive_path;
    int  getbit_mask;
    struct RegisteredCompressionType{
        RegisteredCompressionType *next;
        char *ext;
        int type;
        RegisteredCompressionType(){
            ext = NULL;
            next = NULL;
        };
        RegisteredCompressionType( const char *ext, int type ){
            this->ext = new char[ strlen(ext)+1 ];
            for ( unsigned int i=0 ; i<strlen(ext)+1 ; i++ ){
                this->ext[i] = ext[i];
                if ( this->ext[i] >= 'a' && this->ext[i] <= 'z' )
                    this->ext[i] += 'A' - 'a';
            }
            this->type = type;
            this->next = NULL;
        };
        ~RegisteredCompressionType(){
            if (ext) delete[] ext;
        };
    } root_registered_compression_type, *last_registered_compression_type;

    FILE *fopen(const char *path, const char *mode);
    unsigned char readChar( FILE *fp );
    unsigned short readShort( FILE *fp );
    unsigned long readLong( FILE *fp );
    void writeChar( FILE *fp, unsigned char ch );
    void writeShort( FILE *fp, unsigned short ch );
    void writeLong( FILE *fp, unsigned long ch );
    char capital_name[ MAX_FILE_NAME_LENGTH + 1 ];
    size_t decodeNBZ( FILE *fp, size_t offset, unsigned char *buf );
    size_t encodeNBZ( FILE *fp, size_t length, unsigned char *buf );
    int getbit( FILE *fp, int n );
    size_t decodeSPB( FILE *fp, size_t offset, unsigned char *buf );
    size_t decodeLZSS( struct ArchiveInfo *ai, int no, unsigned char *buf );
    int getRegisteredCompressionType( const char *file_name );
    size_t getDecompressedFileLength( int type, FILE *fp, size_t offset );
    
private:
    FILE *getFileHandle( const char *file_name, int &compression_type, size_t *length );
};

#endif // __DIRECT_READER_H__
