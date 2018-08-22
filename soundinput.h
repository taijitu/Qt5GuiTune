/***************************************************************************
                          soundinput.h  -  description
                             -------------------
    begin                : di apr 29 2003
    copyright            : (C) 2003 by Reinier Lamers
    email                : tux_rocker@planet.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

 // 22-08-2003: Added ALSA support, and included config.h and added #ifdefs for audio system selection at compile time - Reinier Lamers
 
#ifndef SOUNDINPUT_H
#define SOUNDINPUT_H

#include <stdexcept>

extern "C" {
#include <alsa/asoundlib.h>
}

/* an exception to throw when a creation of a SoundInput fails */
class SoundInputException : public std::exception {
};

 /*
  * a dummy kind of class to derive other classes of, that handle specific
  * ways of sound input, like OSS, Arts, ALSA, etc.
  */
 class SoundInput {
  public:
    static SoundInput *autodetect( const char *alsa_devicename, const char *dsp_devicename, int &blksize, int &sampfreq, double &sampfreq_exact );
    virtual ~SoundInput();
    virtual int getData( short int *buf, int blksize ) = 0;
 };

 class ALSASoundInput : public SoundInput {
   public:
    ALSASoundInput( const char *devicename, int &blksize, int &sampfreq, double &sampfreq_exact );
    ~ALSASoundInput();
    int getData( short int *buf, int blksize );
   private:
    snd_pcm_t *pcm_handle;
 };

#endif // SOUNDINPUT_H 
