/*******************************************************************************************************
 DkNoMacs.cpp
 Created on:	26.10.2014

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2014 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2014 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2014 Florian Kleber <florian@nomacs.org>

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

#include "DkBatch.h"
#include "DkActionManager.h"
#include "DkBasicWidgets.h"
#include "DkDialog.h"
#include "DkImageLoader.h"
#include "DkImageStorage.h"
#include "DkManipulatorWidgets.h"
#include "DkMessageBox.h"
#include "DkPluginInterface.h"
#include "DkPluginManager.h"
#include "DkProcess.h"
#include "DkSettings.h"
#include "DkSettingsWidget.h"
#include "DkThumbsWidgets.h"
#include "DkUtils.h"
#include "DkWidgets.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDropEvent>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QMessageBox>
#include <QMimeData>
#include <QProgressBar>
#include <QRadioButton>
#include <QSplitter>
#include <QStackedLayout>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QTextBlock>
#include <QTextEdit>
#include <QTreeView>
#pragma warning(pop) // no warnings from includes - end

namespace nmc
{

// DkBatchTabButton --------------------------------------------------------------------
DkBatchTabButton::DkBatchTabButton(const QString &title, const QString &info, QWidget *parent)
    : QPushButton(title, parent)
{
    // TODO: add info
    mInfo = info;
    setFlat(true);
    setCheckable(true);
}

void DkBatchTabButton::setInfo(const QString &info)
{
    mInfo = info;
    update();

    emit infoChanged(mInfo);
}

QString DkBatchTabButton::info() const
{
    return mInfo;
}

void DkBatchTabButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    // fixes stylesheets which are not applied to custom widgets
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    // style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QFont f;
    f.setPointSize(10); // one size smaller than default font see:stylesheet.css
    f.setItalic(true);
    p.setFont(f);

    // change opacity
    QColor c = p.pen().color();
    c.setAlpha(200);
    QPen fPen = p.pen();
    fPen.setColor(c);
    p.setPen(fPen);

    // Try to position the info text reasonably when dpi scaling is in effect
    // NOTE: The stylesheet gives us 30px of bottom padding, however
    // I could not derive a reliable rectangle with dpi scale factor
    double dpiFactor = DkSettingsManager::instance().settings().dpiScaleFactor(this);
    int x = 25 * dpiFactor;
    int y = opt.rect.bottom() - p.fontMetrics().descent();
    // p.drawRect(opt.rect.x(), opt.rect.bottom() - 30 * dpiFactor, opt.rect.width(), 30 * dpiFactor);
    p.drawText(QPoint(x, y), mInfo);
}

// DkBatchContainer --------------------------------------------------------------------
DkBatchContainer::DkBatchContainer(const QString &titleString, const QString &headerString, QWidget *parent)
    : QObject(parent)
{
    mHeaderButton = new DkBatchTabButton(titleString, headerString, parent);
    createLayout();
}

void DkBatchContainer::createLayout()
{
}

void DkBatchContainer::setContentWidget(QWidget *batchContent)
{
    mBatchContent = dynamic_cast<DkBatchContent *>(batchContent);

    connect(mHeaderButton, &DkBatchTabButton::toggled, this, &DkBatchContainer::showContent);
    connect(mBatchContent, &DkBatchContent::newHeaderText, mHeaderButton, &DkBatchTabButton::setInfo);
}

QWidget *DkBatchContainer::contentWidget() const
{
    return dynamic_cast<QWidget *>(mBatchContent);
}

DkBatchContent *DkBatchContainer::batchContent() const
{
    return mBatchContent;
}

DkBatchTabButton *DkBatchContainer::headerWidget() const
{
    return mHeaderButton;
}

void DkBatchContainer::showContent(bool show) const
{
    if (show)
        emit showSignal();

    // mShowButton->click();
    // contentWidget()->setVisible(show);
}

// void DkBatchContainer::setTitle(const QString& titleString) {
//	mTitleString = titleString;
//	mTitleLabel->setText(titleString);
// }
//
// void DkBatchContainer::setHeader(const QString& headerString) {
//	mHeaderString = headerString;
//	mHeaderLabel->setText(headerString);
// }

// DkInputTextEdit --------------------------------------------------------------------
DkInputTextEdit::DkInputTextEdit(QWidget *parent /* = 0 */)
    : QTextEdit(parent)
{
    setAcceptDrops(true);
    connect(this, &DkInputTextEdit::textChanged, this, &DkInputTextEdit::fileListChangedSignal);
}

void DkInputTextEdit::appendFiles(const QStringList &fileList)
{
    QStringList cFileList = getFileList();
    QStringList newFiles;

    // unique!
    for (const QString &cStr : fileList) {
        if (!cFileList.contains(cStr))
            newFiles.append(cStr);
    }

    if (!newFiles.empty()) {
        append(newFiles.join("\n"));
        fileListChangedSignal();
    }
}

void DkInputTextEdit::appendDir(const QString &newDir, bool recursive)
{
    if (recursive) {
        qDebug() << "adding recursive...";
        QDir tmpDir = newDir;
        QFileInfoList subDirs = tmpDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);

        for (QFileInfo cDir : subDirs)
            appendDir(cDir.absoluteFilePath(), recursive);
    }

    QDir tmpDir = newDir;
    tmpDir.setSorting(QDir::LocaleAware);
    QFileInfoList fileList = tmpDir.entryInfoList(DkSettingsManager::param().app().fileFilters);
    QStringList strFileList;

    for (QFileInfo entry : fileList) {
        strFileList.append(entry.absoluteFilePath());
    }

    qDebug() << "appending " << strFileList.size() << " files";

    appendFiles(strFileList);
}

void DkInputTextEdit::appendFromMime(const QMimeData *mimeData, bool recursive)
{
    if (!mimeData || !mimeData->hasUrls())
        return;

    QStringList files;

    const auto urls = mimeData->urls();
    for (auto &url : urls) {
        DkFileInfo fileInfo(DkUtils::urlToLocalFile(url));

        if (fileInfo.isDir())
            appendDir(fileInfo.path(), recursive);
        else if (DkUtils::isLoadable(fileInfo))
            files.append(fileInfo.path());
    }

    if (!files.empty())
        appendFiles(files);
}

void DkInputTextEdit::insertFromMimeData(const QMimeData *mimeData)
{
    appendFromMime(mimeData);
    QTextEdit::insertFromMimeData(mimeData);
}

