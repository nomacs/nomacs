/*******************************************************************************************************
 DkDialog.cpp
 Created on:	21.04.2011

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2013 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2013 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2013 Florian Kleber <florian@nomacs.org>

 This file is part of nomacs.

 nomacs is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 nomacs is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 *******************************************************************************************************/

#include "DkDialog.h"

#include "DkActionManager.h"
#include "DkBaseViewPort.h"
#include "DkBasicWidgets.h"
#include "DkCentralWidget.h"
#include "DkImageStorage.h"
#include "DkPluginManager.h"
#include "DkSettings.h"
#include "DkThumbs.h"
#include "DkTimer.h"
#include "DkUtils.h"
#include "DkViewPort.h"
#include "DkWidgets.h"

#if defined(Q_OS_WIN) && !defined(SOCK_STREAM)
#include <winsock2.h> // needed since libraw 0.16
#endif

#pragma warning(push, 0) // no warnings from includes - begin
#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QCompleter>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFuture>
#include <QHeaderView>
#include <QInputDialog>
#include <QItemEditorFactory>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QListView>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPageSetupDialog>
#include <QPrintDialog>
#include <QPrinterInfo>
#include <QProgressBar>
#include <QProgressDialog>
#include <QPushButton>
#include <QRadioButton>
#include <QRandomGenerator>
#include <QScreen>
#include <QSlider>
#include <QSpinBox>
#include <QSplashScreen>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QTableView>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QWidget>
#include <QtConcurrentRun>
#include <qmath.h>

// quazip
#ifdef WITH_QUAZIP
#ifdef WITH_QUAZIP1
#include <quazip/JlCompress.h>
#else
#include <quazip5/JlCompress.h>
#endif
#endif

#pragma warning(pop) // no warnings from includes - end

namespace nmc
{

QFileDialog::Options DkDialog::fileDialogOptions()
{
    QFileDialog::Options flags;

    if (!DkSettingsManager::param().resources().nativeDialog)
        flags = QFileDialog::DontUseNativeDialog;

    return flags;
}

// DkSplashScreen --------------------------------------------------------------------
DkSplashScreen::DkSplashScreen(QWidget * /*parent*/, Qt::WindowFlags flags)
    : QDialog(0, flags)
{
    QPixmap img(":/nomacs/img/splash-screen.png");
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setMouseTracking(true);

#ifdef Q_OS_MAC
    setObjectName("DkSplashScreenMac");
#else
    setObjectName("DkSplashScreen");
    setAttribute(Qt::WA_TranslucentBackground);
#endif

    imgLabel = new QLabel(this, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    imgLabel->setObjectName("DkSplashInfoLabel");
    imgLabel->setMouseTracking(true);
    imgLabel->setScaledContents(true);
    imgLabel->setPixmap(img);
    imgLabel->setFixedSize(600, 474);
    imgLabel->show();

    setFixedSize(imgLabel->size());

    exitButton = new QPushButton(DkImage::loadIcon(":/nomacs/img/close.svg"), "", this);
    exitButton->setObjectName("cancelButtonSplash");
    exitButton->setFlat(true);
    exitButton->setToolTip(tr("Close (ESC)"));
    exitButton->setShortcut(QKeySequence(Qt::Key_Escape));
    exitButton->move(10, 435);
    exitButton->hide();
    connect(exitButton, SIGNAL(clicked()), this, SLOT(close()));

    // set the text
    text = QString(
        "Flo was here und w&uuml;nscht<br>"
        "Stefan fiel Spa&szlig; w&auml;hrend<br>"
        "Markus rockt... <br><br>"

        "<a href=\"https://github.com/nomacs/nomacs\">https://github.com/nomacs/nomacs</a><br>"

        "This program is licensed under GNU General Public License v3<br>"
        "&#169; Markus Diem, Stefan Fiel and Florian Kleber, 2011-2020<br><br>"

        "Press [ESC] to exit");

    textLabel = new QLabel(this, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    textLabel->setObjectName("DkSplashInfoLabel");
    textLabel->setMouseTracking(true);
    textLabel->setScaledContents(true);
    textLabel->setTextFormat(Qt::RichText);
    textLabel->setText(text);
    textLabel->move(131, 280);
    textLabel->setOpenExternalLinks(true);

    // textLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    QLabel *versionLabel = new QLabel(this, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    versionLabel->setObjectName("DkSplashInfoLabel");
    versionLabel->setTextFormat(Qt::RichText);

    versionLabel->setText(versionText());
    versionLabel->setAlignment(Qt::AlignRight);
    versionLabel->move(360, 280);
    versionLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    showTimer = new QTimer(this);
    showTimer->setInterval(5000);
    showTimer->setSingleShot(true);
    connect(showTimer, SIGNAL(timeout()), exitButton, SLOT(hide()));

    qDebug() << "splash screen created...";
}

void DkSplashScreen::mousePressEvent(QMouseEvent *event)
{
    setCursor(Qt::ClosedHandCursor);
    mouseGrab = event->globalPos();
    QDialog::mousePressEvent(event);
}

void DkSplashScreen::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        move(pos() - (mouseGrab - event->globalPos()));
        mouseGrab = event->globalPos();
        qDebug() << "moving...";
    } else
        setCursor(Qt::OpenHandCursor);
    showClose();
    QDialog::mouseMoveEvent(event);
}

void DkSplashScreen::mouseReleaseEvent(QMouseEvent *event)
{
    setCursor(Qt::OpenHandCursor);
    showClose();
    QDialog::mouseReleaseEvent(event);
}

void DkSplashScreen::showClose()
{
    exitButton->show();
    showTimer->start();
}

QString DkSplashScreen::versionText() const
{
    QString vt;

    // print out if the name is changed (e.g. READ build)
    if (QApplication::applicationName() != "Image Lounge") {
        vt += QApplication::applicationName() + "<br>";
    }

    // architecture
    QString platform = "";
#ifdef _WIN64
    platform = " [x64] ";
#elif defined _WIN32
    platform = " [x86] ";
#endif

    // version & build date
    vt += QApplication::applicationVersion() + platform + "<br>";

// reproducable builds for linux (see #139)
#ifdef Q_OS_WIN
    vt += QString(__DATE__) + "<br>";
#endif

    // supplemental info
    vt += "<p style=\"color: #666; font-size: 7pt; margin: 0; padding: 0;\">";

    // OpenCV
#ifdef WITH_OPENCV
    vt += "OpenCV " + QString(CV_VERSION) + "<br>";
#else
    vt += "No CV support<br>";
#endif

    // Qt
    vt += "Qt " + QString(QT_VERSION_STR) + "<br>";

    // LibRAW
#ifndef WITH_LIBRAW
    vt += "No RAW support<br>";
#endif

    // portable
    vt += (DkSettingsManager::param().isPortable() ? tr("Portable") : "");
    vt += "</p>";

    return vt;
}

// file validator --------------------------------------------------------------------
DkFileValidator::DkFileValidator(const QString &lastFile, QObject *parent)
    : QValidator(parent)
{
    mLastFile = lastFile;
}

void DkFileValidator::fixup(QString &input) const
{
    if (!QFileInfo(input).exists())
        input = mLastFile;
}

QValidator::State DkFileValidator::validate(QString &input, int &) const
{
    if (QFileInfo(input).exists())
        return QValidator::Acceptable;
    else
        return QValidator::Intermediate;
}

// train dialog --------------------------------------------------------------------
DkTrainDialog::DkTrainDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    setWindowTitle(tr("Add New Image Format"));
    createLayout();
    setFixedSize(340, 400); // due to the baseViewport we need fixed sized dialogs : (
    setAcceptDrops(true);
}

void DkTrainDialog::createLayout()
{
    // first row
    QLabel *newImageLabel = new QLabel(tr("Load New Image Format"), this);
    mPathEdit = new QLineEdit(this);
    mPathEdit->setValidator(&mFileValidator);
    mPathEdit->setObjectName("DkWarningEdit");
    connect(mPathEdit, SIGNAL(textChanged(const QString &)), this, SLOT(textChanged(const QString &)));
    connect(mPathEdit, SIGNAL(editingFinished()), this, SLOT(loadFile()));

    QPushButton *openButton = new QPushButton(tr("&Browse"), this);
    connect(openButton, SIGNAL(pressed()), this, SLOT(openFile()));

    mFeedbackLabel = new QLabel("", this);
    mFeedbackLabel->setObjectName("DkDecentInfo");

    // shows the image if it could be loaded
    mViewport = new DkBaseViewPort(this);
    mViewport->setForceFastRendering(true);
    mViewport->setPanControl(QPointF(0.0f, 0.0f));

    // mButtons
    mButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    mButtons->button(QDialogButtonBox::Ok)->setText(tr("&Add"));
    mButtons->button(QDialogButtonBox::Ok)->setEnabled(false);
    mButtons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    connect(mButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(mButtons, SIGNAL(rejected()), this, SLOT(reject()));

    QWidget *trainWidget = new QWidget(this);
    QGridLayout *gdLayout = new QGridLayout(trainWidget);
    gdLayout->addWidget(newImageLabel, 0, 0);
    gdLayout->addWidget(mPathEdit, 1, 0);
    gdLayout->addWidget(openButton, 1, 1);
    gdLayout->addWidget(mFeedbackLabel, 2, 0, 1, 2);
    gdLayout->addWidget(mViewport, 3, 0, 1, 2);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(trainWidget);
    layout->addWidget(mButtons);
}

void DkTrainDialog::textChanged(const QString &text)
{
    if (QFileInfo(text).exists())
        mPathEdit->setProperty("warning", false);
    else
        mPathEdit->setProperty("warning", false);

    mPathEdit->style()->unpolish(mPathEdit);
    mPathEdit->style()->polish(mPathEdit);
    mPathEdit->update();
}

void DkTrainDialog::openFile()
{
    // load system default open dialog
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Image"), mFile, tr("All Files (*.*)"), nullptr, DkDialog::fileDialogOptions());

    if (QFileInfo(filePath).exists()) {
        mPathEdit->setText(filePath);
        loadFile(filePath);
    }
}

void DkTrainDialog::userFeedback(const QString &msg, bool error)
{
    if (!error)
        mFeedbackLabel->setProperty("warning", false);
    else
        mFeedbackLabel->setProperty("warning", true);

    mFeedbackLabel->setText(msg);
    mFeedbackLabel->style()->unpolish(mFeedbackLabel);
    mFeedbackLabel->style()->polish(mFeedbackLabel);
    mFeedbackLabel->update();
}

void DkTrainDialog::loadFile(const QString &filePath)
{
    QString lFilePath = filePath;

    if (filePath.isEmpty() && !mPathEdit->text().isEmpty())
        lFilePath = mPathEdit->text();
    else if (filePath.isEmpty())
        return;

    QFileInfo fileInfo(lFilePath);
    if (!fileInfo.exists() || mAcceptedFile == lFilePath)
        return; // error message?!

    // update validator
    mFileValidator.setLastFile(lFilePath);

    DkBasicLoader basicLoader;
    basicLoader.setTraining(true);

    // TODO: archives cannot be trained currently
    bool imgLoaded = basicLoader.loadGeneral(lFilePath, true);

    if (!imgLoaded) {
        mViewport->setImage(QImage()); // remove the image
        mAcceptedFile = "";
        userFeedback(tr("Sorry, currently we don't support: *.%1 files").arg(fileInfo.suffix()), true);
        return;
    }

    if (DkSettingsManager::param().app().fileFilters.join(" ").contains(fileInfo.suffix(), Qt::CaseInsensitive)) {
        userFeedback(tr("*.%1 is already supported.").arg(fileInfo.suffix()), false);
        imgLoaded = false;
    } else
        userFeedback(tr("*.%1 is supported.").arg(fileInfo.suffix()), false);

    mViewport->setImage(basicLoader.image());
    mAcceptedFile = lFilePath;

    // try loading the file
    // if loaded !
    mButtons->button(QDialogButtonBox::Ok)->setEnabled(imgLoaded);
}

void DkTrainDialog::accept()
{
    QFileInfo acceptedFileInfo(mAcceptedFile);

    // add the extension to user filters
    if (!DkSettingsManager::param().app().fileFilters.join(" ").contains(acceptedFileInfo.suffix(), Qt::CaseInsensitive)) {
        QString name = QInputDialog::getText(this, "Format Name", tr("Please name the new format:"), QLineEdit::Normal, "Your File Format");
        QString tag = name + " (*." + acceptedFileInfo.suffix() + ")";

        // load user filters
        DefaultSettings settings;
        QStringList userFilters = settings.value("ResourceSettings/userFilters", QStringList()).toStringList();
        userFilters.append(tag);
        settings.setValue("ResourceSettings/userFilters", userFilters);
        DkSettingsManager::param().app().openFilters.append(tag);
        DkSettingsManager::param().app().fileFilters.append("*." + acceptedFileInfo.suffix());
        DkSettingsManager::param().app().browseFilters += acceptedFileInfo.suffix();
    }

    QDialog::accept();
}

void DkTrainDialog::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
        QUrl url = event->mimeData()->urls().at(0);
        qDebug() << "dropping: " << url;
        url = url.toLocalFile();

        mPathEdit->setText(url.toString());
        loadFile();
    }
}

void DkTrainDialog::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QUrl url = event->mimeData()->urls().at(0);
        url = url.toLocalFile();
        QFileInfo file = QFileInfo(url.toString());

        if (file.exists())
            event->acceptProposedAction();
    }
}

// DkAppManagerDialog --------------------------------------------------------------------
DkAppManagerDialog::DkAppManagerDialog(DkAppManager *manager /* = 0 */, QWidget *parent /* = 0 */, Qt::WindowFlags flags /* = 0 */)
    : QDialog(parent, flags)
{
    this->manager = manager;
    setWindowTitle(tr("Manage Applications"));
    createLayout();
}

void DkAppManagerDialog::createLayout()
{
    QVector<QAction *> appActions = manager->getActions();

    model = new QStandardItemModel(this);
    for (int rIdx = 0; rIdx < appActions.size(); rIdx++)
        model->appendRow(getItems(appActions.at(rIdx)));

    appTableView = new QTableView(this);
    appTableView->setModel(model);
    appTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    appTableView->verticalHeader()->hide();
    appTableView->horizontalHeader()->hide();
    // appTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    appTableView->setShowGrid(false);
    appTableView->resizeColumnsToContents();
    appTableView->resizeRowsToContents();
    appTableView->setWordWrap(false);

    QPushButton *runButton = new QPushButton(tr("&Run"), this);
    runButton->setObjectName("runButton");

    QPushButton *addButton = new QPushButton(tr("&Add"), this);
    addButton->setObjectName("addButton");

    QPushButton *deleteButton = new QPushButton(tr("&Delete"), this);
    deleteButton->setObjectName("deleteButton");
    deleteButton->setShortcut(QKeySequence::Delete);

    // mButtons
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
    buttons->addButton(runButton, QDialogButtonBox::ActionRole);
    buttons->addButton(addButton, QDialogButtonBox::ActionRole);
    buttons->addButton(deleteButton, QDialogButtonBox::ActionRole);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(appTableView);
    layout->addWidget(buttons);
    QMetaObject::connectSlotsByName(this);
}

QList<QStandardItem *> DkAppManagerDialog::getItems(QAction *action)
{
    QList<QStandardItem *> items;
    QStandardItem *item = new QStandardItem(action->icon(), action->text().remove("&"));
    // item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
    items.append(item);
    item = new QStandardItem(action->toolTip());
    item->setFlags(Qt::ItemIsSelectable);
    items.append(item);

    return items;
}

void DkAppManagerDialog::on_addButton_clicked()
{
    // load system default open dialog
    QString appFilter;
    QString defaultPath;
#ifdef Q_OS_WIN
    appFilter += tr("Executable Files (*.exe);;");
    defaultPath = getenv("PROGRAMFILES");
#else
    defaultPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation); // still retrieves startmenu on windows
#endif

    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Application"), defaultPath, appFilter, nullptr, DkDialog::fileDialogOptions());

    if (filePath.isEmpty())
        return;

    QAction *newApp = manager->createAction(filePath);

    if (newApp)
        model->appendRow(getItems(newApp));
}

void DkAppManagerDialog::on_deleteButton_clicked()
{
    QModelIndexList selRows = appTableView->selectionModel()->selectedRows();

    while (!selRows.isEmpty()) {
        model->removeRows(selRows.last().row(), 1);
        selRows.removeLast();
    }
}

void DkAppManagerDialog::on_runButton_clicked()
{
    accept();

    QItemSelectionModel *sel = appTableView->selectionModel();

    if (!sel->hasSelection() && !manager->getActions().isEmpty())
        emit openWithSignal(manager->getActions().first());

    else if (!manager->getActions().isEmpty()) {
        QModelIndexList rows = sel->selectedRows();

        for (int idx = 0; idx < rows.size(); idx++) {
            emit openWithSignal(manager->getActions().at(rows.at(idx).row()));
        }
    }
}

void DkAppManagerDialog::accept()
{
    QVector<QAction *> apps;

    for (int idx = 0; idx < model->rowCount(); idx++) {
        QString filePath = model->item(idx, 1)->text();
        QString name = model->item(idx, 0)->text();
        QAction *action = manager->findAction(filePath);

        if (!action)
            action = manager->createAction(filePath);
        // obviously I cannot create this action - should we tell user?
        if (!action)
            continue;

        if (name != action->text().remove("&"))
            action->setText(name);

        qDebug() << "pushing back: " << action->text();

        apps.append(action);
    }

    manager->setActions(apps);

    QDialog::accept();
}

