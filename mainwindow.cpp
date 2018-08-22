#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QPainter>

#include <math.h>
#include <iostream>

#include "resources.h"
#include "logview.h"


//global
double KAMMERTON, KAMMERTON_LOG;

MainWindow::MainWindow(QWidget *parent) :    QMainWindow(parent),  ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    stop_audio();
    delete ui;
}

void MainWindow::init()
{
    setMinimumSize(490,240);

    // Set up sound input
    KAMMERTON=KAMMERTON_NORM;
    KAMMERTON_LOG=KAMMERTON_LOG_NORM;

    strcpy(alsa_devicename, "default");

    blksize=256; //might be a nice default, dunno :)
    sampnr=2048; //
    sampfreq=44100; // Best to be as high as possible!
    sampfreq_exact = sampfreq;
    processing_audio=0;

    ui->osziView->setSampleNr(sampnr);

    setSoundInputType(SoundInputType::ALSA);
    soundinput = 0;
    start_audio();  // sets soundinput

    freqs[0]=KAMMERTON; lfreqs[0]=KAMMERTON_LOG;
    for(int i=1;i<12;i++){
        freqs [i] = freqs [i-1] * D_NOTE;
        lfreqs[i] = lfreqs[i-1] + D_NOTE_LOG;
    }

    audioDialog = new AudioDialog(this);
    tuningsDialog = new TuningsDialog(this);
    scaleDialog = new ScaleDialog(this);

    //setCentralWidget(this);

    // Init checkboxes
    ui->actionShow_Toolbar->setChecked(true);
    ui->actionHide_Status_Bar->setChecked(false);
    ui->actionOscilloscope->setChecked(true);
    ui->actionLog_View->setChecked(true);
    ui->notchFilterCheckBox->setChecked(false);

    ui->sampleFreqSpinBox->setValue(sampfreq);
    ui->sampleNumberSpinBox->setValue(sampnr);

    // slots

    connect(tuningsDialog, SIGNAL(normalSignal()), this, SLOT(setTuningNorm()));
    connect(tuningsDialog, SIGNAL(physicalSignal()), this , SLOT(setTuningPhys()));
    connect(tuningsDialog, SIGNAL(wienSignal()), this, SLOT(setTuningWien()));
    connect(tuningsDialog, SIGNAL(naturalModeSignal()), this, SLOT(setTuningNat()));
    connect(tuningsDialog, SIGNAL(equidistantModeSignal()), this, SLOT(setTuningEqui()));
    connect(audioDialog, SIGNAL(alsaDeviceSignal(QString)), this, SLOT(setAlsaName(QString)));
    connect(audioDialog, SIGNAL(ossDeviceSignal(QString)), this, SLOT(setOSSName(QString)));
    connect(audioDialog, SIGNAL(useAlsaSignal()), this, SLOT(setAlsaInput()));
    connect(audioDialog, SIGNAL(useOSSSignal()), this, SLOT(setOSSInput()));
    connect(audioDialog, SIGNAL(useArtsSignal()), this, SLOT(setArtsInput()));
    connect(audioDialog, SIGNAL(useJACKSignal()), this, SLOT(setJackInput()));
    connect(scaleDialog, SIGNAL(scaleChanged(int)), this, SLOT(setScale(int)));

    timer = new QTimer(this);
    connect(  timer, SIGNAL(timeout()), this, SLOT(proc_audio()) );
    timer->start();
}

void MainWindow::on_actionQuit_triggered()
{
    this->close();
}

void MainWindow::on_actionShow_Toolbar_changed()
{
    if( ui->actionShow_Toolbar->isChecked() ) {
        ui->mainToolBar->show();
    }
    else {
        ui->mainToolBar->hide();
    }
}

void MainWindow::on_actionHide_Status_Bar_changed()
{

    if( ui->actionHide_Status_Bar->isChecked() ) {
        ui->statusBar->show();
    }
    else {
        ui->statusBar->hide();
    }
}

void MainWindow::on_actionOscilloscope_triggered()
{
    if( ui->actionOscilloscope->isChecked() ) {
        ui->osziView->show();
        ui->osziView->update();
    }
    else {
        ui->osziView->hide();
    }
}

void MainWindow::on_actionLog_View_triggered()
{
    if( ui->actionLog_View->isChecked() ) {
        ui->logView->show();
        ui->logView->update();
    }
    else {
        ui->logView->hide();
    }
}