void DkInputTextEdit::dragEnterEvent(QDragEnterEvent *event)
{
    QTextEdit::dragEnterEvent(event);

    if (event->source() == this)
        event->acceptProposedAction();
    else if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void DkInputTextEdit::dragMoveEvent(QDragMoveEvent *event)
{
    QTextEdit::dragMoveEvent(event);

    if (event->source() == this)
        event->acceptProposedAction();
    else if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void DkInputTextEdit::dropEvent(QDropEvent *event)
{
    if (event->source() == this) {
        event->accept();
        return;
    }

    appendFromMime(event->mimeData(), (event->modifiers() & Qt::ControlModifier) != 0);

    // do not propagate!
    // QTextEdit::dropEvent(event);
}

QStringList DkInputTextEdit::getFileList() const
{
    QStringList fileList;
    QString textString;
    QTextStream textStream(&textString);
    textStream << toPlainText();

    QString line;
    do {
        line = textStream.readLine(); // we don't want to get into troubles with carriage returns of different OS
        if (!line.isNull() && !line.trimmed().isEmpty())
            fileList.append(line);
    } while (!line.isNull());

    return fileList;
}

void DkInputTextEdit::clear()
{
    mResultList.clear();
    QTextEdit::clear();
}

QString DkInputTextEdit::firstDirPath() const
{
    QStringList fl = getFileList();

    if (fl.isEmpty())
        return "";

    return QFileInfo(fl[0]).absolutePath();
}

// File Selection --------------------------------------------------------------------
DkBatchInput::DkBatchInput(DkThumbLoader *thumbLoader, QWidget *parent /* = 0 */, Qt::WindowFlags f /* = 0 */)
    : DkBatchContent(parent, f)
{
    setObjectName("DkBatchInput");
    createLayout(thumbLoader);
    setMinimumHeight(300);
}

void DkBatchInput::createLayout(DkThumbLoader *thumbLoader)
{
    mDirectoryEdit = new DkDirectoryEdit(this);

    QWidget *upperWidget = new QWidget(this);
    QGridLayout *upperWidgetLayout = new QGridLayout(upperWidget);
    upperWidgetLayout->setContentsMargins(0, 0, 0, 0);
    upperWidgetLayout->addWidget(mDirectoryEdit, 0, 1);

    mInputTextEdit = new DkInputTextEdit(this);

    mResultTextEdit = new QTextEdit(this);
    mResultTextEdit->setReadOnly(true);
    mResultTextEdit->setVisible(false);

    mThumbScrollWidget = new DkThumbScrollWidget(thumbLoader, this);
    mThumbScrollWidget->getThumbWidget()->setImageLoader(mLoader);

    // add explorer
    mExplorer = new DkExplorer(tr("File Explorer"));
    mExplorer->getModel()->setFilter(QDir::Dirs | QDir::Drives | QDir::NoDotAndDotDot | QDir::AllDirs);
    mExplorer->getModel()->setNameFilters(QStringList());
    mExplorer->setMaximumWidth(300);

    QStringList folders = DkSettingsManager::param().global().recentFiles;

    if (folders.size() > 0)
        mExplorer->setCurrentPath(folders[0]);

    // tab widget
    mInputTabs = new QTabWidget(this);
    mInputTabs->addTab(mThumbScrollWidget, DkImage::loadIcon(":/nomacs/img/rects.svg"), tr("Thumbnails"));
    mInputTabs->addTab(mInputTextEdit, DkImage::loadIcon(":/nomacs/img/bars.svg"), tr("File List"));

    QGridLayout *widgetLayout = new QGridLayout(this);
    widgetLayout->setContentsMargins(0, 0, 0, 0);
    widgetLayout->addWidget(mExplorer, 0, 0, 2, 1);
    widgetLayout->addWidget(upperWidget, 0, 1);
    widgetLayout->addWidget(mInputTabs, 1, 1);
    setLayout(widgetLayout);

    connect(mThumbScrollWidget->getThumbWidget(), &DkThumbScene::selectionChanged, this, &DkBatchInput::selectionChanged);
    connect(mThumbScrollWidget, &DkThumbScrollWidget::batchProcessFilesSignal, mInputTextEdit, &DkInputTextEdit::appendFiles);
    connect(mThumbScrollWidget, &DkThumbScrollWidget::updateDirSignal, this, &DkBatchInput::setDir);
    connect(mThumbScrollWidget, &DkThumbScrollWidget::filterChangedSignal, mLoader.data(), &DkImageLoader::setFolderFilter, Qt::UniqueConnection);

    connect(mInputTextEdit, &DkInputTextEdit::fileListChangedSignal, this, &DkBatchInput::selectionChanged);

    connect(mDirectoryEdit, &DkDirectoryEdit::textChanged, this, &DkBatchInput::parameterChanged);
    connect(mDirectoryEdit, &DkDirectoryEdit::directoryChanged, this, &DkBatchInput::setDir);
    connect(mExplorer, &DkExplorer::openDir, this, &DkBatchInput::setDir);
    connect(mLoader.data(), &DkImageLoader::updateDirSignal, mThumbScrollWidget, &DkThumbScrollWidget::updateThumbs);
}

void DkBatchInput::applyDefault()
{
    mInputTextEdit->clear();
    selectionChanged();
}

void DkBatchInput::changeTab(int tabIdx) const
{
    if (tabIdx < 0 || tabIdx >= mInputTabs->count())
        return;

    mInputTabs->setCurrentIndex(tabIdx);
}

void DkBatchInput::updateDir(QVector<QSharedPointer<DkImageContainerT>> thumbs)
{
    emit updateDirSignal(thumbs);
}

void DkBatchInput::browse()
{
    // load system default open dialog
    QString dirName =
        QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"), mCDirPath, QFileDialog::ShowDirsOnly | DkDialog::fileDialogOptions());

    if (dirName.isEmpty())
        return;

    setDir(dirName);
}

QString DkBatchInput::getDir() const
{
    return mDirectoryEdit->existsDirectory() ? QDir(mDirectoryEdit->text()).absolutePath() : "";
}

QStringList DkBatchInput::getSelectedFiles() const
{
    QStringList textList = mInputTextEdit->getFileList();

    if (textList.empty())
        return mThumbScrollWidget->getThumbWidget()->getSelectedFiles();
    else
        return textList;
}

QStringList DkBatchInput::getSelectedFilesBatch()
{
    QStringList textList = mInputTextEdit->getFileList();

    if (textList.empty()) {
        textList = mThumbScrollWidget->getThumbWidget()->getSelectedFiles();
        mInputTextEdit->appendFiles(textList);
    }

    return textList;
}

DkInputTextEdit *DkBatchInput::getInputEdit() const
{
    return mInputTextEdit;
}

void DkBatchInput::setFileInfo(QFileInfo file)
{
    setDir(file.absoluteFilePath());
}

void DkBatchInput::setDir(const QString &dirPath)
{
    mExplorer->setCurrentPath(dirPath);

    mCDirPath = dirPath;
    qDebug() << "setting directory to:" << dirPath;
    mDirectoryEdit->setText(mCDirPath);
    emit newHeaderText(mCDirPath);
    emit updateInputDir(mCDirPath);
    mLoader->loadDir(mCDirPath, false);
    mThumbScrollWidget->updateThumbs(mLoader->getImages());
}

void DkBatchInput::selectionChanged()
{
    QString msg;
    if (getSelectedFiles().empty())
        msg = tr("No Files Selected");
    else if (getSelectedFiles().size() == 1)
        msg = tr("%1 File Selected").arg(getSelectedFiles().size());
    else
        msg = tr("%1 Files Selected").arg(getSelectedFiles().size());

    QString d = mInputTextEdit->firstDirPath();

    if (!d.isEmpty() && mCDirPath != d)
        setDir(d);

    emit newHeaderText(msg);
    emit changed();
}

void DkBatchInput::parameterChanged()
{
    QString newDirPath = mDirectoryEdit->text();

    if (QDir(newDirPath).exists() && newDirPath != mCDirPath) {
        setDir(newDirPath);
        emit changed();
    }
}

void DkBatchInput::setResults(const QStringList &results)
{
    if (mInputTabs->count() < 3) {
        mInputTabs->addTab(mResultTextEdit, tr("Results"));
    }

    mResultTextEdit->clear();
    mResultTextEdit->setHtml(results.join("<br> "));
    QTextCursor c = mResultTextEdit->textCursor();
    c.movePosition(QTextCursor::End);
    mResultTextEdit->setTextCursor(c);
    mResultTextEdit->setVisible(true);
}

void DkBatchInput::startProcessing()
{
    if (mInputTabs->count() < 3) {
        mInputTabs->addTab(mResultTextEdit, tr("Results"));
    }

    changeTab(tab_results);
    mInputTextEdit->setEnabled(false);
    mResultTextEdit->clear();
}

void DkBatchInput::stopProcessing()
{
    // mInputTextEdit->clear();
    mInputTextEdit->setEnabled(true);
}

// DkFileNameWdiget --------------------------------------------------------------------
DkFilenameWidget::DkFilenameWidget(QWidget *parent)
    : DkWidget(parent)
{
    createLayout();
    showOnlyFilename();
}

void DkFilenameWidget::createLayout()
{
    mLayout = new QGridLayout(this);
    mLayout->setContentsMargins(0, 0, 0, 5);
    setMaximumWidth(500);

    mCbType = new QComboBox(this);
    mCbType->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    mCbType->insertItem(fileNameTypes_fileName, tr("Current Filename"));
    mCbType->insertItem(fileNameTypes_Text, tr("Text"));
    mCbType->insertItem(fileNameTypes_Number, tr("Number"));
    connect(mCbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DkFilenameWidget::typeCBChanged);
    connect(mCbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DkFilenameWidget::checkForUserInput);
    connect(mCbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DkFilenameWidget::changed);

    mCbCase = new QComboBox(this);
    mCbCase->addItem(tr("Keep Case"));
    mCbCase->addItem(tr("To lowercase"));
    mCbCase->addItem(tr("To UPPERCASE"));
    connect(mCbCase, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DkFilenameWidget::checkForUserInput);
    connect(mCbCase, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DkFilenameWidget::changed);

    mSbNumber = new QSpinBox(this);
    mSbNumber->setValue(1);
    mSbNumber->setMinimum(0);
    mSbNumber->setMaximum(999); // changes - if cbDigits->setCurrentIndex() is changed!
    connect(mSbNumber, QOverload<int>::of(&QSpinBox::valueChanged), this, &DkFilenameWidget::changed);

    mCbDigits = new QComboBox(this);
    mCbDigits->addItem(tr("1 digit"));
    mCbDigits->addItem(tr("2 digits"));
    mCbDigits->addItem(tr("3 digits"));
    mCbDigits->addItem(tr("4 digits"));
    mCbDigits->addItem(tr("5 digits"));
    mCbDigits->setCurrentIndex(2); // see sBNumber->setMaximum()
    connect(mCbDigits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DkFilenameWidget::digitCBChanged);

    mLeText = new QLineEdit(this);
    connect(mLeText, &QLineEdit::textChanged, this, &DkFilenameWidget::changed);

    mPbPlus = new QPushButton("+", this);
    mPbPlus->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mPbPlus->setMinimumSize(10, 10);
    mPbPlus->setMaximumSize(30, 30);
    mPbMinus = new QPushButton("-", this);
    mPbMinus->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mPbMinus->setMinimumSize(10, 10);
    mPbMinus->setMaximumSize(30, 30);
    connect(mPbPlus, &QPushButton::clicked, this, &DkFilenameWidget::pbPlusPressed);
    connect(mPbMinus, &QPushButton::clicked, this, &DkFilenameWidget::pbMinusPressed);
    connect(mPbPlus, &QPushButton::clicked, this, &DkFilenameWidget::changed);
    connect(mPbMinus, &QPushButton::clicked, this, &DkFilenameWidget::changed);
}

void DkFilenameWidget::typeCBChanged(int index)
{
    switch (index) {
    case fileNameTypes_fileName: {
        showOnlyFilename();
        break;
    };
    case fileNameTypes_Text: {
        showOnlyText();
        break;
    };
    case fileNameTypes_Number: {
        showOnlyNumber();
        break;
    };
    default:
        break;
    }
}

void DkFilenameWidget::showOnlyFilename()
{
    mCbCase->show();

    mSbNumber->hide();
    mCbDigits->hide();
    mLeText->hide();

    mLayout->addWidget(mCbType, 0, fileNameWidget_type);
    mLayout->addWidget(mCbCase, 0, fileNameWidget_input1);
    // curLayout->addWidget(new QWidget(this), 0, fileNameWidget_input2 );
    mLayout->addWidget(mPbPlus, 0, fileNameWidget_plus);
    mLayout->addWidget(mPbMinus, 0, fileNameWidget_minus);
}

void DkFilenameWidget::showOnlyNumber()
{
    mSbNumber->show();
    mCbDigits->show();

    mCbCase->hide();
    mLeText->hide();

    mLayout->addWidget(mCbType, 0, fileNameWidget_type);
    mLayout->addWidget(mSbNumber, 0, fileNameWidget_input1);
    mLayout->addWidget(mCbDigits, 0, fileNameWidget_input2);
    mLayout->addWidget(mPbPlus, 0, fileNameWidget_plus);
    mLayout->addWidget(mPbMinus, 0, fileNameWidget_minus);
}

void DkFilenameWidget::showOnlyText()
{
    mLeText->show();

    mSbNumber->hide();
    mCbDigits->hide();
    mCbCase->hide();

    mLayout->addWidget(mCbType, 0, fileNameWidget_type);
    mLayout->addWidget(mLeText, 0, fileNameWidget_input1);
    // curLayout->addWidget(new QWidget(this), 0, fileNameWidget_input2);
    mLayout->addWidget(mPbPlus, 0, fileNameWidget_plus);
    mLayout->addWidget(mPbMinus, 0, fileNameWidget_minus);
}

void DkFilenameWidget::pbPlusPressed()
{
    emit plusPressed(this);
}

void DkFilenameWidget::pbMinusPressed()
{
    emit minusPressed(this);
}

void DkFilenameWidget::enableMinusButton(bool enable)
{
    mPbMinus->setEnabled(enable);
}

void DkFilenameWidget::enablePlusButton(bool enable)
{
    mPbPlus->setEnabled(enable);
}

void DkFilenameWidget::checkForUserInput()
{
    if (mCbType->currentIndex() == 0 && mCbCase->currentIndex() == 0)
        hasChanged = false;
    else
        hasChanged = true;
    // emit changed();
}

void DkFilenameWidget::digitCBChanged(int index)
{
    mSbNumber->setMaximum(qRound(pow(10, index + 1) - 1));
    emit changed();
}

QString DkFilenameWidget::getTag() const
{
    QString tag;

    switch (mCbType->currentIndex()) {
    case fileNameTypes_Number: {
        tag += "<d:";
        tag += QString::number(mCbDigits->currentIndex()); // is sensitive to the index
        tag += ":" + QString::number(mSbNumber->value());
        tag += ">";
        break;
    }
    case fileNameTypes_fileName: {
        tag += "<c:";
        tag += QString::number(mCbCase->currentIndex()); // is sensitive to the index
        tag += ">";
        break;
    }
    case fileNameTypes_Text: {
        tag += mLeText->text();
    }
    }

    return tag;
}

bool DkFilenameWidget::setTag(const QString &tag)
{
    QString cTag = tag;
    QStringList cmds = cTag.split(":");

    if (cmds.size() == 1) {
        mCbType->setCurrentIndex(fileNameTypes_Text);
        mLeText->setText(tag);
    } else {
        if (cmds[0] == "c") {
            mCbType->setCurrentIndex(fileNameTypes_fileName);
            mCbCase->setCurrentIndex(cmds[1].toInt());
        } else if (cmds[0] == "d") {
            mCbType->setCurrentIndex(fileNameTypes_Number);
            mCbDigits->setCurrentIndex(cmds[1].toInt());
            mSbNumber->setValue(cmds[2].toInt());
        } else {
            qWarning() << "cannot parse" << cmds;
            return false;
        }
    }

    return true;
}

// DkBatchOutput --------------------------------------------------------------------
DkBatchOutput::DkBatchOutput(QWidget *parent, Qt::WindowFlags f)
    : DkBatchContent(parent, f)
{
    setObjectName("DkBatchOutput");
    createLayout();
}

void DkBatchOutput::createLayout()
{
    // Output Directory Groupbox
    QLabel *outDirLabel = new QLabel(tr("Output Directory"), this);
    outDirLabel->setObjectName("subTitle");

    mOutputBrowseButton = new QPushButton(tr("Browse"));
    mOutputlineEdit = new DkDirectoryEdit(this);
    mOutputlineEdit->setPlaceholderText(tr("Select a Directory"));
    connect(mOutputBrowseButton, &QPushButton::clicked, this, &DkBatchOutput::browse);
    connect(mOutputlineEdit, &DkDirectoryEdit::textChanged, this, [this](const QString &path) {
        setDir(path);
    });

    // overwrite existing
    mCbOverwriteExisting = new QCheckBox(tr("Overwrite Existing Files"));
    mCbOverwriteExisting->setToolTip(tr("If checked, existing files are overwritten.\nThis option might destroy your images - so be careful!"));
    connect(mCbOverwriteExisting, &QCheckBox::clicked, this, &DkBatchOutput::changed);

    // overwrite existing
    mCbDoNotSave = new QCheckBox(tr("Do not Save Output Images"));
    mCbDoNotSave->setToolTip(tr("If checked, output images are not saved at all.\nThis option is only useful if plugins save sidecar files - so be careful!"));
    connect(mCbDoNotSave, &QCheckBox::clicked, this, &DkBatchOutput::changed);

    // Use Input Folder
    mCbUseInput = new QCheckBox(tr("Use Input Folder"));
    mCbUseInput->setToolTip(tr("If checked, the batch is applied to the input folder - so be careful!"));
    connect(mCbUseInput, &QCheckBox::clicked, this, &DkBatchOutput::useInputFolderChanged);

    // delete original
    mCbDeleteOriginal = new QCheckBox(tr("Delete Input Files"));
    mCbDeleteOriginal->setToolTip(tr("If checked, the original file will be deleted if the conversion was successful.\n So be careful!"));

    QWidget *cbWidget = new QWidget(this);
    QVBoxLayout *cbLayout = new QVBoxLayout(cbWidget);
    cbLayout->setContentsMargins(0, 0, 0, 0);
    cbLayout->addWidget(mCbUseInput);
    cbLayout->addWidget(mCbOverwriteExisting);
    cbLayout->addWidget(mCbDoNotSave);
    cbLayout->addWidget(mCbDeleteOriginal);

    QWidget *outDirWidget = new QWidget(this);
    QGridLayout *outDirLayout = new QGridLayout(outDirWidget);
    // outDirLayout->setContentsMargins(0, 0, 0, 0);
    outDirLayout->addWidget(mOutputBrowseButton, 0, 0);
    outDirLayout->addWidget(mOutputlineEdit, 0, 1);
    outDirLayout->addWidget(cbWidget, 1, 0);

    // Filename Groupbox
    QLabel *fileNameLabel = new QLabel(tr("Filename"), this);
    fileNameLabel->setObjectName("subTitle");

    QWidget *fileNameWidget = new QWidget(this);
    mFilenameVBLayout = new QVBoxLayout(fileNameWidget);
    mFilenameVBLayout->setSpacing(0);

    DkFilenameWidget *fwidget = new DkFilenameWidget(this);
    fwidget->enableMinusButton(false);
    mFilenameWidgets.push_back(fwidget);
    mFilenameVBLayout->addWidget(fwidget);
    connect(fwidget, &DkFilenameWidget::plusPressed, this, [this](DkFilenameWidget *widget) {
        plusPressed(widget);
    });
    connect(fwidget, &DkFilenameWidget::minusPressed, this, &DkBatchOutput::minusPressed);
    connect(fwidget, &DkFilenameWidget::changed, this, &DkBatchOutput::parameterChanged);

    QWidget *extensionWidget = new QWidget(this);
    QHBoxLayout *extensionLayout = new QHBoxLayout(extensionWidget);
    extensionLayout->setAlignment(Qt::AlignLeft);
    // extensionLayout->setSpacing(0);
    extensionLayout->setContentsMargins(0, 0, 0, 0);
    mCbExtension = new QComboBox(this);
    mCbExtension->addItem(tr("Keep Extension"));
    mCbExtension->addItem(tr("Convert To"));
    connect(mCbExtension, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DkBatchOutput::extensionCBChanged);

    mCbNewExtension = new QComboBox(this);
    mCbNewExtension->addItems(DkSettingsManager::param().app().saveFilters);
    mCbNewExtension->setFixedWidth(150);
    mCbNewExtension->setEnabled(false);
    connect(mCbNewExtension, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DkBatchOutput::parameterChanged);

    mCbCompression = new QComboBox(this);
    updateCBCompression();
    mCbCompression->setEnabled(false);

    extensionLayout->addWidget(mCbExtension);
    extensionLayout->addWidget(mCbNewExtension);
    extensionLayout->addWidget(mCbCompression);
    // extensionLayout->addStretch();
    mFilenameVBLayout->addWidget(extensionWidget);

    QLabel *previewLabel = new QLabel(tr("Preview"), this);
    previewLabel->setObjectName("subTitle");

    QLabel *oldLabel = new QLabel(tr("Old Filename: "));
    oldLabel->setObjectName("FileNamePreviewLabel");
    mOldFileNameLabel = new QLabel("");
    mOldFileNameLabel->setObjectName("FileNamePreviewLabel");

    QLabel *newLabel = new QLabel(tr("New Filename: "));
    newLabel->setObjectName("FileNamePreviewLabel");
    mNewFileNameLabel = new QLabel("");
    mNewFileNameLabel->setObjectName("FileNamePreviewLabel");

    // Preview Widget
    QWidget *previewWidget = new QWidget(this);
    QGridLayout *previewGBLayout = new QGridLayout(previewWidget);
    // previewWidget->hide();	// show if more than 1 file is selected
    previewGBLayout->addWidget(oldLabel, 0, 0);
    previewGBLayout->addWidget(mOldFileNameLabel, 0, 1);
    previewGBLayout->addWidget(newLabel, 1, 0);
    previewGBLayout->addWidget(mNewFileNameLabel, 1, 1);
    previewGBLayout->setColumnStretch(3, 10);
    previewGBLayout->setAlignment(Qt::AlignTop);

    QGridLayout *contentLayout = new QGridLayout(this);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    contentLayout->addWidget(outDirLabel, 2, 0);
    contentLayout->addWidget(outDirWidget, 3, 0);
    contentLayout->addWidget(fileNameLabel, 4, 0);
    contentLayout->addWidget(fileNameWidget, 5, 0);
    contentLayout->addWidget(previewLabel, 6, 0);
    contentLayout->addWidget(previewWidget, 7, 0);
    setLayout(contentLayout);
}

void DkBatchOutput::updateCBCompression()
{
    const QString quality_label[5] = {tr("Best Quality"), tr("High Quality"), tr("Medium Quality"), tr("Low Quality"), tr("Bad Quality")};
    int quality[5];

    const QString extStr = mCbNewExtension->currentText();
    if (extStr.contains(QRegularExpression("(avif)", QRegularExpression::CaseInsensitiveOption))) {
        quality[0] = 100;
        quality[1] = 90;
        quality[2] = 76;
        quality[3] = 57;
        quality[4] = 36;
    } else { // quality used for JPG and other formats
        quality[0] = 100;
        quality[1] = 97;
        quality[2] = 90;
        quality[3] = 80;
        quality[4] = 60;
    }

    int previous_index = mCbCompression->currentIndex();

    mCbCompression->clear();
    for (int index = 0; index < 5; index++) {
        mCbCompression->insertItem(index, quality_label[index], quality[index]);
    }

    if (previous_index == -1) {
        mCbCompression->setCurrentIndex(1);
    } else {
        mCbCompression->setCurrentIndex(previous_index);
    }
}

void DkBatchOutput::browse()
{
    QString dirGuess = (mOutputlineEdit->text().isEmpty()) ? mInputDirectory : mOutputlineEdit->text();

    // load system default open dialog
    QString dirName =
        QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"), dirGuess, QFileDialog::ShowDirsOnly | DkDialog::fileDialogOptions());

    if (dirName.isEmpty())
        return;

    setDir(dirName);
}

void DkBatchOutput::setDir(const QString &dirPath, bool updateLineEdit)
{
    mOutputDirectory = dirPath;
    emit newHeaderText(dirPath);

    if (updateLineEdit)
        mOutputlineEdit->setText(dirPath);
}

void DkBatchOutput::setInputDir(const QString &dirPath)
{
    mInputDirectory = dirPath;

    if (mCbUseInput->isChecked())
        setDir(mInputDirectory);
}

void DkBatchOutput::useInputFolderChanged(bool checked)
{
    mOutputlineEdit->setEnabled(!checked);
    mOutputBrowseButton->setEnabled(!checked);

    if (checked)
        setDir(mInputDirectory);
}

void DkBatchOutput::plusPressed(DkFilenameWidget *widget, const QString &tag)
{
    DkFilenameWidget *fw = createFilenameWidget(tag);

    int index = mFilenameVBLayout->indexOf(widget);
    mFilenameWidgets.insert(index + 1, fw);
    if (mFilenameWidgets.size() > 4) {
        for (int i = 0; i < mFilenameWidgets.size(); i++)
            mFilenameWidgets[i]->enablePlusButton(false);
    }
    mFilenameVBLayout->insertWidget(index + 1, fw); // append to current widget

    parameterChanged();
}

void DkBatchOutput::addFilenameWidget(const QString &tag)
{
    DkFilenameWidget *fw = createFilenameWidget(tag);
    mFilenameWidgets.append(fw);
    mFilenameVBLayout->insertWidget(mFilenameWidgets.size() - 1, fw); // append to current widget
}

DkFilenameWidget *DkBatchOutput::createFilenameWidget(const QString &tag)
{
    DkFilenameWidget *fw = new DkFilenameWidget(this);
    fw->setTag(tag);

    connect(fw, &DkFilenameWidget::plusPressed, this, [this](DkFilenameWidget *widget) {
        plusPressed(widget);
    });
    connect(fw, &DkFilenameWidget::minusPressed, this, &DkBatchOutput::minusPressed);
    connect(fw, &DkFilenameWidget::changed, this, &DkBatchOutput::parameterChanged);

    return fw;
}

void DkBatchOutput::minusPressed(DkFilenameWidget *widget)
{
    mFilenameVBLayout->removeWidget(widget);
    mFilenameWidgets.remove(mFilenameWidgets.indexOf(widget));
    if (mFilenameWidgets.size() <= 4) {
        for (int i = 0; i < mFilenameWidgets.size(); i++)
            mFilenameWidgets[i]->enablePlusButton(true);
    }

    widget->hide();

    parameterChanged();
}

void DkBatchOutput::extensionCBChanged(int index)
{
    mCbNewExtension->setEnabled(index > 0);
    parameterChanged();
}

bool DkBatchOutput::hasUserInput() const
{
    // TODO add output directory
    return mFilenameWidgets.size() > 1 || mFilenameWidgets[0]->hasUserInput() || mCbExtension->currentIndex() == 1;
}

void DkBatchOutput::parameterChanged()
{
    // enable/disable compression combo
    QString extStr = mCbNewExtension->currentText();
    mCbCompression->setEnabled(extStr.contains(QRegularExpression("(avif|jpg|jp2|jxl|webp)", QRegularExpression::CaseInsensitiveOption)));

    updateCBCompression();
    updateFileLabelPreview();
    emit changed();
}

void DkBatchOutput::updateFileLabelPreview()
{
    if (mExampleName.isEmpty())
        return;

    DkFileNameConverter converter(getFilePattern());

    mOldFileNameLabel->setText(mExampleName);
    mNewFileNameLabel->setText(converter.convert(mExampleName, 0));
}

QString DkBatchOutput::getOutputDirectory()
{
    return mOutputlineEdit->text();
}

QString DkBatchOutput::getFilePattern()
{
    QString pattern = "";

    for (int idx = 0; idx < mFilenameWidgets.size(); idx++)
        pattern += mFilenameWidgets.at(idx)->getTag();

    if (mCbExtension->currentIndex() == 0) {
        pattern += ".<old>";
    } else {
        QString ext = mCbNewExtension->itemText(mCbNewExtension->currentIndex());

        QStringList tmp = ext.split("(");

        if (tmp.size() == 2) {
            QString filters = tmp.at(1);
            filters.replace(")", "");
            filters.replace("*", "");

            QStringList extList = filters.split(" ");

            if (!extList.empty())
                pattern += extList[0];
        }
    }

    return pattern;
}

void DkBatchOutput::loadFilePattern(const QString &pattern)
{
    QStringList nameList = pattern.split(".");
    QString ext = nameList.last();

    QString p = pattern;
    p = p.replace("." + ext, ""); // remove extension
    p = p.replace(">", "<");

    QStringList cmdsRaw = p.split("<");
    QStringList cmds;

    for (const QString &c : cmdsRaw) {
        if (!c.trimmed().isEmpty())
            cmds << c;
    }

    // uff - first is special
    if (!cmds.empty() && !mFilenameWidgets.empty()) {
        mFilenameWidgets.first()->setTag(cmds.first());
        cmds.pop_front();
    }

    for (const QString &c : cmds) {
        if (c.isEmpty())
            continue;

        qDebug() << "processing: " << c;
        addFilenameWidget(c);
    }

    if (ext != "<old>") {
        mCbExtension->setCurrentIndex(1);
        int idx = mCbNewExtension->findText(ext, Qt::MatchContains);
        mCbNewExtension->setCurrentIndex(idx);
    } else {
        mCbExtension->setCurrentIndex(0);
    }
}

int DkBatchOutput::getCompression() const
{
    if (!mCbCompression->isEnabled())
        return -1;

    return mCbCompression->itemData(mCbCompression->currentIndex()).toInt();
}

void DkBatchOutput::applyDefault()
{
    mCbUseInput->setChecked(false);
    mCbDeleteOriginal->setChecked(false);
    mCbOverwriteExisting->setChecked(false);
    mCbDoNotSave->setChecked(false);
    mCbExtension->setCurrentIndex(0);
    mCbNewExtension->setCurrentIndex(0);
    mCbCompression->setCurrentIndex(0);
    mOutputDirectory = "";
    mInputDirectory = "";
    mHUserInput = false;
    mRUserInput = false;

    // remove all but the first
    for (int idx = mFilenameWidgets.size() - 1; idx > 0; idx--) {
        mFilenameWidgets[idx]->deleteLater();
        mFilenameWidgets.pop_back();
    }

    if (!mFilenameWidgets.empty()) {
        mFilenameWidgets[0]->setTag("c:0"); // current filename
    } else
        qWarning() << "no filename widgets...";

    mOutputlineEdit->setText(mOutputDirectory);
}

void DkBatchOutput::loadProperties(const DkBatchConfig &config)
{
    DkSaveInfo si = config.saveInfo();
    mCbOverwriteExisting->setChecked((si.mode() & DkSaveInfo::mode_overwrite) != 0);
    mCbDoNotSave->setChecked((si.mode() & DkSaveInfo::mode_do_not_save_output) != 0);
    mCbDeleteOriginal->setChecked(si.isDeleteOriginal());
    mCbUseInput->setChecked(si.isInputDirOutputDir());
    mOutputlineEdit->setText(config.getOutputDirPath());

    int c = si.compression();

    for (int idx = 0; idx < mCbCompression->count(); idx++) {
        if (mCbCompression->itemData(idx).toInt() == c) {
            mCbCompression->setCurrentIndex(idx);
            break;
        }
    }

    loadFilePattern(config.getFileNamePattern());

    parameterChanged();
}

DkSaveInfo::OverwriteMode DkBatchOutput::overwriteMode() const
{
    DkSaveInfo::OverwriteMode mode = DkSaveInfo::mode_skip_existing;

    if (mCbOverwriteExisting->isChecked())
        mode = (DkSaveInfo::OverwriteMode)(mode | DkSaveInfo::mode_overwrite);
    if (mCbDoNotSave->isChecked())
        mode = (DkSaveInfo::OverwriteMode)(mode | DkSaveInfo::mode_do_not_save_output);

    return mode;
}

bool DkBatchOutput::useInputDir() const
{
    return mCbUseInput->isChecked();
}

bool DkBatchOutput::deleteOriginal() const
{
    return mCbDeleteOriginal->isChecked();
}

void DkBatchOutput::setExampleFilename(const QString &exampleName)
{
    mExampleName = exampleName;
    updateFileLabelPreview();
}

// DkProfileSummaryWidget --------------------------------------------------------------------
DkProfileSummaryWidget::DkProfileSummaryWidget(QWidget *parent)
    : DkFadeWidget(parent)
{
    createLayout();
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
}

void DkProfileSummaryWidget::setProfile(const QString &profileName, const DkBatchConfig &config)
{
    mTitle->setText(tr("Summary: ") + profileName);
    mNumFiles->setText(QString::number(config.getFileList().size()) + " " + tr("Files"));
    mOutputDir->setText(config.getOutputDirPath());

    QString functions;
    for (QSharedPointer<DkAbstractBatch> cf : config.getProcessFunctions()) {
        functions += cf->name() + "\n";
    }
    mFunctions->setText(functions);
}

void DkProfileSummaryWidget::onDeleteButtonClicked()
{
    emit deleteCurrentProfile();
}

void DkProfileSummaryWidget::onUpdateButtonClicked()
{
    emit updateCurrentProfile();
}

void DkProfileSummaryWidget::onExportButtonClicked()
{
    emit exportCurrentProfile();
}

void DkProfileSummaryWidget::createLayout()
{
    mTitle = new QLabel("", this);
    mTitle->setObjectName("subTitle");

    QLabel *numFilesTitle = new QLabel(tr("Input"), this);
    numFilesTitle->setObjectName("summaryMeta");
    mNumFiles = new QLabel(this);

    QLabel *outputDirTitle = new QLabel(tr("Output"), this);
    outputDirTitle->setObjectName("summaryMeta");
    mOutputDir = new QLabel(this);

    QLabel *functionsTitle = new QLabel(tr("Functions"), this);
    functionsTitle->setObjectName("summaryMeta");
    mFunctions = new QLabel(this);

    QWidget *sw = new QWidget(this);
    QGridLayout *summaryLayout = new QGridLayout(sw);
    summaryLayout->setContentsMargins(0, 0, 0, 0);
    summaryLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    summaryLayout->addWidget(mTitle, 1, 1, 1, 3);
    summaryLayout->addWidget(numFilesTitle, 2, 1);
    summaryLayout->addWidget(mNumFiles, 2, 2);
    summaryLayout->addWidget(outputDirTitle, 3, 1);
    summaryLayout->addWidget(mOutputDir, 3, 2);
    summaryLayout->addWidget(functionsTitle, 4, 1);
    summaryLayout->addWidget(mFunctions, 4, 2);

    QPushButton *updateButton = new QPushButton(DkImage::loadIcon(":/nomacs/img/save.svg"), "", this);
    updateButton->setToolTip(tr("Update"));
    connect(updateButton, &QPushButton::clicked, this, &DkProfileSummaryWidget::onUpdateButtonClicked);

    QPushButton *deleteButton = new QPushButton(DkImage::loadIcon(":/nomacs/img/trash.svg"), "", this);
    deleteButton->setToolTip(tr("Delete"));
    connect(deleteButton, &QPushButton::clicked, this, &DkProfileSummaryWidget::onDeleteButtonClicked);

    QPushButton *exportButton = new QPushButton(DkImage::loadIcon(":/nomacs/img/export.svg"), "", this);
    exportButton->setToolTip(tr("Export"));
    connect(exportButton, &QPushButton::clicked, this, &DkProfileSummaryWidget::onExportButtonClicked);

    QWidget *bw = new QWidget(this);
    QHBoxLayout *bLayout = new QHBoxLayout(bw);
    bLayout->setContentsMargins(0, 0, 0, 0);
    bLayout->setAlignment(Qt::AlignRight);
    bLayout->addWidget(updateButton);
    bLayout->addWidget(exportButton);
    bLayout->addWidget(deleteButton);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(sw);
    layout->addWidget(bw);
}

// DkProfileWidget --------------------------------------------------------------------
DkProfileWidget::DkProfileWidget(QWidget *parent, Qt::WindowFlags f)
    : DkBatchContent(parent, f)
{
    createLayout();
}

void DkProfileWidget::createLayout()
{
    mProfileList = new QListWidget(this);
    mProfileList->setObjectName("profileList");
    connect(mProfileList, &QListWidget::itemSelectionChanged, this, &DkProfileWidget::onProfileListItemSelectionChanged);

    mSummary = new DkProfileSummaryWidget(this);

    // buttons
    QPushButton *saveButton = new QPushButton(tr("Create New Profile"), this);
    connect(saveButton, &QPushButton::clicked, this, &DkProfileWidget::onSaveButtonClicked);

    QPushButton *resetButton = new QPushButton(tr("Apply Default"), this);
    connect(resetButton, &QPushButton::clicked, this, &DkProfileWidget::onResetButtonClicked);

    QWidget *buttonWidget = new QWidget(this);
    QHBoxLayout *bLayout = new QHBoxLayout(buttonWidget);
    bLayout->setContentsMargins(0, 0, 0, 0);
    bLayout->setAlignment(Qt::AlignLeft);
    bLayout->addWidget(saveButton);
    bLayout->addWidget(resetButton);

    QWidget *descWidget = new QWidget(this);
    QVBoxLayout *descLayout = new QVBoxLayout(descWidget);
    descLayout->setContentsMargins(0, 0, 0, 0);
    descLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    descLayout->addWidget(mSummary);
    descLayout->addWidget(buttonWidget);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mProfileList);
    layout->addWidget(descWidget);

    updateProfileList();

    connect(mSummary, &DkProfileSummaryWidget::updateCurrentProfile, this, &DkProfileWidget::updateCurrentProfile);
    connect(mSummary, &DkProfileSummaryWidget::deleteCurrentProfile, this, &DkProfileWidget::deleteCurrentProfile);
    connect(mSummary, &DkProfileSummaryWidget::exportCurrentProfile, this, &DkProfileWidget::exportCurrentProfile);
}

bool DkProfileWidget::hasUserInput() const
{
    return false;
}

bool DkProfileWidget::requiresUserInput() const
{
    return false;
}

void DkProfileWidget::applyDefault()
{
    emit newHeaderText(tr("inactive"));
}

void DkProfileWidget::profileSaved(const QString &profileName)
{
    updateProfileList();

    QList<QListWidgetItem *> items = mProfileList->findItems(profileName, Qt::MatchExactly);

    for (auto i : items)
        i->setSelected(true);
}

void DkProfileWidget::onProfileListItemSelectionChanged()
{
    changeProfile(currentProfile());
}

void DkProfileWidget::updateProfileList()
{
    mProfileList->clear();

    DkBatchProfile bp;
    QStringList pn = bp.profileNames();

    mProfileList->addItem(tr("Default"));

    for (const QString &p : pn) {
        mProfileList->addItem(p);
    }
}

void DkProfileWidget::changeProfile(const QString &profileName)
{
    // is the default profile selected?
    if (profileName.isEmpty() || (mProfileList->count() > 0 && mProfileList->item(0)->text() == profileName)) {
        loadDefaultProfile();
        return;
    }

    QString profilePath = DkBatchProfile::profileNameToPath(profileName);
    DkBatchConfig bc = DkBatchProfile::loadProfile(profilePath);
    mSummary->setProfile(profileName, bc);
    mSummary->show();

    emit loadProfileSignal(profilePath);
    emit newHeaderText(profileName);
}

void DkProfileWidget::loadDefaultProfile()
{
    // select default profile
    if (mProfileList->count() > 0)
        mProfileList->item(0)->setSelected(true);

    mSummary->hide();
    emit newHeaderText(tr("inactive"));
    emit applyDefaultSignal();
}

QString DkProfileWidget::currentProfile() const
{
    QString profileName;

    for (auto pItem : mProfileList->selectedItems()) {
        profileName = pItem->text();
    }

    return profileName;
}

void DkProfileWidget::onSaveButtonClicked()
{
    saveProfile();
}

void DkProfileWidget::onResetButtonClicked()
{
    loadDefaultProfile();
}

void DkProfileWidget::updateCurrentProfile()
{
    emit saveProfileSignal(DkBatchProfile::profileNameToPath(currentProfile()));
}

void DkProfileWidget::deleteCurrentProfile()
{
    QFile profile(DkBatchProfile::profileNameToPath(currentProfile()));

    if (!profile.remove()) {
        QMessageBox::critical(this, tr("Deleting Profile"), tr("Sorry, I cannot delete %1").arg(currentProfile()), QMessageBox::Ok);
        return;
    }

    updateProfileList();
    loadDefaultProfile();
}

void DkProfileWidget::exportCurrentProfile()
{
    QString expPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QDir::separator() + currentProfile() + "." + DkBatchProfile::extension();

    QString sPath = QFileDialog::getSaveFileName(this,
                                                 tr("Export Batch Profile"),
                                                 expPath,
                                                 tr("nomacs Batch Profile (*.%1)").arg(DkBatchProfile::extension()),
                                                 nullptr,
                                                 DkDialog::fileDialogOptions());

    emit saveProfileSignal(sPath);
}

void DkProfileWidget::saveProfile()
{
    // default mode is overwrite (UI is asking anyway)
    QString cn = currentProfile();
    QString dName = cn.isEmpty() || cn == mProfileList->item(0)->text() ? "Profile 1" : cn;

    bool ok;
    QString text = QInputDialog::getText(this, tr("Profile Name"), tr("Profile Name:"), QLineEdit::Normal, dName, &ok);

    if (!ok || text.isEmpty())
        return; // user canceled

    // is the profile name unique?
    if (!mProfileList->findItems(text, Qt::MatchExactly).isEmpty()) {
        QMessageBox::StandardButton button =
            QMessageBox::information(this, tr("Profile Already Exists"), tr("Do you want to overwrite %1?").arg(text), QMessageBox::Yes | QMessageBox::No);

        if (button == QMessageBox::No) {
            saveProfile(); // start over
            return;
        }
    }

    emit saveProfileSignal(DkBatchProfile::profileNameToPath(text));
}

#ifdef WITH_PLUGINS
// DkBatchPlugin --------------------------------------------------------------------
DkBatchPluginWidget::DkBatchPluginWidget(QWidget *parent /* = 0 */, Qt::WindowFlags f /* = 0 */)
    : DkBatchContent(parent, f)
{
    // mSettings = DkSettingsManager::instance().qSettings();
    DkPluginManager::instance().loadPlugins();
    createLayout();

    connect(mSettingsEditor, &DkSettingsWidget::changeSettingSignal, this, &DkBatchPluginWidget::changeSetting);
    connect(mSettingsEditor, &DkSettingsWidget::removeSettingSignal, this, &DkBatchPluginWidget::removeSetting);
}

void DkBatchPluginWidget::transferProperties(QSharedPointer<DkPluginBatch> batchPlugin) const
{
    QVector<QSharedPointer<DkPluginContainer>> plugins = DkPluginManager::instance().getBatchPlugins();

    //// I don't like const casts
    //// BUT: load settings effectively only opens groups (which is not const)
    // QSettings& settings = const_cast<QSettings&>(mSettings);

    // for (auto p : plugins) {
    //
    //	if (p->batchPlugin()) {
    //		// the const ca
    //		p->batchPlugin()->loadSettings(settings);
    //	}
    // }

    QStringList pluginList = selectedPlugins();
    batchPlugin->setProperties(pluginList);
}

void DkBatchPluginWidget::createLayout()
{
    QLabel *listLabel = new QLabel(tr("Select Plugins"));
    listLabel->setObjectName("subTitle");

    mModel = new QStandardItemModel(this);

    QTreeView *pluginList = new QTreeView(this);
    pluginList->setModel(mModel);
    pluginList->header()->hide();

    // settings
    mSettingsTitle = new QLabel(this);
    mSettingsTitle->setObjectName("subTitle");

    mSettingsEditor = new DkSettingsWidget(this);
    mSettingsEditor->hide();

    addPlugins(mModel);

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(listLabel, 0, 0);
    layout->addWidget(mSettingsTitle, 0, 1);
    layout->addWidget(pluginList, 1, 0);
    layout->addWidget(mSettingsEditor, 1, 1);

    connect(mModel, &QStandardItemModel::itemChanged, this, &DkBatchPluginWidget::itemChanged);
    connect(pluginList->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DkBatchPluginWidget::selectionChanged);
}

bool DkBatchPluginWidget::loadProperties(QSharedPointer<DkPluginBatch> batchPlugin)
{
    if (!batchPlugin) {
        qWarning() << "cannot load properties, DkPluginBatch is NULL";
        return false;
    }

    mModel->blockSignals(true);
    QStringList sPlugins = batchPlugin->pluginList();

    for (int pIdx = 0; pIdx < mModel->rowCount(); pIdx++) {
        QStandardItem *pItem = mModel->item(pIdx);

        for (int idx = 0; idx < pItem->rowCount(); idx++) {
            // see if the plugin is contained in the plugin list
            QStandardItem *item = pItem->child(idx);
            QString key = item->data(Qt::UserRole).toString() + " | " + item->text();
            item->setCheckState(sPlugins.contains(key) ? Qt::Checked : Qt::Unchecked);
        }
    }
    mModel->blockSignals(false);
    updateHeader();

    return true;
}

void DkBatchPluginWidget::itemChanged(QStandardItem *item)
{
    if (!item)
        return;

    if (item->checkState() == Qt::Checked)
        selectPlugin(item->data(Qt::UserRole).toString());

    updateHeader();
}

bool DkBatchPluginWidget::hasUserInput() const
{
    return !selectedPlugins().isEmpty();
}

bool DkBatchPluginWidget::requiresUserInput() const
{
    return false;
}

void DkBatchPluginWidget::applyDefault()
{
    mSettings.clear();

    QStringList selectedPlugins;

    for (int pIdx = 0; pIdx < mModel->rowCount(); pIdx++) {
        QStandardItem *pItem = mModel->item(pIdx);

        for (int idx = 0; idx < pItem->rowCount(); idx++) {
            pItem->child(idx)->setCheckState(Qt::Unchecked);
        }
    }

    selectPlugin("");
    updateHeader();
}

void DkBatchPluginWidget::setSettingsPath(const QString &settingsPath)
{
    mSettings = QSharedPointer<QSettings>(new QSettings(settingsPath, QSettings::IniFormat));

    // choose the correct sub-group
    mSettings->beginGroup("General");
    mSettings->beginGroup("PluginBatch");
}

void DkBatchPluginWidget::selectionChanged(const QItemSelection &selected)
{
    for (auto mIdx : selected.indexes()) {
        QStandardItem *item;

        if (mIdx.parent().isValid()) {
            item = mModel->item(mIdx.parent().row());
            item = item->child(mIdx.row());
        } else
            item = mModel->item(mIdx.row());

        if (!item)
            continue;

        selectPlugin(item->data(Qt::UserRole).toString());
    }
}

void DkBatchPluginWidget::addPlugins(QStandardItemModel *model) const
{
    if (!model)
        return;

    QVector<QSharedPointer<DkPluginContainer>> plugins = DkPluginManager::instance().getBatchPlugins();

    for (auto p : plugins) {
        QStandardItem *mPluginItem = new QStandardItem(p->pluginName());
        mPluginItem->setEditable(false);
        mPluginItem->setCheckable(false);
        // mPluginItem->setAutoTristate(true);
        mPluginItem->setData(p->pluginName(), Qt::UserRole);
        mModel->appendRow(mPluginItem);

        QList<QAction *> actions = p->plugin()->pluginActions();

        for (const QAction *a : actions) {
            QStandardItem *item = new QStandardItem(a->icon(), a->text());
            item->setEditable(false);
            item->setCheckable(true);
            item->setData(p->pluginName(), Qt::UserRole);
            mPluginItem->appendRow(item);
        }
    }
}

void DkBatchPluginWidget::selectPlugin(const QString &pluginName)
{
    mCurrentPlugin = 0; // unset
    QSharedPointer<DkPluginContainer> plugin = DkPluginManager::instance().getPluginByName(pluginName);

    if (!plugin || !plugin->batchPlugin()) {
        mSettingsTitle->setText("");
        mSettingsTitle->hide();
        mSettingsEditor->hide();
        return;
    }

    mCurrentPlugin = plugin->batchPlugin();

    QSharedPointer<QSettings> s = settings();
    DkSettingsGroup g = DkSettingsGroup::fromSettings(*s, mCurrentPlugin->name());

    if (!g.isEmpty()) {
        mSettingsTitle->setText(plugin->pluginName() + tr(" Settings"));
        mSettingsTitle->show();

        mSettingsEditor->clear();
        mSettingsEditor->addSettingsGroup(g);
        mSettingsEditor->expandAll();
        mSettingsEditor->show();
    } else {
        mSettingsTitle->setText("");
        mSettingsTitle->hide();
        mSettingsEditor->hide();
    }
}

void DkBatchPluginWidget::changeSetting(const QString &key, const QVariant &value, const QStringList &groups) const
{
    if (!mCurrentPlugin) {
        qWarning() << "cannot change settings if no plugin is selected";
        return;
    }

    QSharedPointer<QSettings> s = settings();
    DkSettingsWidget::changeSetting(*s, key, value, groups);
    mCurrentPlugin->loadSettings(*s); // update
}

void DkBatchPluginWidget::removeSetting(const QString &key, const QStringList &groups) const
{
    if (!mCurrentPlugin) {
        qWarning() << "cannot delete settings if no plugin is selected";
        return;
    }

    QSharedPointer<QSettings> s = settings();
    DkSettingsWidget::removeSetting(*s, key, groups);
    mCurrentPlugin->loadSettings(*s); // update
}

QStringList DkBatchPluginWidget::selectedPlugins(bool selected) const
{
    QStringList selectedPlugins;

    for (int pIdx = 0; pIdx < mModel->rowCount(); pIdx++) {
        QStandardItem *pItem = mModel->item(pIdx);

        for (int idx = 0; idx < pItem->rowCount(); idx++) {
            QStandardItem *item = pItem->child(idx);
            if (!selected || item->checkState() == Qt::Checked)
                selectedPlugins << item->data(Qt::UserRole).toString() + " | " + item->text();
        }
    }

    return selectedPlugins;
}

QSharedPointer<QSettings> DkBatchPluginWidget::settings() const
{
    if (mSettings)
        return mSettings;

    if (mCurrentPlugin)
        return QSharedPointer<QSettings>(new QSettings(mCurrentPlugin->settingsFilePath(), QSettings::IniFormat));

    qWarning() << "DkBatchPluginWidget: I need to default the settings...";
    return QSharedPointer<QSettings>(new DefaultSettings());
}

void DkBatchPluginWidget::updateHeader() const
{
    int c = selectedPlugins().size();
    if (!c)
        emit newHeaderText(tr("inactive"));
    else
        emit newHeaderText(tr("%1 plugins selected").arg(c));
}
#endif

// DkBatchManipulatorWidget --------------------------------------------------------------------
DkBatchManipulatorWidget::DkBatchManipulatorWidget(QWidget *parent /* = 0 */, Qt::WindowFlags f /* = 0 */)
    : DkBatchContent(parent, f)
{
    mManager.createManipulators(this);
    createLayout();
    addSettingsWidgets(mManager);
}

void DkBatchManipulatorWidget::createLayout()
{
    QLabel *listLabel = new QLabel(tr("Select Image Adjustments"));
    listLabel->setObjectName("subTitle");

    mModel = new QStandardItemModel(this);

    int idx = 0;
    for (auto mpl : mManager.manipulators()) {
        QStandardItem *item = new QStandardItem(mpl->action()->icon(), mpl->name());
        item->setEditable(false);
        item->setCheckable(true);

        mModel->setItem(idx, item);
        idx++;
    }

    QListView *manipulatorList = new QListView(this);
    manipulatorList->setModel(mModel);

    // settings
    mSettingsTitle = new QLabel(this);
    mSettingsTitle->setObjectName("subTitle");

    QWidget *settingsWidget = new QWidget(this);
    mSettingsLayout = new QVBoxLayout(settingsWidget);
    mSettingsLayout->setContentsMargins(0, 0, 0, 0);
    mSettingsLayout->setAlignment(Qt::AlignTop);

    mPreviewLabel = new QLabel(this);
    mPreviewLabel->setAlignment(Qt::AlignHCenter);
    mPreviewLabel->hide();

    QWidget *rightWidget = new QWidget(this);
    QVBoxLayout *rLayout = new QVBoxLayout(rightWidget);
    rLayout->setContentsMargins(0, 0, 0, 0);
    rLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    rLayout->addWidget(settingsWidget);
    rLayout->addWidget(mPreviewLabel);

    QGridLayout *layout = new QGridLayout(this);
    // layout->setAlignment(Qt::AlignHCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(listLabel, 0, 0);
    layout->addWidget(mSettingsTitle, 0, 1);
    layout->addWidget(manipulatorList, 1, 0);
    layout->addWidget(rightWidget, 1, 1);

    connect(mModel, &QStandardItemModel::itemChanged, this, &DkBatchManipulatorWidget::itemChanged);
    connect(manipulatorList->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DkBatchManipulatorWidget::selectionChanged);
}

void DkBatchManipulatorWidget::addSettingsWidgets(DkManipulatorManager &manager)
{
    for (auto w : mMplWidgets) {
        mSettingsLayout->removeWidget(w);
        delete w;
    }

    mMplWidgets.clear(); // TODO: delete the old ones now?

    mMplWidgets << new DkTinyPlanetWidget(manager.manipulatorExt(DkManipulatorManager::m_tiny_planet), this);
    mMplWidgets << new DkBlurWidget(manager.manipulatorExt(DkManipulatorManager::m_blur), this);
    mMplWidgets << new DkUnsharpMaskWidget(manager.manipulatorExt(DkManipulatorManager::m_unsharp_mask), this);
    mMplWidgets << new DkRotateWidget(manager.manipulatorExt(DkManipulatorManager::m_rotate), this);
    mMplWidgets << new DkThresholdWidget(manager.manipulatorExt(DkManipulatorManager::m_threshold), this);
    mMplWidgets << new DkHueWidget(manager.manipulatorExt(DkManipulatorManager::m_hue), this);
    mMplWidgets << new DkExposureWidget(manager.manipulatorExt(DkManipulatorManager::m_exposure), this);
    mMplWidgets << new DkColorWidget(manager.manipulatorExt(DkManipulatorManager::m_color), this);
    mMplWidgets << new DkResizeWidget(manager.manipulatorExt(DkManipulatorManager::m_resize), this);

    for (QWidget *w : mMplWidgets)
        mSettingsLayout->addWidget(w);

    for (QAction *a : manager.actions())
        connect(a, &QAction::triggered, this, QOverload<>::of(&DkBatchManipulatorWidget::selectManipulator), Qt::UniqueConnection);
}

bool DkBatchManipulatorWidget::loadProperties(QSharedPointer<DkManipulatorBatch> batchManipulators)
{
    if (!batchManipulators) {
        qWarning() << "cannot load properties, DkManipulatorBatch is NULL";
        return false;
    }

    setManager(batchManipulators->manager());
    addSettingsWidgets(mManager);

    return true;
}

void DkBatchManipulatorWidget::setManager(const DkManipulatorManager &manager)
{
    mManager = manager;
    addSettingsWidgets(mManager);

    for (const QSharedPointer<DkBaseManipulator> &mpl : mManager.manipulators()) {
        auto items = mModel->findItems(mpl->name());

        for (auto i : items)
            i->setCheckState(mpl->isSelected() ? Qt::Checked : Qt::Unchecked);
    }
}

void DkBatchManipulatorWidget::transferProperties(QSharedPointer<DkManipulatorBatch> batchManipulator) const
{
    batchManipulator->setProperties(mManager);
}

bool DkBatchManipulatorWidget::hasUserInput() const
{
    return mManager.numSelected() > 0;
}

bool DkBatchManipulatorWidget::requiresUserInput() const
{
    return false;
}

void DkBatchManipulatorWidget::applyDefault()
{
    for (int rIdx = 0; rIdx < mModel->rowCount(); rIdx++) {
        mModel->item(rIdx)->setCheckState(Qt::Unchecked);
    }
}

void DkBatchManipulatorWidget::setExampleFile(const QString &filePath)
{
    mPreviewPath = filePath;
    mPreview = QImage();
}

void DkBatchManipulatorWidget::selectionChanged(const QItemSelection &selected)
{
    for (auto mIdx : selected.indexes()) {
        QStandardItem *item = mModel->item(mIdx.row());

        if (!item)
            continue;

        selectManipulator(mManager.manipulator(item->text()));
    }

    // qDebug() << "selection changed...";
}

void DkBatchManipulatorWidget::selectManipulator(QSharedPointer<DkBaseManipulator> mpl)
{
    for (auto w : mMplWidgets)
        w->hide();

    auto mplExt = qSharedPointerDynamicCast<DkBaseManipulatorExt>(mpl);

    if (!mplExt)
        return;

    if (!mplExt->widget()) {
        qCritical() << mpl->name() << "does not have a corresponding UI";
        Q_ASSERT(mplExt->widget()); // bang
        return;
    }

    mSettingsTitle->setText(mplExt->name());
    mSettingsTitle->show();
    mplExt->widget()->show();

    if (!mplExt)
        mSettingsTitle->hide();

    // load the preview
    if (!mPreviewPath.isEmpty() && mPreview.isNull()) {
        DkBasicLoader bl;
        if (bl.loadGeneral(mPreviewPath)) {
            QImage img = bl.image();

            if (img.height() > img.width())
                img = img.scaledToHeight(qMin(img.height(), mMaxPreview));
            else
                img = img.scaledToWidth(qMin(img.width(), mMaxPreview));

            mPreview = img;
        } else
            qInfo() << "could not load" << mPreviewPath << "for preview...";
    }

    // update preview
    if (!mPreview.isNull()) {
        mPreviewLabel->setPixmap(QPixmap::fromImage(mpl->apply(mPreview)));
        mPreviewLabel->show();
    } else
        mPreviewLabel->hide();
}

void DkBatchManipulatorWidget::selectManipulator()
{
    QAction *action = dynamic_cast<QAction *>(QObject::sender());
    QSharedPointer<DkBaseManipulator> mpl = mManager.manipulator(action);

    if (mpl && action->isChecked())
        selectManipulator(mpl);
}

void DkBatchManipulatorWidget::itemChanged(QStandardItem *item)
{
    auto mpl = mManager.manipulator(item->text());

    if (!mpl) {
        qCritical() << "could not cast item in DkBatchManipulatorWidget!";
        return;
    }

    mpl->setSelected(item->checkState() == Qt::Checked);
    selectManipulator(mpl);

    updateHeader();
    // qDebug() << "item: " << item->text();
}

void DkBatchManipulatorWidget::updateHeader() const
{
    int c = mManager.numSelected();
    if (!c)
        emit newHeaderText(tr("inactive"));
    else
        emit newHeaderText(tr("%1 manipulators selected").arg(c));
}

// DkBatchTransform --------------------------------------------------------------------
DkBatchTransformWidget::DkBatchTransformWidget(QWidget *parent /* = 0 */, Qt::WindowFlags f /* = 0 */)
    : DkBatchContent(parent, f)
{
    createLayout();
    applyDefault();
}

void DkBatchTransformWidget::createLayout()
{
    // resize
    QLabel *resizeLabel = new QLabel(tr("Resize"), this);
    resizeLabel->setObjectName("subTitle");

    mResizeComboMode = new QComboBox(this);
    QStringList modeItems;
    modeItems << tr("Percent") << tr("Long Side") << tr("Short Side") << tr("Width") << tr("Height") << tr("Zoom");
    mResizeComboMode->addItems(modeItems);

    mResizeSbPercent = new QDoubleSpinBox(this);
    mResizeSbPercent->setSuffix(tr("%"));
    mResizeSbPercent->setMaximum(1000);
    mResizeSbPercent->setMinimum(0.1);

    mResizeSbPx = new QSpinBox(this);
    mResizeSbPx->setSuffix(tr(" px"));
    mResizeSbPx->setMaximum(SHRT_MAX);
    mResizeSbPx->setMinimum(1);

    mResizeSbZoomLabel = new QLabel("x", this);

    mResizeSbZoomHeightPx = new QSpinBox(this);
    mResizeSbZoomHeightPx->setSuffix(tr(" px"));
    mResizeSbZoomHeightPx->setMaximum(SHRT_MAX);
    mResizeSbZoomHeightPx->setMinimum(1);

    mResizeComboProperties = new QComboBox(this);
    QStringList propertyItems;
    propertyItems << tr("Transform All") << tr("Shrink Only") << tr("Enlarge Only");
    mResizeComboProperties->addItems(propertyItems);

    QWidget *resizeWidget = new QWidget(this);
    QHBoxLayout *resizeLayout = new QHBoxLayout(resizeWidget);
    resizeLayout->setContentsMargins(0, 0, 0, 0);
    resizeLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    resizeLayout->addWidget(mResizeComboMode);
    resizeLayout->addWidget(mResizeSbPercent);
    resizeLayout->addWidget(mResizeSbPx);
    resizeLayout->addWidget(mResizeSbZoomLabel);
    resizeLayout->addWidget(mResizeSbZoomHeightPx);
    resizeLayout->addWidget(mResizeComboProperties);
    resizeLayout->addStretch();

    // rotation
    QLabel *rotateLabel = new QLabel(tr("Orientation"), this);
    rotateLabel->setObjectName("subTitle");

    mRbRotate0 = new QRadioButton(tr("Do &Not Rotate"));
    mRbRotate0->setChecked(true);
    mRbRotateLeft = new QRadioButton(tr("90%1 Counter Clockwise").arg(dk_degree_str));
    mRbRotateRight = new QRadioButton(tr("90%1 Clockwise").arg(dk_degree_str));
    mRbRotate180 = new QRadioButton(tr("180%1").arg(dk_degree_str));

    mRotateGroup = new QButtonGroup(this);

    mRotateGroup->addButton(mRbRotate0);
    mRotateGroup->addButton(mRbRotateLeft);
    mRotateGroup->addButton(mRbRotateRight);
    mRotateGroup->addButton(mRbRotate180);

    QLabel *transformLabel = new QLabel(tr("Transformations"), this);
    transformLabel->setObjectName("subTitle");

    mCbCropMetadata = new QCheckBox(tr("&Crop from Metadata"));

    // crop rectangle
    mCbCropRectangle = new QCheckBox(tr("&Crop Rectangle"));
    mCropRectWidget = new DkRectWidget(QRect(), this);
    mCbCropRectCenter = new QCheckBox(tr("&Crop to center"));

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    layout->addWidget(resizeLabel, 0, 0);
    layout->addWidget(resizeWidget, 1, 0);
    layout->addWidget(rotateLabel, 2, 0);
    layout->addWidget(mRbRotate0, 3, 0);
    layout->addWidget(mRbRotateRight, 4, 0);
    layout->addWidget(mRbRotateLeft, 5, 0);
    layout->addWidget(mRbRotate180, 6, 0);

    layout->addWidget(transformLabel, 7, 0);
    layout->addWidget(mCbCropMetadata, 8, 0);
    layout->setColumnStretch(3, 10);
    layout->addWidget(mCbCropRectangle, 9, 0);
    layout->setColumnStretch(3, 10);
    layout->addWidget(mCropRectWidget, 10, 0);
    layout->setColumnStretch(3, 10);
    layout->addWidget(mCbCropRectCenter, 11, 0);

    connect(mResizeComboMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DkBatchTransformWidget::modeChanged);
    connect(mResizeSbPercent, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DkBatchTransformWidget::updateHeader);
    connect(mResizeSbPx, QOverload<int>::of(&QSpinBox::valueChanged), this, &DkBatchTransformWidget::updateHeader);
    connect(mResizeSbZoomHeightPx, QOverload<int>::of(&QSpinBox::valueChanged), this, &DkBatchTransformWidget::updateHeader);

    connect(mRotateGroup, &QButtonGroup::idClicked, this, &DkBatchTransformWidget::updateHeader);
    connect(mCbCropMetadata, &QCheckBox::clicked, this, &DkBatchTransformWidget::updateHeader);
    connect(mCbCropRectangle, &QCheckBox::clicked, this, &DkBatchTransformWidget::modeChanged);
    connect(mCbCropRectCenter, &QCheckBox::clicked, this, &DkBatchTransformWidget::modeChanged);
}

void DkBatchTransformWidget::applyDefault()
{
    mRbRotate0->setChecked(true);
    mCbCropMetadata->setChecked(false);
    mCbCropRectangle->setChecked(false);
    mCropRectWidget->setRect(QRect());
    mCropRectWidget->setSizeOnly(false);
    mCbCropRectCenter->setChecked(false);

    mResizeSbPercent->setValue(100.0);
    mResizeSbPx->setValue(1920);
    mResizeSbZoomHeightPx->setValue(1080);
    mResizeComboMode->setCurrentIndex(0);
    mResizeComboProperties->setCurrentIndex(0);
    modeChanged(); // init gui
}

bool DkBatchTransformWidget::hasUserInput() const
{
    return !mRbRotate0->isChecked() || mCbCropMetadata->isChecked() || mCbCropRectangle->isChecked()
        || !(mResizeComboMode->currentIndex() == DkBatchTransform::resize_mode_default && mResizeSbPercent->value() == 100.0);
}

bool DkBatchTransformWidget::requiresUserInput() const
{
    return false;
}

void DkBatchTransformWidget::updateHeader() const
{
    if (!hasUserInput())
        emit newHeaderText(tr("inactive"));
    else {
        QString txt;

        if (mResizeComboMode->currentIndex() == DkBatchTransform::resize_mode_default && mResizeSbPercent->value() != 100.0) {
            txt += tr("Resize by: %1%").arg(QString::number(mResizeSbPercent->value()));
        }

        else if (mResizeComboMode->currentIndex() == DkBatchTransform::resize_mode_zoom) {
            txt += tr("Zoom to: %1 px by %2 px").arg(QString::number(mResizeSbPx->value())).arg(QString::number(mResizeSbZoomHeightPx->value()));
        }

        else if (mResizeComboMode->currentIndex() != DkBatchTransform::resize_mode_default) {
            txt += tr("Resize %1 to: %2 px").arg(mResizeComboMode->itemText(mResizeComboMode->currentIndex())).arg(QString::number(mResizeSbPx->value()));
        }

        if (getAngle() != 0) {
            if (!txt.isEmpty())
                txt += " | ";
            txt += tr("Rotating by: %1").arg(getAngle());
        }

        if (mCbCropMetadata->isChecked() || (mCbCropRectangle->isEnabled() && mCbCropRectangle->isChecked())) {
            if (!txt.isEmpty())
                txt += " | ";
            txt += tr("Crop");
        }

        emit newHeaderText(txt);
    }
}

void DkBatchTransformWidget::modeChanged()
{
    if (mResizeComboMode->currentIndex() == DkBatchTransform::resize_mode_default) {
        mResizeSbPx->hide();
        mResizeSbZoomLabel->hide();
        mResizeSbZoomHeightPx->hide();
        mResizeSbPercent->show();
        mResizeComboProperties->hide();
    } else if (mResizeComboMode->currentIndex() == DkBatchTransform::resize_mode_zoom) {
        mResizeSbPx->show();
        mResizeSbZoomLabel->show();
        mResizeSbZoomHeightPx->show();
        mResizeSbPercent->hide();
        mResizeComboProperties->hide();

        // Uncheck disabled checkboxes
        mCbCropMetadata->setChecked(false);
        mCbCropRectangle->setChecked(false);
    } else {
        mResizeSbPx->show();
        mResizeSbZoomLabel->hide();
        mResizeSbZoomHeightPx->hide();
        mResizeSbPercent->hide();
        mResizeComboProperties->show();
    }

    // Cropping not available for resize mode 'Zoom'
    mCbCropMetadata->setEnabled(mResizeComboMode->currentIndex() != DkBatchTransform::resize_mode_zoom);
    mCbCropRectangle->setEnabled(mResizeComboMode->currentIndex() != DkBatchTransform::resize_mode_zoom);

    // Crop rect and crop center only available when crop rectangle is active
    mCropRectWidget->setEnabled(mResizeComboMode->currentIndex() != DkBatchTransform::resize_mode_zoom && mCbCropRectangle->isChecked());
    mCbCropRectCenter->setEnabled(mResizeComboMode->currentIndex() != DkBatchTransform::resize_mode_zoom && mCbCropRectangle->isChecked());

    if (!mCbCropRectCenter->isEnabled())
        mCbCropRectCenter->setChecked(false);

    mCropRectWidget->setSizeOnly(mCbCropRectCenter->isChecked());

    updateHeader();
}

void DkBatchTransformWidget::transferProperties(QSharedPointer<DkBatchTransform> batchTransform) const
{
    if (mResizeComboMode->currentIndex() == DkBatchTransform::resize_mode_default) {
        batchTransform->setProperties(getAngle(),
                                      mCbCropMetadata->isChecked(),
                                      mCbCropRectangle->isChecked() ? cropRect() : QRect(),
                                      mCbCropRectCenter->isChecked(),
                                      (float)mResizeSbPercent->value() / 100.0f,
                                      (float)mResizeSbZoomHeightPx->value(),
                                      (DkBatchTransform::ResizeMode)mResizeComboMode->currentIndex());
    } else if (mResizeComboMode->currentIndex() == DkBatchTransform::resize_mode_zoom) {
        batchTransform->setProperties(getAngle(),
                                      mCbCropMetadata->isChecked(),
                                      mCbCropRectangle->isChecked() ? cropRect() : QRect(),
                                      mCbCropRectCenter->isChecked(),
                                      (float)mResizeSbPx->value(),
                                      (float)mResizeSbZoomHeightPx->value(),
                                      (DkBatchTransform::ResizeMode)mResizeComboMode->currentIndex(),
                                      (DkBatchTransform::ResizeProperty)mResizeComboProperties->currentIndex());
    } else {
        batchTransform->setProperties(getAngle(),
                                      mCbCropMetadata->isChecked(),
                                      mCbCropRectangle->isChecked() ? cropRect() : QRect(),
                                      mCbCropRectCenter->isChecked(),
                                      (float)mResizeSbPx->value(),
                                      (float)mResizeSbZoomHeightPx->value(),
                                      (DkBatchTransform::ResizeMode)mResizeComboMode->currentIndex(),
                                      (DkBatchTransform::ResizeProperty)mResizeComboProperties->currentIndex());
    }
}

bool DkBatchTransformWidget::loadProperties(QSharedPointer<DkBatchTransform> batchTransform)
{
    if (!batchTransform) {
        qWarning() << "cannot load settings, DkBatchTransform is NULL";
        return false;
    }

    bool errored = false;

    switch (batchTransform->angle()) {
    case -90:
        mRbRotateLeft->setChecked(true);
        break;
    case 90:
        mRbRotateLeft->setChecked(true);
        break;
    case 180:
        mRbRotateLeft->setChecked(true);
        break;
    case 0:
        break; // nothing todo
    default:
        errored = true;
    }

    // crop
    mCbCropMetadata->setChecked(batchTransform->cropMetatdata());
    mCbCropRectangle->setChecked(batchTransform->mode() != DkBatchTransform::resize_mode_zoom && batchTransform->cropFromRectangle());
    mCropRectWidget->setRect(batchTransform->cropRectangle());
    mCbCropRectCenter->setChecked(batchTransform->mode() != DkBatchTransform::resize_mode_zoom && batchTransform->cropMetatdata()
                                  && batchTransform->cropRectCenter());

    // resize
    mResizeComboMode->setCurrentIndex(batchTransform->mode());
    mResizeComboProperties->setCurrentIndex(batchTransform->prop());

    float sf = batchTransform->scaleFactor();
    if (batchTransform->mode() == DkBatchTransform::resize_mode_default)
        mResizeSbPercent->setValue(sf * 100.0f);
    else
        mResizeSbPx->setValue(qRound(sf));

    mResizeSbZoomHeightPx->setValue(batchTransform->zoomHeight());

    modeChanged();
    return !errored;
}

int DkBatchTransformWidget::getAngle() const
{
    if (mRbRotate0->isChecked())
        return 0;
    else if (mRbRotateLeft->isChecked())
        return -90;
    else if (mRbRotateRight->isChecked())
        return 90;
    else if (mRbRotate180->isChecked())
        return 180;

    return 0;
}

QRect DkBatchTransformWidget::cropRect() const
{
    return mCropRectWidget->rect();
}

// Batch Buttons --------------------------------------------------------------------
DkBatchButtonsWidget::DkBatchButtonsWidget(QWidget *parent)
    : DkWidget(parent)
{
    createLayout();
    setPaused();
}

void DkBatchButtonsWidget::createLayout()
{
    // play - pause button
    QSize s(32, 32);
    QIcon icon;
    QPixmap pm(DkImage::loadIcon(":/nomacs/img/play.svg", QColor(255, 255, 255), s));
    icon.addPixmap(pm, QIcon::Normal, QIcon::Off);
    pm = DkImage::loadIcon(":/nomacs/img/stop.svg", QColor(255, 255, 255), s);
    icon.addPixmap(pm, QIcon::Normal, QIcon::On);

    mPlayButton = new QPushButton(icon, "", this);
    mPlayButton->setIconSize(pm.size());
    mPlayButton->setCheckable(true);
    mPlayButton->setFlat(true);
    mPlayButton->setShortcut(Qt::ALT | Qt::Key_Return);
    mPlayButton->setToolTip(tr("Start/Cancel Batch Processing (%1)").arg(mPlayButton->shortcut().toString()));

    icon = QIcon();
    pm = QPixmap(DkImage::loadIcon(":/nomacs/img/bars.svg", QColor(255, 255, 255), s));
    icon.addPixmap(pm, QIcon::Normal, QIcon::On);
    pm = QPixmap(DkImage::loadIcon(":/nomacs/img/bars.svg", QColor(100, 100, 100), s));
    icon.addPixmap(pm, QIcon::Disabled, QIcon::On);

    mLogButton = new QPushButton(icon, "", this);
    mLogButton->setIconSize(pm.size());
    mLogButton->setFlat(true);
    mLogButton->setEnabled(false);

    // connect
    connect(mPlayButton, &QPushButton::clicked, this, &DkBatchButtonsWidget::playSignal);
    connect(mLogButton, &QPushButton::clicked, this, &DkBatchButtonsWidget::showLogSignal);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(mPlayButton);
    layout->addWidget(mLogButton);
}

void DkBatchButtonsWidget::setPaused(bool paused)
{
    mPlayButton->setChecked(!paused);
}

QPushButton *DkBatchButtonsWidget::logButton()
{
    return mLogButton;
}

QPushButton *DkBatchButtonsWidget::playButton()
{
    return mPlayButton;
}

// DkBatchInfo --------------------------------------------------------------------
DkBatchInfoWidget::DkBatchInfoWidget(QWidget *parent)
    : DkFadeWidget(parent)
{
    createLayout();
}

void DkBatchInfoWidget::createLayout()
{
    mInfo = new QLabel(this);
    mInfo->setObjectName("BatchInfo");

    mIcon = new QLabel(this);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setAlignment(Qt::AlignLeft);
    layout->addWidget(mIcon);
    layout->addWidget(mInfo);
}

void DkBatchInfoWidget::setInfo(const QString &message, const InfoMode &mode)
{
    if (message == "")
        hide();
    else
        show();

    QPixmap pm;
    switch (mode) {
    case info_warning:
        pm = QIcon(":/nomacs/img/warning.svg").pixmap(24);
        break;
    case info_critical:
        pm = QIcon(":/nomacs/img/warning.svg").pixmap(24);
        break;
    default:
        pm = QIcon(":/nomacs/img/info.svg").pixmap(24);
        break;
    }
    pm = DkImage::colorizePixmap(pm, QColor(255, 255, 255));
    mIcon->setPixmap(pm);

    mInfo->setText(message);
}

// Batch Widget --------------------------------------------------------------------
DkBatchWidget::DkBatchWidget(DkThumbLoader *thumbLoader, const QString &currentDirectory, QWidget *parent /* = 0 */)
    : DkWidget(parent)
{
    mCurrentDirectory = currentDirectory;
    mBatchProcessing = new DkBatchProcessing(DkBatchConfig(), this);

    connect(mBatchProcessing, &DkBatchProcessing::progressValueChanged, this, &DkBatchWidget::updateProgress);
    connect(mBatchProcessing, &DkBatchProcessing::finished, this, &DkBatchWidget::processingFinished);

    createLayout(thumbLoader);

    connect(inputWidget(), &DkBatchInput::updateInputDir, outputWidget(), &DkBatchOutput::setInputDir);
    connect(&mLogUpdateTimer, &QTimer::timeout, this, &DkBatchWidget::updateLog);
    connect(profileWidget(), &DkProfileWidget::saveProfileSignal, this, &DkBatchWidget::saveProfile);
    connect(profileWidget(), &DkProfileWidget::loadProfileSignal, this, &DkBatchWidget::loadProfile);
    connect(profileWidget(), &DkProfileWidget::applyDefaultSignal, this, &DkBatchWidget::applyDefault);

    inputWidget()->setDir(currentDirectory);
    outputWidget()->setInputDir(currentDirectory);

    // change tabs with page up page down
    QAction *nextAction = new QAction(tr("next"), this);
    nextAction->setShortcut(Qt::Key_PageDown);
    connect(nextAction, &QAction::triggered, this, &DkBatchWidget::nextTab);
    addAction(nextAction);

    QAction *previousAction = new QAction(tr("previous"), this);
    previousAction->setShortcut(Qt::Key_PageUp);
    connect(previousAction, &QAction::triggered, this, &DkBatchWidget::previousTab);
    addAction(previousAction);
}

DkBatchWidget::~DkBatchWidget()
{
    // close cancels the current process
    if (!cancel())
        mBatchProcessing->waitForFinished();
}

void DkBatchWidget::createLayout(DkThumbLoader *thumbLoader)
{
    // setStyleSheet("QWidget{border: 1px solid #000000;}");

    mWidgets.resize(batchWidgets_end);

    // Input Directory
    mWidgets[batch_input] = new DkBatchContainer(tr("Input"), tr("no files selected"), this);
    const auto bi = new DkBatchInput(thumbLoader, this);
    mWidgets[batch_input]->setContentWidget(bi);
    inputWidget()->setDir(mCurrentDirectory);
    connect(bi, &DkBatchInput::changed, this, &DkBatchWidget::widgetChanged);

    // fold content
    mWidgets[batch_manipulator] = new DkBatchContainer(tr("Adjustments"), tr("inactive"), this);
    mWidgets[batch_manipulator]->setContentWidget(new DkBatchManipulatorWidget(this));

    mWidgets[batch_transform] = new DkBatchContainer(tr("Transform"), tr("inactive"), this);
    mWidgets[batch_transform]->setContentWidget(new DkBatchTransformWidget(this));

#ifdef WITH_PLUGINS
    mWidgets[batch_plugin] = new DkBatchContainer(tr("Plugins"), tr("inactive"), this);
    mWidgets[batch_plugin]->setContentWidget(new DkBatchPluginWidget(this));
#endif

    mWidgets[batch_output] = new DkBatchContainer(tr("Output"), tr("not set"), this);
    const auto bo = new DkBatchOutput(this);
    mWidgets[batch_output]->setContentWidget(bo);
    connect(bo, &DkBatchOutput::changed, this, &DkBatchWidget::widgetChanged);

    // profiles
    mWidgets[batch_profile] = new DkBatchContainer(tr("Profiles"), tr("inactive"), this);
    mWidgets[batch_profile]->setContentWidget(new DkProfileWidget(this));

    mProgressBar = new DkProgressBar(this);
    mProgressBar->setVisible(false);
    mProgressBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed); // progressbar is greedy otherwise

    QWidget *centralWidget = new QWidget(this);
    mCentralLayout = new QStackedLayout(centralWidget);
    mCentralLayout->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    for (DkBatchContainer *w : mWidgets) {
        if (!w)
            continue;
        mCentralLayout->addWidget(w->contentWidget());
        connect(w, &DkBatchContainer::showSignal, this, [this]() {
            changeWidget();
        });
    }

    mContentTitle = new QLabel("", this);
    mContentTitle->setObjectName("batchContentTitle");
    mContentInfo = new QLabel("", this);
    mContentInfo->setObjectName("batchContentInfo");

    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *dialogLayout = new QVBoxLayout(contentWidget);
    dialogLayout->addWidget(mContentTitle);
    dialogLayout->addWidget(mContentInfo);
    dialogLayout->addWidget(centralWidget); // almost everything
    // dialogLayout->addStretch(10);
    // dialogLayout->addWidget(mButtons);

    // the tabs left
    QWidget *tabWidget = new QWidget(this);
    tabWidget->setObjectName("DkBatchTabs");

    QVBoxLayout *tabLayout = new QVBoxLayout(tabWidget);
    tabLayout->setAlignment(Qt::AlignTop);
    tabLayout->setContentsMargins(0, 0, 0, 0);
    tabLayout->setSpacing(0);

    // tab buttons must be checked exclusively
    QButtonGroup *tabGroup = new QButtonGroup(this);

    for (DkBatchContainer *w : mWidgets) {
        if (!w)
            continue;
        tabLayout->addWidget(w->headerWidget());
        tabGroup->addButton(w->headerWidget());
    }

    mInfoWidget = new DkBatchInfoWidget(this);

    mButtonWidget = new DkBatchButtonsWidget(this);
    mButtonWidget->show();
    tabLayout->addStretch();
    tabLayout->addWidget(mInfoWidget);
    tabLayout->addWidget(mProgressBar);
    tabLayout->addWidget(mButtonWidget);

    DkResizableScrollArea *tabScroller = new DkResizableScrollArea(this);
    tabScroller->setWidgetResizable(true);
    tabScroller->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    tabScroller->setWidget(tabWidget);
    tabScroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    DkResizableScrollArea *contentScroller = new DkResizableScrollArea(this);
    contentScroller->setWidgetResizable(true);
    contentScroller->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    contentScroller->setWidget(contentWidget);
    // contentScroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(tabScroller);
    layout->addWidget(contentScroller);

    // open the first tab
    if (!mWidgets.empty())
        mWidgets[0]->headerWidget()->click();

    connect(mButtonWidget, &DkBatchButtonsWidget::playSignal, this, &DkBatchWidget::toggleBatch);
    connect(mButtonWidget, &DkBatchButtonsWidget::showLogSignal, this, &DkBatchWidget::showLog);
    connect(this, &DkBatchWidget::infoSignal, mInfoWidget, &DkBatchInfoWidget::setInfo);
}

