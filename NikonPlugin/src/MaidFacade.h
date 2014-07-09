#pragma once

#include <memory>
#include <array>
#include <set>
#include <vector>
#include <functional>
#include <QObject>
#include <QMutex>
#include <QString>
#include <QFileInfo>
#include <QImage>
#include <QStringList>
#include <QFutureWatcher>
#include <QBuffer>
#include <QMessageBox>

#include "MaidObject.h"

void CALLPASCAL CALLBACK eventProc(NKREF ref, ULONG ulEvent, NKPARAM data);
NKERROR CALLPASCAL CALLBACK dataProc(NKREF ref, LPVOID info, LPVOID data);
void CALLPASCAL CALLBACK completionProc(LPNkMAIDObject pObject, ULONG ulCommand, ULONG ulParam, ULONG ulDataType, NKPARAM data, NKREF refComplete, NKERROR nResult);
void CALLPASCAL CALLBACK progressProc(ULONG command, ULONG param, NKREF ref, ULONG done, ULONG total);

namespace nmc {

class DkNoMacs;

class MaidFacade : public QObject {
	Q_OBJECT

public:
	struct StringValues {
		std::vector<std::string> values;
		size_t currentValue;
	};

	struct UnsignedValues {
		std::vector<uint32_t> values;
		size_t currentValue;
	};

	struct DataProcData {
		DataProcData(MaidFacade* maidFacade) : maidFacade(maidFacade), buffer(nullptr), offset(0), totalLines(0), id(-1) {}
		char* buffer;
		size_t offset;
		size_t totalLines;
		long id;
		MaidFacade* const maidFacade;
	};

	struct CompletionProcData {
		CompletionProcData() : result(kNkMAIDResult_NotInitialized), count(nullptr), data(nullptr) {}
		NKERROR result;
		ULONG* count;
		DataProcData* data;
	};

	struct ProgressProcData {
		ProgressProcData(MaidFacade* maidFacade) : maidFacade(maidFacade) {}
		MaidFacade* const maidFacade;
	};

	typedef std::pair<MaidFacade::StringValues, bool> MaybeStringValues;
	typedef std::pair<MaidFacade::UnsignedValues, bool> MaybeUnsignedValues;

	MaidFacade();
	virtual ~MaidFacade() {}

	// some callbacks are public because they have to be called from a function outside the class
	std::function<void(unsigned long)> capValueChangeCallback;

	bool init();
	bool isInitialized();
	bool isClosed();
	void setCapValueChangeCallback(std::function<void(uint32_t)> capValueChangeCallback);
	std::set<uint32_t> listDevices();
	void openSource(ULONG id);
	bool checkCameraType();
	void closeModule();
	void closeSource();
	void closeEverything();
	bool isSourceAlive();
	MaybeStringValues readAperture();
	MaybeStringValues readSensitivity();
	MaybeStringValues readShutterSpeed();
	MaybeUnsignedValues readExposureMode();
	MaybeStringValues getAperture();
	MaybeStringValues getSensitivity();
	MaybeStringValues getShutterSpeed();
	MaybeUnsignedValues getExposureMode();
	bool setAperture(size_t newValue);
	bool setSensitivity(size_t newValue);
	bool setShutterSpeed(size_t newValue);
	bool setExposureMode(size_t newValue);
	bool isLensAttached();
	bool isAutoIso();
	bool shoot(bool withAf = false);
	bool acquireItemObjects();
	void startAcquireItemObjects();
	bool toggleLiveView();
	bool isLiveViewActive();
	QImage getLiveViewImage();
	std::pair<QStringList, size_t> toQStringList(const StringValues&);
	NKERROR processMaidData(NKREF ref, LPVOID info, LPVOID data);
	void progressCallbackUpdate(ULONG command, ULONG param, ULONG done, ULONG total);
	void enumerateCapsAll();
	QString getCurrentSavePath();
	void setAutoSaveNaming(bool);
	QFileInfo getLastFileInfo();
	QString makePictureFilename();

public slots:
	bool autoFocus();

signals:
	void acquireStart();
	void shootAndAcquireFinished();
	void updateAcquireProgress(unsigned int done, unsigned int total);

private slots:
	void shootFinished();
	void acquireItemObjectsFinished();

private:
	std::unique_ptr<Maid::MaidObject> moduleObject;
	std::unique_ptr<Maid::MaidObject> sourceObject;
	MaybeStringValues aperture;
	MaybeStringValues sensitivity;
	MaybeStringValues shutterSpeed;
	MaybeUnsignedValues exposureMode;
	QMutex mutex;
	bool lensAttached;
	QFileInfo lastFileInfo;
	QString firstFilename;
	unsigned int prevFileNumber;
	QFutureWatcher<int> shootFutureWatcher;
	QFutureWatcher<bool> acquireFutureWatcher;
	unsigned long captureCount;
	DataProcData* currentFileData;
	NkMAIDDataInfo currentFileDataInfo;
	NkMAIDFileInfo currentFileFileInfo;
	bool allItemsAcquired;
	bool currentlyAcquiringObjects;
	bool initialized;
	bool autoSaveNaming;
	bool closed;
	
	//void closeChildren(std::unique_ptr<Maid::MaidObject> mo);
	MaybeStringValues readPackedStringCap(ULONG capId);
	MaybeUnsignedValues readUnsignedEnumCap(ULONG capId);
	bool writeEnumCap(ULONG capId, size_t newValue);
	void sourceIdleLoop(ULONG* count);
	bool setMaybeStringEnumValue(std::pair<StringValues, bool>& theMaybeValue, ULONG capId, size_t newValue);
	bool setMaybeUnsignedEnumValue(std::pair<UnsignedValues, bool>& theMaybeValue, ULONG capId, size_t newValue);
	QString increaseFilenameNumber(const QFileInfo& fileInfo);
	void setCurrentFileData(DataProcData* currentFileData, void* info);
	QString getCapturedFileName(const QFileInfo& saveFile);
};

};
