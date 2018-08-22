/***************************************************************************
                          soundinput.cpp  -  description
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

 // 15-08-2005: Added JACK support, made OSS and ALSA drivers do 
 // non-blocking opens - Reinier Lamers
 // 22-08-2003: Added ALSA support, and included config.h and added 
 // #ifdefs for audio system selection at compile time - Reinier Lamers

// Ported to Qt5 by John Warriner, 2018

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <exception>
#include <stdexcept>

#ifdef USE_OSS
 /* includes for OSS input driver */
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <sys/ioctl.h>
# include <linux/soundcard.h>
#endif

#include <alsa/asoundlib.h>
#include <iostream>
#include <asm/errno.h>          /* for EAGAIN */
#include "soundinput.h"

/*
 * Autodetects which sound system should be used and returns a corresponding
 * SoundInput. Returns 0 if no valid sound system was found.
 */
SoundInput *SoundInput::autodetect(const char *alsa_devicename,
                                   const char *devicename, int &blksize,
                                   int &sampfreq, double &sampfreq_exact)
{
    SoundInput *soundinput = 0;

    try {
        soundinput =
            new ALSASoundInput(alsa_devicename, blksize, sampfreq,
                               sampfreq_exact);
    } catch(SoundInputException) {
    }
    return soundinput;
}

/* this solves some linker problems. Really needed? Is it time to go actually learn C++? */
SoundInput::~SoundInput(void)
{
}

ALSASoundInput::ALSASoundInput(const char *devicename, int &blksize,
                               int &sampfreq, double &sampfreq_exact)
{
    snd_pcm_hw_params_t *hwparams;

    /* used to store direction sample rate has been modified. quite useless */
    int dir;
    int err;

    snd_pcm_hw_params_malloc(&hwparams);

    /* create a PCM handle that is pointed to by this->pcm_handle
       such a handle will be what we operate on further */

    if ((err =
         snd_pcm_open(&pcm_handle, devicename, SND_PCM_STREAM_CAPTURE,
                      SND_PCM_NONBLOCK)) < 0) {
        std::cerr << "Couldn't open ALSA device: " << devicename << std::endl;
        std::cerr << snd_strerror(err) << std::endl;
        snd_pcm_hw_params_free(hwparams);
        throw SoundInputException();
    }

    /* Set the handle to be blocking, so that we won't waste CPU cycles waiting for input */
    if ((err = snd_pcm_nonblock(pcm_handle, 0)) < 0) {
        std::cerr << "Couldn't set blocking I/O for ALSA device: " <<
            devicename << std::endl;
        std::cerr << snd_strerror(err) << std::endl;
        snd_pcm_hw_params_free(hwparams);
        snd_pcm_close(pcm_handle);
        throw SoundInputException();
    }

    /* Get the sound card configuration in hwparams */
    if ((err = snd_pcm_hw_params_any(pcm_handle, hwparams)) < 0) {
        std::cerr << "Couldn't get sound card configuration for ALSA device: "
            << devicename << std::endl;
        std::cerr << snd_strerror(err) << std::endl;
        snd_pcm_hw_params_free(hwparams);
        snd_pcm_close(pcm_handle);
        throw SoundInputException();
    }

    /* Set everything we have to set */
    if ((err =
         snd_pcm_hw_params_set_access(pcm_handle, hwparams,
                                      SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        std::cerr << "Couldn't set access for ALSA device: ";
        std::cerr << devicename << std::endl;
        std::cerr << snd_strerror(err) << std::endl;
        snd_pcm_hw_params_free(hwparams);
        snd_pcm_close(pcm_handle);
        throw SoundInputException();
    }

    if ((err =
         snd_pcm_hw_params_set_format(pcm_handle, hwparams,
                                      SND_PCM_FORMAT_S16_LE)) < 0) {
        std::cerr << "Couldn't set format for ALSA device: ";
        std::cerr << devicename << std::endl;
        std::cerr << snd_strerror(err) << std::endl;
        snd_pcm_hw_params_free(hwparams);
        snd_pcm_close(pcm_handle);
        throw SoundInputException();
    }

    if ((err = snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams,
                                         (unsigned *)&sampfreq, &dir)) < 0) {
        std::cerr << "Couldn't set sample frequency for ALSA device: ";
        std::cerr << devicename << std::endl;
        std::cerr << snd_strerror(err);
        snd_pcm_hw_params_free(hwparams);
        snd_pcm_close(pcm_handle);
        throw SoundInputException();
    }
    sampfreq_exact = sampfreq;

    if ((err = snd_pcm_hw_params_set_channels(pcm_handle, hwparams, 1)) < 0) {
        std::cerr << "Couldn't set number of channels for ALSA device: ";
        std::cerr << devicename << std::endl;
        std::cerr << snd_strerror(err) << std::endl;
        snd_pcm_hw_params_free(hwparams);
        snd_pcm_close(pcm_handle);
        throw SoundInputException();
    }

    /* Apply all these idiotic settings */
    if ((err = snd_pcm_hw_params(pcm_handle, hwparams)) < 0) {
        std::cerr << "Couldn't set parameters for ALSA device: ";
        std::cerr << devicename << std::endl;
        std::cerr << snd_strerror(err) << std::endl;
        snd_pcm_hw_params_free(hwparams);
        snd_pcm_close(pcm_handle);
        throw SoundInputException();
    }

    snd_pcm_hw_params_free(hwparams);
    // std::cout << "ALSA audio initialized succesfully" << std::endl;
}

ALSASoundInput::~ALSASoundInput(void)
{
    snd_pcm_close(pcm_handle);
}

int ALSASoundInput::getData(short int *buf, int blksize)
{
    int numblocks;

    numblocks = snd_pcm_readi(pcm_handle, (void *)buf, blksize / 2);
    if (numblocks < 0) {
        /* I don't know if there is a more elegant solution to this. This works and it is badly needed */
        if (-numblocks != EAGAIN) {
            fprintf(stderr, "Error reading sound: %s\n",
                    snd_strerror(numblocks));
            snd_pcm_prepare(pcm_handle);
        }
        numblocks = 0;
    }
    return numblocks;
}