void MainWindow::on_actionTuning_triggered()
{
    if( tuningsDialog->isVisible() ) {
        tuningsDialog->close();
    }
    else {
        tuningsDialog->raise();
        tuningsDialog->show();
    }

}

void MainWindow::on_actionScale_triggered()
{
    if( scaleDialog->isVisible() ) {
        scaleDialog->close();
    }
    else {
        scaleDialog->raise();
        scaleDialog->show();
    }
}

void MainWindow::on_actionAudio_Settings_triggered()
{
    if( audioDialog->isVisible() ) {
        audioDialog->close();
    }
    else {
        audioDialog->raise();
        audioDialog->show();
    }
}

void MainWindow::on_actionManual_triggered()
{

}

void MainWindow::on_actionWhats_this_triggered()
{

}

void MainWindow::on_actionReport_Bug_triggered()
{

}

void MainWindow::on_actionChange_Language_triggered()
{

}

void MainWindow::on_actionAbout_Qt5GuiTune_triggered()
{

}
void MainWindow::setTuningNorm()
{
    KAMMERTON=KAMMERTON_NORM;
    KAMMERTON_LOG=KAMMERTON_LOG_NORM;
    ui->logView->update();
}

void MainWindow::setTuningWien()
{
    KAMMERTON=KAMMERTON_WIEN;
    KAMMERTON_LOG=KAMMERTON_LOG_WIEN;
    ui->logView->update();
}
void MainWindow::setTuningPhys()
{
    KAMMERTON=KAMMERTON_PHYS;
    KAMMERTON_LOG=KAMMERTON_LOG_PHYS;
    ui->logView->update();
}


void MainWindow::setTuningEqui()
{
    ui->logView->nat_tuning_on=false;
    ui->logView->update();
}

void MainWindow::setScale(int scale ) {
    ui->logView->setScale(scale);
    ui->logView->update();
}

void MainWindow::setScaleUS()
{
    ui->logView->setScale(LogView::us_scale);
    ui->logView->update();
}

void MainWindow::setTuningNat()
{
    ui->logView->nat_tuning_on=true;
    ui->logView->update();
}

void MainWindow::setScaleUSAlt()
{
    ui->logView->setScale(LogView::us_scale_alt);
}

void MainWindow::setScaleGE()
{
    ui->logView->setScale(LogView::german_scale);
}

void MainWindow::setScaleGEAlt()
{
    ui->logView->setScale(LogView::german_scale_alt);
}

void MainWindow::setScaleFR()
{
    ui->logView->setScale(LogView::french_scale);
}

void MainWindow::setScaleFRAlt()
{
    ui->logView->setScale(LogView::french_scale_alt);
}

void MainWindow::showlogView()
{
    ui->logView->show();
    resizeEvent(0);   // rearrange widgets
}

void MainWindow::hidelogView()
{
    ui->logView->hide();
    resizeEvent(0);
}

void MainWindow::showOszi()
{
    ui->osziView->show();
    resizeEvent(0);
}

void MainWindow::hideOszi()
{
    ui->osziView->hide();
    resizeEvent(0);
}

void MainWindow::setSampFreq(int f)
{
    sampfreq = f;

    must_restart_audio = true;
}

void MainWindow::setSampNr(int nr)
{
    sampnr=nr;
    ui->osziView->setSampleNr(nr);
    emit signalSampNrChanged();
}

// Sets whether notch filter is done or not, 
// depending on the state of the   notch checkbox.
//
void MainWindow::setNotch() {
    ui->osziView->setNotch(ui->notchFilterCheckBox->isChecked());
}

// These functions can't be entirely ifdeffed away because they're slots, 
// and moc doesn't like ifdeffed-out slots

void MainWindow::setOSSName(QString name) {

    strcpy(dsp_devicename, name.toStdString().c_str());
    must_restart_audio = true;
}

void MainWindow::setAlsaName( QString name )
{
    strcpy(alsa_devicename, name.toStdString().c_str());
    must_restart_audio = true;
}

void MainWindow::showAudioDialog( void )
{
    AudioDialog *audiodialog = new AudioDialog(this);
    audiodialog->exec();
    delete audiodialog;

    if (must_restart_audio)
    {
        stop_audio();
        start_audio();
        must_restart_audio = false;
    }
}

/*
 * Following getSomething() methods are for now meant for the K2GuituneApp::saveOptions() thing
 */

QString MainWindow::getOSSName( void )
{
    return( QString(dsp_devicename) );
}


QString MainWindow::getALSAName( void )
{
    return (QString(alsa_devicename));
}


