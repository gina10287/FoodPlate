/********************************************************************************
** Form generated from reading UI file 'QtGuiApplication1.ui'
**
** Created by: Qt User Interface Compiler version 5.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTGUIAPPLICATION1_H
#define UI_QTGUIAPPLICATION1_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QtGuiApplication1Class
{
public:
    QAction *actionSelect_an_image;
    QAction *actionExit;
    QAction *actionBackground;
    QWidget *centralWidget;
    QGridLayout *gridLayout_2;
    QHBoxLayout *horizontalLayout_3;
    QHBoxLayout *horizontalLayout_2;
    QTabWidget *tabWidget;
    QWidget *tab;
    QHBoxLayout *horizontalLayout;
    QTreeWidget *treeWidget;
    QWidget *tab_2;
    QGridLayout *gridLayout;
    QTreeWidget *treeWidget_2;
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget_3;
    QWidget *tab_3;
    QGridLayout *gridLayout_3;
    QTableWidget *tableWidget_selectedFood;
    QWidget *tab_4;
    QGridLayout *gridLayout_4;
    QTableWidget *tableWidget_3;
    QGridLayout *gridLayout_5;
    QTableWidget *tableWidget_2;
    QPushButton *ApplyButton;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_4;
    QTabWidget *tabWidget_2;
    QWidget *tab_5;
    QGridLayout *gridLayout_6;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_show;
    QFrame *line;
    QLabel *label_input;
    QWidget *tab_6;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label;
    QPushButton *scaleBtn_0;
    QPushButton *scaleBtn_1;
    QPushButton *scaleBtn_2;
    QLabel *label_4;
    QProgressBar *progressBar;
    QTableWidget *tableWidget_6;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *RunRecipeButton;
    QPushButton *RunImageButton;
    QPushButton *stopButton;
    QPushButton *ExitButton;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *QtGuiApplication1Class)
    {
        if (QtGuiApplication1Class->objectName().isEmpty())
            QtGuiApplication1Class->setObjectName(QStringLiteral("QtGuiApplication1Class"));
        QtGuiApplication1Class->resize(1097, 678);
        QFont font;
        font.setKerning(true);
        QtGuiApplication1Class->setFont(font);
        actionSelect_an_image = new QAction(QtGuiApplication1Class);
        actionSelect_an_image->setObjectName(QStringLiteral("actionSelect_an_image"));
        QFont font1;
        font1.setPointSize(15);
        font1.setBold(false);
        font1.setWeight(50);
        actionSelect_an_image->setFont(font1);
        actionExit = new QAction(QtGuiApplication1Class);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        QFont font2;
        font2.setPointSize(15);
        actionExit->setFont(font2);
        actionBackground = new QAction(QtGuiApplication1Class);
        actionBackground->setObjectName(QStringLiteral("actionBackground"));
        centralWidget = new QWidget(QtGuiApplication1Class);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        gridLayout_2 = new QGridLayout(centralWidget);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(10, 10, 10, 10);
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setBaseSize(QSize(0, 0));
        QFont font3;
        font3.setFamily(QStringLiteral("Times New Roman"));
        font3.setPointSize(17);
        font3.setBold(true);
        font3.setWeight(75);
        tabWidget->setFont(font3);
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        horizontalLayout = new QHBoxLayout(tab);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        treeWidget = new QTreeWidget(tab);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QStringLiteral("1"));
        treeWidget->setHeaderItem(__qtreewidgetitem);
        treeWidget->setObjectName(QStringLiteral("treeWidget"));
        QFont font4;
        font4.setBold(true);
        font4.setWeight(75);
        treeWidget->setFont(font4);
        treeWidget->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);
        treeWidget->setDragEnabled(true);
        treeWidget->setDragDropMode(QAbstractItemView::DragOnly);
        treeWidget->setSelectionMode(QAbstractItemView::NoSelection);
        treeWidget->header()->setStretchLastSection(true);

        horizontalLayout->addWidget(treeWidget);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        gridLayout = new QGridLayout(tab_2);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        treeWidget_2 = new QTreeWidget(tab_2);
        QTreeWidgetItem *__qtreewidgetitem1 = new QTreeWidgetItem();
        __qtreewidgetitem1->setText(0, QStringLiteral("1"));
        treeWidget_2->setHeaderItem(__qtreewidgetitem1);
        treeWidget_2->setObjectName(QStringLiteral("treeWidget_2"));
        treeWidget_2->setFont(font4);
        treeWidget_2->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);
        treeWidget_2->setDragEnabled(true);
        treeWidget_2->setDragDropOverwriteMode(true);
        treeWidget_2->setDragDropMode(QAbstractItemView::DragOnly);
        treeWidget_2->setDefaultDropAction(Qt::IgnoreAction);
        treeWidget_2->setSelectionMode(QAbstractItemView::NoSelection);

        gridLayout->addWidget(treeWidget_2, 0, 0, 1, 1);

        tabWidget->addTab(tab_2, QString());

        horizontalLayout_2->addWidget(tabWidget);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        tabWidget_3 = new QTabWidget(centralWidget);
        tabWidget_3->setObjectName(QStringLiteral("tabWidget_3"));
        tabWidget_3->setFont(font3);
        tab_3 = new QWidget();
        tab_3->setObjectName(QStringLiteral("tab_3"));
        gridLayout_3 = new QGridLayout(tab_3);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        tableWidget_selectedFood = new QTableWidget(tab_3);
        tableWidget_selectedFood->setObjectName(QStringLiteral("tableWidget_selectedFood"));
        QFont font5;
        font5.setFamily(QStringLiteral("Times New Roman"));
        font5.setPointSize(16);
        font5.setBold(true);
        font5.setItalic(true);
        font5.setWeight(75);
        tableWidget_selectedFood->setFont(font5);
        tableWidget_selectedFood->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidget_selectedFood->setDragEnabled(true);
        tableWidget_selectedFood->setDragDropOverwriteMode(false);
        tableWidget_selectedFood->setDragDropMode(QAbstractItemView::DragOnly);
        tableWidget_selectedFood->setSelectionMode(QAbstractItemView::SingleSelection);
        tableWidget_selectedFood->setRowCount(0);
        tableWidget_selectedFood->setColumnCount(0);

        gridLayout_3->addWidget(tableWidget_selectedFood, 0, 0, 1, 1);

        tabWidget_3->addTab(tab_3, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QStringLiteral("tab_4"));
        gridLayout_4 = new QGridLayout(tab_4);
        gridLayout_4->setSpacing(6);
        gridLayout_4->setContentsMargins(11, 11, 11, 11);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        tableWidget_3 = new QTableWidget(tab_4);
        tableWidget_3->setObjectName(QStringLiteral("tableWidget_3"));
        QFont font6;
        font6.setFamily(QStringLiteral("Times New Roman"));
        font6.setPointSize(16);
        font6.setBold(true);
        font6.setWeight(75);
        tableWidget_3->setFont(font6);
        tableWidget_3->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidget_3->setDragEnabled(true);
        tableWidget_3->setDragDropOverwriteMode(false);
        tableWidget_3->setDragDropMode(QAbstractItemView::DragOnly);
        tableWidget_3->setSelectionMode(QAbstractItemView::SingleSelection);
        tableWidget_3->setSelectionBehavior(QAbstractItemView::SelectItems);

        gridLayout_4->addWidget(tableWidget_3, 0, 0, 1, 1);

        tabWidget_3->addTab(tab_4, QString());

        verticalLayout->addWidget(tabWidget_3);

        gridLayout_5 = new QGridLayout();
        gridLayout_5->setSpacing(6);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));

        verticalLayout->addLayout(gridLayout_5);

        tableWidget_2 = new QTableWidget(centralWidget);
        tableWidget_2->setObjectName(QStringLiteral("tableWidget_2"));
        QFont font7;
        font7.setFamily(QStringLiteral("Times New Roman"));
        font7.setPointSize(15);
        font7.setBold(false);
        font7.setWeight(50);
        tableWidget_2->setFont(font7);
        tableWidget_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidget_2->setSelectionMode(QAbstractItemView::NoSelection);
        tableWidget_2->setSelectionBehavior(QAbstractItemView::SelectRows);

        verticalLayout->addWidget(tableWidget_2);

        ApplyButton = new QPushButton(centralWidget);
        ApplyButton->setObjectName(QStringLiteral("ApplyButton"));
        QFont font8;
        font8.setFamily(QStringLiteral("Times New Roman"));
        font8.setPointSize(12);
        ApplyButton->setFont(font8);

        verticalLayout->addWidget(ApplyButton);


        horizontalLayout_2->addLayout(verticalLayout);


        horizontalLayout_3->addLayout(horizontalLayout_2);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        tabWidget_2 = new QTabWidget(centralWidget);
        tabWidget_2->setObjectName(QStringLiteral("tabWidget_2"));
        tabWidget_2->setFont(font3);
        tabWidget_2->setLayoutDirection(Qt::RightToLeft);
        tab_5 = new QWidget();
        tab_5->setObjectName(QStringLiteral("tab_5"));
        gridLayout_6 = new QGridLayout(tab_5);
        gridLayout_6->setSpacing(6);
        gridLayout_6->setContentsMargins(11, 11, 11, 11);
        gridLayout_6->setObjectName(QStringLiteral("gridLayout_6"));
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        label_show = new QLabel(tab_5);
        label_show->setObjectName(QStringLiteral("label_show"));

        horizontalLayout_6->addWidget(label_show);

        line = new QFrame(tab_5);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);

        horizontalLayout_6->addWidget(line);

        label_input = new QLabel(tab_5);
        label_input->setObjectName(QStringLiteral("label_input"));

        horizontalLayout_6->addWidget(label_input);


        gridLayout_6->addLayout(horizontalLayout_6, 0, 0, 1, 1);

        tabWidget_2->addTab(tab_5, QString());
        tab_6 = new QWidget();
        tab_6->setObjectName(QStringLiteral("tab_6"));
        tabWidget_2->addTab(tab_6, QString());

        horizontalLayout_4->addWidget(tabWidget_2);


        verticalLayout_3->addLayout(horizontalLayout_4);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        horizontalLayout_7->setContentsMargins(-1, 0, -1, 0);
        label = new QLabel(centralWidget);
        label->setObjectName(QStringLiteral("label"));
        QFont font9;
        font9.setFamily(QStringLiteral("Times New Roman"));
        font9.setPointSize(15);
        label->setFont(font9);

        horizontalLayout_7->addWidget(label);

        scaleBtn_0 = new QPushButton(centralWidget);
        scaleBtn_0->setObjectName(QStringLiteral("scaleBtn_0"));
        scaleBtn_0->setFont(font8);
        scaleBtn_0->setLayoutDirection(Qt::LeftToRight);

        horizontalLayout_7->addWidget(scaleBtn_0);

        scaleBtn_1 = new QPushButton(centralWidget);
        scaleBtn_1->setObjectName(QStringLiteral("scaleBtn_1"));
        scaleBtn_1->setFont(font8);

        horizontalLayout_7->addWidget(scaleBtn_1);

        scaleBtn_2 = new QPushButton(centralWidget);
        scaleBtn_2->setObjectName(QStringLiteral("scaleBtn_2"));
        scaleBtn_2->setFont(font8);

        horizontalLayout_7->addWidget(scaleBtn_2);


        verticalLayout_3->addLayout(horizontalLayout_7);

        label_4 = new QLabel(centralWidget);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setFont(font8);
        label_4->setAlignment(Qt::AlignCenter);

        verticalLayout_3->addWidget(label_4);

        progressBar = new QProgressBar(centralWidget);
        progressBar->setObjectName(QStringLiteral("progressBar"));
        progressBar->setValue(0);

        verticalLayout_3->addWidget(progressBar);

        tableWidget_6 = new QTableWidget(centralWidget);
        tableWidget_6->setObjectName(QStringLiteral("tableWidget_6"));
        QFont font10;
        font10.setFamily(QStringLiteral("Times New Roman"));
        tableWidget_6->setFont(font10);
        tableWidget_6->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidget_6->setTabKeyNavigation(false);
        tableWidget_6->setProperty("showDropIndicator", QVariant(false));
        tableWidget_6->setDragDropOverwriteMode(false);
        tableWidget_6->setDragDropMode(QAbstractItemView::DragOnly);
        tableWidget_6->setDefaultDropAction(Qt::CopyAction);
        tableWidget_6->setSelectionMode(QAbstractItemView::SingleSelection);

        verticalLayout_3->addWidget(tableWidget_6);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        horizontalLayout_5->setContentsMargins(0, 0, 0, 0);
        RunRecipeButton = new QPushButton(centralWidget);
        RunRecipeButton->setObjectName(QStringLiteral("RunRecipeButton"));
        RunRecipeButton->setFont(font8);

        horizontalLayout_5->addWidget(RunRecipeButton);

        RunImageButton = new QPushButton(centralWidget);
        RunImageButton->setObjectName(QStringLiteral("RunImageButton"));
        RunImageButton->setFont(font8);

        horizontalLayout_5->addWidget(RunImageButton);

        stopButton = new QPushButton(centralWidget);
        stopButton->setObjectName(QStringLiteral("stopButton"));
        stopButton->setFont(font8);

        horizontalLayout_5->addWidget(stopButton);

        ExitButton = new QPushButton(centralWidget);
        ExitButton->setObjectName(QStringLiteral("ExitButton"));
        ExitButton->setFont(font8);

        horizontalLayout_5->addWidget(ExitButton);


        verticalLayout_3->addLayout(horizontalLayout_5);


        horizontalLayout_3->addLayout(verticalLayout_3);


        gridLayout_2->addLayout(horizontalLayout_3, 0, 0, 1, 1);

        QtGuiApplication1Class->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(QtGuiApplication1Class);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        statusBar->setEnabled(true);
        QtGuiApplication1Class->setStatusBar(statusBar);

        retranslateUi(QtGuiApplication1Class);

        tabWidget->setCurrentIndex(1);
        tabWidget_3->setCurrentIndex(0);
        tabWidget_2->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(QtGuiApplication1Class);
    } // setupUi

    void retranslateUi(QMainWindow *QtGuiApplication1Class)
    {
        QtGuiApplication1Class->setWindowTitle(QApplication::translate("QtGuiApplication1Class", "QtGuiApplication1", 0));
        actionSelect_an_image->setText(QApplication::translate("QtGuiApplication1Class", "select image", 0));
        actionExit->setText(QApplication::translate("QtGuiApplication1Class", "exit", 0));
        actionBackground->setText(QApplication::translate("QtGuiApplication1Class", "change background", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("QtGuiApplication1Class", "Recipe", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("QtGuiApplication1Class", "Food List", 0));
        tabWidget_3->setTabText(tabWidget_3->indexOf(tab_3), QApplication::translate("QtGuiApplication1Class", "Selected Food  ", 0));
        tabWidget_3->setTabText(tabWidget_3->indexOf(tab_4), QApplication::translate("QtGuiApplication1Class", "Suggest", 0));
        ApplyButton->setText(QApplication::translate("QtGuiApplication1Class", "Compare", 0));
        label_show->setText(QString());
        label_input->setText(QString());
        tabWidget_2->setTabText(tabWidget_2->indexOf(tab_5), QApplication::translate("QtGuiApplication1Class", "Automation", 0));
        tabWidget_2->setTabText(tabWidget_2->indexOf(tab_6), QApplication::translate("QtGuiApplication1Class", "customize", 0));
        label->setText(QApplication::translate("QtGuiApplication1Class", "Plate diameter:", 0));
        scaleBtn_0->setText(QApplication::translate("QtGuiApplication1Class", "10cm", 0));
        scaleBtn_1->setText(QApplication::translate("QtGuiApplication1Class", "20cm", 0));
        scaleBtn_2->setText(QApplication::translate("QtGuiApplication1Class", "30cm", 0));
        label_4->setText(QString());
        RunRecipeButton->setText(QApplication::translate("QtGuiApplication1Class", "Compare All Recipe", 0));
        RunImageButton->setText(QApplication::translate("QtGuiApplication1Class", "Compare All Image", 0));
        stopButton->setText(QApplication::translate("QtGuiApplication1Class", "Stop", 0));
        ExitButton->setText(QApplication::translate("QtGuiApplication1Class", "Exit", 0));
    } // retranslateUi

};

namespace Ui {
    class QtGuiApplication1Class: public Ui_QtGuiApplication1Class {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTGUIAPPLICATION1_H
