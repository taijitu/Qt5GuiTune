//
//
//    guitune - program for tuning instruments (actually an oscilloscope)
//    Copyright (C) 1999  Florian Berger
//    Email: florian.berger@jk.uni-linz.ac.at
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License Version 2 as
//    published by the Free Software Foundation;
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//

// 2003-05-18: merged in 16-bit audio code by Florian Berger from gtkguitune-0.6
// modified 2003 by Reinier Lamers - changed fonts to 'QApplication::font()'
// instead of hardcoded 'System' (didn't work for me anymore :))

// 2007-03-27: Pierre Dumuid added more advanced method of determining the frequency: 
//              The Period of the frequency is determined by the first peak of the 
//              auto-correlation of the signal. 
//
// if (do_notch) {
//
//   Block of input signal --> FFT --> design notch filter (with bandwidth +- 5 frequency bins) 
//
//   Block of input signal --> notch filter --> (A)
//
// } else {
//
//   Block of input signal --> (A)
//
// }
//
// (A) --> zero-pad by N and FFT --> Abs(FFT)^2 --> (B) Power Spectra
//
// (B) --> IFFT -> (C) Auto-Correlation
//
// (C) --> Find first peak after the peak t = 0 --> Freq = 1 / (t(firstpeak))

// Ported to Qt5 by John Warriner, 2018

#include <QApplication>         // for QApplication::font()
#include <QPainter>
#include <QObject>
#include <QBuffer>
#include <QImage>
#include <QSize>
#include <math.h>

#include "resources.h"
#include "osziview.h"


OsziView::OsziView(QWidget * parent):QWidget(parent)
{
        // set black background
       //setPaletteBackgroundColor(qRgb(0,0,0));
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);

    // Initialized an image buffer so we can write
    // to it from anywhere
    picBuffer.setBuffer(&picArray);
    picBuffer.open(QIODevice::WriteOnly);
    QImage picture(QSize(380,230), QImage::Format_RGB32);
    picture.fill(Qt::black);
    picture.save(&picBuffer, "PNG");
    picBuffer.close();

    i_adaptive_scale = 1;
    fftw_p = 0;
    do_notch = false;
}

OsziView::~OsziView()
{
    if(fftw_p) {
        fftw_destroy_plan(fftw_p);
        fftw_free(fftw_in);
        fftw_free(fftw_out);

        fftw_destroy_plan(fftw_p2);
        fftw_free(fftw_in2);
        fftw_free(fftw_out2);
    }
};

void OsziView::calc_minmax(void)
{
    int i;

    i_minsamp= 33000;
    i_maxsamp=-33000;
    for(i=0;i<sampnr;i++){
        if ( samp[i-1] > i_maxsamp ) i_maxsamp = samp[i-1];
        if ( samp[i-1] < i_minsamp ) i_minsamp = samp[i-1];
    }
    if ( !i_adaptive_scale ){
        i_divisor = 0x10000;
    } else {
        i_divisor = 2048;
        if ( 2*Abs(i_maxsamp) > i_divisor ) i_divisor=2*Abs(i_maxsamp);
        if ( 2*Abs(i_minsamp) > i_divisor ) i_divisor=2*Abs(i_minsamp);
    }
}

void OsziView::paintSample(void)
{int i,x1,x2,y1,y2;
    //erase();
    QImage picture;
    picture.loadFromData(picArray, "PNG");
    QPainter paint;
    paint.begin(&picture);

    paint.eraseRect(xscr,yscr,wscr+1,hscr+1);
    paint.setPen( Qt::gray );
    paint.drawLine(xscr,yscr+128*hscr/256,xscr+wscr-1,yscr+128*hscr/256);
    paint.setPen( Qt::green );
    for(i=1;i<sampnr;i++){
        x1=xscr+(i-1)*wscr/sampnr;
        x2=xscr+i*wscr/sampnr;
        y1=yscr+((int)samp[i-1]+i_divisor/2)*hscr/i_divisor;
        y2=yscr+((int)samp[i]  +i_divisor/2)*hscr/i_divisor;
        paint.drawLine( x1, y1, x2, y2 );
    }
    //paint.flush();
    //paint.end();

    picBuffer.open(QIODevice::WriteOnly);
    picture.save(&picBuffer, "PNG" );
    picBuffer.close();
    update();
}

#define FFTWC_ABS(v)  sqrt(pow(v[0],2) + pow(v[1],2))
#define CLIP(v,mi,ma)  (v < mi ? mi : (v > ma ? ma : v))

