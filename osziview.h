//
//    osziview.h
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

// 18-05-2003: merged in 16-bit audio code by Florian Berger from gtkguitune-0.6
// Modified 2002 by Reinier Lamers

#ifndef OSZIVIEW_H
#define OSZIVIEW_H

#include <QWidget>
#include <QImage>
#include <QBuffer>
#include <vector>
#include <fftw3.h>

class OsziView : public QWidget
{
    Q_OBJECT

public:
    explicit OsziView(QWidget *parent = 0);
    ~OsziView();

    void paintSample(void);
    double getfreq_s();   
	 /* in units of sampfreq */	
    double getfreq_acorr(bool do_notch, int plot_fft, int plot_acorr); 
    double getfreq_schmitt(void);
    void setSamplePtr(short int *s);
    void setSampleFreq(double f);
    void setSampleNr(int nr);
    void setNotch(bool do_notch);


protected:

    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

private:
    short int *samp;
    int sampnr;
    double sampfreq;
    int wscr, hscr, xscr, yscr;
    int              i_minsamp;
    int              i_maxsamp;
    int              i_divisor;
    int              i_adaptive_scale;

    /** Whether a notch filter is applied tot the input signal or not */
    bool             do_notch, do_fft, do_plot_acorr;

    fftw_plan        fftw_p, fftw_p2, fftw_p3;
    double           *fftw_in, *fftw_in2;
    fftw_complex     *fftw_out, *fftw_out2;

	 // stuff needed by paintEvent()
     QImage osziImage;
	 std::vector <double> *fftwAbsList;
	 double currFreq;
	 double currMax_o;

     QByteArray picArray;
     QBuffer picBuffer;

    void  calc_minmax(void);
    void  tdbpf(int nc, int nz, double fc1i, double fc2i, double *fcoeffs);
    void  convolute(double *c1, double *c2, double *out, int l1, int l2);
    void  debug_draw(double *dv, int dl, int r, int g, int b);

};

#endif
