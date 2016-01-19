#include "gui.h"

#include <QApplication>
#include <QString>
#include <QTimer>
#include <QEvent>
#include <QKeyEvent>
#include <QSound>

#include <QMenuBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QDockWidget>

GUI::GUI(QWidget *parent) : QMainWindow(parent)
{
    this->setMinimumSize(640, 480);
    this->setWindowTitle(tr("Chip8Emulator"));

    this->createActions();
    this->createMenus();

    textView = new QTextBrowser(this);
    textView->setFontPointSize(6);

    dockWidget = new QDockWidget(tr("Emulator state"), this);
    infoView = new QTextBrowser(this);
    infoView->setFontPointSize(10);
    dockWidget->setWidget(infoView);

    this->setCentralWidget(textView);
    this->addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    chip8_emu = new chip8();
    memset(key, 0, sizeof(char) * 16);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(executeOneCycle()));
}

GUI::~GUI()
{
    timer->stop();
    delete chip8_emu;
}

void GUI::createActions()
{
    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    exitAct = new QAction(QIcon(":/images/exit.png"), tr("&Exit"), this);
    exitAct->setStatusTip(tr("Exit emulator"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(exit()));

    aboutAct = new QAction(QIcon(":/images/about.png"), tr("About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
}

void GUI::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File(F)"));
    fileMenu->addAction(openAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    helpMenu = menuBar()->addMenu(tr("&Help(H)"));
    helpMenu->addAction(aboutAct);
}

void GUI::open()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
    {
        chip8_emu->initialize();
        chip8_emu->loadGame(fileName.toStdString().c_str());

        timer->start(10);
    }
}

void GUI::exit()
{
    timer->stop();
    QApplication::quit();
}

void GUI::about()
{
    QMessageBox::about(this, tr("Chip8Emulator"),
             tr("Chip8Emulator 0.8 (MinGW 32bit)\nBuilt on 2016/1/18\n\nCopyright 2016 NCKU CSIE 陳冠斌. All rights reserved."));
}

bool GUI::event(QEvent *event)
{
    if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        switch (ke->key())
        {
        case Qt::Key_1:
            key[0x1] = 0;
            return true;
        case Qt::Key_2:
            key[0x2] = 0;
            return true;
        case Qt::Key_3:
            key[0x3] = 0;
            return true;
        case Qt::Key_4:
            key[0xC] = 0;
            return true;
        case Qt::Key_Q:
            key[0x4] = 0;
            return true;
        case Qt::Key_W:
            key[0x5] = 0;
            return true;
        case Qt::Key_E:
            key[0x6] = 0;
            return true;
        case Qt::Key_R:
            key[0xD] = 0;
            return true;
        case Qt::Key_A:
            key[0x7] = 0;
            return true;
        case Qt::Key_S:
            key[0x8] = 0;
            return true;
        case Qt::Key_D:
            key[0x9] = 0;
            return true;
        case Qt::Key_F:
            key[0xE] = 0;
            return true;
        case Qt::Key_Z:
            key[0xA] = 0;
            return true;
        case Qt::Key_X:
            key[0x0] = 0;
            return true;
        case Qt::Key_C:
            key[0xB] = 0;
            return true;
        case Qt::Key_V:
            key[0xF] = 0;
            return true;
        default:
            return QWidget::event(event);
        }
    }
    else if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        //memset(key, 0, sizeof(char) * 16);
        switch (ke->key())
        {
        case Qt::Key_1:
            key[0x1] = 1;
            return true;
        case Qt::Key_2:
            key[0x2] = 1;
            return true;
        case Qt::Key_3:
            key[0x3] = 1;
            return true;
        case Qt::Key_4:
            key[0xC] = 1;
            return true;
        case Qt::Key_Q:
            key[0x4] = 1;
            return true;
        case Qt::Key_W:
            key[0x5] = 1;
            return true;
        case Qt::Key_E:
            key[0x6] = 1;
            return true;
        case Qt::Key_R:
            key[0xD] = 1;
            return true;
        case Qt::Key_A:
            key[0x7] = 1;
            return true;
        case Qt::Key_S:
            key[0x8] = 1;
            return true;
        case Qt::Key_D:
            key[0x9] = 1;
            return true;
        case Qt::Key_F:
            key[0xE] = 1;
            return true;
        case Qt::Key_Z:
            key[0xA] = 1;
            return true;
        case Qt::Key_X:
            key[0x0] = 1;
            return true;
        case Qt::Key_C:
            key[0xB] = 1;
            return true;
        case Qt::Key_V:
            key[0xF] = 1;
            return true;
        default:
            return QWidget::event(event);
        }
    }

    return QWidget::event(event);
}

void GUI::executeOneCycle()
{
    QString infoStr;
    infoStr.sprintf("PC: 0x%x\n", chip8_emu->getPC());
    infoView->setText(infoStr);

    chip8_emu->emulateCycle();

    QString str;
    if (chip8_emu->drawFlag) {

        for (int y = 0; y < 32; ++y)
        {
            for (int x = 0; x < 64; ++x)
            {
                str.append((chip8_emu->gfx[(y*64) + x] == 0) ? "　" : "▉");
            }
            str.append("\n");
        }
        str.append("\n");

        textView->setText(str);
        chip8_emu->drawFlag = false;
    }

    if (chip8_emu->isBeep) {
        QSound::play("bells.wav");
        chip8_emu->isBeep = false;
    }

    chip8_emu->setKeys(key);

}