DkBatchInput *DkBatchWidget::inputWidget() const
{
    DkBatchInput *w = dynamic_cast<DkBatchInput *>(mWidgets[batch_input]->contentWidget());
    if (!w)
        qCritical() << "cannot cast to DkBatchInput";

    return w;
}

DkBatchOutput *DkBatchWidget::outputWidget() const
{
    DkBatchOutput *w = dynamic_cast<DkBatchOutput *>(mWidgets[batch_output]->contentWidget());
    if (!w)
        qCritical() << "cannot cast to DkBatchOutput";

    return w;
}

DkBatchManipulatorWidget *DkBatchWidget::manipulatorWidget() const
{
    DkBatchManipulatorWidget *w = dynamic_cast<DkBatchManipulatorWidget *>(mWidgets[batch_manipulator]->contentWidget());
    if (!w)
        qCritical() << "cannot cast to DkBatchManipulatorWidget";

    return w;
}

DkProfileWidget *DkBatchWidget::profileWidget() const
{
    DkProfileWidget *w = dynamic_cast<DkProfileWidget *>(mWidgets[batch_profile]->contentWidget());
    if (!w)
        qCritical() << "cannot cast to DkBatchProfileWidget";

    return w;
}

#ifdef WITH_PLUGINS
DkBatchPluginWidget *DkBatchWidget::pluginWidget() const
{
    DkBatchPluginWidget *w = dynamic_cast<DkBatchPluginWidget *>(mWidgets[batch_plugin]->contentWidget());
    if (!w)
        qCritical() << "cannot cast to DkBatchPluginWidget";

    return w;
}
#endif