MainWindow::SoundInputType MainWindow::getSoundInputType( void )
{
    return soundinputtype;
}

void MainWindow::setSoundInputType( MainWindow::SoundInputType _soundinputtype )
{
    if (_soundinputtype != soundinputtype)
    {
        soundinputtype = _soundinputtype;
        must_restart_audio=true;
    }
}

void MainWindow::setAlsaInput() {
    setSoundInputType(SoundInputType::ALSA);
}

void MainWindow::setOSSInput() {
    setSoundInputType((SoundInputType::OSS));
}

void MainWindow::setJackInput() {
    setSoundInputType(SoundInputType::JACK);
}

void MainWindow::setArtsInput() {
    setSoundInputType(SoundInputType::aRts);
}

void MainWindow::start_audio( void )
{
    /* the numerical arguments to the SoundInput::* functions are passed by
   * reference so they get set to the right values */
    switch( soundinputtype )
    {
    case autodetect:
        soundinput = SoundInput::autodetect(alsa_devicename, dsp_devicename, blksize, sampfreq, sampfreq_exact);
        break;

    case ALSA:
        try {
        soundinput = new ALSASoundInput( alsa_devicename, blksize, sampfreq, sampfreq_exact );
    } catch (SoundInputException) {
            soundinput = 0;
        }
        break;
    }

    if (!soundinput) {
        std::cerr << "No Sound Input Found" << std::endl;

    }
}

void MainWindow::stop_audio( void )
{
    if (soundinput)
    {
        delete soundinput;
    }
    timer->stop();
}

void MainWindow::proc_audio()
{
    int i,j,n,trig,trigpos;
    static int k=0;
    short int *s;
    short int *sample_s16le;
    double ldf,mldf;
    char str[50];
    processing_audio=1;
    trigpos=0;
    sample_s16le = (short int *)sample;
    s = sample_s16le;
    n=0;

    n = soundinput->getData(s, blksize);
    //std::cout << n << " bytes of sound." << std::endl;

    for( i=0; i<n && Abs(s[i])<256; i++ );

    //i=-1;
    j=0; trig=0;
    if (i<n)
        do {
        for( ; i<n-1; i++ )   /* n-1 because of POSTRIG uses i+1 */
            if ( POSTRIG_S16(s,i) ) { trig=1; trigpos=i; }
        if( trig==0 ){
            n = soundinput->getData(s, blksize);
            //std::cout << n << " bytes of sound (inner)." << std::endl;
            j++;
            i=0;
        }
    } while( (!trig) && j<100 );
    
    //   else printf("No Signal %d\n",blksize);
    //
    if( trig ){
        for( i=n-trigpos; i<sampnr; i+=n ){
            s+=n;
            n = soundinput->getData(s, blksize);
            //std::cout << n << " bytes of sound (trig)." << std::endl;
        }

        // This method sets samp (the osziview sample array)
        ui->osziView->setSamplePtr( &sample_s16le[trigpos] );
        ui->osziView->setSampleNr( sampnr );
        ui->osziView->paintSample();

        freq_0t = (double)sampfreq*ui->osziView->getfreq_s();

        emit toneAvailable(freq_0t);

        lfreq_0t = log(freq_0t);
        while ( lfreq_0t < lfreqs[0]-D_NOTE_LOG/2.0 ) lfreq_0t+=LOG_2;
        while ( lfreq_0t >= lfreqs[0]+LOG_2-D_NOTE_LOG/2.0 ) lfreq_0t-=LOG_2;
        mldf=D_NOTE_LOG; note_0t=0;
        for( i=0; i<12; i++ ) {
            ldf = fabs(lfreq_0t-lfreqs[i]);
            if (ldf<mldf) { mldf=ldf; note_0t=i; }
        }

        ui->logView->change_lfreq(lfreq_0t);
        sprintf(str,"%0.3f",freq_0t);

        ui->freqView->display(str);

        double nfreq_0t=freqs[note_0t];
        while( nfreq_0t/freq_0t > D_NOTE_SQRT ) nfreq_0t/=2.0;
        while( freq_0t/nfreq_0t > D_NOTE_SQRT ) nfreq_0t*=2.0;
        sprintf(str,"%0.3f",nfreq_0t);

        ui->nfreqView->display(str);

        //      printf("Note: %s (%lfHz) Freq=%lf\n",
        //              note[note_0t],freqs[note_0t],freq_0t);
    }
    k++;
    processing_audio=0;
    update();
}




