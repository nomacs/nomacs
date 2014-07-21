#include <stdexcept>
#include "MaidUtil.h"
#include "MaidError.h"

using Maid::MaidUtil;

MaidUtil::MaidUtil()
	: instModule(nullptr), maidEntryPoint(nullptr) {
}

MaidUtil::~MaidUtil() {
	if (instModule != nullptr) {
		FreeLibrary(instModule);
	}
}

#include "QCoreApplication"
#include "QDebug"
#include "DkUtils.h"

/*!
 * Load the maid library
 * throws InitError
 */
void MaidUtil::loadLibrary() {

	// TODO: think how we best load md3 others than the 0007.md3
	instModule = loadSingleLibrary("Type0007.md3");
	//instModule = loadSingleLibrary("Type0003.md3");

	if (!instModule)
		throw InitError();

	if (!loadSingleLibrary("NkdPTP.dll"))
		throw InitError();

}

HINSTANCE MaidUtil::loadSingleLibrary(const QString& libName) const {

	QStringList pathList = QCoreApplication::libraryPaths();

	for (int idx = 0; idx < pathList.size(); idx++) {

		QString winPath = QDir::toNativeSeparators(pathList.at(idx) + QString("/") + libName);

		WCHAR* wDirName = new WCHAR[winPath.length()+1];

		// CMakeLists.txt:
		// if compile error that toWCharArray is not recognized:
		// in msvc: Project Properties -> C/C++ -> Language -> Treat WChar_t as built-in type: set to No (/Zc:wchar_t-)
		wDirName = (WCHAR*)winPath.utf16();
		wDirName[winPath.length()] = L'\0';	// append null character


		if (HINSTANCE instModule = LoadLibraryW(wDirName)) {
			qDebug() << libName << " loaded from: " << winPath;
			return instModule;	// ok, we could locate it - we're done
		}
		else
			qDebug() << "could not locate " << libName << " in: " << winPath;
	}

	return nullptr;
}

/*!
 * Initialize maid (after loading library)
 * throws InitError
 */
void MaidUtil::initMAID() {
	if (instModule == nullptr) {
		throw InitError();
	}

	maidEntryPoint = (LPMAIDEntryPointProc) GetProcAddress(instModule, "MAIDEntryPoint");
	if (maidEntryPoint == NULL) {
		throw InitError();
	}
}

int MaidUtil::callMAIDEntryPoint( 
		LPNkMAIDObject pObject, // module, source, item, or data object
		ULONG ulCommand, // Command, one of eNkMAIDCommand
		ULONG ulParam, // parameter for the command
		ULONG ulDataType, // Data type, one of eNkMAIDDataType
		NKPARAM data, // Pointer or long integer
		LPNKFUNC pfnComplete, // Completion function, may be NULL
		NKREF refComplete) // Value passed to pfnComplete
{

	return (*(LPMAIDEntryPointProc) maidEntryPoint)(pObject, ulCommand, ulParam, ulDataType, data, pfnComplete, refComplete);
}

/*!
 * unpack enum of type kNkMAIDArrayType_PackedString to vector
 * throws invalid_argument
 */
std::vector<std::string> MaidUtil::packedStringEnumToVector(NkMAIDEnum* e) {
	std::vector<std::string> v;

	if (!checkEnumType(e, kNkMAIDArrayType_PackedString, 1)) {
		throw std::invalid_argument("Invalid enum type for conversion");
		return v;
	}

	char* current;
	for (size_t i = 0; i < e->ulElements;) {
		current = (char*) ((ULONG) e->pData + i);
		i += strlen(current) + 1;
		v.push_back(std::string(current));
	}
	
	return v;
}

/*!
 * convert enum of type kNkMAIDArrayType_Unsigned to vector
 * throws invalid_argument
 */
std::vector<uint32_t> MaidUtil::unsignedEnumToVector(NkMAIDEnum* e) {
	std::vector<uint32_t> v;

	if (!checkEnumType(e, kNkMAIDArrayType_Unsigned, 4)) {
		throw std::invalid_argument("Invalid enum type for conversion");
		return v;
	}

	for (size_t i = 0; i < e->ulElements; ++i) {
		v.push_back(((uint32_t*) e->pData)[i]);
	}

	return v;
}

/*!
 * convert enum of type kNkMAIDArrayType_String to vector
 * throws invalid_argument
 */
std::vector<NkMAIDString> MaidUtil::stringEnumToVector(NkMAIDEnum* e) {
	std::vector<NkMAIDString> v;

	if (!checkEnumType(e, kNkMAIDArrayType_String, 256)) {
		throw std::invalid_argument("Invalid enum type for conversion");
		return v;
	}

	for (size_t i = 0; i < e->ulElements; ++i) {
		v.push_back(((NkMAIDString*) e->pData)[i]);
	}

	return v;
}

/*!
 * checks if the enum exists and has data
 */
bool MaidUtil::checkEnum(NkMAIDEnum* e) {
	if (e == nullptr || e->pData == nullptr) {
		return false;
	}
	return true;
}

/*
 * checks if the enum exists, has data and has the right type
 */
bool MaidUtil::checkEnumType(NkMAIDEnum* e, ULONG arrayType, SWORD physicalBytes) {
	if (!checkEnum(e)) {
		return false;
	}

	if (e->ulType != arrayType || e->wPhysicalBytes != physicalBytes) {
		return false;
	}

	return true;
}
