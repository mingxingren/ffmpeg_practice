#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(CPlayerThread * _pPlayerThread, QWidget *parent)
    : QMainWindow(parent)
    , m_pPlayerThread(_pPlayerThread)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(m_pPlayerThread, &CPlayerThread::SIGNAL_FrameRGB, ui->widget, &CVideoWidget::SLOT_UpdateFrame);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    ui->widget->Save();
}