double OsziView::getfreq_acorr(bool do_notch, int plot_fft, int plot_acorr)    /* in units of sampfreq */
{
    int i;

    if (do_notch) {

        // OBTAIN FFT response
        for(i=0;i<sampnr;i++) fftw_in[i] = samp[i];
        fftw_execute(fftw_p);

        // FIND PEAK IN FFT
        double min_i = 1;
        double max_i = 1;
        double min_o = FFTWC_ABS(fftw_out[1]);
        double max_o = FFTWC_ABS(fftw_out[1]);

        double fftw_abs[sampnr];
        for(i=1;i<sampnr;i++) {
            fftw_abs[i] = FFTWC_ABS(fftw_out[i]);
            if (fftw_abs[i] < min_o) { min_o = fftw_abs[i]; min_i = i; }
            if (fftw_abs[i] > max_o) { max_o = fftw_abs[i]; max_i = i; }
        }
        //  printf("Min,max = %.3f, %.3f, [%d,%d]\n",min_o, max_o, min_i, max_i);

        // PLOT FFT
        if (plot_fft) {
            QImage picture;
            picture.loadFromData(picArray, "PNG");
            QPainter paint1;
            paint1.begin(&picture);

            paint1.setPen(qRgb(50,50,200));
            double lrat = 0;
            for (i=0;i<sampnr/2;i++) {
                double rat = CLIP(20*log10(max_o) - 20*log10(fftw_abs[i]), 0, 20) / 20;
                paint1.drawLine(xscr + i * wscr / (sampnr / 2), (int) (yscr + lrat * hscr),
                                xscr + i * wscr / (sampnr / 2), (int) (yscr + rat * hscr));
                lrat = rat;
            }
            picBuffer.open(QIODevice::WriteOnly);
            picture.save(&picBuffer, "PNG" );
            picBuffer.close();
        }

        // DESIGN NOTCH FILTER
        double fcoeffs[16];
        double fc1i = (max_i - 10) / (sampnr/2);
        double fc2i = (max_i + 10) / (sampnr/2);
        tdbpf(16, 0, fc1i, fc2i, fcoeffs);

        // PERFORM THE FILTERING
        double conv_out[sampnr + 16];

        for(i=0;i<sampnr;i++)
            fftw_in[i] = samp[i];

        convolute(fftw_in, fcoeffs, conv_out, sampnr, 16);

        // fill FFT array and zero padded to avoid wrap-around!
        for(i=0;i<sampnr;i++)
            fftw_in2[i] = conv_out[i];
    }

    // ------------------ DEBUG
    //    debug_draw(fftw_in, sampnr, 255, 255, 255);
    //    debug_draw(conv_out, sampnr, 255, 100, 100);
    // ------------------------

    // DETERMINE THE AUTO-CORRELATION
    // fill FFT array and zero padded to avoid wrap-around!
    if (!do_notch)
        for(i=0;i<sampnr;i++)
            fftw_in2[i] = samp[i];

    for(;i<sampnr * 2; i++)
        fftw_in2[i] = 0;

    fftw_execute(fftw_p2);
    for(i=0;i<sampnr;i++) {
        fftw_out2[i][0] = pow(fftw_out2[i][0],2) + pow(fftw_out2[i][1],2);
        fftw_out2[i][1] = 0;
    }
    fftw_execute(fftw_p3);

    if (plot_acorr)
        debug_draw(fftw_in2, sampnr, 100, 100, 255);

    // FIND FIRST PEAK (after the peak at 0)
    int firstminfind = 1;
    double maxval = fftw_in2[1]; // Start at 1!!
    int maxvali = 1;
    for(i=2;i < sampnr/2; i++) {
        if (firstminfind && fftw_in2[i] > fftw_in2[i-1]) {
            firstminfind = 0;
            maxval = fftw_in2[i];
            maxvali = i;
        } else {
            if (fftw_in2[i] > maxval) { maxval = fftw_in2[i]; maxvali = i; }
        }
    }
    double freq = (double) 1 / (double) maxvali;
    if (freq < 1E-15) freq = 1E-15;

    if (plot_acorr) {
        QImage picture;
        picture.loadFromData(picArray, "PNG");
        QPainter paint;
        paint.begin(&picture);

        paint.setPen(qRgb(255,255,255));
        paint.drawLine(xscr+(int)((double)wscr/(double)sampnr/freq),yscr,
                       xscr+(int)((double)wscr/(double)sampnr/freq),yscr+hscr);
        picBuffer.open(QIODevice::WriteOnly);
        picture.save(&picBuffer, "PNG" );
        picBuffer.close();
    }

    return(freq);
}


double OsziView::getfreq_s()
{
    double f_acorr = getfreq_acorr(do_notch, 0, 1);

    return(f_acorr);
}



void OsziView::setSamplePtr(short int *s) { samp = s; calc_minmax();}

void OsziView::setSampleFreq(double f) { sampfreq = f; repaint(); }