// DkSearchDialaog --------------------------------------------------------------------
DkSearchDialog::DkSearchDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    init();
}

void DkSearchDialog::init()
{
    setObjectName("DkSearchDialog");
    setWindowTitle(tr("Find & Filter"));

    mEndMessage = tr("Load All");

    QVBoxLayout *layout = new QVBoxLayout(this);

    QCompleter *history = new QCompleter(DkSettingsManager::param().global().searchHistory, this);
    history->setCompletionMode(QCompleter::InlineCompletion);
    mSearchBar = new QLineEdit();
    mSearchBar->setObjectName("searchBar");
    mSearchBar->setToolTip(tr("Type search words or a regular expression"));
    mSearchBar->setCompleter(history);

    mStringModel = new QStringListModel(this);

    mResultListView = new QListView(this);
    mResultListView->setObjectName("resultListView");
    mResultListView->setModel(mStringModel);
    mResultListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mResultListView->setSelectionMode(QAbstractItemView::SingleSelection);

    mFilterButton = new QPushButton(tr("&Filter"), this);
    mFilterButton->setObjectName("filterButton");

    mButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
    mButtons->button(QDialogButtonBox::Ok)->setDefault(true);
    mButtons->button(QDialogButtonBox::Ok)->setText(tr("F&ind"));
    mButtons->addButton(mFilterButton, QDialogButtonBox::ActionRole);

    connect(mButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(mButtons, SIGNAL(rejected()), this, SLOT(reject()));

    layout->addWidget(mSearchBar);
    layout->addWidget(mResultListView);
    layout->addWidget(mButtons);

    mSearchBar->setFocus(Qt::MouseFocusReason);

    QMetaObject::connectSlotsByName(this);
}

void DkSearchDialog::setFiles(const QStringList &fileList)
{
    mFileList = fileList;
    mResultList = fileList;
    mStringModel->setStringList(makeViewable(fileList));
}

void DkSearchDialog::setPath(const QString &dirPath)
{
    mPath = dirPath;
}

bool DkSearchDialog::filterPressed() const
{
    return mIsFilterPressed;
}

void DkSearchDialog::on_searchBar_textChanged(const QString &text)
{
    DkTimer dt;

    if (text == mCurrentSearch)
        return;

    mResultList = DkUtils::filterStringList(text, mFileList);
    qDebug() << "searching [" << text << "] - converted to individual keywords [" << text.split(" ") << "] takes: " << dt;
    mCurrentSearch = text;

    if (mResultList.empty()) {
        QStringList answerList;
        answerList.append(tr("No Matching Items"));
        mStringModel->setStringList(answerList);

        mResultListView->setProperty("empty", true);

        mFilterButton->setEnabled(false);
        mButtons->button(QDialogButtonBox::Ok)->setEnabled(false);
    } else {
        mFilterButton->setEnabled(true);
        mButtons->button(QDialogButtonBox::Ok)->setEnabled(true);
        mStringModel->setStringList(makeViewable(mResultList));
        mResultListView->selectionModel()->setCurrentIndex(mStringModel->index(0, 0), QItemSelectionModel::SelectCurrent);
        mResultListView->setProperty("empty", false);
    }

    mResultListView->style()->unpolish(mResultListView);
    mResultListView->style()->polish(mResultListView);
    mResultListView->update();

    qDebug() << "searching takes (total): " << dt;
}

void DkSearchDialog::on_resultListView_doubleClicked(const QModelIndex &modelIndex)
{
    if (modelIndex.data().toString() == mEndMessage) {
        mStringModel->setStringList(makeViewable(mResultList, true));
        return;
    }

    emit loadFileSignal(QFileInfo(mPath, modelIndex.data().toString()).absoluteFilePath());
    close();
}

void DkSearchDialog::on_resultListView_clicked(const QModelIndex &modelIndex)
{
    if (modelIndex.data().toString() == mEndMessage)
        mStringModel->setStringList(makeViewable(mResultList, true));
}

void DkSearchDialog::accept()
{
    if (mResultListView->selectionModel()->currentIndex().data().toString() == mEndMessage) {
        mStringModel->setStringList(makeViewable(mResultList, true));
        return;
    }

    updateHistory();

    // ok load the selected file
    QString fileName = mResultListView->selectionModel()->currentIndex().data().toString();
    qDebug() << "opening filename: " << fileName;

    if (!fileName.isEmpty())
        emit loadFileSignal(QFileInfo(mPath, fileName).absoluteFilePath());

    QDialog::accept();
}

void DkSearchDialog::on_filterButton_pressed()
{
    filterSignal(mCurrentSearch);
    mIsFilterPressed = true;
    DkSearchDialog::accept();
    done(filter_button);
}

void DkSearchDialog::setDefaultButton(int defaultButton /* = find_button */)
{
    if (defaultButton == find_button) {
        mButtons->button(QDialogButtonBox::Ok)->setAutoDefault(true);
        mButtons->button(QDialogButtonBox::Cancel)->setAutoDefault(false);
        mFilterButton->setAutoDefault(false);
    } else if (defaultButton == filter_button) {
        mButtons->button(QDialogButtonBox::Ok)->setAutoDefault(false);
        mButtons->button(QDialogButtonBox::Cancel)->setAutoDefault(false);
        mFilterButton->setAutoDefault(true);
    }
}

void DkSearchDialog::updateHistory()
{
    DkSettingsManager::param().global().searchHistory.append(mCurrentSearch);

    // keep the history small
    if (DkSettingsManager::param().global().searchHistory.size() > 50)
        DkSettingsManager::param().global().searchHistory.pop_front();

    // QCompleter* history = new QCompleter(DkSettingsManager::param().global().searchHistory);
    // searchBar->setCompleter(history);
}

QStringList DkSearchDialog::makeViewable(const QStringList &resultList, bool forceAll)
{
    QStringList answerList;

    // if size > 1000 it gets slow -> cut at 1000 and make an entry for 'expand'
    if (!forceAll && resultList.size() > 1000) {
        for (int idx = 0; idx < 1000; idx++)
            answerList.append(resultList[idx]);
        answerList.append(mEndMessage);

        mAllDisplayed = false;
    } else {
        mAllDisplayed = true;
        answerList = resultList;
    }

    return answerList;
}

// DkResizeDialog --------------------------------------------------------------------
DkResizeDialog::DkResizeDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    init();
    resize(DkUtils::getInitialDialogSize());
}

void DkResizeDialog::accept()
{
    saveSettings();

    QDialog::accept();
}

void DkResizeDialog::saveSettings()
{
    DefaultSettings settings;
    settings.beginGroup(objectName());

    settings.setValue("ResampleMethod", mResampleBox->currentIndex());
    settings.setValue("Resample", mResampleCheck->isChecked());
    settings.setValue("CorrectGamma", mGammaCorrection->isChecked());

    if (mSizeBox->currentIndex() == size_percent) {
        settings.setValue("Width", mWPixelSpin->value());
        settings.setValue("Height", mHPixelSpin->value());
    } else {
        settings.setValue("Width", 0);
        settings.setValue("Height", 0);
    }
    settings.endGroup();
}

void DkResizeDialog::loadSettings()
{
    qDebug() << "loading new settings...";

    DefaultSettings settings;
    settings.beginGroup(objectName());

    mResampleBox->setCurrentIndex(settings.value("ResampleMethod", ipl_cubic).toInt());
    mResampleCheck->setChecked(settings.value("Resample", true).toBool());
    mGammaCorrection->setChecked(settings.value("CorrectGamma", true).toBool());

    if (settings.value("Width", 0).toDouble() != 0) {
        double w = settings.value("Width", 0).toDouble();
        double h = settings.value("Height", 0).toDouble();

        qDebug() << "width loaded from settings: " << w;

        if (w != h) {
            mLockButton->setChecked(false);
            mLockButtonDim->setChecked(false);
        }
        mSizeBox->setCurrentIndex(size_percent);

        mWPixelSpin->setValue(w);
        mHPixelSpin->setValue(h);

        updateWidth();
        updateHeight();
    }
    settings.endGroup();
}

void DkResizeDialog::init()
{
    setObjectName("DkResizeDialog");

    mUnitFactor.resize(unit_end);
    mUnitFactor.insert(unit_cm, 1.0f);
    mUnitFactor.insert(unit_mm, 10.0f);
    mUnitFactor.insert(unit_inch, 1.0f / 2.54f);

    mResFactor.resize(res_end);
    mResFactor.insert(res_ppi, 2.54f);
    mResFactor.insert(res_ppc, 1.0f);

    setWindowTitle(tr("Resize Image"));
    // setFixedSize(600, 512);
    createLayout();
    initBoxes();

    mWPixelSpin->setFocus(Qt::ActiveWindowFocusReason);

    QMetaObject::connectSlotsByName(this);
}

void DkResizeDialog::createLayout()
{
    // preview
    int minPx = 1;
    int maxPx = 100000;
    double minWidth = 1;
    double maxWidth = 500000;
    int decimals = 2;

    QLabel *origLabelText = new QLabel(tr("Original"));
    origLabelText->setAlignment(Qt::AlignHCenter);
    QLabel *newLabel = new QLabel(tr("New"));
    newLabel->setAlignment(Qt::AlignHCenter);

    // shows the original image
    mOrigView = new DkBaseViewPort(this);
    mOrigView->setForceFastRendering(true);
    mOrigView->setPanControl(QPointF(0.0f, 0.0f));
    connect(mOrigView, SIGNAL(imageUpdated()), this, SLOT(drawPreview()));

    //// maybe we should report this:
    //// if a stylesheet (with border) is set, the var
    //// cornerPaintingRect in QAbstractScrollArea (which we don't even need : )
    //// is invalid which blocks re-paints unless the widget gets a focus...
    // origView->setStyleSheet("QViewPort{border: 1px solid #888;}");

    // shows the preview
    mPreviewLabel = new QLabel(this);
    mPreviewLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    mPreviewLabel->setMinimumHeight(100);

    // all text dialogs...
    QDoubleValidator *doubleValidator = new QDoubleValidator(1, 1000000, 2, 0);
    doubleValidator->setRange(0, 100, 2);

    QWidget *resizeBoxes = new QWidget(this);
    resizeBoxes->setGeometry(QRect(QPoint(40, 300), QSize(400, 170)));

    QGridLayout *gridLayout = new QGridLayout(resizeBoxes);

    QLabel *wPixelLabel = new QLabel(tr("Width: "), this);
    wPixelLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mWPixelSpin = new DkSelectAllDoubleSpinBox(this);
    mWPixelSpin->setObjectName("wPixelSpin");
    mWPixelSpin->setRange(minPx, maxPx);
    mWPixelSpin->setDecimals(0);

    mLockButton = new DkButton(DkImage::loadIcon(":/nomacs/img/lock.svg"), DkImage::loadIcon(":/nomacs/img/lock-unlocked.svg"), "lock", this);
    mLockButton->setObjectName("lockButton");
    mLockButton->setCheckable(true);
    mLockButton->setChecked(true);

    QLabel *hPixelLabel = new QLabel(tr("Height: "), this);
    hPixelLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mHPixelSpin = new DkSelectAllDoubleSpinBox(this);
    mHPixelSpin->setObjectName("hPixelSpin");
    mHPixelSpin->setRange(minPx, maxPx);
    mHPixelSpin->setDecimals(0);

    mSizeBox = new QComboBox(this);
    QStringList sizeList;
    sizeList.insert(size_pixel, "pixel");
    sizeList.insert(size_percent, "%");
    mSizeBox->addItems(sizeList);
    mSizeBox->setObjectName("sizeBox");

    // first row
    int rIdx = 0;
    gridLayout->addWidget(wPixelLabel, 0, rIdx);
    gridLayout->addWidget(mWPixelSpin, 0, ++rIdx);
    gridLayout->addWidget(mLockButton, 0, ++rIdx);
    gridLayout->addWidget(hPixelLabel, 0, ++rIdx);
    gridLayout->addWidget(mHPixelSpin, 0, ++rIdx);
    gridLayout->addWidget(mSizeBox, 0, ++rIdx);

    // Document dimensions
    QLabel *widthLabel = new QLabel(tr("Width: "));
    widthLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mWidthSpin = new DkSelectAllDoubleSpinBox();
    mWidthSpin->setObjectName("widthSpin");
    mWidthSpin->setRange(minWidth, maxWidth);
    mWidthSpin->setDecimals(decimals);

    mLockButtonDim = new DkButton(DkImage::loadIcon(":/nomacs/img/lock.svg"), DkImage::loadIcon(":/nomacs/img/lock-unlocked.svg"), "lock");
    // mLockButtonDim->setFixedSize(QSize(16,16));
    mLockButtonDim->setObjectName("lockButtonDim");
    mLockButtonDim->setCheckable(true);
    mLockButtonDim->setChecked(true);

    QLabel *heightLabel = new QLabel(tr("Height: "));
    heightLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mHeightSpin = new DkSelectAllDoubleSpinBox();
    mHeightSpin->setObjectName("heightSpin");
    mHeightSpin->setRange(minWidth, maxWidth);
    mHeightSpin->setDecimals(decimals);

    mUnitBox = new QComboBox();
    QStringList unitList;
    unitList.insert(unit_cm, "cm");
    unitList.insert(unit_mm, "mm");
    unitList.insert(unit_inch, "inch");
    // unitList.insert(unit_percent, "%");
    mUnitBox->addItems(unitList);
    mUnitBox->setObjectName("unitBox");

    // second row
    rIdx = 0;
    gridLayout->addWidget(widthLabel, 1, rIdx);
    gridLayout->addWidget(mWidthSpin, 1, ++rIdx);
    gridLayout->addWidget(mLockButtonDim, 1, ++rIdx);
    gridLayout->addWidget(heightLabel, 1, ++rIdx);
    gridLayout->addWidget(mHeightSpin, 1, ++rIdx);
    gridLayout->addWidget(mUnitBox, 1, ++rIdx);

    // resolution
    QLabel *resolutionLabel = new QLabel(tr("Resolution: "));
    resolutionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mResolutionSpin = new DkSelectAllDoubleSpinBox();
    mResolutionSpin->setObjectName("resolutionSpin");
    mResolutionSpin->setRange(minWidth, maxWidth);
    mResolutionSpin->setDecimals(decimals);

    mResUnitBox = new QComboBox();
    QStringList resUnitList;
    resUnitList.insert(res_ppi, tr("pixel/inch"));
    resUnitList.insert(res_ppc, tr("pixel/cm"));
    mResUnitBox->addItems(resUnitList);
    mResUnitBox->setObjectName("resUnitBox");

    // third row
    rIdx = 0;
    gridLayout->addWidget(resolutionLabel, 2, rIdx);
    gridLayout->addWidget(mResolutionSpin, 2, ++rIdx);
    gridLayout->addWidget(mResUnitBox, 2, ++rIdx, 1, 2);

    // resample
    mResampleCheck = new QCheckBox(tr("Resample Image:"));
    mResampleCheck->setChecked(true);
    mResampleCheck->setObjectName("resampleCheck");

    // TODO: disable items if no opencv is available
    mResampleBox = new QComboBox();
    QStringList resampleList;
    resampleList.insert(ipl_nearest, tr("Nearest Neighbor"));
    resampleList.insert(ipl_area, tr("Area (best for downscaling)"));
    resampleList.insert(ipl_linear, tr("Linear"));
    resampleList.insert(ipl_cubic, tr("Bicubic (4x4 pixel interpolation)"));
    resampleList.insert(ipl_lanczos, tr("Lanczos (8x8 pixel interpolation)"));
    mResampleBox->addItems(resampleList);
    mResampleBox->setObjectName("resampleBox");
    mResampleBox->setCurrentIndex(ipl_cubic);

    // last two rows
    gridLayout->addWidget(mResampleCheck, 3, 1, 1, 3);
    gridLayout->addWidget(mResampleBox, 4, 1, 1, 3);

    mGammaCorrection = new QCheckBox(tr("Gamma Correction"));
    mGammaCorrection->setObjectName("gammaCorrection");
    mGammaCorrection->setChecked(false); // default: false since gamma might destroy soft gradients

    gridLayout->addWidget(mGammaCorrection, 5, 1, 1, 3);

    // add stretch
    gridLayout->setColumnStretch(6, 1);

    // mButtons
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    QGridLayout *layout = new QGridLayout(this);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 1);

    layout->addWidget(origLabelText, 0, 0);
    layout->addWidget(newLabel, 0, 1);
    layout->addWidget(mOrigView, 1, 0);
    layout->addWidget(mPreviewLabel, 1, 1);
    layout->addWidget(resizeBoxes, 2, 0, 1, 2, Qt::AlignLeft);
    // layout->addStretch();
    layout->addWidget(buttons, 3, 0, 1, 2, Qt::AlignRight);

    adjustSize();
    resize(700, 500);

    // show();
}

void DkResizeDialog::initBoxes(bool updateSettings)
{
    if (mImg.isNull())
        return;

    if (mSizeBox->currentIndex() == size_pixel) {
        mWPixelSpin->setValue(mImg.width());
        mHPixelSpin->setValue(mImg.height());
    } else {
        mWPixelSpin->setValue(100);
        mHPixelSpin->setValue(100);
    }

    float units = mResFactor.at(mResUnitBox->currentIndex()) * mUnitFactor.at(mUnitBox->currentIndex());
    float width = (float)mImg.width() / mExifDpi * units;
    mWidthSpin->setValue(width);

    float height = (float)mImg.height() / mExifDpi * units;
    mHeightSpin->setValue(height);

    if (updateSettings)
        loadSettings();
}

