#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow* MainWindow::mainwindow=NULL;

MainWindow* MainWindow::getMainWindow(){
    return mainwindow;
}

bool MainWindow::instanceExists()
{
    return (MainWindow::getMainWindow() != NULL || dynamic_cast<MainWindow*>(MainWindow::getMainWindow()) != NULL);
}

void MainWindow::errorWindow(QString errorText)
{
    if ( !MainWindow::instanceExists() )
    {
        return;
    }
    QMessageBox messageBox;
    messageBox.critical(0,"Error",errorText);
    messageBox.setFixedSize(500,200);
}

void MainWindow::textWindow(QString text)
{
    if ( !MainWindow::instanceExists() )
    {
        return;
    }
    QMessageBox messageBox;
    messageBox.information(0,"Information",text);
    messageBox.setFixedSize(500,200);
}

MainWindow::MainWindow(Qt::WindowFlags window_flags, QWidget *parent) :
    QMainWindow(parent, window_flags),
    ui(new Ui::MainWindow)
{
    _dockLV = NULL;
    _dockCNC = NULL;
    _dockHost = NULL;
    mainwindow = this;
    ui->setupUi(this);
    ui->statusBar->addPermanentWidget( &_connectedLed);
    _statusMessage = new QLabel(this);
    ui->statusBar->addPermanentWidget( _statusMessage );

    _polybox = PolyboxModule::getInstance( this );
    connect(_polybox, SIGNAL(updateHardware()),this,SLOT(updateHardware()));

    this->setAttribute(Qt::WA_DeleteOnClose);
    //_polybox->connectToPrinter();


    _joypadActivated = false;
    _webcam = NULL;
    _dockLV = NULL;

    /**             ATU BUTTON              **/
    _atuON = !Polyplexer::isConnected();
    toggleATU();
    _atu = new ATUButton( 60, 30, _atuON, this );
    _atu->setGeometry( this->width()-_atu->width()-10,
                     ui->menuBar->height()+2,
                     _atu->width(), _atu->height());
    connect ( _atu, SIGNAL(released()), this, SLOT(toggleATU()));

    _atu->setEnabled( _atuON );

    changeStatePage( Start );
    
    setupLanguage();
    setupThemes();
    setupWebcamMenu();
    setupSerialMenu();

    MaintenanceChecker::check();
    _polyfabscan = PolyFabScanWindow::getInstance();
    _polyfabscan->setWindowFlags(Qt::NoDropShadowWindowHint| Qt::Window);
    //_polyfabscan->setAttribute(Qt::WA_DeleteOnClose);
}

void MainWindow::destroyWindow(QObject* obj)
{
    if ( QWidget* w = dynamic_cast<QWidget*>(obj) )
    {
        if ( w->windowTitle().contains("LabView", Qt::CaseInsensitive) )
        {
            _dockLV = NULL;
        }
        if ( w->windowTitle().contains("Host", Qt::CaseInsensitive) )
        {
            _dockHost = NULL;
        }

    }
}

