#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("LeeSaaR's Code Editor");
    highlighter = new PythonHighlighter(ui->codeEditor->document());
}

MainWindow::~MainWindow()
{
    delete ui;
}