DkBatchTransformWidget *DkBatchWidget::transformWidget() const
{
    DkBatchTransformWidget *w = dynamic_cast<DkBatchTransformWidget *>(mWidgets[batch_transform]->contentWidget());

    if (!w)
        qCritical() << "cannot cast to DkBatchTransformWidget";

    return w;
}

void DkBatchWidget::toggleBatch(bool start)
{
    if (start)
        startBatch();
    else
        cancel();
}

void DkBatchWidget::startBatch()
{
    const DkBatchConfig &bc = createBatchConfig();

    if (!bc.isOk()) {
        mButtonWidget->setPaused();
        qWarning() << "could not create batch config...";
        return;
    }

    mBatchProcessing->setBatchConfig(bc);

    // reopen the input widget to show the status
    if (!mWidgets.empty())
        mWidgets[0]->headerWidget()->click();

    startProcessing();
    mBatchProcessing->compute();
}

DkBatchConfig DkBatchWidget::createBatchConfig(bool strict) const
{
    // QMainWindow* mw = DkActionManager::instance().getMainWindow();

    // check if we are good to go
    if (strict && inputWidget()->getSelectedFiles().empty()) {
        emit infoSignal(tr("Please select files for processing."), DkBatchInfoWidget::InfoMode::info_warning);
        // QMessageBox::information(mw, tr("Wrong Configuration"), tr("Please select files for processing."), QMessageBox::Ok, QMessageBox::Ok);
        return DkBatchConfig();
    }

    if (!outputWidget()) {
        qDebug() << "FATAL ERROR: could not cast output widget";
        emit infoSignal(tr("I am missing a widget."), DkBatchInfoWidget::InfoMode::info_critical);
        // QMessageBox::critical(mw, tr("Fatal Error"), tr("I am missing a widget."), QMessageBox::Ok, QMessageBox::Ok);
        return DkBatchConfig();
    }

    if (strict && mWidgets[batch_output] && mWidgets[batch_input]) {
        bool outputChanged = outputWidget()->hasUserInput();
        QString inputDirPath = inputWidget()->getDir();
        QString outputDirPath = outputWidget()->getOutputDirectory();

        if (!outputChanged && inputDirPath.toLower() == outputDirPath.toLower()
            && !(outputWidget()->overwriteMode() & DkSaveInfo::mode_overwrite || outputWidget()->overwriteMode() & DkSaveInfo::mode_do_not_save_output)) {
            emit infoSignal(tr("Please check 'Overwrite Existing Files' or choose a different output directory."));
            // QMessageBox::information(mw, tr("Wrong Configuration"),
            //	tr("Please check 'Overwrite Existing Files' or choose a different output directory."),
            //	QMessageBox::Ok, QMessageBox::Ok);
            return DkBatchConfig();
        }
    }

    DkSaveInfo si;
    si.setMode(outputWidget()->overwriteMode());
    si.setDeleteOriginal(outputWidget()->deleteOriginal());
    si.setInputDirIsOutputDir(outputWidget()->useInputDir());
    si.setCompression(outputWidget()->getCompression());

    DkBatchConfig config(inputWidget()->getSelectedFilesBatch(), outputWidget()->getOutputDirectory(), outputWidget()->getFilePattern());
    config.setSaveInfo(si);

    if (!config.getOutputDirPath().isEmpty() && !QDir(config.getOutputDirPath()).exists()) {
        DkMessageBox *msgBox = new DkMessageBox(QMessageBox::Question,
                                                tr("Create Output Directory"),
                                                tr("Should I create:\n%1").arg(config.getOutputDirPath()),
                                                (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel),
                                                DkUtils::getMainWindow());

        msgBox->setDefaultButton(QMessageBox::Yes);
        msgBox->setObjectName("batchOutputDirDialog");

        int answer = msgBox->exec();

        if (answer != QMessageBox::Accepted && answer != QMessageBox::Yes) {
            return DkBatchConfig();
        }
    }

    if (strict && !config.isOk()) {
        if (config.getOutputDirPath().isEmpty()) {
            emit infoSignal(tr("Please select an output directory."), DkBatchInfoWidget::InfoMode::info_warning);
            // QMessageBox::information(mw, tr("Info"), tr("Please select an output directory."), QMessageBox::Ok, QMessageBox::Ok);
            return DkBatchConfig();
        } else if (!QDir(config.getOutputDirPath()).exists()) {
            emit infoSignal(tr("Sorry, I cannot create %1.").arg(config.getOutputDirPath()), DkBatchInfoWidget::InfoMode::info_critical);
            // QMessageBox::critical(mw, tr("Error"), tr("Sorry, I cannot create %1.").arg(config.getOutputDirPath()), QMessageBox::Ok, QMessageBox::Ok);
            return DkBatchConfig();
        } else if (config.getFileList().empty()) {
            emit infoSignal(tr("Sorry, I cannot find files to process."), DkBatchInfoWidget::InfoMode::info_critical);
            // QMessageBox::critical(mw, tr("Error"), tr("Sorry, I cannot find files to process."), QMessageBox::Ok, QMessageBox::Ok);
            return DkBatchConfig();
        } else if (config.getFileNamePattern().isEmpty()) {
            emit infoSignal(tr("Sorry, the file pattern is empty."), DkBatchInfoWidget::InfoMode::info_critical);
            // QMessageBox::critical(mw, tr("Error"), tr("Sorry, the file pattern is empty."), QMessageBox::Ok, QMessageBox::Ok);
            return DkBatchConfig();
        }
        // else if (config.getOutputDir() == QDir()) {
        //	QMessageBox::information(this, tr("Input Missing"), tr("Please choose a valid output directory\n%1").arg(config.getOutputDir().absolutePath()),
        // QMessageBox::Ok, QMessageBox::Ok); 	return;
        // }

        qDebug() << "config not ok - canceling";
        emit infoSignal(tr("Sorry, I cannot start processing - please check the configuration."), DkBatchInfoWidget::InfoMode::info_critical);
        // QMessageBox::critical(mw, tr("Fatal Error"), tr("Sorry, I cannot start processing - please check the configuration."), QMessageBox::Ok,
        // QMessageBox::Ok);
        return DkBatchConfig();
    }

    // create processing functions
    QSharedPointer<DkManipulatorBatch> manipulatorBatch(new DkManipulatorBatch);
    manipulatorWidget()->transferProperties(manipulatorBatch);

    // create processing functions
    QSharedPointer<DkBatchTransform> transformBatch(new DkBatchTransform);
    transformWidget()->transferProperties(transformBatch);

#ifdef WITH_PLUGINS
    // create processing functions
    QSharedPointer<DkPluginBatch> pluginBatch(new DkPluginBatch);
    pluginWidget()->transferProperties(pluginBatch);
#endif

    QVector<QSharedPointer<DkAbstractBatch>> processFunctions;

    if (manipulatorBatch->isActive())
        processFunctions.append(manipulatorBatch);

    if (transformBatch->isActive())
        processFunctions.append(transformBatch);

#ifdef WITH_PLUGINS
    if (pluginBatch->isActive()) {
        processFunctions.append(pluginBatch);
        pluginBatch->preLoad();
    }
#endif

    config.setProcessFunctions(processFunctions);

    return config;
}