void DkResizeDialog::setImage(const QImage &img)
{
    mImg = img;
    initBoxes(true);
    updateSnippets();
    drawPreview();
    mWPixelSpin->selectAll();
}

QImage DkResizeDialog::getResizedImage()
{
    return resizeImg(mImg, false);
}

void DkResizeDialog::setExifDpi(float exifDpi)
{
    mExifDpi = exifDpi;
    mResolutionSpin->blockSignals(true);
    mResolutionSpin->setValue(exifDpi);
    mResolutionSpin->blockSignals(false);
}

float DkResizeDialog::getExifDpi()
{
    return mExifDpi;
}

bool DkResizeDialog::resample()
{
    return mResampleCheck->isChecked();
}

void DkResizeDialog::updateWidth()
{
    float pWidth = (float)mWPixelSpin->value();

    if (mSizeBox->currentIndex() == size_percent)
        pWidth = (float)qRound(pWidth / 100 * mImg.width());

    float units = mResFactor.at(mResUnitBox->currentIndex()) * mUnitFactor.at(mUnitBox->currentIndex());
    float width = pWidth / mExifDpi * units;
    mWidthSpin->setValue(width);
}

void DkResizeDialog::updateHeight()
{
    float pHeight = (float)mHPixelSpin->value();

    if (mSizeBox->currentIndex() == size_percent)
        pHeight = (float)qRound(pHeight / 100 * mImg.height());

    float units = mResFactor.at(mResUnitBox->currentIndex()) * mUnitFactor.at(mUnitBox->currentIndex());
    float height = pHeight / mExifDpi * units;
    mHeightSpin->setValue(height);
}

void DkResizeDialog::updateResolution()
{
    qDebug() << "updating resolution...";
    float wPixel = (float)mWPixelSpin->value();
    float width = (float)mWidthSpin->value();

    float units = mResFactor.at(mResUnitBox->currentIndex()) * mUnitFactor.at(mUnitBox->currentIndex());
    float resolution = wPixel / width * units;
    mResolutionSpin->setValue(resolution);
}

void DkResizeDialog::updatePixelHeight()
{
    float height = (float)mHeightSpin->value();

    // *1000/10 is for more beautiful values
    float units = mResFactor.at(mResUnitBox->currentIndex()) * mUnitFactor.at(mUnitBox->currentIndex());
    float pixelHeight =
        (mSizeBox->currentIndex() != size_percent) ? qRound(height * mExifDpi / units) : qRound(1000.0f * height * mExifDpi / (units * mImg.height())) / 10.0f;

    mHPixelSpin->setValue(pixelHeight);
}

void DkResizeDialog::updatePixelWidth()
{
    float width = (float)mWidthSpin->value();

    float units = mResFactor.at(mResUnitBox->currentIndex()) * mUnitFactor.at(mUnitBox->currentIndex());
    float pixelWidth =
        (mSizeBox->currentIndex() != size_percent) ? qRound(width * mExifDpi / units) : qRound(1000.0f * width * mExifDpi / (units * mImg.width())) / 10.0f;
    mWPixelSpin->setValue(pixelWidth);
}

void DkResizeDialog::on_lockButtonDim_clicked()
{
    mLockButton->setChecked(mLockButtonDim->isChecked());
    if (!mLockButtonDim->isChecked())
        return;

    initBoxes();
    drawPreview();
}

void DkResizeDialog::on_lockButton_clicked()
{
    mLockButtonDim->setChecked(mLockButton->isChecked());

    if (!mLockButton->isChecked())
        return;

    initBoxes();
    drawPreview();
}

void DkResizeDialog::on_wPixelSpin_valueChanged(double val)
{
    if (!mWPixelSpin->hasFocus())
        return;

    updateWidth();

    if (!mLockButton->isChecked()) {
        drawPreview();
        return;
    }

    int newHeight = (mSizeBox->currentIndex() != size_percent) ? qRound((float)val / (float)mImg.width() * mImg.height()) : qRound(val);
    mHPixelSpin->setValue(newHeight);
    updateHeight();
    drawPreview();
}

void DkResizeDialog::on_hPixelSpin_valueChanged(double val)
{
    if (!mHPixelSpin->hasFocus())
        return;

    updateHeight();

    if (!mLockButton->isChecked()) {
        drawPreview();
        return;
    }

    int newWidth = (mSizeBox->currentIndex() != size_percent) ? qRound(val / (float)mImg.height() * (float)mImg.width()) : qRound(val);
    mWPixelSpin->setValue(newWidth);
    updateWidth();
    drawPreview();
}

void DkResizeDialog::on_widthSpin_valueChanged(double val)
{
    if (!mWidthSpin->hasFocus())
        return;

    if (mResampleCheck->isChecked())
        updatePixelWidth();

    if (!mLockButton->isChecked()) {
        drawPreview();
        return;
    }

    mHeightSpin->setValue(val / (float)mImg.width() * (float)mImg.height());

    if (mResampleCheck->isChecked())
        updatePixelHeight();

    if (!mResampleCheck->isChecked())
        updateResolution();

    drawPreview();
}

void DkResizeDialog::on_heightSpin_valueChanged(double val)
{
    if (!mHeightSpin->hasFocus())
        return;

    if (mResampleCheck->isChecked())
        updatePixelHeight();

    if (!mLockButton->isChecked()) {
        drawPreview();
        return;
    }

    mWidthSpin->setValue(val / (float)mImg.height() * (float)mImg.width());

    if (mResampleCheck->isChecked())
        updatePixelWidth();

    if (!mResampleCheck->isChecked())
        updateResolution();
    drawPreview();
}

void DkResizeDialog::on_resolutionSpin_valueChanged(double val)
{
    mExifDpi = (float)val;

    if (!mResolutionSpin->hasFocus())
        return;

    updatePixelWidth();
    updatePixelHeight();

    if (mResampleCheck->isChecked()) {
        drawPreview();
        return;
    }

    initBoxes();
}

void DkResizeDialog::on_unitBox_currentIndexChanged(int)
{
    updateHeight();
    updateWidth();
    // initBoxes();
}

void DkResizeDialog::on_sizeBox_currentIndexChanged(int idx)
{
    if (idx == size_pixel) {
        mWPixelSpin->setDecimals(0);
        mHPixelSpin->setDecimals(0);
    } else {
        mWPixelSpin->setDecimals(2);
        mHPixelSpin->setDecimals(2);
    }

    updatePixelHeight();
    updatePixelWidth();
}

void DkResizeDialog::on_resUnitBox_currentIndexChanged(int)
{
    updateResolution();
    // initBoxes();
}

void DkResizeDialog::on_resampleCheck_clicked()
{
    mResampleBox->setEnabled(mResampleCheck->isChecked());
    mWPixelSpin->setEnabled(mResampleCheck->isChecked());
    mHPixelSpin->setEnabled(mResampleCheck->isChecked());

    if (!mResampleCheck->isChecked()) {
        mLockButton->setChecked(true);
        mLockButtonDim->setChecked(true);
        initBoxes();
    } else
        drawPreview();
}

void DkResizeDialog::on_gammaCorrection_clicked()
{
    drawPreview(); // diem: just update
}

void DkResizeDialog::on_resampleBox_currentIndexChanged(int)
{
    drawPreview();
}

void DkResizeDialog::updateSnippets()
{
    if (mImg.isNull() /*|| !isVisible()*/)
        return;

    mOrigView->setImage(mImg);
    mOrigView->fullView();
    mOrigView->zoomConstraints(mOrigView->get100Factor());
}

void DkResizeDialog::drawPreview()
{
    if (mImg.isNull() || !isVisible())
        return;

    QImage newImg = mOrigView->getCurrentImageRegion();

    // TODO: thread here!
    QImage img = resizeImg(newImg);

    // TODO: is there a better way of mapping the pixels? (ipl here introduces artifacts that are not in the final image)
    img = img.scaled(mPreviewLabel->size(), Qt::KeepAspectRatio, Qt::FastTransformation);
    mPreviewLabel->setPixmap(QPixmap::fromImage(img));
}

QImage DkResizeDialog::resizeImg(QImage img, bool silent)
{
    if (img.isNull())
        return img;

    QSize newSize;

    if (mSizeBox->currentIndex() == size_percent)
        newSize = QSize(qRound(mWPixelSpin->value() / 100.0f * mImg.width()), qRound(mHPixelSpin->value() / 100.0f * mImg.height()));
    else
        newSize = QSize(qRound(mWPixelSpin->value()), qRound(mHPixelSpin->value()));

    QSize imgSize = mImg.size();

    // nothing to do
    if (mImg.size() == newSize)
        return img;

    if (mImg.size() != img.size()) {
        // compute relative size
        float relWidth = (float)newSize.width() / (float)imgSize.width();
        float relHeight = (float)newSize.height() / (float)imgSize.height();

        newSize = QSize(qRound(img.width() * relWidth), qRound(img.height() * relHeight));
    }

    if (newSize.width() < mWPixelSpin->minimum() || newSize.width() > mWPixelSpin->maximum() || newSize.height() < mHPixelSpin->minimum()
        || newSize.height() > mHPixelSpin->maximum()) {
        if (!silent) {
            QMessageBox errorDialog(this);
            errorDialog.setIcon(QMessageBox::Critical);
            errorDialog.setText(tr("Sorry, but the image size %1 x %2 is illegal.").arg(newSize.width()).arg(newSize.height()));
            errorDialog.show();
            errorDialog.exec();
        }
    }

    QImage rImg = DkImage::resizeImage(img, newSize, 1.0f, mResampleBox->currentIndex(), mGammaCorrection->isChecked());

    if (rImg.isNull() && !silent) {
        qDebug() << "image size: " << newSize;
        QMessageBox errorDialog(this);
        errorDialog.setIcon(QMessageBox::Critical);
        errorDialog.setText(tr("Sorry, the image is too large: %1").arg(DkImage::getBufferSize(newSize, 32)));
        errorDialog.show();
        errorDialog.exec();
    }

    return rImg;
}

void DkResizeDialog::setVisible(bool visible)
{
    QDialog::setVisible(visible);

    updateSnippets();
    drawPreview();
}

void DkResizeDialog::resizeEvent(QResizeEvent *re)
{
    drawPreview();
    QDialog::resizeEvent(re);
}

// DkShortcutDelegate --------------------------------------------------------------------
DkShortcutDelegate::DkShortcutDelegate(QObject *parent)
    : QItemDelegate(parent)
{
    mItem = 0;
    mClearPm = DkImage::loadIcon(":/nomacs/img/close.svg");
}

QWidget *DkShortcutDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget *scW = QItemDelegate::createEditor(parent, option, index);

    if (!scW)
        return scW;

    connect(scW, SIGNAL(keySequenceChanged(const QKeySequence &)), this, SLOT(keySequenceChanged(const QKeySequence &)));

    return scW;
}

QSize DkShortcutDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize s = QItemDelegate::sizeHint(option, index);

    if (index.column() == 1)
        s.setWidth(s.width() + s.height()); // make room for our x

    return s;
}

void DkShortcutDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    const_cast<DkShortcutDelegate *>(this)->mItem = index.internalPointer();
    emit clearDuplicateSignal();

    QItemDelegate::setEditorData(editor, index);
}

bool DkShortcutDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    // did the user click the x?
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *e = (QMouseEvent *)event;
        int clickX = e->x();
        int clickY = e->y();

        QRect r = option.rect;
        int x = r.left() + r.width() - r.height();

        if (clickX > x && clickX < x + r.height())
            if (clickY > r.top() && clickY < r.top() + r.height()) {
                model->setData(index, QKeySequence());
            }
    }

    mItem = index.internalPointer();

    return QItemDelegate::editorEvent(event, model, option, index);
}

void DkShortcutDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // calling before means, that our x is always in front
    QItemDelegate::paint(painter, option, index);

    TreeItem *ti = static_cast<TreeItem *>(index.internalPointer());

    if (index.column() == 1 && ti && !ti->data(1).toString().isEmpty()) {
        QRect r = option.rect; // getting the rect of the cell
        int s = r.height();
        QRect pmr(r.right() - s, r.top(), s, s);

        painter->drawPixmap(pmr, mClearPm);
    }
}

void DkShortcutDelegate::textChanged(const QString &)
{
} // dummy since the moccer is to dumb to get #if defs

void DkShortcutDelegate::keySequenceChanged(const QKeySequence &keySequence)
{
    emit checkDuplicateSignal(keySequence, mItem);
}

// fun fact: there are ~10^4500 (binary) images of size 128x128
// increase counter if you think this is fascinating: 1

// DkShortcutsModel --------------------------------------------------------------------
DkShortcutsModel::DkShortcutsModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    // create root
    QVector<QVariant> rootData;
    rootData << tr("Name") << tr("Shortcut");

    mRootItem = new TreeItem(rootData);
}

DkShortcutsModel::~DkShortcutsModel()
{
    delete mRootItem;
}

QModelIndex DkShortcutsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<TreeItem *>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);

    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex DkShortcutsModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem *>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == mRootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int DkShortcutsModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<TreeItem *>(parent.internalPointer());

    return parentItem->childCount();
}

int DkShortcutsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem *>(parent.internalPointer())->columnCount();
    else
        return mRootItem->columnCount();
}

QVariant DkShortcutsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        qDebug() << "invalid row: " << index.row();
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
        return item->data(index.column());
    }

    return QVariant();
}

QVariant DkShortcutsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    return mRootItem->data(section);
}

bool DkShortcutsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (index.column() == 1) {
        QKeySequence ks = value.value<QKeySequence>();
        TreeItem *duplicate = mRootItem->find(ks, index.column());
        if (duplicate)
            duplicate->setData(QKeySequence(), index.column());
        // if (!duplicate) qDebug() << ks << " no duplicate found...";

        TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
        item->setData(ks, index.column());
    } else {
        TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
        item->setData(value, index.column());
    }

    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags DkShortcutsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEditable;

    Qt::ItemFlags flags;

    if (index.column() == 0)
        flags = QAbstractItemModel::flags(index);
    if (index.column() == 1)
        flags = QAbstractItemModel::flags(index) | Qt::ItemIsEditable;

    return flags;
}

void DkShortcutsModel::addDataActions(QVector<QAction *> actions, const QString &name)
{
    // create root
    QVector<QVariant> menuData;
    menuData << name;

    TreeItem *menuItem = new TreeItem(menuData, mRootItem);

    for (int idx = 0; idx < actions.size(); idx++) {
        // skip NULL actions - this should never happen!
        if (actions[idx]->text().isNull()) {
            qDebug() << "NULL Action detected when creating shortcuts...";
            continue;
        }

        QString text = actions[idx]->text().remove("&");

        QVector<QVariant> actionData;
        actionData << text << actions[idx]->shortcut();

        TreeItem *dataItem = new TreeItem(actionData, menuItem);
        menuItem->appendChild(dataItem);
    }

    mRootItem->appendChild(menuItem);
    mActions.append(actions);
}

void DkShortcutsModel::checkDuplicate(const QString &text, void *item)
{
    if (text.isEmpty()) {
        emit duplicateSignal("");
        return;
    }

    QKeySequence ks(text);
    checkDuplicate(ks, item);
}

void DkShortcutsModel::checkDuplicate(const QKeySequence &ks, void *item)
{
    if (ks.isEmpty()) {
        emit duplicateSignal("");
        return;
    }

    TreeItem *duplicate = mRootItem->find(ks, 1);

    if (duplicate == item)
        return;

    if (duplicate && duplicate->parent()) {
        emit duplicateSignal(tr("%1 already used by %2 > %3\nPress ESC to undo changes")
                                 .arg(duplicate->data(1).toString())
                                 .arg(duplicate->parent()->data(0).toString())
                                 .arg(duplicate->data(0).toString()));
    } else if (duplicate) {
        emit duplicateSignal(tr("%1 already used by %2\nPress ESC to undo changes").arg(duplicate->data(1).toString()).arg(duplicate->data(0).toString()));
    } else
        emit duplicateSignal("");
}

void DkShortcutsModel::clearDuplicateInfo() const
{
    qDebug() << "duplicates cleared...";
    emit duplicateSignal("");
}

void DkShortcutsModel::resetActions()
{
    DefaultSettings settings;
    settings.beginGroup("CustomShortcuts");

    for (int pIdx = 0; pIdx < mActions.size(); pIdx++) {
        QVector<QAction *> cActions = mActions.at(pIdx);

        for (int idx = 0; idx < cActions.size(); idx++) {
            QString val = settings.value(cActions[idx]->text(), "no-shortcut").toString();

            if (val != "no-shortcut") {
                cActions[idx]->setShortcut(QKeySequence());
            }
        }
    }

    settings.endGroup();
}

