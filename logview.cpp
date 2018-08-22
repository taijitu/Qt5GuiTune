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

// modified 2003 by Reinier Lamers - changed fonts to 'QApplication::font()'
// instead of hardcoded 'System' (didn't work for me anymore :))

// Ported to Qt5 by John Warriner, 2018

#include "logview.h"

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <QWidget>
#include <QFrame>
#include <QApplication> //for QApplication::font()
#include <QPainter>
#include <QPalette>
#include <QImage>
#include <QBuffer>
#include <QLocale>
#include <QMouseEvent>
#include <math.h>
#include "resources.h"



const char *note_ger[12] = {"A","B", "H","C","Db","D","Eb","E","F","Gb","G","Ab"};
const char *note_us[12] =  {"A","Bb","B","C","Db","D","Eb","E","F","Gb","G","Ab"};
const char *note_fr[12] =  {"La","Sib","Si","Do","Reb","Re","Mib","Mi","Fa","Solb","Sol","Lab"};
const char *note_ger_alt[12] = {"A","B", "H","C","C#","D","D#","E","F","F#","G","G#"};
const char *note_us_alt[12] =  {"A","A#","B","C","C#","D","D#","E","F","F#","G","G#"};
const char *note_fr_alt[12] =  {"La","La#","Si","Do","Do#","Re","Re#","Mi","Fa","Fa#","Sol","Sol#"};

const char **note;

const char *tuning_nat[23] =
{"25/24","16/15","9/8","7/6","75/64","6/5","5/4","32/25","125/96",
 "4/3","25/18","45/32","36/25","3/2","25/16","8/5","5/3","125/72",
 "16/9","9/5","15/8","48/25","125/64"};



LogView::LogView(QWidget *parent) : QWidget(parent)
{
    int r,g,b,gray;

    grundton=0;
    nat_tuning_on=false;

    note=note_us;

    lfreq=1.0;
    x0=10;
    y0=26;
    h=10;

    backgroundColor = parent->palette().background().color();
    r = backgroundColor.red();
    g = backgroundColor.green();
    b = backgroundColor.blue();

    gray=r+g+b;
    if (gray == 0) gray = 1;
    r=r*3*0xD0/gray; if (r>0xFF) r=0xFF;
    g=g*3*0xD0/gray; if (g>0xFF) g=0xFF;
    b=b*3*0xD0/gray; if (b>0xFF) b=0xFF;
    backgroundColor = qRgb(r,g,b);

    // Prepare to store image to memory
    picBuffer.setBuffer(&picArray);
    picBuffer.open(QIODevice::WriteOnly);
    QImage picture(QSize(510, 120), QImage::Format_RGB32);

    //setBackgroundColor(qRgb(r,g,b));
    picture.fill(backgroundColor);

    // Save iimage to memory
    picture.save(&picBuffer, "PNG");
    picBuffer.close();

    r = backgroundColor.red();
    g = backgroundColor.green();
    b = backgroundColor.blue();

    gray=r+g+b;
    if (gray == 0) gray = 1;
    r=r*3*0x80/gray; if (r>0xFF) r=0xFF;
    g=g*3*0x80/gray; if (g>0xFF) g=0xFF;
    b=b*3*0x80/gray; if (b>0xFF) b=0xFF;
    arrowBackgroundColor.setRgb(r,g,b);
}

LogView::~LogView() {};

int LogView::lfreq_pos(double lfreq)
{
    return (int)( (double)x0+(double)(width()-2*x0)
                  *(double)(lfreq-lfreq0)/LOG_2+0.5 );
}

int LogView::pos_note(int pos)
{
    return( (pos-x0)*12/(width()-2*x0) );
}

void LogView::drawarrow(QColor col)
{
    int x,y;
    QImage picture;
    picture.loadFromData(picArray, "PNG");
    QPainter p;
    p.begin(&picture);

    p.setPen(col);
    //        x=(int)( x0+(lfreq-lfreq0)*(width()-20)/LOG_2+0.5 );
    p.setPen(qRgb(0,0,0));
    x=lfreq_pos(lfreq);
    y=y0;
    p.fillRect ( x-6, y+1, 13, h-1, col );
    p.drawLine(x,y+1,x,y+h-1);
    //      	p.drawLine(x,y+1,x,y+h-1);
    //      	p.drawLine(x,y+1,x+(h/4-1),y+(h/2-1));
    //      	p.drawLine(x+(h/4-1),y+(h/2-1),x,y+(h-1));
    //      	p.drawLine(x,y+h-1,x-(h/4-1),y+h/2-1);
    //      	p.drawLine(x-(h/4-1),y+h/2-1,x,y+1);

    picBuffer.open(QIODevice::WriteOnly);
    picture.save(&picBuffer, "PNG" );
    picBuffer.close();
	 do_drawarrow = true;
    update();
}