bool DkBatchWidget::cancel()
{
    if (mBatchProcessing->isComputing()) {
        emit infoSignal(tr("Canceling..."), DkBatchInfoWidget::InfoMode::info_message);
        mBatchProcessing->cancel();
        // mButtonWidget->playButton()->setEnabled(false);
        // stopProcessing();
        return false;
    }

    return true;
}

void DkBatchWidget::processingFinished()
{
    stopProcessing();
}

void DkBatchWidget::startProcessing()
{
    inputWidget()->startProcessing();
    mInfoWidget->setInfo("");

    // mProgressBar->setFixedWidth(100);
    qDebug() << "progressbar width: " << mProgressBar->width();
    mProgressBar->show();
    mProgressBar->reset();
    mProgressBar->setMaximum(inputWidget()->getSelectedFiles().size());
    mProgressBar->setTextVisible(false);
    mButtonWidget->logButton()->setEnabled(false);
    mButtonWidget->setPaused(false);

    mLogUpdateTimer.start(1000);
}

void DkBatchWidget::stopProcessing()
{
    inputWidget()->stopProcessing();

    if (mBatchProcessing)
        mBatchProcessing->postLoad();

    mProgressBar->hide();
    mProgressBar->reset();
    mButtonWidget->logButton()->setEnabled(true);
    mButtonWidget->setPaused(true);

    int numFailures = mBatchProcessing->getNumFailures();
    int numProcessed = mBatchProcessing->getNumProcessed();
    int numItems = mBatchProcessing->getNumItems();

    DkBatchInfoWidget::InfoMode im = (numFailures > 0) ? DkBatchInfoWidget::InfoMode::info_warning : DkBatchInfoWidget::InfoMode::info_message;
    mInfoWidget->setInfo(tr("%1/%2 files processed... %3 failed.").arg(numProcessed).arg(numItems).arg(numFailures), im);

    mLogNeedsUpdate = false;
    mLogUpdateTimer.stop();

    updateLog();
}

