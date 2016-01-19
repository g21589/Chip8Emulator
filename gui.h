#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include <QTextBrowser>

#include "chip8.h"

class GUI : public QMainWindow
{
    Q_OBJECT

public:
    GUI(QWidget *parent = 0);
    ~GUI();

protected:
    bool event(QEvent *event);

private slots:
    void open();
    void exit();
    void about();
    void executeOneCycle();

private:
    void createActions();
    void createMenus();

    QMenu *fileMenu;
    QMenu *helpMenu;
    QAction *openAct;
    QAction *exitAct;
    QAction *aboutAct;

    QDockWidget *dockWidget;
    QTextBrowser *textView;
    QTextBrowser *infoView;

    QTimer *timer;

    chip8 *chip8_emu;
    unsigned char key[16];
};

#endif // GUI_H