void LogView::deletearrow(QColor col)
{
    int x,y;

    QImage picture;
    picture.loadFromData(picArray, "PNG");
    QPainter p;
    p.begin(&picture);

    p.setPen(col);
    p.setPen( arrowBackgroundColor );
    //        x=(int)( x0+(lfreq-lfreq0)*(width()-20)/LOG_2+0.5 );
    x=lfreq_pos(lfreq);
    y=y0;
    p.fillRect ( x-6, y+1, 13, h-1, arrowBackgroundColor  );

    picBuffer.open(QIODevice::WriteOnly);
    picture.save(&picBuffer, "PNG" );
    picBuffer.close();
	 do_deletearrow = true;
    update();
}

void LogView::change_lfreq(double freq)
{

    double mldf,ldf;
    int i,note;

    deletearrow(backgroundColor);
    lfreq=freq;
    while (lfreq>lfreq1) lfreq-=LOG_2;
    while (lfreq<lfreq0) lfreq+=LOG_2;
    mldf=D_NOTE_LOG; note=0;
    for( i=0; i<12; i++ ){
        ldf = fabs(lfreq-lfreqs[i]);
        if (ldf<mldf) { mldf=ldf; note=i; }
    }

    drawarrow( qRgb( int(0x90+0x6F*mldf/D_NOTE_LOG*2),
                     int(0xFF-0x6F*mldf/D_NOTE_LOG*2),
                     0x60 )
               );
}

void LogView::setScale(int scale)
{
    switch(scale){
    case us_scale:
        note=note_us;
        break;
    case german_scale:
        note=note_ger;
        break;
    case french_scale:
        note=note_fr;
        break;
    case us_scale_alt:
        note=note_us_alt;
        break;
    case german_scale_alt:
        note=note_ger_alt;
        break;
    case french_scale_alt:
        note=note_fr_alt;
        break;
    }
	 scaleChanged = true;
    update();
}