void DkShortcutsModel::saveActions() const
{
    if (!mRootItem)
        return;

    DefaultSettings settings;
    settings.beginGroup("CustomShortcuts");

    // loop all menu entries
    for (int idx = 0; idx < mRootItem->childCount(); idx++) {
        TreeItem *cMenu = mRootItem->child(idx);
        QVector<QAction *> cActions = mActions.at(idx);

        // loop all action entries
        for (int mIdx = 0; mIdx < cMenu->childCount(); mIdx++) {
            TreeItem *cItem = cMenu->child(mIdx);
            QKeySequence ks = cItem->data(1).value<QKeySequence>();

            if (cActions.at(mIdx)->shortcut() != ks) {
                if (cActions.at(mIdx)->text().isEmpty()) {
                    qDebug() << "empty action detected! shortcut is: " << ks;
                    continue;
                }

                QString aT = cActions.at(mIdx)->text().remove("&");

                cActions.at(mIdx)->setShortcut(ks); // assign new shortcut
                settings.setValue(aT, ks.toString()); // note this works as long as you don't change the language!
            }
        }
    }
    settings.endGroup();
}

// DkShortcutsDialog --------------------------------------------------------------------
DkShortcutsDialog::DkShortcutsDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    createLayout();
}

void DkShortcutsDialog::createLayout()
{
    setWindowTitle(tr("Keyboard Shortcuts"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    // register our special shortcut editor
    QItemEditorFactory *factory = new QItemEditorFactory;

    QItemEditorCreatorBase *shortcutListCreator = new QStandardItemEditorCreator<QKeySequenceEdit>();

    factory->registerEditor(QVariant::KeySequence, shortcutListCreator);

    QItemEditorFactory::setDefaultFactory(factory);

    // create our beautiful shortcut view
    mModel = new DkShortcutsModel(this);

    DkShortcutDelegate *scDelegate = new DkShortcutDelegate(this);

    QTreeView *treeView = new QTreeView(this);
    treeView->setModel(mModel);
    treeView->setItemDelegate(scDelegate);
    treeView->setAlternatingRowColors(true);
    treeView->setIndentation(8);
    treeView->header()->resizeSection(0, 200);

    mNotificationLabel = new QLabel(this);
    mNotificationLabel->setObjectName("DkDecentInfo");
    mNotificationLabel->setProperty("warning", true);
    // notificationLabel->setTextFormat(Qt::)

    mDefaultButton = new QPushButton(tr("Set to &Default"), this);
    mDefaultButton->setToolTip(tr("Removes All Custom Shortcuts"));
    connect(mDefaultButton, SIGNAL(clicked()), this, SLOT(defaultButtonClicked()));
    connect(mModel, SIGNAL(duplicateSignal(const QString &)), mNotificationLabel, SLOT(setText(const QString &)));

    connect(scDelegate, SIGNAL(checkDuplicateSignal(const QKeySequence &, void *)), mModel, SLOT(checkDuplicate(const QKeySequence &, void *)));
    connect(scDelegate, SIGNAL(clearDuplicateSignal()), mModel, SLOT(clearDuplicateInfo()));

    // mButtons
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    buttons->addButton(mDefaultButton, QDialogButtonBox::ActionRole);

    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    layout->addWidget(treeView);
    layout->addWidget(mNotificationLabel);
    // layout->addSpacing()
    layout->addWidget(buttons);
    resize(420, 500);
}

void DkShortcutsDialog::addActions(const QVector<QAction *> &actions, const QString &name)
{
    QString cleanName = name;
    mModel->addDataActions(actions, cleanName.remove("&"));
}

void DkShortcutsDialog::contextMenu(const QPoint &)
{
}

void DkShortcutsDialog::defaultButtonClicked()
{
    if (mModel)
        mModel->resetActions();

    DefaultSettings settings;
    settings.remove("CustomShortcuts");

    QDialog::reject();
}

void DkShortcutsDialog::accept()
{
    // assign shortcuts & save them if they are changed
    if (mModel)
        mModel->saveActions();

    QDialog::accept();
}

// DkTextDialog --------------------------------------------------------------------
DkTextDialog::DkTextDialog(QWidget *parent /* = 0 */, Qt::WindowFlags flags /* = 0 */)
    : QDialog(parent, flags)
{
    setWindowTitle(tr("Text Editor"));
    createLayout();
}

void DkTextDialog::createLayout()
{
    textEdit = new QTextEdit(this);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true); // ok is auto-default
    buttons->button(QDialogButtonBox::Ok)->setText(tr("&Save"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Close"));

    connect(buttons, SIGNAL(accepted()), this, SLOT(save()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    // dialog layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(textEdit);
    layout->addWidget(buttons);
}

void DkTextDialog::setText(const QStringList &text)
{
    textEdit->setText(text.join("\n"));
}

void DkTextDialog::save()
{
    QStringList folders = DkSettingsManager::param().global().recentFolders;
    QString savePath = QDir::rootPath();

    if (folders.size() > 0)
        savePath = folders.first();

    QStringList extList;
    extList << tr("Text File (*.txt)") << tr("All Files (*.*)");
    QString saveFilters(extList.join(";;"));

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Text File"), savePath, saveFilters, nullptr, DkDialog::fileDialogOptions());

    if (fileName.isEmpty())
        return;

    QFile file(fileName);

    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << textEdit->toPlainText();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Could not save: %1\n%2").arg(fileName).arg(file.errorString()));
        return;
    }

    file.close();
    accept();
}

// DkUpdateDialog --------------------------------------------------------------------
DkUpdateDialog::DkUpdateDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    init();
}

void DkUpdateDialog::init()
{
    createLayout();

    connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(okButtonClicked()));
}

void DkUpdateDialog::createLayout()
{
    setFixedWidth(300);
    setFixedHeight(150);
    setWindowTitle(tr("nomacs updater"));

    QGridLayout *gridlayout = new QGridLayout;
    upperLabel = new QLabel;
    upperLabel->setOpenExternalLinks(true);

    QWidget *lowerWidget = new QWidget;
    QHBoxLayout *hbox = new QHBoxLayout;
    okButton = new QPushButton(tr("Install Now"));
    cancelButton = new QPushButton(tr("Cancel"));
    hbox->addStretch();
    hbox->addWidget(okButton);
    hbox->addWidget(cancelButton);
    lowerWidget->setLayout(hbox);

    gridlayout->addWidget(upperLabel, 0, 0);
    gridlayout->addWidget(lowerWidget, 1, 0);

    setLayout(gridlayout);
}

void DkUpdateDialog::okButtonClicked()
{
    emit startUpdate();
    close();
}

// DkPrintPreviewDialog --------------------------------------------------------------------
DkPrintPreviewDialog::DkPrintPreviewDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    setWindowTitle(tr("Print Preview"));
    init();
}

void DkPrintPreviewDialog::setImage(const QImage &img)
{
    mPreview->setImage(img);

    if (!img.isNull() && img.width() > img.height())
        mPreview->setLandscapeOrientation();
    else
        mPreview->setPortraitOrientation();
}

void DkPrintPreviewDialog::addImage(const QImage &img)
{
    mPreview->addImage(img);
}

void DkPrintPreviewDialog::init()
{
    if (!mPrinter) {
#ifdef Q_OS_WIN
        if (QPrinterInfo::defaultPrinter().isNull())
            mPrinter = new QPrinter(); // new QPrinter(QPrinter::HighResolution);
        else
            mPrinter = new QPrinter(QPrinterInfo::defaultPrinter());

#else
        mPrinter = new QPrinter();
#endif
    }

    mPreview = new DkPrintPreviewWidget(mPrinter, this);

    createIcons();
    createLayout();

    setMinimumHeight(600);
    setMinimumWidth(800);

    connect(mPreview, SIGNAL(dpiChanged(int)), mDpiBox, SLOT(setValue(int)));
}

void DkPrintPreviewDialog::createIcons()
{
    mIcons.resize(print_end);

    mIcons[print_fit_width] = DkImage::loadIcon(":/nomacs/img/fit-width.svg");
    mIcons[print_fit_page] = DkImage::loadIcon(":/nomacs/img/zoom-reset.svg");
    mIcons[print_zoom_in] = DkImage::loadIcon(":/nomacs/img/zoom-in.svg");
    mIcons[print_zoom_out] = DkImage::loadIcon(":/nomacs/img/zoom-out.svg");
    mIcons[print_reset_dpi] = DkImage::loadIcon(":/nomacs/img/zoom-100.svg");
    mIcons[print_landscape] = DkImage::loadIcon(":/nomacs/img/landscape.svg");
    mIcons[print_portrait] = DkImage::loadIcon(":/nomacs/img/portrait.svg");
    mIcons[print_setup] = DkImage::loadIcon(":/nomacs/img/print-setup.svg");
    mIcons[print_printer] = DkImage::loadIcon(":/nomacs/img/print.svg");
}

void DkPrintPreviewDialog::createLayout()
{
    QAction *fitWidth = new QAction(mIcons[print_fit_width], tr("Fit Width"), this);
    QAction *fitPage = new QAction(mIcons[print_fit_page], tr("Fit Page"), this);

    QAction *zoomIn = new QAction(mIcons[print_zoom_in], tr("Zoom in"), this);
    zoomIn->setShortcut(Qt::Key_Plus);

    QAction *zoomOut = new QAction(mIcons[print_zoom_out], tr("Zoom out"), this);
    zoomOut->setShortcut(Qt::Key_Minus);

    QString zoomTip = tr("keep ALT key pressed to zoom with the mouse wheel");
    zoomIn->setToolTip(zoomTip);
    zoomOut->setToolTip(zoomTip);

    mDpiBox = new QSpinBox(this);
    mDpiBox->setSuffix(" dpi");
    mDpiBox->setMinimum(10);
    mDpiBox->setMaximum(9999);
    mDpiBox->setSingleStep(100);

    // Portrait/Landscape
    QAction *prt = new QAction(mIcons[print_portrait], tr("Portrait"), this);
    prt->setObjectName("portrait");

    QAction *lsc = new QAction(mIcons[print_landscape], tr("Landscape"), this);
    lsc->setObjectName("landscape");

    // Print
    QAction *pageSetup = new QAction(mIcons[print_setup], tr("Page setup"), this);
    QAction *printAction = new QAction(mIcons[print_printer], tr("Print"), this);

    // create toolbar
    QToolBar *toolbar = new QToolBar(tr("Print Preview"), this);

    toolbar->addAction(fitWidth);
    toolbar->addAction(fitPage);

    toolbar->addAction(zoomIn);
    toolbar->addAction(zoomOut);

    toolbar->addWidget(mDpiBox);

    toolbar->addAction(prt);
    toolbar->addAction(lsc);

    toolbar->addSeparator();
    toolbar->addAction(pageSetup);
    toolbar->addAction(printAction);

    toolbar->setIconSize(QSize(DkSettingsManager::param().effectiveIconSize(this), DkSettingsManager::param().effectiveIconSize(this)));

    // Cannot use the actions' triggered signal here, since it doesn't autorepeat
    QToolButton *zoomInButton = static_cast<QToolButton *>(toolbar->widgetForAction(zoomIn));
    zoomInButton->setAutoRepeat(true);
    zoomInButton->setAutoRepeatInterval(200);
    zoomInButton->setAutoRepeatDelay(200);

    QToolButton *zoomOutButton = static_cast<QToolButton *>(toolbar->widgetForAction(zoomOut));
    zoomOutButton->setAutoRepeat(true);
    zoomOutButton->setAutoRepeatInterval(200);
    zoomOutButton->setAutoRepeatDelay(200);

    connect(mDpiBox, SIGNAL(valueChanged(int)), mPreview, SLOT(changeDpi(int)));
    connect(zoomInButton, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(zoomOutButton, SIGNAL(clicked()), this, SLOT(zoomOut()));

    connect(lsc, SIGNAL(triggered()), mPreview, SLOT(setLandscapeOrientation()));
    connect(prt, SIGNAL(triggered()), mPreview, SLOT(setPortraitOrientation()));
    connect(fitWidth, SIGNAL(triggered()), this, SLOT(previewFitWidth()));
    connect(fitPage, SIGNAL(triggered()), this, SLOT(previewFitPage()));

    connect(printAction, SIGNAL(triggered(bool)), this, SLOT(print()));
    connect(pageSetup, SIGNAL(triggered(bool)), this, SLOT(pageSetup()));

    QMainWindow *mw = new QMainWindow();
    mw->addToolBar(toolbar);
    mw->setCentralWidget(mPreview);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mw);

    setLayout(layout);
}

void DkPrintPreviewDialog::zoomIn()
{
    mPreview->zoomIn();
}

void DkPrintPreviewDialog::zoomOut()
{
    mPreview->zoomOut();
}

void DkPrintPreviewDialog::zoom(int scale)
{
    mPreview->setZoomFactor(scale / 100.0);
}

void DkPrintPreviewDialog::previewFitWidth()
{
    mPreview->fitToWidth();
}

void DkPrintPreviewDialog::previewFitPage()
{
    mPreview->fitInView();
}

void DkPrintPreviewDialog::updateDpiFactor(qreal dpi)
{
    mDpiBox->setValue(qRound(dpi));
}

void DkPrintPreviewDialog::pageSetup()
{
    QPageSetupDialog pageSetup(mPrinter, this);

    if (pageSetup.exec() == QDialog::Accepted) {
        // update possible orientation changes
        if (mPreview->orientation() == QPrinter::Portrait) {
            mPreview->setPortraitOrientation();
        } else {
            mPreview->setLandscapeOrientation();
        }
    }
}

void DkPrintPreviewDialog::print()
{
    // TODO: move from QRect to smth else
    QRect pr = mPrinter->pageLayout().paintRectPixels(mPrinter->resolution());

    QPrintDialog *pDialog = new QPrintDialog(mPrinter, this);

    if (pDialog->exec() == QDialog::Accepted) {
        // if the page rect is changed - we have to newly fit the images...
        if (pr != mPrinter->pageRect(QPrinter::Inch))
            mPreview->fitImages();

        mPreview->paintForPrinting();
        close();
    }
}

// DkPrintPreviewWidget --------------------------------------------------------------------
DkPrintPreviewWidget::DkPrintPreviewWidget(QPrinter *printer, QWidget *parent, Qt::WindowFlags flags)
    : QPrintPreviewWidget(printer, parent, flags)
{
    mPrinter = printer;
    connect(this, SIGNAL(paintRequested(QPrinter *)), this, SLOT(paintPreview(QPrinter *)));
}

void DkPrintPreviewWidget::paintEvent(QPaintEvent *event)
{
    // TODO: can we get a better anti-aliasing here?
    QPrintPreviewWidget::paintEvent(event);
}

void DkPrintPreviewWidget::setImage(const QImage &img)
{
    mPrintImages.clear();
    addImage(img);
}

void DkPrintPreviewWidget::addImage(const QImage &img)
{
    if (!mPrinter) {
        qWarning() << "cannot add images to preview if the printer is empty";
        return;
    }

    QSharedPointer<DkPrintImage> pi(new DkPrintImage(img, mPrinter));

    // for now - think of adding multiple images here
    mPrintImages << pi;
    fitImages();
}

void DkPrintPreviewWidget::fitImages()
{
    double dpi = 0;

    for (auto pi : mPrintImages) {
        pi->fit();
        dpi = pi->dpi();
    }

    updatePreview();
    emit dpiChanged(qRound(dpi));
}

void DkPrintPreviewWidget::wheelEvent(QWheelEvent *event)
{
    auto delta = event->angleDelta().y();

    if ((event->modifiers() != Qt::AltModifier) || (delta == 0)) {
        QPrintPreviewWidget::wheelEvent(event);
        return;
    }

    if (DkSettingsManager::param().display().invertZoom)
        delta *= -1;
    if (delta > 0)
        zoomIn();
    else
        zoomOut();
    emit zoomChanged();

    QPrintPreviewWidget::wheelEvent(event);
}

void DkPrintPreviewWidget::centerImage()
{
    for (auto pi : mPrintImages)
        pi->center();

    updatePreview();
}

void DkPrintPreviewWidget::setLandscapeOrientation()
{
    QPrintPreviewWidget::setLandscapeOrientation();

    fitImages();
    fitInView();
}

void DkPrintPreviewWidget::setPortraitOrientation()
{
    QPrintPreviewWidget::setPortraitOrientation();

    fitImages();
    fitInView();
}

void DkPrintPreviewWidget::changeDpi(int value)
{
    double inchW = mPrinter->pageRect(QPrinter::Inch).width();
    int pxW = mPrinter->pageRect(QPrinter::DevicePixel).width();
    double sf = ((double)pxW / inchW) / value;

    for (auto pi : mPrintImages)
        pi->scale(sf);

    updatePreview();
}

void DkPrintPreviewWidget::paintPreview(QPrinter *printer)
{
    QPainter painter(printer);

    for (auto pi : mPrintImages) {
        pi->draw(painter);

        if (pi != mPrintImages.last())
            printer->newPage();
    }
}

void DkPrintPreviewWidget::paintForPrinting()
{
    int to = mPrinter->toPage() ? mPrinter->toPage() : mPrintImages.size();

    QPainter painter(mPrinter);

    for (int idx = mPrinter->fromPage(); idx <= to && idx < mPrintImages.size(); idx++) {
        mPrintImages[idx]->draw(painter, true);

        if (idx + 1 < to)
            mPrinter->newPage();
    }
}

// DkOpacityDialog --------------------------------------------------------------------
DkOpacityDialog::DkOpacityDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    createLayout();
}

void DkOpacityDialog::createLayout()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    slider = new DkSlider(tr("Window Opacity"), this);
    slider->setMinimum(5);

    // mButtons
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    layout->addWidget(slider);
    layout->addWidget(buttons);
}

int DkOpacityDialog::value() const
{
    return slider->value();
}