void OsziView::setSampleNr(int nr)
{
    //repaint();
    sampnr = nr;
    if ( fftw_p ) {
            fftw_destroy_plan(fftw_p);
            fftw_destroy_plan(fftw_p2);
            fftw_free(fftw_in);
            fftw_free(fftw_out);
            fftw_free(fftw_in2);
            fftw_free(fftw_out2);
    }
    fftw_in = (double *) fftw_malloc(sizeof(double) * nr);
    int nc = (nr / 2) + 1;
    fftw_out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * nc);
    fftw_p =
        fftw_plan_dft_r2c_1d(nr, fftw_in,  fftw_out, FFTW_ESTIMATE);

    fftw_in2 = (double *)fftw_malloc(sizeof(double) * nr * 2);
    fftw_out2 = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * nr * 2);
    fftw_p2 =
        fftw_plan_dft_r2c_1d(nr * 2, fftw_in2, fftw_out2, FFTW_ESTIMATE);
    fftw_p3 =
        fftw_plan_dft_c2r_1d(nr * 2, fftw_out2, fftw_in2, FFTW_ESTIMATE);
}

void OsziView::setNotch(bool n)
{
    do_notch = n;
}

void OsziView::resizeEvent(QResizeEvent *)
{
    xscr=20;
    yscr=5;
    wscr=width()-xscr*2;
    hscr=height()-yscr-20;
}

// The function, tdbpf given below is from:
// 
// http://www.exstrom.com/journal/sigproc/

/*
 *                            COPYRIGHT
 *
 *  tdbpf - time domain bandpass filter coefficient calculator.
 *  Copyright (C) 2003, 2004, 2005, 2006 Exstrom Laboratories LLC
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
 *  A copy of the GNU General Public License is available on the internet at:
 *
 *  http://www.gnu.org/copyleft/gpl.html
 *
 *  or you can write to:
 *
 *  The Free Software Foundation, Inc.
 *  675 Mass Ave
 *  Cambridge, MA 02139, USA
 *
 *  You can contact Exstrom Laboratories LLC via Email at:
 *
 *  info(AT)exstrom.com
 *
 *  or you can write to:
 *
 *  Exstrom Laboratories LLC
 *  P.O. Box 7651
 *  Longmont, CO 80501, USA
 *
 */

//    fc1 = lower frequency as a fraction of PI [0,1]
//    fc2 = upper frequency as a fraction of PI [0,1]
//    nc = number of coefficients
//    nz = number of zeros to pad

void OsziView::tdbpf(int nc, int nz, double fc1i, double fc2i, double *fcoeffs)  {
    int i;
    double d1, d2;
    double fc1, fc2, h;
    fc1 = fc1i * M_PI;
    fc2 = fc2i * M_PI;
    d1 = ((double)nc - 1.0)/2.0;

    for( i = 0; i < nc; ++i )
    {
        d2 = (double)i - d1;
        h = d2 == 0 ? (fc2 - fc1) / M_PI : (sin(fc2 * d2) - sin(fc1 * d2)) / (M_PI * d2);
        fcoeffs[i] = h;
    }
}


void OsziView::convolute(double *c1, double *c2, double *out, int l1, int l2) 
{
    for (int out_i = 0; out_i < l1 + l2; out_i++)
    {
        out[out_i] = 0;
        int upto = (out_i < l2) ? out_i : l2;

        for (int c_i = 0; c_i < upto; c_i++) {
            out[out_i] += c1[out_i - c_i] * c2[c_i];
        }
    }
}

void OsziView::debug_draw(double *dv, int dl, int r, int g, int b)
{
    double min_v = dv[0];
    double max_v = dv[0];
    for(int i=0;i<dl;i++) {
        if (dv[i] < min_v) min_v = dv[i];
        if (dv[i] > max_v) max_v = dv[i];
    }

    QImage picture;
    picture.loadFromData(picArray, "PNG");
    QPainter paint1;
    paint1.begin(&picture);
    paint1.setPen(qRgb(r,g,b));

    double ratlast = 0;
    for (int i=0;i<dl;i++) {
        double rat = 1 - ((dv[i] - min_v)/(max_v - min_v));

        paint1.drawLine(xscr+i*wscr/dl, (int) (yscr + ratlast * hscr),
                        xscr+i*wscr/dl, (int) (yscr + rat * hscr));
        ratlast = rat;
    }
    picBuffer.open(QIODevice::WriteOnly);
    picture.save(&picBuffer, "PNG" );
    picBuffer.close();
}

// protected functions

void OsziView::paintEvent(QPaintEvent *)
{

    QImage newPic;
    newPic.loadFromData(picArray, "PNG");
    QPainter painter(this);
    painter.drawImage(0,0, newPic);
}