void DkBatchWidget::updateLog()
{
    inputWidget()->setResults(mBatchProcessing->getResultList());
}

void DkBatchWidget::updateProgress(int progress)
{
    mProgressBar->setValue(progress);
    mLogNeedsUpdate = true;
}

void DkBatchWidget::showLog()
{
    QStringList log = mBatchProcessing->getLog();

    DkTextDialog *textDialog = new DkTextDialog(this);
    textDialog->setWindowTitle(tr("Batch Log"));
    textDialog->getTextEdit()->setReadOnly(true);
    textDialog->setText(log);

    textDialog->exec();
}

void DkBatchWidget::setSelectedFiles(const QStringList &selFiles)
{
    if (!selFiles.empty()) {
        inputWidget()->getInputEdit()->appendFiles(selFiles);
        inputWidget()->changeTab(DkBatchInput::tab_text_input);
    }
}

void DkBatchWidget::changeWidget(DkBatchContainer *widget)
{
    if (!widget)
        widget = dynamic_cast<DkBatchContainer *>(sender());

    if (!widget) {
        qWarning() << "changeWidget() called with NULL widget";
        return;
    }

    for (DkBatchContainer *cw : mWidgets) {
        if (cw == widget) {
            mCentralLayout->setCurrentWidget(cw->contentWidget());
            mContentTitle->setText(cw->headerWidget()->text());
            mContentInfo->setText(cw->headerWidget()->info());
            cw->headerWidget()->setChecked(true);
            connect(cw->headerWidget(), &DkBatchTabButton::infoChanged, mContentInfo, &QLabel::setText, Qt::UniqueConnection);
        }
    }
}