// DkExportTiffDialog --------------------------------------------------------------------
DkExportTiffDialog::DkExportTiffDialog(QWidget *parent /* = 0 */, Qt::WindowFlags f /* = 0 */)
    : QDialog(parent, f)
{
    setWindowTitle(tr("Export Multi-Page TIFF"));
    createLayout();
    setAcceptDrops(true);

    connect(this, SIGNAL(updateImage(const QImage &)), mViewport, SLOT(setImage(const QImage &)));
    connect(&mWatcher, SIGNAL(finished()), this, SLOT(processingFinished()));
    connect(this, SIGNAL(infoMessage(const QString &)), mMsgLabel, SLOT(setText(const QString &)));
    connect(this, SIGNAL(updateProgress(int)), mProgress, SLOT(setValue(int)));
    QMetaObject::connectSlotsByName(this);
}

void DkExportTiffDialog::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
        QUrl url = event->mimeData()->urls().at(0);
        url = url.toLocalFile();

        setFile(url.toString());
    }
}

void DkExportTiffDialog::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QUrl url = event->mimeData()->urls().at(0);
        url = url.toLocalFile();
        QFileInfo file = QFileInfo(url.toString());

        if (file.exists() && file.suffix().indexOf(QRegularExpression("tif"), QRegularExpression::CaseInsensitiveOption) != -1)
            event->acceptProposedAction();
    }
}

void DkExportTiffDialog::createLayout()
{
    // progress bar
    mProgress = new QProgressBar(this);
    mProgress->hide();

    mMsgLabel = new QLabel(this);
    mMsgLabel->setObjectName("DkWarningInfo");
    mMsgLabel->hide();

    // open handles
    QLabel *openLabel = new QLabel(tr("Multi-Page TIFF:"), this);
    openLabel->setAlignment(Qt::AlignRight);

    QPushButton *openButton = new QPushButton(tr("&Browse"), this);
    openButton->setObjectName("openButton");

    mTiffLabel = new QLabel(tr("No Multi-Page TIFF loaded"), this);

    // save handles
    QLabel *saveLabel = new QLabel(tr("Save Folder:"), this);
    saveLabel->setAlignment(Qt::AlignRight);

    QPushButton *saveButton = new QPushButton(tr("&Browse"), this);
    saveButton->setObjectName("saveButton");

    mFolderLabel = new QLabel(tr("Specify a Save Folder"), this);

    // file name handles
    QLabel *fileLabel = new QLabel(tr("Filename:"), this);
    fileLabel->setAlignment(Qt::AlignRight);

    mFileEdit = new QLineEdit("tiff_page", this);
    mFileEdit->setObjectName("fileEdit");

    mSuffixBox = new QComboBox(this);
    mSuffixBox->addItems(DkSettingsManager::param().app().saveFilters);
    mSuffixBox->setCurrentIndex(DkSettingsManager::param().app().saveFilters.indexOf(QRegularExpression(".*tif.*")));

    // export handles
    QLabel *exportLabel = new QLabel(tr("Export Pages"));
    exportLabel->setAlignment(Qt::AlignRight);

    mFromPage = new QSpinBox(0);

    mToPage = new QSpinBox(0);

    mOverwrite = new QCheckBox(tr("Overwrite"));

    mControlWidget = new QWidget(this);
    QGridLayout *controlLayout = new QGridLayout(mControlWidget);
    controlLayout->addWidget(openLabel, 0, 0);
    controlLayout->addWidget(openButton, 0, 1, 1, 2);
    controlLayout->addWidget(mTiffLabel, 0, 3, 1, 2);
    // controlLayout->setColumnStretch(3, 1);

    controlLayout->addWidget(saveLabel, 1, 0);
    controlLayout->addWidget(saveButton, 1, 1, 1, 2);
    controlLayout->addWidget(mFolderLabel, 1, 3, 1, 2);
    // controlLayout->setColumnStretch(3, 1);

    controlLayout->addWidget(fileLabel, 2, 0);
    controlLayout->addWidget(mFileEdit, 2, 1, 1, 2);
    controlLayout->addWidget(mSuffixBox, 2, 3, 1, 2);
    // controlLayout->setColumnStretch(3, 1);

    controlLayout->addWidget(exportLabel, 3, 0);
    controlLayout->addWidget(mFromPage, 3, 1);
    controlLayout->addWidget(mToPage, 3, 2);
    controlLayout->addWidget(mOverwrite, 3, 3);
    controlLayout->setColumnStretch(5, 1);

    // shows the image if it could be loaded
    mViewport = new DkBaseViewPort(this);
    mViewport->setForceFastRendering(true);
    mViewport->setPanControl(QPointF(0.0f, 0.0f));

    // Buttons
    mButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    mButtons->button(QDialogButtonBox::Ok)->setText(tr("&Export"));
    mButtons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    connect(mButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(mButtons, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mViewport);
    layout->addWidget(mProgress);
    layout->addWidget(mMsgLabel);
    layout->addWidget(mControlWidget);
    layout->addWidget(mButtons);

    enableTIFFSave(false);
}

void DkExportTiffDialog::on_openButton_pressed()
{
    // load system default open dialog
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open TIFF"),
                                                    mFilePath,
                                                    DkSettingsManager::param().app().saveFilters.filter(QRegularExpression(".*tif.*")).join(";;"),
                                                    nullptr,
                                                    DkDialog::fileDialogOptions());

    setFile(fileName);
}

void DkExportTiffDialog::on_saveButton_pressed()
{
    qDebug() << "save triggered...";

    // load system default open dialog
    QString dirName =
        QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"), mSaveDirPath, QFileDialog::ShowDirsOnly | DkDialog::fileDialogOptions());

    if (QDir(dirName).exists()) {
        mSaveDirPath = dirName;
        mFolderLabel->setText(mSaveDirPath);
    }
}

void DkExportTiffDialog::on_fileEdit_textChanged(const QString &filename)
{
    qDebug() << "new file name: " << filename;
}

void DkExportTiffDialog::reject()
{
    // not sure if this is a nice way to do: but we change cancel behavior while processing
    if (mProcessing)
        mProcessing = false;
    else
        QDialog::reject();
}

void DkExportTiffDialog::accept()
{
    mProgress->setMinimum(mFromPage->value() - 1);
    mProgress->setMaximum(mToPage->value());
    mProgress->setValue(mProgress->minimum());
    mProgress->show();
    mMsgLabel->show();

    enableAll(false);

    QString suffix = mSuffixBox->currentText();

    for (int idx = 0; idx < DkSettingsManager::param().app().fileFilters.size(); idx++) {
        if (suffix.contains("(" + DkSettingsManager::param().app().fileFilters.at(idx))) {
            suffix = DkSettingsManager::param().app().fileFilters.at(idx);
            suffix.replace("*", "");
            break;
        }
    }

    QFileInfo sFile(mSaveDirPath, mFileEdit->text() + "-" + suffix);

    emit infoMessage("");

    QFuture<int> future = QtConcurrent::run(this,
                                            &nmc::DkExportTiffDialog::exportImages,
                                            sFile.absoluteFilePath(),
                                            mFromPage->value(),
                                            mToPage->value(),
                                            mOverwrite->isChecked());
    mWatcher.setFuture(future);
}

void DkExportTiffDialog::processingFinished()
{
    enableAll(true);
    mProgress->hide();
    mMsgLabel->hide();

    if (mWatcher.future() == QDialog::Accepted)
        QDialog::accept();
}

int DkExportTiffDialog::exportImages(const QString &saveFilePath, int from, int to, bool overwrite)
{
    mProcessing = true;

    QFileInfo saveInfo(saveFilePath);

    // Do your job
    for (int idx = from; idx <= to; idx++) {
        QFileInfo cInfo(saveInfo.absolutePath(), saveInfo.baseName() + QString::number(idx) + "." + saveInfo.suffix());
        qDebug() << "trying to save: " << cInfo.absoluteFilePath();

        emit updateProgress(idx - 1);

        // user wants to overwrite files
        if (cInfo.exists() && overwrite) {
            QFile f(cInfo.absoluteFilePath());
            f.remove();
        } else if (cInfo.exists()) {
            emit infoMessage(tr("%1 exists, skipping...").arg(cInfo.fileName()));
            continue;
        }

        if (!mLoader.loadPageAt(idx)) { // load next
            emit infoMessage(tr("Sorry, I could not load page: %1").arg(idx));
            continue;
        }

        QString lSaveFilePath = mLoader.save(cInfo.absoluteFilePath(), mLoader.image(), 90); // TODO: ask user for compression?
        QFileInfo lSaveInfo = QFileInfo(lSaveFilePath);

        if (!lSaveInfo.exists() || !lSaveInfo.isFile())
            emit infoMessage(tr("Sorry, I could not save: %1").arg(cInfo.fileName()));

        emit updateImage(mLoader.image());
        emit updateProgress(idx);

        // user canceled?
        if (!mProcessing)
            return QDialog::Rejected;
    }

    mProcessing = false;

    return QDialog::Accepted;
}

void DkExportTiffDialog::setFile(const QString &filePath)
{
    if (!QFileInfo(filePath).exists())
        return;

    QFileInfo fInfo(filePath);
    mFilePath = filePath;
    mSaveDirPath = fInfo.absolutePath();
    mFolderLabel->setText(mSaveDirPath);
    mTiffLabel->setText(filePath);
    mFileEdit->setText(fInfo.baseName());

    mLoader.loadGeneral(filePath, true);
    mViewport->setImage(mLoader.image());

    enableTIFFSave(mLoader.getNumPages() > 1);

    mFromPage->setRange(1, mLoader.getNumPages());
    mToPage->setRange(1, mLoader.getNumPages());

    mFromPage->setValue(1);
    mToPage->setValue(mLoader.getNumPages());
}

void DkExportTiffDialog::enableAll(bool enable)
{
    enableTIFFSave(enable);
    mControlWidget->setEnabled(enable);
}

void DkExportTiffDialog::enableTIFFSave(bool enable)
{
    mFileEdit->setEnabled(enable);
    mSuffixBox->setEnabled(enable);
    mFromPage->setEnabled(enable);
    mToPage->setEnabled(enable);
    mButtons->button(QDialogButtonBox::Ok)->setEnabled(enable);
}

#ifdef WITH_OPENCV

// DkMosaicDialog --------------------------------------------------------------------
DkMosaicDialog::DkMosaicDialog(QWidget *parent /* = 0 */, Qt::WindowFlags f /* = 0 */)
    : QDialog(parent, f)
{
    mProcessing = false;
    mPostProcessing = false;
    mUpdatePostProcessing = false;

    setWindowTitle(tr("Create Mosaic Image"));
    createLayout();
    setAcceptDrops(true);

    connect(this, SIGNAL(updateImage(const QImage &)), mPreview, SLOT(setImage(const QImage &)));
    connect(&mMosaicWatcher, SIGNAL(finished()), this, SLOT(mosaicFinished()));
    connect(&mPostProcessWatcher, SIGNAL(finished()), this, SLOT(postProcessFinished()));
    connect(&mPostProcessWatcher, SIGNAL(canceled()), this, SLOT(postProcessFinished()));
    connect(this, SIGNAL(infoMessage(const QString &)), mMsgLabel, SLOT(setText(const QString &)));
    connect(this, SIGNAL(updateProgress(int)), mProgress, SLOT(setValue(int)));
    QMetaObject::connectSlotsByName(this);
}

void DkMosaicDialog::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
        QUrl url = event->mimeData()->urls().at(0);
        url = url.toLocalFile();

        setFile(url.toString());
    }
}

void DkMosaicDialog::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QUrl url = event->mimeData()->urls().at(0);
        url = url.toLocalFile();
        QFileInfo file = QFileInfo(url.toString());

        if (file.exists() && DkUtils::isValid(file))
            event->acceptProposedAction();
    }
}

void DkMosaicDialog::createLayout()
{
    // progress bar
    mProgress = new QProgressBar(this);
    mProgress->hide();

    mMsgLabel = new QLabel(this);
    mMsgLabel->setObjectName("DkWarningInfo");
    mMsgLabel->hide();

    // post processing sliders
    mDarkenSlider = new QSlider(Qt::Horizontal, this);
    mDarkenSlider->setObjectName("darkenSlider");
    mDarkenSlider->setValue(40);
    // darkenSlider->hide();

    mLightenSlider = new QSlider(Qt::Horizontal, this);
    mLightenSlider->setObjectName("lightenSlider");
    mLightenSlider->setValue(40);
    // lightenSlider->hide();

    mSaturationSlider = new QSlider(Qt::Horizontal, this);
    mSaturationSlider->setObjectName("saturationSlider");
    mSaturationSlider->setValue(60);
    // saturationSlider->hide();

    mSliderWidget = new QWidget(this);
    QGridLayout *sliderLayout = new QGridLayout(mSliderWidget);
    sliderLayout->addWidget(new QLabel(tr("Darken")), 0, 0);
    sliderLayout->addWidget(new QLabel(tr("Lighten")), 0, 1);
    sliderLayout->addWidget(new QLabel(tr("Saturation")), 0, 2);

    sliderLayout->addWidget(mDarkenSlider, 1, 0);
    sliderLayout->addWidget(mLightenSlider, 1, 1);
    sliderLayout->addWidget(mSaturationSlider, 1, 2);
    mSliderWidget->hide();

    // open handles
    QLabel *openLabel = new QLabel(tr("Mosaic Image:"), this);
    openLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QPushButton *openButton = new QPushButton(tr("&Browse"), this);
    openButton->setObjectName("openButton");
    openButton->setToolTip(tr("Choose which image to mosaic."));

    mFileLabel = new QLabel(tr("No Image loaded"), this);

    // save handles
    QLabel *saveLabel = new QLabel(tr("Mosaic Elements Folder:"), this);
    saveLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QPushButton *dbButton = new QPushButton(tr("&Browse"), this);
    dbButton->setObjectName("dbButton");
    dbButton->setToolTip(tr("Specify the root folder of images used for mosaic elements."));

    mFolderLabel = new QLabel(tr("Specify an Image Database"), this);

    // resolution handles
    QLabel *sizeLabel = new QLabel(tr("Resolution:"));
    sizeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mNewWidthBox = new QSpinBox();
    mNewWidthBox->setObjectName("newWidthBox");
    mNewWidthBox->setToolTip(tr("Pixel Width"));
    mNewWidthBox->setMinimum(100);
    mNewWidthBox->setMaximum(30000);
    mNewHeightBox = new QSpinBox();
    mNewHeightBox->setObjectName("newHeightBox");
    mNewHeightBox->setToolTip(tr("Pixel Height"));
    mNewHeightBox->setMinimum(100);
    mNewHeightBox->setMaximum(30000);
    mRealResLabel = new QLabel("");
    // realResLabel->setToolTip(tr("."));

    // num patch handles
    QLabel *patchLabel = new QLabel(tr("Patches:"));
    patchLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mNumPatchesH = new QSpinBox(this);
    mNumPatchesH->setObjectName("numPatchesH");
    mNumPatchesH->setToolTip(tr("Number of Horizontal Patches"));
    mNumPatchesH->setMinimum(1);
    mNumPatchesH->setMaximum(1000);
    mNumPatchesV = new QSpinBox(this);
    mNumPatchesV->setObjectName("numPatchesV");
    mNumPatchesV->setToolTip(tr("Number of Vertical Patches"));
    mNumPatchesV->setMinimum(1);
    mNumPatchesV->setMaximum(1000);
    mPatchResLabel = new QLabel("", this);
    mPatchResLabel->setObjectName("DkDecentInfo");
    mPatchResLabel->setToolTip(tr("If this label turns red, the computation might be slower."));

    // file filters
    QLabel *filterLabel = new QLabel(tr("Filters:"), this);
    filterLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    mFilterEdit = new QLineEdit("", this);
    mFilterEdit->setObjectName("fileEdit");
    mFilterEdit->setToolTip(tr("You can split multiple ignore words with ;"));

    QStringList filters = DkSettingsManager::param().app().openFilters;
    filters.pop_front(); // replace for better readability
    filters.push_front(tr("All Images"));
    mSuffixBox = new QComboBox(this);
    mSuffixBox->addItems(filters);
    // suffixBox->setCurrentIndex(DkImageLoader::saveFilters.indexOf(QRegExp(".*tif.*")));

    mControlWidget = new QWidget(this);
    QGridLayout *controlLayout = new QGridLayout(mControlWidget);
    controlLayout->addWidget(openLabel, 0, 0);
    controlLayout->addWidget(openButton, 0, 1, 1, 2);
    controlLayout->addWidget(mFileLabel, 0, 3, 1, 2);
    // controlLayout->setColumnStretch(3, 1);

    controlLayout->addWidget(saveLabel, 1, 0);
    controlLayout->addWidget(dbButton, 1, 1, 1, 2);
    controlLayout->addWidget(mFolderLabel, 1, 3, 1, 2);
    // controlLayout->setColumnStretch(3, 1);

    controlLayout->addWidget(sizeLabel, 2, 0);
    controlLayout->addWidget(mNewWidthBox, 2, 1);
    controlLayout->addWidget(mNewHeightBox, 2, 2);
    controlLayout->addWidget(mRealResLabel, 2, 3);

    controlLayout->addWidget(patchLabel, 4, 0);
    controlLayout->addWidget(mNumPatchesH, 4, 1);
    controlLayout->addWidget(mNumPatchesV, 4, 2);
    controlLayout->addWidget(mPatchResLabel, 4, 3);

    controlLayout->addWidget(filterLabel, 5, 0);
    controlLayout->addWidget(mFilterEdit, 5, 1, 1, 2);
    controlLayout->addWidget(mSuffixBox, 5, 3, 1, 2);
    controlLayout->setColumnStretch(5, 1);

    // shows the image if it could be loaded
    mViewport = new DkBaseViewPort(this);
    mViewport->setForceFastRendering(true);
    mViewport->setPanControl(QPointF(0.0f, 0.0f));

    mPreview = new DkBaseViewPort(this);
    mPreview->setForceFastRendering(true);
    mPreview->setPanControl(QPointF(0.0f, 0.0f));
    mPreview->hide();

    QWidget *viewports = new QWidget(this);
    QHBoxLayout *viewLayout = new QHBoxLayout(viewports);
    viewLayout->addWidget(mViewport);
    viewLayout->addWidget(mPreview);

    // mButtons
    mButtons = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    mButtons->button(QDialogButtonBox::Save)->setText(tr("&Save"));
    mButtons->button(QDialogButtonBox::Apply)->setText(tr("&Generate"));
    mButtons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    // connect(mButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(mButtons, SIGNAL(clicked(QAbstractButton *)), this, SLOT(buttonClicked(QAbstractButton *)));
    connect(mButtons, SIGNAL(rejected()), this, SLOT(reject()));
    mButtons->button(QDialogButtonBox::Save)->setEnabled(false);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(viewports);
    layout->addWidget(mProgress);
    layout->addWidget(mSliderWidget);
    layout->addWidget(mMsgLabel);
    layout->addWidget(mControlWidget);
    layout->addWidget(mButtons);

    enableMosaicSave(false);
}