void MainWindow::displayStatusMessage(QString mess)
{
    _statusMessage->setText( mess );
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if ( Polyplexer::isConnected() )
    {
        DialogWidget m_close( tr("La connexion est active avec la machine."
                                 "Quitter le programme va intérompte toute impréssion ou usinage en activité. ")
                              , this );
        if ( m_close.exec() )
        {
            Polyplexer::getInstance( this )->disConnect(); // @todo internal_
            //Polyplexer::getInstance()->stop();  // @todo master
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
    else
    {
        event->accept();
    }
}

void MainWindow::startConsoleWindow()
{
    if ( Polyplexer::isConnected() )
    {
        Console* c = new Console();
        c->setWindowTitle(tr("Console"));
        c->setAttribute(Qt::WA_DeleteOnClose);
        c->show();
    }
}

void MainWindow::restartApp()
{
    QProcess::startDetached(QApplication::applicationFilePath());
    exit(12);
}

void MainWindow::translateApp()
{
    if ( QAction* act = dynamic_cast<QAction*>(sender()) )
    {
        QSettings().setValue("lang",act->text());
        this->restartApp();
    }
}
void MainWindow::changeTheme()
{
    if ( QAction* act = dynamic_cast<QAction*>(sender()) )
    {
        Theme::installTheme( act->text() );
        qApp->processEvents();
        this->restartApp();
    }
}

void MainWindow::setupThemes()
{
    QAction* act;

    act = ui->menuThemes->addAction("Polybox3D");
    connect(act,SIGNAL(triggered()),this, SLOT(changeTheme()));
    act = ui->menuThemes->addAction("Fusion White");
    connect(act,SIGNAL(triggered()),this, SLOT(changeTheme()));

    act = ui->menuThemes->addAction("Fusion Dark");
    connect(act,SIGNAL(triggered()),this, SLOT(changeTheme()));

    act = ui->menuThemes->addAction("Fusion Purple");
    connect(act,SIGNAL(triggered()),this, SLOT(changeTheme()));

    act = ui->menuThemes->addAction("Fusion Clean");
    connect(act,SIGNAL(triggered()),this, SLOT(changeTheme()));

    act = ui->menuThemes->addAction("White");
    connect(act,SIGNAL(triggered()),this, SLOT(changeTheme()));

}

void MainWindow::setupLanguage()
{
    QDir directory( Config::translationPath() );
    QStringList ts_files = directory.entryList(QStringList("*.ts")) ;
    QString lang;
    QAction* act;

    foreach( QString file, ts_files)
    {
        lang = file.split(".").first().split("_").last();
        act = ui->menuLangage->addAction( lang );
        connect(act,SIGNAL(triggered()),this, SLOT(translateApp()));
    }

}

void MainWindow::toggleATU()
{
    _atuON = !_atuON;
    /*this->centralWidget()->setEnabled( _atuON );
    if ( _dockLV != NULL )
    {
        _dockLV->setEnabled( _atuON );
    }*/
}

MainWindow::~MainWindow()
{
    Logger::stop();
    if ( _polyfabscan != NULL )
    {
        delete _polyfabscan;
    }
    if ( Polyplexer::getInstance()->isConnected() )
    {
        Polyplexer::getInstance()->stop();
    }
    //delete ui;
}
void MainWindow::setupSerialMenu()
{

    ui->menuConnexion->clear();
    QAction* act;
    foreach ( QString serial, SerialPort::getDevicesNames(Config::pathToSerialDevice(), "tty*"))
    {
        if ( serial.startsWith("tty"))
        {
            if ( ! serial.startsWith( "ttyS" ))
            {
                if ( serial.startsWith("ttyACM") || serial.startsWith("ttyUSB") )
                {
                    act = ui->menuConnexion->addAction( serial );
                    act->setCheckable( true );
                    /*if ( Polyplexer::connectorType() == Polyplexer::Serial )
                    {*/
                        connect( act, SIGNAL(triggered()),this,SLOT(startConnexion()) );
                   // }
                    if ( Polyplexer::connectorType() == Polyplexer::Serial  && ! serial.compare(static_cast<SerialPort*>(Polyplexer::getInstance()->connector())->name() ) && Polyplexer::isConnected()  ) // We are already connected to this serial !
                    {
                        act->setChecked( true );
                    }
                }
            }
        }
    }
}


void MainWindow::startConnexion()
{
    if ( QAction* act = dynamic_cast<QAction*>(sender()) )
    {
        Polyplexer* poly = Polyplexer::getInstance();

        if ( poly->connectorType() == Polyplexer::Serial && ! static_cast<SerialPort*>(poly->connector())->name().compare( act->text().split('/').last() ) && poly->isConnected() ) //already connected
        {
            _atu->setState( true ); // true => activate ATU
            _atu->setEnabled( false );
            poly->disConnect();
            act->setChecked(false);

        }
        else
        {
            //@todo poly->disconnect();
            Config::setSerialPortName( act->text().split('/').last() );

            int connected = ComModule::getInstance()->connectionGUI( Config::blockedConnectionThread() );
            _atu->setEnabled( connected );
            _atu->setState( !connected ); // false => ATU off, machine works/ON
            //act->setChecked( connected ) ;
        }
    }
}

void MainWindow::setupWebcamMenu()
{
    // We add all camera available
    ui->menuVisualiser->clear();
    QAction* act;
    foreach ( QString camera, LabViewModule::getAllCamera( Config::pathToWebcamDevice() ))
    {
        act = ui->menuVisualiser->addAction( camera );
        connect( act, SIGNAL(triggered()),this,SLOT(startCamera()) );
    }
}

void MainWindow::startCamera()
{
    if ( QAction* act = dynamic_cast<QAction*>(sender()) )
    {
        _webcam = LabViewModule::startCamera( _webcam, act->text());
    }
}
void MainWindow::updateHardware()
{
    setupSerialMenu();
    if ( Polyplexer::connectorType() == Polyplexer::Serial )
    {
        ui->actionMode_Serveur->setVisible( true );
        ui->actionStart_Client_Mode->setVisible( true );
        if ( Polyplexer::isConnected() )
        {
            _connectedLed.setActivated( true );
            this->displayStatusMessage( tr("Status : Connected") );
        }
        else
        {
            _connectedLed.setActivated( false );
            this->displayStatusMessage( tr("Status : Unconnected") );

        }
    }
    else if ( Polyplexer::isConnected() && _tcp_server.isListening() )
    {
        ui->actionMode_Serveur->setVisible( true );
        ui->actionStart_Client_Mode->setVisible( false );
        this->displayStatusMessage( tr("Status : Mode Server") );
    }
    else if ( Polyplexer::connectorType() == Polyplexer::Tcp )
    {
        ui->actionMode_Serveur->setVisible( false );
        ui->actionStart_Client_Mode->setVisible( true );
        this->displayStatusMessage( tr("Status : Mode Client") );
    }
    else
    {
        _connectedLed.setActivated( false );
        this->displayStatusMessage( tr("Status : Unconnected") );
    }

    setupWebcamMenu();
}

void MainWindow::changeStatePage(PageState new_state)
{
    _previousState = _currentState;
    _currentState = new_state;

    //No Welcome page, skip this step and go to Module Page
#ifndef WELCOME_PAGE
    if ( _currentState == Start )
    {
        _currentState = Module;
    }
#endif
    updateStatePage();
}

void MainWindow::updateStatePage()
{

    switch ( _currentState )
    {
    case Start:
        delete this->centralWidget();
        qApp->processEvents();

        this->setCentralWidget( new OpenPage( _polybox, this ) );
        connect(this,SIGNAL(joypadOff()),this->centralWidget(),SLOT(disableJoypad()));
        connect(this,SIGNAL(joypadOn(QJoystick*)),this->centralWidget(),SLOT(setJoypad(QJoystick*)));
        if ( _joypadActivated )
        {
            emit joypadOn( PolyboxModule::getJoypad() );
        }
        break;
    case Module :
        delete this->centralWidget();
        qApp->processEvents();

        this->setCentralWidget( new ModulePage( _polybox, this ) );

        connect(this,SIGNAL(joypadOff()),this->centralWidget(),SLOT(disableJoypad()));
        connect(this,SIGNAL(joypadOn(QJoystick*)),this->centralWidget(),SLOT(setJoypad(QJoystick*)));
        if ( _joypadActivated )
        {
            emit joypadOn( PolyboxModule::getJoypad() );
        }
        break;
    case Help :
        delete this->centralWidget();
        qApp->processEvents();

        this->setCentralWidget( new HelpPage( this ) );
        break;
    case Warning :
        delete this->centralWidget();
        qApp->processEvents();

        this->setCentralWidget( new WarningPage( _polybox, this ) );
        break;
    case LabView :
        delete this->centralWidget();
        qApp->processEvents();

        this->setCentralWidget( new LabViewPage( _polybox->labViewModule(), this, false ) );
        connect(this,SIGNAL(joypadOff()),this->centralWidget(),SLOT(disableJoypad()));
        connect(this,SIGNAL(joypadOn(QJoystick*)),this->centralWidget(),SLOT(setJoypad(QJoystick*)));
        if ( _joypadActivated )
        {
            emit joypadOn( PolyboxModule::getJoypad() );
        }
        break;
    case LabViewDock :
    {
        // already open ?
        if ( _dockLV != NULL )
            break;

        if (_polybox->isCommonReady() || Config::bypassCheck() )
        {
            _dockLV = new QDockWidget("LabView (dock) ",this);
            _dockLV->setWidget( new LabViewPage( _polybox->labViewModule(), this, true ) );
            _dockLV->setFloating( true );
            _dockLV->setAttribute(Qt::WA_DeleteOnClose);
            connect(_dockLV,SIGNAL(destroyed(QObject*)),this,SLOT(destroyWindow(QObject*)));
            _dockLV->show();
            _dockLV->setEnabled( _atuON );
        }
        else
        {
            CheckerModele* checker = new CheckerModele((QWidget*)this->parent());


            checker->setWindowTitle("Etat de la machine");
            checker->setContentWidget( new GlobalChecker( _polybox->globalModule(),  checker));
            _polybox->globalModule()->updateComponents();
            int value_ret = checker->exec();
            if ( value_ret != 0 )
            {
                CHANGE_PAGE( static_cast<PageState>(value_ret) );
            }

        }
    }
        break;
    case Scanner:
    {
        bool scanner_ok = _polybox->isScannerReady();
        if ( scanner_ok || Config::bypassCheck() )
        {
            CHANGE_PAGE( ScannerLaser );
           /* DialogScanner dialog((QWidget*)this->parent());
            int value_ret = dialog.exec();
            if ( value_ret != 0 )
            {
                CHANGE_PAGE( static_cast<PageState>(value_ret) );
            }*/
        }
        else
        {
            CheckerModele* checker = new CheckerModele((QWidget*)this->parent());

            checker->setWindowTitle("Etat du Scanner");
            checker->setContentWidget( new SCannerChecker( _polybox->scannerModule(), checker));
            int value_ret = checker->exec();
            if ( value_ret != 0 )
            {
                CHANGE_PAGE( static_cast<PageState>(value_ret) );
            }
        }

    }
        break;
    case Printer :
    {
        bool printer_ok = _polybox->isPrinterReady();
        if ( printer_ok || Config::bypassCheck() )
        {
            delete this->centralWidget();
            qApp->processEvents();

            this->setCentralWidget( new PrinterPage( _polybox->printerModule(), this ) );
            this->centralWidget()->setAttribute(Qt::WA_DeleteOnClose);

        }
        else
        {
            CheckerModele* checker = new CheckerModele((QWidget*)this->parent());

            checker->setWindowTitle("Etat de l'imprimante");
            checker->setContentWidget( new PrinterChecker( _polybox->printerModule(), checker));
            int value_ret = checker->exec();
            if ( value_ret != 0 )
            {
                CHANGE_PAGE( static_cast<PageState>(value_ret) );
            }
        }
    }
        break;
    case CNC :
    {
        // Is LinuxCNC already running/opened ?
        if ( _polybox->cncModule()->isRunningLinuxCNC() )
        {
            break;
        }
        bool cnc_ok = _polybox->isCncReady();
        if ( cnc_ok || Config::bypassCheck() )
        {
            DialogCNC dialog((QWidget*)this->parent());
            int value_ret = dialog.exec();
            if ( value_ret == CNC )
            {
                _dockCNC = new QDockWidget("LinuxCNC ",this);
                _dockCNC->setWidget( new CNCPage( _polybox->cncModule(), _dockCNC ) );
                _dockCNC->setFloating( true );
                _dockCNC->setFixedHeight(230+15);
                _dockCNC->setFixedWidth(150);
                _dockCNC->show();
                _dockCNC->setEnabled( _atuON );

                //CHANGE_PAGE( static_cast<PageState>(value_ret) );
            }
            else if ( value_ret == Help )
            {
                CHANGE_PAGE( Help );
            }
            else if ( value_ret == ConfigureCNC )
            {

            }
        }
        else
        {
            CheckerModele* checker = new CheckerModele((QWidget*)this->parent());


            checker->setWindowTitle("Etat de la CN");
            checker->setContentWidget( new CNCChecker( _polybox->cncModule(), checker));
            int value_ret = checker->exec();
            if ( value_ret != 0 )
            {
                CHANGE_PAGE( static_cast<PageState>(value_ret) );
            }

        }
    }
        break;
    case ConfigureCNC :
        delete this->centralWidget();
        qApp->processEvents();

        this->setCentralWidget( new ConfigCNCPage( this ) );
        break;
    case ScannerLaser :
    {
#if !defined NO_SCAN
 //this->setCentralWidget( new FsMainWindow( _polybox->port(), this ) );

        if ( _polyfabscan->isVisible() )
            break;
        _polyfabscan->show();
        _polyfabscan->activateWindow();
        _polyfabscan->raise();
#endif
        /*QProcess* laser = new QProcess(this);
        laser->start( Config::scannerLaserPath );*/
        break;
    }
    case CalibratePlate :
        delete this->centralWidget();
        qApp->processEvents();

        this->setCentralWidget( new PlateCalibratorWidget( this ) );
        break;
    case DockPage :
    {
        if ( _dockHost != NULL )
            return;

        _dockHost = new DockHost( this );
        connect(_dockHost, SIGNAL(destroyed(QObject*)),this,SLOT(destroyWindow(QObject*)));
        _dockHost->show();
        break;
    }
    default:
        break;
        
    }

    _atu->raise();
    _atu->show();
    _atu->activateWindow();
    //this->centralWidget()->setEnabled( _atuON );
}

void MainWindow::on_actionActiver_Manette_triggered()
{
    if ( _joypadActivated )
    {
        ui->actionActiver_Manette->setText(tr("Activer manette"));
        _polybox->unloadJoypad();
        emit joypadOff();
    }
    else
    {
        if ( _polybox->loadJoypad() )
        {
            ui->actionActiver_Manette->setText(tr("Désactiver manette"));
            emit joypadOn( PolyboxModule::getJoypad() );
        }
        else
        {
            return;
        }
    }
    _joypadActivated = !_joypadActivated;
}

void MainWindow::backToModulePage()
{
    changeStatePage( Module );
}

void MainWindow::backToHelpPage()
{
    changeStatePage( Help );
}

void MainWindow::on_actionCNC_triggered()
{
    changeStatePage( CNC );
}

void MainWindow::on_actionImprimante_triggered()
{
    changeStatePage( Printer );
}

void MainWindow::on_actionScanner_triggered()
{
    changeStatePage( Scanner );
}

void MainWindow::on_actionLabView_triggered()
{
    changeStatePage( LabView );
}


void MainWindow::on_actionLabView_dock_triggered()
{
    changeStatePage( LabViewDock );
}

void MainWindow::on_actionConsole_triggered()
{
    this->startConsoleWindow();
}

void MainWindow::on_actionParametres_triggered()
{
    QDesktopServices::openUrl(QUrl(Config::pathToConfigFile()+"/"+QCoreApplication::organizationName()+"/"+QCoreApplication::applicationName()+".conf"));
}

void MainWindow::on_actionCredits_triggered()
{
    DialogCredits dialog( (QWidget*)this->parent());
    int value_ret = dialog.exec();
    if ( value_ret != 0 )
    {
    }
}

void MainWindow::on_actionMode_Serveur_toggled(bool arg1)
{
    if ( arg1 )
    {
        if ( Polyplexer::isConnected() )
        {
            _tcp_server.startListening( Config::serverListeningAddress(), Config::serverListeningPort() );
            ui->actionMode_Serveur->setText(tr("Stop Server Mode"));
            //_polybox->setConnectorType( PolyboxModule::ServerTCP );
            this->updateHardware();
        }
        else
        {
            ui->actionMode_Serveur->setChecked( false );
        }
    }
    else if ( _tcp_server.isListening() )
    {
        ui->actionMode_Serveur->setText(tr("Start Server Mode"));
        _tcp_server.stopListening();
      //  _polybox->setConnectorType( PolyboxModule::Serial );
        this->updateHardware();
    }
}

void MainWindow::on_actionStart_Client_Mode_toggled(bool arg1)
{
    if ( arg1 )
    {
        DialogClient dc( this );

        if ( dc.exec() )
        {
            QTcpSocket* client = new QTcpSocket();
            client->connectToHost( Config::broadcastIP(), Config::broadcastPort());
            //@todo Polyplexer::getInstance(this)->start( client, Polyplexer::Tcp);
            this->updateHardware();
        }
        else
        {
             ui->actionStart_Client_Mode->setChecked( false );
             Polyplexer::getInstance(this)->setConnectorType( Polyplexer::Serial );
             this->updateHardware();
        }
    }

}

void MainWindow::on_actionCalibration_Ecran_triggered()
{
    WarningPage::on_touchscreen_clicked();
}

void MainWindow::on_actionCalibration_Plateau_triggered()
{
    CHANGE_PAGE( CalibratePlate );
}