void DkBatchWidget::nextTab()
{
    int idx = mCentralLayout->currentIndex() + 1;
    idx %= mWidgets.size();

    changeWidget(mWidgets[idx]);
}

void DkBatchWidget::previousTab()
{
    int idx = mCentralLayout->currentIndex() - 1;
    if (idx < 0)
        idx = mWidgets.size() - 1;

    changeWidget(mWidgets[idx]);
}

void DkBatchWidget::saveProfile(const QString &profilePath) const
{
    DkBatchConfig bc = createBatchConfig(false); // false: no input/output must be profided

    // if (!bc.isOk()) {
    //	QMessageBox::critical(DkActionManager::instance().getMainWindow(), tr("Error"), tr("Sorry, I cannot save the settings, since they are incomplete..."));
    //	return;
    // }

    // allow saving without functions (i.e. image conversions)
    // if (bc.getProcessFunctions().empty()) {
    //	QMessageBox::information(DkUtils::getMainWindow(), tr("Save Profile"), tr("Cannot save empty profile."));
    //	return;
    //}

    if (!DkBatchProfile::saveProfile(profilePath, bc)) {
        QMessageBox::critical(DkUtils::getMainWindow(), tr("Error"), tr("Sorry, I cannot save the settings..."));
        return;
    } else
        qInfo() << "batch profile written to: " << profilePath;

    profileWidget()->profileSaved(DkBatchProfile::makeUserFriendly(profilePath));
}