void DkMosaicDialog::on_openButton_pressed()
{
    // load system default open dialog
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open TIFF"),
                                                    mFilePath,
                                                    DkSettingsManager::param().app().openFilters.join(";;"),
                                                    nullptr,
                                                    DkDialog::fileDialogOptions());

    setFile(fileName);
}

void DkMosaicDialog::on_dbButton_pressed()
{
    qDebug() << "save triggered...";

    // load system default open dialog
    QString dirName =
        QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"), mSavePath, QFileDialog::ShowDirsOnly | DkDialog::fileDialogOptions());

    if (QFileInfo(dirName).exists()) {
        mSavePath = dirName;
        mFolderLabel->setText(mSavePath);
    }
}

void DkMosaicDialog::on_fileEdit_textChanged(const QString &filename)
{
    qDebug() << "new file name: " << filename;
}

void DkMosaicDialog::on_newWidthBox_valueChanged(int)
{
    if (!mLoader.hasImage())
        return;

    mNewHeightBox->blockSignals(true);
    mNewHeightBox->setValue(qRound((float)mNewWidthBox->value() / mLoader.image().width() * mLoader.image().height()));
    mNewHeightBox->blockSignals(false);
    mRealResLabel->setText(
        tr("%1 x %2 cm @150 dpi").arg(mNewWidthBox->value() / 150.0 * 2.54, 0, 'f', 1).arg(mNewHeightBox->value() / 150.0 * 2.54, 0, 'f', 1));
    updatePatchRes();
}

void DkMosaicDialog::on_newHeightBox_valueChanged(int)
{
    if (!mLoader.hasImage())
        return;

    mNewWidthBox->blockSignals(true);
    mNewWidthBox->setValue(qRound((float)mNewHeightBox->value() / mLoader.image().height() * mLoader.image().width()));
    mNewWidthBox->blockSignals(false);
    mRealResLabel->setText(
        tr("%1 x %2 cm @150 dpi").arg(mNewWidthBox->value() / 150.0 * 2.54, 0, 'f', 1).arg(mNewHeightBox->value() / 150.0 * 2.54, 0, 'f', 1));
    updatePatchRes();
}

void DkMosaicDialog::on_numPatchesH_valueChanged(int)
{
    if (!mLoader.hasImage())
        return;

    mNumPatchesV->blockSignals(true);
    mNumPatchesV->setValue(qFloor((float)mLoader.image().height() / ((float)mLoader.image().width() / mNumPatchesH->value())));
    mNumPatchesV->blockSignals(false);
    updatePatchRes();
}

void DkMosaicDialog::on_numPatchesV_valueChanged(int)
{
    if (!mLoader.hasImage())
        return;

    mNumPatchesH->blockSignals(true);
    mNumPatchesH->setValue(qFloor((float)mLoader.image().width() / ((float)mLoader.image().height() / mNumPatchesV->value())));
    mNumPatchesH->blockSignals(false);
    updatePatchRes();
}

void DkMosaicDialog::on_darkenSlider_valueChanged(int)
{
    updatePostProcess();
}

void DkMosaicDialog::on_lightenSlider_valueChanged(int)
{
    updatePostProcess();
}

void DkMosaicDialog::on_saturationSlider_valueChanged(int)
{
    updatePostProcess();
}

void DkMosaicDialog::updatePatchRes()
{
    int patchResD = qFloor((float)mNewWidthBox->value() / mNumPatchesH->value());

    mPatchResLabel->setText(tr("Patch Resolution: %1 px").arg(patchResD));
    mPatchResLabel->show();

    // show the user if we can work with the thumbnails or not
    if (patchResD > 97)
        mPatchResLabel->setProperty("warning", true);
    else
        mPatchResLabel->setProperty("warning", false);

    mPatchResLabel->style()->unpolish(mPatchResLabel);
    mPatchResLabel->style()->polish(mPatchResLabel);
    mPatchResLabel->update();
}

QImage DkMosaicDialog::getImage()
{
    if (mMosaic.isNull() && !mMosaicMat.empty())
        return DkImage::mat2QImage(mMosaicMat);

    return mMosaic;
}

void DkMosaicDialog::reject()
{
    // not sure if this is a nice way to do: but we change cancel behavior while processing
    if (mProcessing)
        mProcessing = false;
    else if (!mMosaic.isNull() && !mButtons->button(QDialogButtonBox::Apply)->isEnabled()) {
        mButtons->button(QDialogButtonBox::Apply)->setEnabled(true);
        enableAll(true);
        mViewport->show();
        mSliderWidget->hide();
    } else
        QDialog::reject();
}

void DkMosaicDialog::buttonClicked(QAbstractButton *button)
{
    if (button == mButtons->button(QDialogButtonBox::Save)) {
        // render the full image
        if (!mMosaic.isNull()) {
            mSliderWidget->hide();
            mProgress->setValue(mProgress->minimum());
            mProgress->show();
            enableAll(false);
            button->setEnabled(false);

            QFuture<bool> future = QtConcurrent::run(this,
                                                     &nmc::DkMosaicDialog::postProcessMosaic,
                                                     mDarkenSlider->value() / 100.0f,
                                                     mLightenSlider->value() / 100.0f,
                                                     mSaturationSlider->value() / 100.0f,
                                                     false);
            mPostProcessWatcher.setFuture(future);
        }
    } else if (button == mButtons->button(QDialogButtonBox::Apply))
        compute();
}

void DkMosaicDialog::compute()
{
    if (mPostProcessing)
        return;

    mProgress->setValue(mProgress->minimum());
    mProgress->show();
    mMsgLabel->setText("");
    mMsgLabel->show();
    mMosaicMatSmall.release();
    mMosaicMat.release();
    mOrigImg.release();
    mMosaic = QImage();
    mSliderWidget->hide();
    mViewport->show();
    mPreview->setForceFastRendering(true);
    mPreview->show();

    enableAll(false);

    QString suffixTmp = mSuffixBox->currentText();
    QString suffix;

    for (int idx = 0; idx < DkSettingsManager::param().app().fileFilters.size(); idx++) {
        if (suffixTmp.contains("(" + DkSettingsManager::param().app().fileFilters.at(idx))) {
            suffix = DkSettingsManager::param().app().fileFilters.at(idx);
            break;
        }
    }

    QString filter = mFilterEdit->text();
    mFilesUsed.clear();

    mProcessing = true;
    QFuture<int> future = QtConcurrent::run(this, &nmc::DkMosaicDialog::computeMosaic, filter, suffix, mNewWidthBox->value(), mNumPatchesH->value());
    mMosaicWatcher.setFuture(future);

    //// debug
    // computeMosaic(
    //	cFile,
    //	filter,
    //	suffix,
    //	fromPage->value(),
    //	toPage->value(),
    //	overwrite->isChecked());
}

void DkMosaicDialog::mosaicFinished()
{
    mProgress->hide();
    // msgLabel->hide();

    if (!mMosaicMat.empty()) {
        mSliderWidget->show();
        mMsgLabel->hide();
        mViewport->hide();
        mPreview->setForceFastRendering(false);

        updatePostProcess(); // add values
        mButtons->button(QDialogButtonBox::Save)->setEnabled(true);
    } else
        enableAll(true);
}

int DkMosaicDialog::computeMosaic(const QString &filter, const QString &suffix, int newWidth, int numPatchesH)
{
    DkTimer dt;

    // compute new image size
    cv::Mat mImg = DkImage::qImage2Mat(mLoader.image());

    QSize numPatches = QSize(numPatchesH, 0);

    // compute new image size
    // float aratio = (float)mImg.rows/mImg.cols;
    int patchResO = qFloor((float)mImg.cols / numPatches.width());
    numPatches.setHeight(qFloor((float)mImg.rows / patchResO));

    int patchResD = qFloor(newWidth / numPatches.width());

    float shR = (mImg.rows - patchResO * numPatches.height()) / 2.0f;
    float shC = (mImg.cols - patchResO * numPatches.width()) / 2.0f;

    mImg = mImg.rowRange(qFloor(shR), mImg.rows - qCeil(shR)).colRange(qFloor(shC), mImg.cols - qCeil(shC));
    cv::Mat mImgLab;
    cv::cvtColor(mImg, mImgLab, CV_RGB2Lab);
    std::vector<cv::Mat> channels;
    cv::split(mImgLab, channels);
    cv::Mat imgL = channels[0];

    // keeps track of the weights
    cv::Mat cc(numPatches.height(), numPatches.width(), CV_32FC1);
    cc.setTo(0);
    cv::Mat ccD(numPatches.height(), numPatches.width(), CV_8UC1); // tells us if we have already computed the real patch

    mFilesUsed.resize(numPatches.height() * numPatches.width());

    // destination image
    cv::Mat dImg(patchResD * numPatches.height(), patchResD * numPatches.width(), CV_8UC1);
    dImg = 255;

    // patch image (preview)
    cv::Mat pImg(patchResO * numPatches.height(), patchResO * numPatches.width(), CV_8UC1);
    pImg = 255;

    qDebug() << "mosaic data --------------------------------";
    qDebug() << "patchRes: " << patchResD;
    qDebug() << "new resolution: " << dImg.cols << " x " << dImg.rows;
    qDebug() << "num patches: " << numPatches.width() << " x " << numPatches.height();
    qDebug() << "mosaic data --------------------------------";

    // progress bar
    int pIdx = 0;
    int maxP = numPatches.width() * numPatches.height();

    int iDidNothing = 0;
    bool force = false;
    bool useTwice = false;

    while (iDidNothing < 10000) {
        if (!mProcessing)
            return QDialog::Rejected;

        if (iDidNothing > 100) {
            force = true;

            if (!useTwice)
                emit infoMessage(tr("Filling empty areas..."));
        }

        if (iDidNothing > 400 && !useTwice) {
            emit infoMessage(tr("I need to use some images twice - maybe the database is too small?"));
            iDidNothing = 0;
            useTwice = true;
        } else if (iDidNothing > 400) {
            emit infoMessage(tr("Sorry, it seems that i cannot create your mosaic with this database."));
            return QDialog::Rejected;
        }

        QString imgPath = getRandomImagePath(mSavePath, filter, suffix);

        if (!useTwice && mFilesUsed.contains(QFileInfo(imgPath))) {
            iDidNothing++;
            continue;
        }

        try {
            DkThumbNail thumb = DkThumbNail(imgPath);
            thumb.compute();

            cv::Mat ccTmp(cc.size(), cc.depth());

            if (!force)
                ccTmp.setTo(0);
            else
                ccTmp = cc.clone();

            cv::Mat thumbPatch = createPatch(thumb, patchResO);

            if (thumbPatch.rows != patchResO || thumbPatch.cols != patchResO) {
                iDidNothing++;
                continue;
            }

            matchPatch(imgL, thumbPatch, patchResO, ccTmp);

            if (force) {
                cv::Mat mask = (cc == 0);
                mask.convertTo(mask, CV_32FC1, 1.0f / 255.0f);
                ccTmp = ccTmp.mul(mask);
            }

            double maxVal = 0;
            cv::Point maxIdx;
            cv::minMaxLoc(ccTmp, 0, &maxVal, 0, &maxIdx);
            float *ccPtr = cc.ptr<float>(maxIdx.y);

            if (maxVal > ccPtr[maxIdx.x]) {
                cv::Mat pPatch =
                    pImg.rowRange(maxIdx.y * patchResO, maxIdx.y * patchResO + patchResO).colRange(maxIdx.x * patchResO, maxIdx.x * patchResO + patchResO);
                thumbPatch.copyTo(pPatch);

                // visualize
                if (pIdx % 10 == 0) {
                    channels[0] = pImg;

                    // debug
                    cv::Mat imgT3;
                    cv::merge(channels, imgT3);
                    cv::cvtColor(imgT3, imgT3, CV_Lab2BGR);
                    emit updateImage(DkImage::mat2QImage(imgT3));
                }

                if (ccPtr[maxIdx.x] == 0) {
                    pIdx++;
                    emit updateProgress(qRound((float)pIdx / maxP * 100));
                }

                // compute it now if we already have the full image loaded
                if (thumb.getImage().width() > patchResD && thumb.getImage().height() > patchResD) {
                    thumbPatch = createPatch(thumb, patchResD);

                    cv::Mat dPatch =
                        dImg.rowRange(maxIdx.y * patchResD, maxIdx.y * patchResD + patchResD).colRange(maxIdx.x * patchResD, maxIdx.x * patchResD + patchResD);
                    thumbPatch.copyTo(dPatch);
                    ccD.ptr<unsigned char>(maxIdx.y)[maxIdx.x] = 1;
                } else
                    ccD.ptr<unsigned char>(maxIdx.y)[maxIdx.x] = 0;

                // update cc
                ccPtr[maxIdx.x] = (float)maxVal;

                mFilesUsed[maxIdx.y * numPatchesH + maxIdx.x] = thumb.getFilePath(); // replaces additionally the old file
                iDidNothing = 0;
            } else
                iDidNothing++;
        }
        // catch cv exceptions e.g. out of memory
        catch (...) {
            iDidNothing++;
        }

        // are we done yet?
        double minVal = 0;
        cv::minMaxLoc(cc, &minVal);

        if (minVal > 0)
            break;
    }

    pIdx = 0;

    // compute real resolution
    for (int rIdx = 0; rIdx < ccD.rows; rIdx++) {
        const unsigned char *ccDPtr = ccD.ptr<unsigned char>(rIdx);

        for (int cIdx = 0; cIdx < ccD.cols; cIdx++) {
            // is the patch already computed?
            if (ccDPtr[cIdx])
                continue;

            QFileInfo cFile = mFilesUsed.at(rIdx * ccD.cols + cIdx);

            if (!cFile.exists()) {
                emit infoMessage(tr("Something is seriously wrong, I could not load: %1").arg(cFile.absoluteFilePath()));
                continue;
            }

            cv::Mat thumbPatch = createPatch(DkThumbNail(cFile.absoluteFilePath()), patchResD);

            cv::Mat dPatch = dImg.rowRange(rIdx * patchResD, rIdx * patchResD + patchResD).colRange(cIdx * patchResD, cIdx * patchResD + patchResD);
            thumbPatch.copyTo(dPatch);
            emit updateProgress(qRound((float)pIdx / maxP * 100));
            pIdx++;
        }
    }

    qDebug() << "I fully rendered: " << ccD.rows * ccD.cols - cv::sum(ccD)[0] << " images";

    // create final images
    mOrigImg = mImgLab;
    mMosaicMat = dImg;
    mMosaicMatSmall = pImg;

    mProcessing = false;

    qDebug() << "mosaic computed in: " << dt;

    return QDialog::Accepted;
}

void DkMosaicDialog::matchPatch(const cv::Mat &img, const cv::Mat &thumb, int patchRes, cv::Mat &cc)
{
    for (int rIdx = 0; rIdx < cc.rows; rIdx++) {
        float *ccPtr = cc.ptr<float>(rIdx);
        const cv::Mat imgStrip = img.rowRange(rIdx * patchRes, rIdx * patchRes + patchRes);

        for (int cIdx = 0; cIdx < cc.cols; cIdx++) {
            // already computed?
            if (ccPtr[cIdx] != 0)
                continue;

            const cv::Mat cPatch = imgStrip.colRange(cIdx * patchRes, cIdx * patchRes + patchRes);

            cv::Mat absDiff;
            cv::absdiff(cPatch, thumb, absDiff);
            ccPtr[cIdx] = 1.0f - ((float)cv::sum(absDiff)[0] / (patchRes * patchRes * 255));
        }
    }
}

