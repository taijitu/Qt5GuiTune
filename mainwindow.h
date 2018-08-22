#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QComboBox>
#include "audiodialog.h"
#include "tuningsdialog.h"
#include "scaledialog.h"
#include "logview.h"
#include "osziview.h"
#include "soundinput.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    enum SoundInputType {
        autodetect = 0
        ,OSS = 1
        ,ALSA = 2
        ,aRts = 3
        ,JACK = 4
    };
    int sampnr;
    int sampfreq;
    double sampfreq_exact;
    int blksize;
    int trig1;
    int trig2;
    int    note_0t;
    int    note_ht;
    double freq_0t;
    double freq_ht;
    double lfreq_0t;
    double lfreq_ht;
    void showOszi();
    void hideOszi();
    void hideLogView();
    bool getNotch();

    SoundInputType getSoundInputType( void );
    void setSoundInputType( SoundInputType _soundinputtype );
    void start_audio( void );

public slots:

    void setSampFreq(int f);
    void setSampNr(int f);
    void setNotch();

    void setTuningNorm();
    void setTuningWien();
    void setTuningPhys();
    void setTuningEqui();
    void setTuningNat();
    void setScale(int);
    void setScaleUS();
    void setScaleUSAlt();
    void setScaleGE();
    void setScaleGEAlt();
    void setScaleFR();
    void setScaleFRAlt();
    void showAudioDialog( void );
    void showlogView();
    void hidelogView();
    void setAlsaInput();
    void setOSSInput();
    void setJackInput();
    void setArtsInput();
    QString getOSSName( void );
    QString getALSAName( void );


private slots:
    void on_actionQuit_triggered();
    void on_actionShow_Toolbar_changed();
    void on_actionHide_Status_Bar_changed();
    void on_actionOscilloscope_triggered();
    void on_actionLog_View_triggered();
    void on_actionTuning_triggered();
    void on_actionScale_triggered();
    void on_actionAudio_Settings_triggered();
    void on_actionManual_triggered();
    void on_actionWhats_this_triggered();
    void on_actionReport_Bug_triggered();
    void on_actionChange_Language_triggered();
    void on_actionAbout_Qt5GuiTune_triggered();
    void setOSSName(QString);
    void setAlsaName(QString);
    void proc_audio();

signals:
    void signalSampFreqChanged();
    void signalSampNrChanged();
    void signalNotchChanged();
    void toneAvailable(double freq);

private:
    Ui::MainWindow *ui;
    void init();
    AudioDialog *audioDialog;
    TuningsDialog *tuningsDialog;
    ScaleDialog *scaleDialog;

    QTimer *timer;
    void stop_audio();

    bool must_restart_audio;
    char dsp_devicename[100];
    char alsa_devicename[100];
	 
    unsigned char sample[64000];
    double freqs[12];
    double lfreqs[12];
    int    processing_audio;
    SoundInputType soundinputtype;
    SoundInput *soundinput;
};

#endif // MAINWINDOW_H