void LogView::drawScale() {
    int i,k,x,y,x2;
    //char  str[100];
    QString str;
    double nat_freq[23];
    double nat_lfreq[23];
    int z,n;

    lfreq0 = KAMMERTON_LOG - D_NOTE_LOG/2.0;
    lfreq1 = KAMMERTON_LOG - D_NOTE_LOG/2.0 + LOG_2;

    for(i=0;i<12;i++){
        freqs [i] = KAMMERTON * pow(D_NOTE,i);
        lfreqs[i] = KAMMERTON_LOG + (double)i*D_NOTE_LOG;
    }

    for(i=0;i<23;i++){
        sscanf(tuning_nat[i],"%d/%d",&z,&n);
        //         printf("z=%d,n=%d\n",z,n);
        nat_freq[i]=freqs[grundton]*(double)z/(double)n;
        nat_lfreq[i]=log(nat_freq[i]);
        if ( nat_lfreq[i]>lfreq1 ){
            nat_lfreq[i]-=LOG_2;
            nat_freq[i]/=2.0;
        }
    }

    QImage picture;
    picture.loadFromData(picArray, "PNG");
    picture.fill(backgroundColor);
    QPainter p;
    p.begin(&picture);

    //        lfreq0=lfreqs[0]-D_NOTE_LOG/2.0;
    p.setPen( qRgb(0x50,0x50,0x50) );
    for(k=0;k<120;k++){
        x2=lfreq_pos(KAMMERTON_LOG + (double)(k-5)/10.0*D_NOTE_LOG);
        //	      x2=(int)( (double)x0+(double)(width()-2*x0)
        //                 *(double)k/120.0+0.5 );
        p.drawLine(x2,y0,x2,y0-((k%5==0)?5:3));
        p.drawLine(x2,y0+h,x2,y0+h+((k%5==0)?5:3));
    }

    if(nat_tuning_on){
        // draw natural tuning
        p.setFont(QFont(QApplication::font().family(),8));
        p.setPen( qRgb(0,0,0) );
        x2=lfreq_pos(lfreqs[grundton]);
        p.drawLine(x2,y0+h,x2,y0+h+10+
                   +fontMetrics().height()*2);
        p.setPen( qRgb(0xFF,0xFF,0xFF) );
        p.drawText( x2-p.fontMetrics().width(tr("Key"))/2+1,
                    y0+h+10+fontMetrics().ascent()
                    +fontMetrics().height()*2+1,
                    tr("Key") );
        p.setPen( qRgb(0,0,0) );
        p.drawText( x2-p.fontMetrics().width(tr("Key"))/2,
                    y0+h+10+fontMetrics().ascent()
                    +fontMetrics().height()*2,
                    tr("Key") );
        for(k=0;k<23;k++){
            x2=lfreq_pos(nat_lfreq[k]);
            p.setPen( qRgb(0xFF,0,0) );
            //    	      p.drawLine(x2,y0+h,x2,y0+h+10);
            p.drawLine(x2,y0+h,x2,y0+h+10+
                       +fontMetrics().height()*(k%3));
        }
        for(k=0;k<23;k++){
            x2=lfreq_pos(nat_lfreq[k]);
            p.setPen( qRgb(0xFF,0xFF,0xFF) );
            p.drawText( x2-p.fontMetrics().width(tuning_nat[k])/2+1,
                        y0+h+10+fontMetrics().ascent()
                        +fontMetrics().height()*(k%3)+1,
                        tuning_nat[k] );
            p.setPen( qRgb(0,0,0) );
            p.drawText( x2-p.fontMetrics().width(tuning_nat[k])/2,
                        y0+h+10+fontMetrics().ascent()
                        +fontMetrics().height()*(k%3),
                        tuning_nat[k] );
        }
    }
    p.fillRect(x0,y0,width()-10-x0,h, arrowBackgroundColor );
    p.setPen( qRgb(0,0,0) );
    p.drawLine(x0,y0,width()-10,y0);
    p.drawLine(x0,y0+h,width()-10,y0+h);
    for(i=0;i<12;i++){
        p.setFont(QFont(QApplication::font().family(),12));
        x=lfreq_pos(lfreqs[i]);
        //    	   x=(int)( (double)x0+(double)(width()-2*x0)
        //                 *(double)(lfreqs[i]-lfreq0)/LOG_2+0.5 );
        y=y0;
        p.drawLine(x,y,x,y-7);
        if(!nat_tuning_on)
            p.drawLine(x,y+h,x,y+h+7);

        p.setPen( qRgb(0xFF,0xFF,0xFF) );
        p.drawText( x-p.fontMetrics().width(note[i])/2+1,y-10+1,
                    note[i] );
        p.setPen( qRgb(0,0,0) );
        p.drawText( x-p.fontMetrics().width(note[i])/2,y-10,
                    note[i] );

        p.setFont(QFont(QApplication::font().family(),8));
        if(!nat_tuning_on)
            for(k=0;k<4;k++){
                //sprintf(str,"%.2f",(double)(freqs[i]/8.0*(1<<k)));
                //str = (KGlobal::_locale)->formatNumber((double)(freqs[i]/8.0*(1<<k)));
                str = QLocale().toString((double)(freqs[i]/8.0*(1<<k)),'f',1);
                p.setPen( qRgb(0xFF,0xFF,0xFF) );
                p.drawText( x-p.fontMetrics().width(str)/2+1,
                            y+h+10+p.fontMetrics().ascent()+1
                            +p.fontMetrics().height()*k, str );
                p.setPen( qRgb(0x00,0x00,0x00) );
                p.drawText( x-p.fontMetrics().width(str)/2,
                            y+h+10+p.fontMetrics().ascent()
                            +p.fontMetrics().height()*k, str );
            }
    }
    //	y=y0+h+10+p.fontMetrics().height()*5;
    //	p.setFont(QFont("System",12));
    //	p.drawText(x0,y,"Frequancy [Hz]:");
    // Save iimage to memory
    picture.save(&picBuffer, "PNG");
    picBuffer.close();

	 scaleChanged = false;
}

void LogView::paintEvent(QPaintEvent *)
{

    if( scaleChanged )  {
		 drawScale();
		 scaleChanged = false;
	 }

	 if( do_deletearrow ) {
		 do_deletearrow = false;
	 }

	 if( do_drawarrow ) {
		 do_drawarrow = false;
	 }

    QImage newPic;
    newPic.loadFromData(picArray, "PNG");
    QPainter outP(this);
    outP.drawImage(0,0,newPic);
}

//   void keyPressEvent(QKeyEvent *);
//   void keyReleaseEvent(QKeyEvent *);

void LogView::mousePressEvent(QMouseEvent *mouse)
{
    grundton=pos_note(mouse->x());
    update();
}