cv::Mat DkMosaicDialog::createPatch(const DkThumbNail &thumb, int patchRes)
{
    QImage img;

    // load full image if we have not enough resolution
    if (thumb.getImage().isNull() || qMin(thumb.getImage().width(), thumb.getImage().height()) < patchRes) {
        DkBasicLoader loader;
        loader.loadGeneral(thumb.getFilePath(), true, true);
        img = loader.image();
    } else
        img = thumb.getImage();

    cv::Mat cvThumb = DkImage::qImage2Mat(img);
    cv::cvtColor(cvThumb, cvThumb, CV_RGB2Lab);
    std::vector<cv::Mat> channels;
    cv::split(cvThumb, channels);
    cvThumb = channels[0];
    channels.clear();

    // make square
    if (cvThumb.rows != cvThumb.cols) {
        if (cvThumb.rows > cvThumb.cols) {
            float sh = (cvThumb.rows - cvThumb.cols) / 2.0f;
            cvThumb = cvThumb.rowRange(qFloor(sh), cvThumb.rows - qCeil(sh));
        } else {
            float sh = (cvThumb.cols - cvThumb.rows) / 2.0f;
            cvThumb = cvThumb.colRange(qFloor(sh), cvThumb.cols - qCeil(sh));
        }
    }

    if (cvThumb.rows < patchRes || cvThumb.cols < patchRes)
        qDebug() << "enlarging thumbs!!";

    cv::resize(cvThumb, cvThumb, cv::Size(patchRes, patchRes), 0.0, 0.0, CV_INTER_AREA);

    return cvThumb;
}

QString DkMosaicDialog::getRandomImagePath(const QString &cPath, const QString &ignore, const QString &suffix)
{
    // TODO: remove hierarchy
    QStringList fileFilters = (suffix.isEmpty()) ? DkSettingsManager::param().app().fileFilters : QStringList(suffix);

    // get all dirs
    QFileInfoList entries = QDir(cPath).entryInfoList(QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot);
    // qDebug() << entries;
    //  get all files
    entries += QDir(cPath).entryInfoList(fileFilters);
    // qDebug() << "current entries: " << e;

    if (!ignore.isEmpty()) {
        QStringList ignoreList = ignore.split(";");
        QFileInfoList entriesTmp = entries;
        entries.clear();

        for (int idx = 0; idx < entriesTmp.size(); idx++) {
            bool lIgnore = false;
            QString p = entriesTmp.at(idx).absoluteFilePath();

            for (int iIdx = 0; iIdx < ignoreList.size(); iIdx++) {
                if (p.contains(ignoreList.at(iIdx))) {
                    lIgnore = true;
                    break;
                }
            }

            if (!lIgnore)
                entries.append(entriesTmp.at(idx));
        }
    }

    if (entries.isEmpty())
        return QString();

    int rIdx = qRound(QRandomGenerator::global()->generateDouble() * (entries.size() - 1));

    // qDebug() << "rand index: " << rIdx;

    QFileInfo rPath = entries.at(rIdx);
    // qDebug() << rPath.absoluteFilePath();

    if (rPath.isDir())
        return getRandomImagePath(rPath.absoluteFilePath(), ignore, suffix);
    else
        return rPath.absoluteFilePath();
}

void DkMosaicDialog::updatePostProcess()
{
    if (mMosaicMat.empty() || mProcessing)
        return;

    if (mPostProcessing) {
        mUpdatePostProcessing = true;
        return;
    }

    mButtons->button(QDialogButtonBox::Apply)->setEnabled(false);
    mButtons->button(QDialogButtonBox::Save)->setEnabled(false);

    QFuture<bool> future = QtConcurrent::run(this,
                                             &nmc::DkMosaicDialog::postProcessMosaic,
                                             mDarkenSlider->value() / 100.0f,
                                             mLightenSlider->value() / 100.0f,
                                             mSaturationSlider->value() / 100.0f,
                                             true);
    mPostProcessWatcher.setFuture(future);

    mUpdatePostProcessing = false;
    // postProcessMosaic(darkenSlider->value()/100.0f, lightenSlider->value()/100.0f, saturationSlider->value()/100.0f);
}

void DkMosaicDialog::postProcessFinished()
{
    if (mPostProcessWatcher.result()) {
        QDialog::accept();
    } else if (mUpdatePostProcessing)
        updatePostProcess();
    else {
        mButtons->button(QDialogButtonBox::Save)->setEnabled(true);
    }
}

bool DkMosaicDialog::postProcessMosaic(float multiply /* = 0.3 */, float screen /* = 0.5 */, float saturation, bool computePreview)
{
    mPostProcessing = true;

    qDebug() << "darken: " << multiply << " lighten: " << screen;

    cv::Mat origR;
    cv::Mat mosaicR;

    try {
        if (computePreview) {
            origR = mOrigImg.clone();
            mosaicR = mMosaicMatSmall.clone();
        } else {
            cv::resize(mOrigImg, origR, mMosaicMat.size(), 0, 0, CV_INTER_LANCZOS4);
            mosaicR = mMosaicMat;
            mOrigImg.release();
        }

        // multiply the two images
        for (int rIdx = 0; rIdx < origR.rows; rIdx++) {
            const unsigned char *mosaicPtr = mosaicR.ptr<unsigned char>(rIdx);
            unsigned char *origPtr = origR.ptr<unsigned char>(rIdx);

            if (!computePreview)
                emit updateProgress(qRound((float)rIdx / origR.rows * 100));

            for (int cIdx = 0; cIdx < origR.cols; cIdx++) {
                // mix the luminance channel
                float mosaic = mosaicPtr[cIdx] / 255.0f;
                float luminance = (*origPtr) / 255.0f;

                float lighten = (1.0f - luminance) * screen + (1.0f - screen);
                lighten *= 1.0f - mosaic; // multiply inverse
                lighten = 1.0f - lighten;

                float darken = luminance * multiply + (1.0f - multiply);
                darken *= lighten; // mix with the mosaic pixel

                // now stretch to the dynamic range and save it
                *origPtr = (unsigned char)qRound(darken * 255.0f);

                // now adopt the saturation
                origPtr++;
                *origPtr = (unsigned char)qRound((*origPtr - 128) * saturation) + 128;
                origPtr++;
                *origPtr = (unsigned char)qRound((*origPtr - 128) * saturation) + 128;
                origPtr++;
            }
        }

        // if (!computePreview)
        //	mosaicMat.release();
        cv::cvtColor(origR, origR, CV_Lab2BGR);
        qDebug() << "color converted";

        mMosaic = DkImage::mat2QImage(origR);
        qDebug() << "mosaicing computed...";

    } catch (...) {
        origR.release();

        QMessageBox::critical(DkUtils::getMainWindow(), tr("Error"), tr("Sorry, I could not mix the image..."));
        qDebug() << "exception caught...";
        mMosaic = DkImage::mat2QImage(mMosaicMat);
    }

    if (computePreview)
        mPreview->setImage(mMosaic);

    mPostProcessing = false;

    return !computePreview;
}

void DkMosaicDialog::setFile(const QString &filePath)
{
    QFileInfo fInfo(filePath);
    if (!fInfo.exists())
        return;

    mFilePath = filePath;
    mSavePath = fInfo.absolutePath();
    mFolderLabel->setText(mSavePath);
    mFileLabel->setText(filePath);

    mLoader.loadGeneral(filePath, true);
    mViewport->setImage(mLoader.image());

    enableMosaicSave(mLoader.hasImage());

    // newWidthBox->blockSignals(true);
    // newHeightBox->blockSignals(true);
    mNewWidthBox->setValue(mLoader.image().width());
    mNumPatchesH->setValue(qFloor((float)mLoader.image().width() / 90)); // 130 is a pretty good patch resolution
    mNumPatchesH->setMaximum(qMin(1000, qFloor(mLoader.image().width() * 0.5f)));
    mNumPatchesV->setMaximum(qMin(1000, qFloor(mLoader.image().height() * 0.5f)));
    // newHeightBox->setValue(loader.image().height());
    // newWidthBox->blockSignals(false);
    // newHeightBox->blockSignals(false);

    // fromPage->setRange(1, loader.getNumPages());
    // toPage->setRange(1, loader.getNumPages());

    // fromPage->setValue(1);
    // toPage->setValue(loader.getNumPages());
}

void DkMosaicDialog::enableAll(bool enable)
{
    enableMosaicSave(enable);
    mControlWidget->setEnabled(enable);
}

void DkMosaicDialog::enableMosaicSave(bool enable)
{
    mFilterEdit->setEnabled(enable);
    mSuffixBox->setEnabled(enable);
    mNewWidthBox->setEnabled(enable);
    mNewHeightBox->setEnabled(enable);
    mNumPatchesH->setEnabled(enable);
    mNumPatchesV->setEnabled(enable);
    mButtons->button(QDialogButtonBox::Apply)->setEnabled(enable);

    if (!enable)
        mButtons->button(QDialogButtonBox::Save)->setEnabled(enable);
}
#endif
// DkForceThumbDialog --------------------------------------------------------------------
DkForceThumbDialog::DkForceThumbDialog(QWidget *parent /* = 0 */, Qt::WindowFlags f /* = 0 */)
    : QDialog(parent, f)
{
    createLayout();
}

void DkForceThumbDialog::createLayout()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    infoLabel = new QLabel();
    infoLabel->setAlignment(Qt::AlignHCenter);

    cbForceSave = new QCheckBox(tr("Overwrite Existing Thumbnails"));
    cbForceSave->setToolTip("If checked, existing thumbnails will be replaced");

    // mButtons
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    layout->addWidget(infoLabel);
    layout->addWidget(cbForceSave);
    layout->addWidget(buttons);
}

bool DkForceThumbDialog::forceSave() const
{
    return cbForceSave->isChecked();
}

void DkForceThumbDialog::setDir(const QDir &fileInfo)
{
    infoLabel->setText(tr("Compute thumbnails for all images in:\n %1\n").arg(fileInfo.absolutePath()));
}

// Welcome dialog --------------------------------------------------------------------
DkWelcomeDialog::DkWelcomeDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    setWindowTitle(tr("Welcome"));
    createLayout();
    mLanguageChanged = false;
}

void DkWelcomeDialog::createLayout()
{
    QGridLayout *layout = new QGridLayout(this);

    QLabel *welcomeLabel = new QLabel(tr("Welcome to nomacs, please choose your preferred language below."), this);

    mLanguageCombo = new QComboBox(this);
    DkUtils::addLanguages(mLanguageCombo, mLanguages);

    mRegisterFilesCheckBox = new QCheckBox(tr("&Register File Associations"), this);
    mRegisterFilesCheckBox->setChecked(!DkSettingsManager::param().isPortable());

    mSetAsDefaultCheckBox = new QCheckBox(tr("Set As &Default Viewer"), this);
    mSetAsDefaultCheckBox->setChecked(!DkSettingsManager::param().isPortable());

    // mButtons
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    layout->addItem(new QSpacerItem(10, 10), 0, 0, -1, -1);
    layout->addWidget(welcomeLabel, 1, 0, 1, 3);
    layout->addItem(new QSpacerItem(10, 10), 2, 0, -1, -1);
    layout->addWidget(mLanguageCombo, 3, 1);

#ifdef Q_OS_WIN
    layout->addWidget(mRegisterFilesCheckBox, 4, 1);
    layout->addWidget(mSetAsDefaultCheckBox, 5, 1);
#else
    mRegisterFilesCheckBox->setChecked(false);
    mRegisterFilesCheckBox->hide();
    mSetAsDefaultCheckBox->setChecked(false);
    mSetAsDefaultCheckBox->hide();
#endif

    layout->addWidget(buttons, 6, 0, 1, 3);
}

void DkWelcomeDialog::accept()
{
    DkFileFilterHandling fh;

    if (mRegisterFilesCheckBox->isChecked())
        DkFileFilterHandling::registerFileAssociations();
    fh.registerNomacs(mSetAsDefaultCheckBox->isChecked()); // register nomacs again - to be save

    // change language
    if (mLanguageCombo->currentIndex() != mLanguages.indexOf(DkSettingsManager::param().global().language) && mLanguageCombo->currentIndex() >= 0) {
        DkSettingsManager::param().global().language = mLanguages.at(mLanguageCombo->currentIndex());
        mLanguageChanged = true;
    }

    QDialog::accept();
}

bool DkWelcomeDialog::isLanguageChanged()
{
    return mLanguageChanged;
}

// archive extraction dialog --------------------------------------------------------------------
#ifdef WITH_QUAZIP
DkArchiveExtractionDialog::DkArchiveExtractionDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    mFileList = QStringList();
    setWindowTitle(tr("Extract images from an archive"));
    createLayout();
    setMinimumSize(340, 400);
    setAcceptDrops(true);
}

void DkArchiveExtractionDialog::createLayout()
{
    // archive file path
    QLabel *archiveLabel = new QLabel(tr("Archive (%1)").arg(DkSettingsManager::param().app().containerRawFilters.replace(" *", ", *")), this);
    mArchivePathEdit = new QLineEdit(this);
    mArchivePathEdit->setObjectName("DkWarningEdit");
    mArchivePathEdit->setValidator(&mFileValidator);
    connect(mArchivePathEdit, SIGNAL(textChanged(const QString &)), this, SLOT(textChanged(const QString &)));
    connect(mArchivePathEdit, SIGNAL(editingFinished()), this, SLOT(loadArchive()));

    QPushButton *openArchiveButton = new QPushButton(tr("&Browse"));
    connect(openArchiveButton, SIGNAL(pressed()), this, SLOT(openArchive()));

    // dir file path
    QLabel *dirLabel = new QLabel(tr("Extract to"));
    mDirPathEdit = new QLineEdit();
    mDirPathEdit->setValidator(&mFileValidator);
    connect(mDirPathEdit, SIGNAL(textChanged(const QString &)), this, SLOT(dirTextChanged(const QString &)));

    QPushButton *openDirButton = new QPushButton(tr("&Browse"));
    connect(openDirButton, SIGNAL(pressed()), this, SLOT(openDir()));

    mFeedbackLabel = new QLabel("", this);
    mFeedbackLabel->setObjectName("DkDecentInfo");

    mFileListDisplay = new QListWidget(this);

    mRemoveSubfolders = new QCheckBox(tr("Remove Subfolders"), this);
    mRemoveSubfolders->setChecked(false);
    connect(mRemoveSubfolders, SIGNAL(stateChanged(int)), this, SLOT(checkbocChecked(int)));

    // mButtons
    mButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    mButtons->button(QDialogButtonBox::Ok)->setText(tr("&Extract"));
    mButtons->button(QDialogButtonBox::Ok)->setEnabled(false);
    mButtons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    connect(mButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(mButtons, SIGNAL(rejected()), this, SLOT(reject()));

    QWidget *extractWidget = new QWidget(this);
    QGridLayout *gdLayout = new QGridLayout(extractWidget);
    gdLayout->addWidget(archiveLabel, 0, 0);
    gdLayout->addWidget(mArchivePathEdit, 1, 0);
    gdLayout->addWidget(openArchiveButton, 1, 1);
    gdLayout->addWidget(dirLabel, 2, 0);
    gdLayout->addWidget(mDirPathEdit, 3, 0);
    gdLayout->addWidget(openDirButton, 3, 1);
    gdLayout->addWidget(mFeedbackLabel, 4, 0, 1, 2);
    gdLayout->addWidget(mFileListDisplay, 5, 0, 1, 2);
    gdLayout->addWidget(mRemoveSubfolders, 6, 0, 1, 2);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(extractWidget);
    layout->addWidget(mButtons);
}

void DkArchiveExtractionDialog::setCurrentFile(const QString &filePath, bool isZip)
{
    userFeedback("", false);
    mArchivePathEdit->setText("");
    mDirPathEdit->setText("");
    mFileListDisplay->clear();
    mRemoveSubfolders->setChecked(false);

    mFilePath = filePath;
    if (isZip) {
        mArchivePathEdit->setText(mFilePath);
        loadArchive();
    }
}

void DkArchiveExtractionDialog::textChanged(const QString &text)
{
    bool oldStyle = mArchivePathEdit->property("error").toBool();
    bool newStyle = false;

    if (QFileInfo(text).exists() && DkBasicLoader::isContainer(text)) {
        newStyle = false;
        mArchivePathEdit->setProperty("error", newStyle);
        loadArchive(text);
    } else {
        newStyle = true;
        mArchivePathEdit->setProperty("error", newStyle);
        userFeedback("", false);
        mFileListDisplay->clear();
        mButtons->button(QDialogButtonBox::Ok)->setEnabled(false);
    }

    if (oldStyle != newStyle) {
        mArchivePathEdit->style()->unpolish(mArchivePathEdit);
        mArchivePathEdit->style()->polish(mArchivePathEdit);
        mArchivePathEdit->update();
    }
}

void DkArchiveExtractionDialog::dirTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        userFeedback("", false);
        mButtons->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}

void DkArchiveExtractionDialog::checkbocChecked(int)
{
    loadArchive();
}

void DkArchiveExtractionDialog::openArchive()
{
    // load system default open dialog
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Open Archive"),
                                                    (mArchivePathEdit->text().isEmpty()) ? QFileInfo(mFilePath).absolutePath() : mArchivePathEdit->text(),
                                                    tr("Archives (%1)").arg(DkSettingsManager::param().app().containerRawFilters.remove(",")),
                                                    nullptr,
                                                    DkDialog::fileDialogOptions());

    if (QFileInfo(filePath).exists()) {
        mArchivePathEdit->setText(filePath);
        loadArchive(filePath);
    }
}

