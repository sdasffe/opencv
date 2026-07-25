/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <widget.h>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QAction *actOpenImage;
    QAction *actOpenFolder;
    QAction *actExit;
    QAction *actUndo;
    QAction *actLangZh;
    QAction *actLangEn;
    QAction *actThemeLight;
    QAction *actThemeDark;
    QAction *actHelp;
    QAction *actAbout;
    QVBoxLayout *rootLayout;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuRoi;
    QMenu *menuSettings;
    QMenu *menuLanguage;
    QMenu *menuTheme;
    QHBoxLayout *horizontalLayout;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *btnApply;
    QPushButton *btnCompare;
    QPushButton *btnSave;
    QSpacerItem *horizontalSpacer;
    QLabel *labelInfo;
    QLabel *label_2;
    QLabel *label_3;
    QGraphicsView *graphicsView;
    QWidget *folderBrowserPanel;
    QVBoxLayout *folderBrowserLayout;
    QLabel *folderBrowserTitle;
    QListWidget *folderImageList;
    QWidget *widget_2;
    QVBoxLayout *verticalLayout_3;
    QLabel *algoTitleLabel;
    MyListWidget *listWidget;
    QSpacerItem *verticalSpacer;
    QWidget *widget_3;
    QVBoxLayout *blockPanelOuterLayout;
    QLabel *chainTitleLabel;
    QHBoxLayout *chainIoLayout;
    QPushButton *btnExportChain;
    QPushButton *btnImportChain;
    QPushButton *btnClearChain;
    QLabel *chainHintLabel;
    QScrollArea *blockScrollArea;
    QWidget *blockListContainer;
    QVBoxLayout *blockListLayout;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName("Widget");
        Widget->resize(1200, 720);
        Widget->setMinimumSize(QSize(800, 520));
        actOpenImage = new QAction(Widget);
        actOpenImage->setObjectName("actOpenImage");
        actOpenFolder = new QAction(Widget);
        actOpenFolder->setObjectName("actOpenFolder");
        actExit = new QAction(Widget);
        actExit->setObjectName("actExit");
        actUndo = new QAction(Widget);
        actUndo->setObjectName("actUndo");
        actUndo->setShortcutContext(Qt::ShortcutContext::WindowShortcut);
        actLangZh = new QAction(Widget);
        actLangZh->setObjectName("actLangZh");
        actLangZh->setCheckable(true);
        actLangEn = new QAction(Widget);
        actLangEn->setObjectName("actLangEn");
        actLangEn->setCheckable(true);
        actThemeLight = new QAction(Widget);
        actThemeLight->setObjectName("actThemeLight");
        actThemeLight->setCheckable(true);
        actThemeDark = new QAction(Widget);
        actThemeDark->setObjectName("actThemeDark");
        actThemeDark->setCheckable(true);
        actHelp = new QAction(Widget);
        actHelp->setObjectName("actHelp");
        actAbout = new QAction(Widget);
        actAbout->setObjectName("actAbout");
        rootLayout = new QVBoxLayout(Widget);
        rootLayout->setSpacing(0);
        rootLayout->setObjectName("rootLayout");
        rootLayout->setContentsMargins(0, 0, 0, 0);
        menuBar = new QMenuBar(Widget);
        menuBar->setObjectName("menuBar");
        menuBar->setNativeMenuBar(false);
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName("menuFile");
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName("menuEdit");
        menuRoi = new QMenu(menuEdit);
        menuRoi->setObjectName("menuRoi");
        menuSettings = new QMenu(menuBar);
        menuSettings->setObjectName("menuSettings");
        menuLanguage = new QMenu(menuSettings);
        menuLanguage->setObjectName("menuLanguage");
        menuTheme = new QMenu(menuSettings);
        menuTheme->setObjectName("menuTheme");

        rootLayout->addWidget(menuBar);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(8);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(10, 8, 10, 10);
        widget = new QWidget(Widget);
        widget->setObjectName("widget");
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setSpacing(8);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(8);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        btnApply = new QPushButton(widget);
        btnApply->setObjectName("btnApply");
        btnApply->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        horizontalLayout_2->addWidget(btnApply);

        btnCompare = new QPushButton(widget);
        btnCompare->setObjectName("btnCompare");
        btnCompare->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        horizontalLayout_2->addWidget(btnCompare);

        btnSave = new QPushButton(widget);
        btnSave->setObjectName("btnSave");
        btnSave->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        horizontalLayout_2->addWidget(btnSave);

        horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        labelInfo = new QLabel(widget);
        labelInfo->setObjectName("labelInfo");

        horizontalLayout_2->addWidget(labelInfo);

        label_2 = new QLabel(widget);
        label_2->setObjectName("label_2");

        horizontalLayout_2->addWidget(label_2);

        label_3 = new QLabel(widget);
        label_3->setObjectName("label_3");

        horizontalLayout_2->addWidget(label_3);


        verticalLayout->addLayout(horizontalLayout_2);

        graphicsView = new QGraphicsView(widget);
        graphicsView->setObjectName("graphicsView");

        verticalLayout->addWidget(graphicsView);

        folderBrowserPanel = new QWidget(widget);
        folderBrowserPanel->setObjectName("folderBrowserPanel");
        folderBrowserPanel->setMinimumSize(QSize(0, 108));
        folderBrowserPanel->setMaximumSize(QSize(16777215, 140));
        folderBrowserPanel->setVisible(false);
        folderBrowserLayout = new QVBoxLayout(folderBrowserPanel);
        folderBrowserLayout->setSpacing(4);
        folderBrowserLayout->setObjectName("folderBrowserLayout");
        folderBrowserLayout->setContentsMargins(0, 4, 0, 0);
        folderBrowserTitle = new QLabel(folderBrowserPanel);
        folderBrowserTitle->setObjectName("folderBrowserTitle");

        folderBrowserLayout->addWidget(folderBrowserTitle);

        folderImageList = new QListWidget(folderBrowserPanel);
        folderImageList->setObjectName("folderImageList");
        folderImageList->setMinimumSize(QSize(0, 80));
        folderImageList->setMovement(QListView::Movement::Static);
        folderImageList->setFlow(QListView::Flow::LeftToRight);
        folderImageList->setResizeMode(QListView::ResizeMode::Adjust);
        folderImageList->setViewMode(QListView::ViewMode::IconMode);
        folderImageList->setUniformItemSizes(true);
        folderImageList->setWordWrap(false);

        folderBrowserLayout->addWidget(folderImageList);


        verticalLayout->addWidget(folderBrowserPanel);

        verticalLayout->setStretch(1, 1);

        horizontalLayout->addWidget(widget);

        widget_2 = new QWidget(Widget);
        widget_2->setObjectName("widget_2");
        widget_2->setMinimumSize(QSize(100, 0));
        verticalLayout_3 = new QVBoxLayout(widget_2);
        verticalLayout_3->setSpacing(4);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(8, 8, 8, 8);
        algoTitleLabel = new QLabel(widget_2);
        algoTitleLabel->setObjectName("algoTitleLabel");

        verticalLayout_3->addWidget(algoTitleLabel);

        listWidget = new MyListWidget(widget_2);
        new QListWidgetItem(listWidget);
        new QListWidgetItem(listWidget);
        new QListWidgetItem(listWidget);
        new QListWidgetItem(listWidget);
        new QListWidgetItem(listWidget);
        new QListWidgetItem(listWidget);
        listWidget->setObjectName("listWidget");
        listWidget->setMinimumSize(QSize(0, 120));

        verticalLayout_3->addWidget(listWidget);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_3->addItem(verticalSpacer);


        horizontalLayout->addWidget(widget_2);

        widget_3 = new QWidget(Widget);
        widget_3->setObjectName("widget_3");
        widget_3->setMinimumSize(QSize(160, 0));
        blockPanelOuterLayout = new QVBoxLayout(widget_3);
        blockPanelOuterLayout->setSpacing(4);
        blockPanelOuterLayout->setObjectName("blockPanelOuterLayout");
        blockPanelOuterLayout->setContentsMargins(8, 8, 8, 8);
        chainTitleLabel = new QLabel(widget_3);
        chainTitleLabel->setObjectName("chainTitleLabel");

        blockPanelOuterLayout->addWidget(chainTitleLabel);

        chainIoLayout = new QHBoxLayout();
        chainIoLayout->setSpacing(6);
        chainIoLayout->setObjectName("chainIoLayout");
        btnExportChain = new QPushButton(widget_3);
        btnExportChain->setObjectName("btnExportChain");
        btnExportChain->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        chainIoLayout->addWidget(btnExportChain);

        btnImportChain = new QPushButton(widget_3);
        btnImportChain->setObjectName("btnImportChain");
        btnImportChain->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        chainIoLayout->addWidget(btnImportChain);

        btnClearChain = new QPushButton(widget_3);
        btnClearChain->setObjectName("btnClearChain");
        btnClearChain->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        chainIoLayout->addWidget(btnClearChain);


        blockPanelOuterLayout->addLayout(chainIoLayout);

        chainHintLabel = new QLabel(widget_3);
        chainHintLabel->setObjectName("chainHintLabel");
        chainHintLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        blockPanelOuterLayout->addWidget(chainHintLabel);

        blockScrollArea = new QScrollArea(widget_3);
        blockScrollArea->setObjectName("blockScrollArea");
        blockScrollArea->setFrameShape(QFrame::Shape::NoFrame);
        blockScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
        blockScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        blockScrollArea->setWidgetResizable(true);
        blockScrollArea->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignTop);
        blockListContainer = new QWidget();
        blockListContainer->setObjectName("blockListContainer");
        blockListContainer->setGeometry(QRect(0, 0, 229, 599));
        blockListLayout = new QVBoxLayout(blockListContainer);
        blockListLayout->setSpacing(8);
        blockListLayout->setObjectName("blockListLayout");
        blockListLayout->setContentsMargins(2, 2, 2, 2);
        blockScrollArea->setWidget(blockListContainer);

        blockPanelOuterLayout->addWidget(blockScrollArea);

        blockPanelOuterLayout->setStretch(3, 1);

        horizontalLayout->addWidget(widget_3);

        horizontalLayout->setStretch(0, 12);
        horizontalLayout->setStretch(1, 3);
        horizontalLayout->setStretch(2, 4);

        rootLayout->addLayout(horizontalLayout);


        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuSettings->menuAction());
        menuBar->addAction(actHelp);
        menuBar->addAction(actAbout);
        menuFile->addAction(actOpenImage);
        menuFile->addAction(actOpenFolder);
        menuFile->addSeparator();
        menuFile->addAction(actExit);
        menuEdit->addAction(actUndo);
        menuEdit->addSeparator();
        menuEdit->addAction(menuRoi->menuAction());
        menuSettings->addAction(menuLanguage->menuAction());
        menuSettings->addAction(menuTheme->menuAction());
        menuLanguage->addAction(actLangZh);
        menuLanguage->addAction(actLangEn);
        menuTheme->addAction(actThemeLight);
        menuTheme->addAction(actThemeDark);

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "\345\233\276\345\203\217\345\244\204\347\220\206\345\267\245\345\205\267", nullptr));
        actOpenImage->setText(QCoreApplication::translate("Widget", "\346\211\223\345\274\200\345\233\276\347\211\207", nullptr));
        actOpenFolder->setText(QCoreApplication::translate("Widget", "\346\211\223\345\274\200\346\226\207\344\273\266\345\244\271", nullptr));
        actExit->setText(QCoreApplication::translate("Widget", "\351\200\200\345\207\272", nullptr));
        actUndo->setText(QCoreApplication::translate("Widget", "\346\222\244\351\224\200", nullptr));