void DkBatchWidget::loadProfile(const QString &profilePath)
{
    DkBatchConfig bc = DkBatchProfile::loadProfile(profilePath);

    // allow loading without functions (i.e. image conversions)
    // if (bc.getProcessFunctions().empty()) {
    //
    //	QMessageBox::critical(DkUtils::getMainWindow(),
    //		tr("Error Loading Profile"),
    //		tr("Sorry, I cannot load batch settings from: \n%1").arg(profilePath));
    //	return;
    //}

    applyDefault();

    if (!bc.getFileList().empty())
        setSelectedFiles(bc.getFileList());

    outputWidget()->loadProperties(bc);

    int warnings = 0;
    auto functions = bc.getProcessFunctions();
    for (QSharedPointer<DkAbstractBatch> cf : functions) {
        if (!cf) {
            qWarning() << "processing function is NULL - ignoring";
            continue;
        }
        // apply manipulator batch settings
        else if (QSharedPointer<DkManipulatorBatch> mf = qSharedPointerDynamicCast<DkManipulatorBatch>(cf)) {
            if (!manipulatorWidget()->loadProperties(mf)) {
                warnings++;
            }
        }
        // apply transform batch settings
        else if (QSharedPointer<DkBatchTransform> tf = qSharedPointerDynamicCast<DkBatchTransform>(cf)) {
            if (!transformWidget()->loadProperties(tf)) {
                warnings++;
            }
        }
#ifdef WITH_PLUGINS
        // apply plugin batch settings
        else if (QSharedPointer<DkPluginBatch> pf = qSharedPointerDynamicCast<DkPluginBatch>(cf)) {
            if (!pluginWidget()->loadProperties(pf)) {
                warnings++;
            }
            pluginWidget()->setSettingsPath(profilePath);
        }
#endif
        else {
            qWarning() << "illegal processing function: " << cf->name() << " - ignoring";
            warnings++;
        }
    }

    // TODO: feedback
    qInfo() << "settings loaded with" << warnings << "warnings";
}

void DkBatchWidget::applyDefault()
{
    for (DkBatchContainer *w : mWidgets) {
        if (!w)
            continue;
        w->batchContent()->applyDefault();
    }
}

void DkBatchWidget::widgetChanged()
{
    if (mWidgets[batch_output] && mWidgets[batch_input]) {
        QString inputDirPath = dynamic_cast<DkBatchInput *>(mWidgets[batch_input]->contentWidget())->getDir();
        QString outputDirPath = dynamic_cast<DkBatchOutput *>(mWidgets[batch_output]->contentWidget())->getOutputDirectory();

        // TODO: shouldn't we enable it always?
        // mButtonWidget->playButton()->setEnabled(inputDirPath == "" || outputDirPath == "");
    }

    if (!inputWidget()->getSelectedFiles().isEmpty()) {
        QUrl url = inputWidget()->getSelectedFiles().first();
        QString fString = url.toString();
        fString = fString.replace("file:///", "");

        QFileInfo cFileInfo = QFileInfo(fString);
        if (!cFileInfo.exists()) // try an alternative conversion
            cFileInfo = QFileInfo(url.toLocalFile());

        outputWidget()->setExampleFilename(cFileInfo.fileName());
        manipulatorWidget()->setExampleFile(cFileInfo.filePath());
        mButtonWidget->playButton()->setEnabled(true);
    }
}

DkBatchContent::DkBatchContent(QWidget *parent /* = 0 */, Qt::WindowFlags f /* = 0 */)
    : DkWidget(parent, f)
{
}
}