void DkArchiveExtractionDialog::openDir()
{
    // load system default open dialog
    QString filePath = QFileDialog::getExistingDirectory(this,
                                                         tr("Open Directory"),
                                                         (mDirPathEdit->text().isEmpty()) ? QFileInfo(mFilePath).absolutePath() : mDirPathEdit->text(),
                                                         QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | DkDialog::fileDialogOptions());

    if (QFileInfo(filePath).exists())
        mDirPathEdit->setText(filePath);
}

void DkArchiveExtractionDialog::userFeedback(const QString &msg, bool error)
{
    if (!error)
        mFeedbackLabel->setProperty("warning", false);
    else
        mFeedbackLabel->setProperty("warning", true);

    mFeedbackLabel->setText(msg);
    mFeedbackLabel->style()->unpolish(mFeedbackLabel);
    mFeedbackLabel->style()->polish(mFeedbackLabel);
    mFeedbackLabel->update();
}

void DkArchiveExtractionDialog::loadArchive(const QString &filePath)
{
    mFileList = QStringList();
    mFileListDisplay->clear();

    QString lFilePath = filePath;
    if (lFilePath.isEmpty())
        lFilePath = mArchivePathEdit->text();

    QFileInfo fileInfo(lFilePath);
    if (!fileInfo.exists())
        return;

    if (!DkBasicLoader::isContainer(lFilePath)) {
        userFeedback(tr("Not a valid archive."), true);
        return;
    }

    if (mDirPathEdit->text().isEmpty()) {
        mDirPathEdit->setText(lFilePath.remove("." + fileInfo.suffix()));
        mDirPathEdit->setFocus();
    }

    QStringList fileNameList = JlCompress::getFileList(lFilePath);

    // remove the * in fileFilters
    QStringList fileFiltersClean = DkSettingsManager::param().app().browseFilters;
    for (int idx = 0; idx < fileFiltersClean.size(); idx++)
        fileFiltersClean[idx].replace("*", "");

    for (int idx = 0; idx < fileNameList.size(); idx++) {
        for (int idxFilter = 0; idxFilter < fileFiltersClean.size(); idxFilter++) {
            if (fileNameList.at(idx).contains(fileFiltersClean[idxFilter], Qt::CaseInsensitive)) {
                mFileList.append(fileNameList.at(idx));
                break;
            }
        }
    }

    if (mFileList.size() > 0)
        userFeedback(tr("Number of images: ") + QString::number(mFileList.size()), false);
    else {
        userFeedback(tr("The archive does not contain any images."), false);
        return;
    }

    mFileListDisplay->addItems(mFileList);

    if (mRemoveSubfolders->checkState() == Qt::Checked) {
        for (int i = 0; i < mFileListDisplay->count(); i++) {
            QFileInfo fi(mFileListDisplay->item(i)->text());
            mFileListDisplay->item(i)->setText(fi.fileName());
        }
    }
    mFileListDisplay->update();

    mButtons->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void DkArchiveExtractionDialog::accept()
{
    QStringList extractedFiles = extractFilesWithProgress(mArchivePathEdit->text(), mFileList, mDirPathEdit->text(), mRemoveSubfolders->isChecked());

    if ((extractedFiles.isEmpty() || extractedFiles.size() != mFileList.size()) && !extractedFiles.contains("userCanceled")) {
        QMessageBox msgBox(this);
        msgBox.setText(tr("The images could not be extracted!"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }

    QDialog::accept();
}

void DkArchiveExtractionDialog::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
        QUrl url = event->mimeData()->urls().at(0);
        qDebug() << "dropping: " << url;
        url = url.toLocalFile();

        if (QFileInfo(url.toString()).isFile()) {
            mArchivePathEdit->setText(url.toString());
            loadArchive(url.toString());
        } else
            mDirPathEdit->setText(url.toString());
    }
}

void DkArchiveExtractionDialog::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QUrl url = event->mimeData()->urls().at(0);
        url = url.toLocalFile();
        QFileInfo file = QFileInfo(url.toString());

        if (file.exists())
            event->acceptProposedAction();
    }
}

QStringList
DkArchiveExtractionDialog::extractFilesWithProgress(const QString &fileCompressed, const QStringList &files, const QString &dir, bool removeSubfolders)
{
    QProgressDialog progressDialog(this);
    progressDialog.setCancelButtonText(tr("&Cancel"));
    progressDialog.setRange(0, files.size() - 1);
    progressDialog.setWindowTitle(tr("Extracting files..."));
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setModal(true);
    progressDialog.hide();
    progressDialog.show();

    QStringList extracted;
    for (int i = 0; i < files.count(); i++) {
        progressDialog.setValue(i);
        progressDialog.setLabelText(tr("Extracting file %1 of %2").arg(i + 1).arg(files.size()));

        QString absPath;
        if (removeSubfolders)
            absPath = QDir(dir).absoluteFilePath(QFileInfo(files.at(i)).fileName());
        else
            absPath = QDir(dir).absoluteFilePath(files.at(i));

        if (JlCompress::extractFile(fileCompressed, files.at(i), absPath).isEmpty()) {
            qDebug() << "unable to extract:" << files.at(i);
            // return QStringList();
        }
        extracted.append(absPath);
        if (progressDialog.wasCanceled()) {
            return QStringList("userCanceled");
        }
    }

    progressDialog.close();

    return extracted;
}

#endif

// DkDialogManager --------------------------------------------------------------------
DkDialogManager::DkDialogManager(QObject *parent)
    : QObject(parent)
{
    DkActionManager &am = DkActionManager::instance();

    connect(am.action(DkActionManager::menu_edit_shortcuts), SIGNAL(triggered()), this, SLOT(openShortcutsDialog()));
    connect(am.action(DkActionManager::menu_file_app_manager), SIGNAL(triggered()), this, SLOT(openAppManager()));
    connect(am.action(DkActionManager::menu_file_print), SIGNAL(triggered()), this, SLOT(openPrintDialog()));
    connect(am.action(DkActionManager::menu_tools_mosaic), SIGNAL(triggered()), this, SLOT(openMosaicDialog()));
}

void DkDialogManager::openShortcutsDialog() const
{
    DkActionManager &am = DkActionManager::instance();

    DkShortcutsDialog *shortcutsDialog = new DkShortcutsDialog(DkUtils::getMainWindow());
    shortcutsDialog->addActions(am.fileActions(), am.fileMenu()->title());
    shortcutsDialog->addActions(am.openWithActions(), am.openWithMenu()->title());
    shortcutsDialog->addActions(am.sortActions(), am.sortMenu()->title());
    shortcutsDialog->addActions(am.editActions(), am.editMenu()->title());
    shortcutsDialog->addActions(am.manipulatorActions(), am.manipulatorMenu()->title());
    shortcutsDialog->addActions(am.viewActions(), am.viewMenu()->title());
    shortcutsDialog->addActions(am.panelActions(), am.panelMenu()->title());
    shortcutsDialog->addActions(am.toolsActions(), am.toolsMenu()->title());
    shortcutsDialog->addActions(am.syncActions(), am.syncMenu()->title());
    shortcutsDialog->addActions(am.previewActions(), tr("Preview"));
#ifdef WITH_PLUGINS // TODO

    DkPluginActionManager *pm = am.pluginActionManager();
    pm->updateMenu();

    QVector<QAction *> allPluginActions = pm->pluginActions();

    for (const QMenu *m : pm->pluginSubMenus()) {
        allPluginActions << m->actions().toVector();
    }

    shortcutsDialog->addActions(allPluginActions, pm->menu()->title());
#endif // WITH_PLUGINS
    shortcutsDialog->addActions(am.helpActions(), am.helpMenu()->title());
    shortcutsDialog->addActions(am.hiddenActions(), tr("Shortcuts"));

    shortcutsDialog->exec();
    shortcutsDialog->deleteLater();
}

void DkDialogManager::setCentralWidget(DkCentralWidget *cw)
{
    mCentralWidget = cw;
}

void DkDialogManager::openAppManager() const
{
    DkActionManager &am = DkActionManager::instance();

    DkAppManagerDialog *appManagerDialog = new DkAppManagerDialog(am.appManager(), DkUtils::getMainWindow());
    connect(appManagerDialog, SIGNAL(openWithSignal(QAction *)), am.appManager(), SIGNAL(openFileSignal(QAction *))); // forward
    appManagerDialog->exec();

    appManagerDialog->deleteLater();

    DkActionManager::instance().updateOpenWithMenu();
}

void DkDialogManager::openMosaicDialog() const
{
    if (!mCentralWidget) {
        qWarning() << "cannot compute mosaic if there is no central widget...";
        return;
    }

#ifdef WITH_OPENCV
    DkMosaicDialog *mosaicDialog = new DkMosaicDialog(DkUtils::getMainWindow(), Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    mosaicDialog->setFile(mCentralWidget->getCurrentFilePath());

    int response = mosaicDialog->exec();

    if (response == QDialog::Accepted && !mosaicDialog->getImage().isNull()) {
        QImage editedImage = mosaicDialog->getImage();

        QSharedPointer<DkImageContainerT> imgC(new DkImageContainerT(""));
        imgC->setImage(mosaicDialog->getImage(), tr("Mosaic"));

        mCentralWidget->addTab(imgC);
        DkActionManager::instance().action(DkActionManager::menu_file_save_as)->trigger();
    }

    mosaicDialog->deleteLater();
#endif
}

void DkDialogManager::openPrintDialog() const
{
    if (!mCentralWidget) {
        qWarning() << "cannot open print dialog if there is no central widget...";
        return;
    }

    QSharedPointer<DkImageContainerT> imgC = mCentralWidget->getCurrentImage();
    if (!imgC) {
        qWarning() << "cannot open print dialog if there is no ImageContainer...";
        return;
    }

    DkPrintPreviewDialog *previewDialog = new DkPrintPreviewDialog(DkUtils::getMainWindow());
    previewDialog->setImage(imgC->image());

    // load all pages of tiffs
    if (imgC->getLoader()->getNumPages() > 1) {
        auto l = imgC->getLoader();

        for (int idx = 1; idx < l->getNumPages(); idx++) {
            l->loadPageAt(idx + 1);
            previewDialog->addImage(l->image());
        }
    }

    previewDialog->exec();
    previewDialog->deleteLater();
}

// -------------------------------------------------------------------- DkPrintImage
DkPrintImage::DkPrintImage(const QImage &img, QPrinter *printer)
{
    mImg = img;
    mPrinter = printer;
}

QImage DkPrintImage::image() const
{
    return mImg;
}

void DkPrintImage::draw(QPainter &p, bool highQuality)
{
    QRect r = mImg.rect();
    r = mTransform.mapRect(r);

    QImage img = mImg;

    if (highQuality)
        img = DkImage::resizeImage(mImg, QSize(), mTransform.m11(), DkImage::ipl_area, false);
    else
        p.setRenderHints(QPainter::SmoothPixmapTransform);

    p.drawImage(r, img, img.rect());
}

void DkPrintImage::fit()
{
    if (!mPrinter) {
        qWarning() << "cannot fit image if the printer is NULL";
        return;
    }

    double sf = 0;

    /* TODO: Check/test wether pageRect() calls below
     * provide correct measurements to scale the image
     */

    QRectF pr = mPrinter->pageRect(QPrinter::DevicePixel);

    // scale image to fit on paper
    if (pr.width() / mImg.width() < pr.height() / mImg.height()) {
        sf = pr.width() / (mImg.width() + DBL_EPSILON);
    } else {
        sf = pr.height() / (mImg.height() + DBL_EPSILON);
    }

    double inchW = mPrinter->pageRect(QPrinter::Inch).width();
    double pxW = mPrinter->pageRect(QPrinter::DevicePixel).width();
    double cDpi = dpi();

    // use at least 150 dpi
    if (cDpi < 150 && sf > 1) {
        cDpi = 150;
        sf = (pxW / inchW) / cDpi;
        qDebug() << "new scale Factor:" << sf;
    }

    mTransform.reset();
    mTransform.scale(sf, sf);

    // TODO: print
    // updateDpiFactor(mDpi);
    center();
}

double DkPrintImage::dpi()
{
    double iW = mPrinter->pageRect(QPrinter::Inch).width();
    double pxW = mPrinter->pageRect(QPrinter::DevicePixel).width();

    return (pxW / iW) / mTransform.m11();
}

void DkPrintImage::center()
{
    center(mTransform);
}

void DkPrintImage::scale(double sf)
{
    mTransform.reset();
    mTransform.scale(sf, sf);

    center();
}

void DkPrintImage::center(QTransform &t) const
{
    QRectF transRect = t.mapRect(mImg.rect());
    qreal xtrans = 0, ytrans = 0;

    /* TODO: Check/test QPrinter->pageRect() values and
     * correct calculation of the center
     */

    xtrans = ((mPrinter->pageRect(QPrinter::DevicePixel).width() - transRect.width()) / 2);
    ytrans = (mPrinter->pageRect(QPrinter::DevicePixel).height() - transRect.height()) / 2;

    t.translate(-t.dx() / (t.m11() + DBL_EPSILON), -t.dy() / (t.m22() + DBL_EPSILON)); // reset old transformation
    t.translate(xtrans / (t.m11() + DBL_EPSILON), ytrans / (t.m22() + DBL_EPSILON));
}

// -------------------------------------------------------------------- DkSvgSizeDialog
DkSvgSizeDialog::DkSvgSizeDialog(const QSize &size, QWidget *parent)
    : QDialog(parent)
{
    mSize = size;
    mARatio = (double)size.width() / size.height();
    setWindowTitle("Resize SVG");
    createLayout();

    QMetaObject::connectSlotsByName(this);
}

void DkSvgSizeDialog::createLayout()
{
    QLabel *wl = new QLabel(tr("width:"), this);

    mSizeBox.resize(b_end);

    mSizeBox[b_width] = new QSpinBox(this);
    mSizeBox[b_width]->setObjectName("width");

    QLabel *hl = new QLabel(tr("height:"), this);

    mSizeBox[b_height] = new QSpinBox(this);
    mSizeBox[b_height]->setObjectName("height");

    for (auto s : mSizeBox) {
        s->setMinimum(1);
        s->setMaximum(50000);
        s->setSuffix(" px");
    }

    mSizeBox[b_width]->setValue(mSize.width());
    mSizeBox[b_height]->setValue(mSize.height());

    // buttons
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(wl, 1, 1);
    layout->addWidget(mSizeBox[b_width], 1, 2);
    layout->addWidget(hl, 1, 3);
    layout->addWidget(mSizeBox[b_height], 1, 4);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(5, 1);
    layout->setRowStretch(0, 1);
    layout->setRowStretch(2, 1);
    layout->addWidget(buttons, 3, 1, 1, 6, Qt::AlignBottom);
}

void DkSvgSizeDialog::on_width_valueChanged(int val)
{
    mSize.setWidth(val);
    mSize.setHeight(qRound(val / mARatio));

    mSizeBox[b_height]->blockSignals(true);
    mSizeBox[b_height]->setValue(mSize.height());
    mSizeBox[b_height]->blockSignals(false);
}

void DkSvgSizeDialog::on_height_valueChanged(int val)
{
    mSize.setWidth(qRound(val * mARatio));
    mSize.setHeight(val);

    mSizeBox[b_width]->blockSignals(true);
    mSizeBox[b_width]->setValue(mSize.width());
    mSizeBox[b_width]->blockSignals(false);
}

QSize DkSvgSizeDialog::size() const
{
    return mSize;
}

// -------------------------------------------------------------------- DkChooseMonitorDialog
DkChooseMonitorDialog::DkChooseMonitorDialog(QWidget *parent)
    : QDialog(parent)
{
    mScreens = screens();
    createLayout();
    loadSettings();
    resize(300, 150);
}

void DkChooseMonitorDialog::createLayout()
{
    mDisplayWidget = new DkDisplayWidget(this);
    mDisplayWidget->show();

    mCbRemember = new QCheckBox(tr("Remember Monitor Settings"), this);

    // buttons
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    QGridLayout *layout = new QGridLayout(this);
    layout->setRowStretch(0, 1);
    layout->addWidget(mDisplayWidget, 1, 1);
    layout->addWidget(mCbRemember, 2, 1);
    layout->addWidget(buttons, 3, 1);
    layout->setRowStretch(4, 1);
}

void DkChooseMonitorDialog::loadSettings()
{
    DefaultSettings settings;
    settings.beginGroup("MonitorSetup");

    int mIdx = settings.value("monitorIndex", 0).toInt();
    mCbRemember->setChecked(!settings.value("showDialog", true).toBool());

    settings.endGroup();

    if (mIdx >= 0 && mIdx < mDisplayWidget->count())
        mDisplayWidget->setCurrentIndex(mIdx);
    else
        mCbRemember->setChecked(false); // fall-back if the count is illegal
}

void DkChooseMonitorDialog::saveSettings() const
{
    DefaultSettings settings;

    settings.beginGroup("MonitorSetup");
    settings.setValue("monitorIndex", mDisplayWidget->currentIndex());
    settings.setValue("showDialog", !mCbRemember->isChecked());
    settings.endGroup();
}

int DkChooseMonitorDialog::exec()
{
    int answer = QDialog::exec();

    if (answer == QDialog::Accepted)
        saveSettings();

    return answer;
}

QList<QScreen *> DkChooseMonitorDialog::screens() const
{
    return QGuiApplication::screens();
}

QRect DkChooseMonitorDialog::screenRect() const
{
    return mDisplayWidget->screenRect();
}

bool DkChooseMonitorDialog::showDialog() const
{
    return !mCbRemember->isChecked();
}

} // close namespace