#if QT_CONFIG(shortcut)
        actUndo->setShortcut(QCoreApplication::translate("Widget", "Ctrl+Z", nullptr));
#endif // QT_CONFIG(shortcut)
        actLangZh->setText(QCoreApplication::translate("Widget", "\344\270\255\346\226\207", nullptr));
        actLangEn->setText(QCoreApplication::translate("Widget", "English", nullptr));
        actThemeLight->setText(QCoreApplication::translate("Widget", "\346\265\205\350\211\262", nullptr));
        actThemeDark->setText(QCoreApplication::translate("Widget", "\346\267\261\350\211\262", nullptr));
        actHelp->setText(QCoreApplication::translate("Widget", "\345\270\256\345\212\251(&H)", nullptr));
        actAbout->setText(QCoreApplication::translate("Widget", "\345\205\263\344\272\216(&A)", nullptr));
        menuFile->setTitle(QCoreApplication::translate("Widget", "\346\226\207\344\273\266(&F)", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("Widget", "\347\274\226\350\276\221(&E)", nullptr));
        menuRoi->setTitle(QCoreApplication::translate("Widget", "ROI", nullptr));
        menuSettings->setTitle(QCoreApplication::translate("Widget", "\350\256\276\347\275\256(&S)", nullptr));
        menuLanguage->setTitle(QCoreApplication::translate("Widget", "\350\257\255\350\250\200", nullptr));
        menuTheme->setTitle(QCoreApplication::translate("Widget", "\345\244\226\350\247\202", nullptr));
#if QT_CONFIG(tooltip)
        btnApply->setToolTip(QCoreApplication::translate("Widget", "\346\214\211\345\275\223\345\211\215 ROI \344\270\216\345\244\204\347\220\206\351\223\276\351\207\215\346\226\260\350\256\241\347\256\227", nullptr));
#endif // QT_CONFIG(tooltip)
        btnApply->setText(QCoreApplication::translate("Widget", "\345\272\224\347\224\250", nullptr));
#if QT_CONFIG(tooltip)
        btnCompare->setToolTip(QCoreApplication::translate("Widget", "\346\214\211\344\275\217\346\237\245\347\234\213\345\216\237\345\233\276\357\274\214\346\235\276\345\274\200\346\201\242\345\244\215\347\273\223\346\236\234", nullptr));
#endif // QT_CONFIG(tooltip)
        btnCompare->setText(QCoreApplication::translate("Widget", "\345\257\271\346\257\224", nullptr));
#if QT_CONFIG(tooltip)
        btnSave->setToolTip(QCoreApplication::translate("Widget", "\344\277\235\345\255\230\345\244\204\347\220\206\347\273\223\346\236\234", nullptr));
#endif // QT_CONFIG(tooltip)
        btnSave->setText(QCoreApplication::translate("Widget", "\344\277\235\345\255\230", nullptr));
#if QT_CONFIG(tooltip)
        labelInfo->setToolTip(QCoreApplication::translate("Widget", "\345\233\276\347\211\207\344\277\241\346\201\257", nullptr));
#endif // QT_CONFIG(tooltip)
        labelInfo->setText(QCoreApplication::translate("Widget", "-", nullptr));
        label_2->setText(QCoreApplication::translate("Widget", "\350\200\227\346\227\266", nullptr));
        label_3->setText(QCoreApplication::translate("Widget", "0 ms", nullptr));
        folderBrowserTitle->setText(QCoreApplication::translate("Widget", "\346\226\207\344\273\266\345\244\271\345\233\276\347\211\207\357\274\210\347\202\271\345\207\273\345\210\207\346\215\242\357\274\211", nullptr));
#if QT_CONFIG(tooltip)
        folderImageList->setToolTip(QCoreApplication::translate("Widget", "\347\202\271\345\207\273\347\274\251\347\225\245\345\233\276\345\210\207\346\215\242\345\275\223\345\211\215\345\233\276\347\211\207", nullptr));
#endif // QT_CONFIG(tooltip)
        algoTitleLabel->setText(QCoreApplication::translate("Widget", "\347\256\227\346\263\225", nullptr));

        const bool __sortingEnabled = listWidget->isSortingEnabled();
        listWidget->setSortingEnabled(false);
        QListWidgetItem *___qlistwidgetitem = listWidget->item(0);
        ___qlistwidgetitem->setText(QCoreApplication::translate("Widget", "\344\272\214\345\200\274\345\214\226\345\244\204\347\220\206", nullptr));
        QListWidgetItem *___qlistwidgetitem1 = listWidget->item(1);
        ___qlistwidgetitem1->setText(QCoreApplication::translate("Widget", "\345\275\242\346\200\201\345\255\246\345\244\204\347\220\206", nullptr));
        QListWidgetItem *___qlistwidgetitem2 = listWidget->item(2);
        ___qlistwidgetitem2->setText(QCoreApplication::translate("Widget", "\346\273\244\346\263\242\345\244\204\347\220\206", nullptr));
        QListWidgetItem *___qlistwidgetitem3 = listWidget->item(3);
        ___qlistwidgetitem3->setText(QCoreApplication::translate("Widget", "\347\201\260\345\272\246\345\217\230\346\215\242", nullptr));
        QListWidgetItem *___qlistwidgetitem4 = listWidget->item(4);
        ___qlistwidgetitem4->setText(QCoreApplication::translate("Widget", "\344\274\252\345\275\251\350\211\262\345\244\204\347\220\206", nullptr));
        QListWidgetItem *___qlistwidgetitem5 = listWidget->item(5);
        ___qlistwidgetitem5->setText(QCoreApplication::translate("Widget", "\347\201\260\345\272\246\345\205\261\347\224\237\347\237\251\351\230\265", nullptr));
        listWidget->setSortingEnabled(__sortingEnabled);

        chainTitleLabel->setText(QCoreApplication::translate("Widget", "\345\233\276\345\203\217\345\244\204\347\220\206\345\267\245\345\205\267\347\256\261", nullptr));
#if QT_CONFIG(tooltip)
        btnExportChain->setToolTip(QCoreApplication::translate("Widget", "\345\257\274\345\207\272\345\275\223\345\211\215\345\244\204\347\220\206\351\223\276\344\270\272 JSON", nullptr));
#endif // QT_CONFIG(tooltip)
        btnExportChain->setText(QCoreApplication::translate("Widget", "\345\257\274\345\207\272", nullptr));
#if QT_CONFIG(tooltip)
        btnImportChain->setToolTip(QCoreApplication::translate("Widget", "\344\273\216 JSON \345\257\274\345\205\245\345\244\204\347\220\206\351\223\276", nullptr));
#endif // QT_CONFIG(tooltip)
        btnImportChain->setText(QCoreApplication::translate("Widget", "\345\257\274\345\205\245", nullptr));
#if QT_CONFIG(tooltip)
        btnClearChain->setToolTip(QCoreApplication::translate("Widget", "\346\270\205\347\251\272\345\233\276\345\203\217\345\244\204\347\220\206\345\267\245\345\205\267\347\256\261", nullptr));
#endif // QT_CONFIG(tooltip)
        btnClearChain->setText(QCoreApplication::translate("Widget", "\346\270\205\347\251\272", nullptr));
        chainHintLabel->setText(QCoreApplication::translate("Widget", "\346\213\226\345\205\245\347\256\227\346\263\225", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
