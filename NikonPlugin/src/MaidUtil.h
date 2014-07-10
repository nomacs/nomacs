#pragma once

#include <vector>
#include <cstdint>
#include "Maid3.h"
#include "Maid3d1.h"
#include "MaidObject.h"
#include <QDir>

#include <iostream>

//#define PATH_TYPE0007 "Type0007.md3"

namespace Maid {

/*!
 * Singleton, provides access to general maid functionality
 */
class MaidUtil {
public:
	static MaidUtil& getInstance() {
		static MaidUtil instance;

		return instance;
	}

	void loadLibrary();
	HINSTANCE loadSingleLibrary(const QString& libName) const;
	void initMAID();
	int callMAIDEntryPoint(LPNkMAIDObject, ULONG, ULONG, ULONG, NKPARAM, LPNKFUNC, NKREF);
	std::vector<std::string> packedStringEnumToVector(NkMAIDEnum* e);
	std::vector<uint32_t> unsignedEnumToVector(NkMAIDEnum* e);
	std::vector<NkMAIDString> stringEnumToVector(NkMAIDEnum* e);
	bool checkEnum(NkMAIDEnum* e);
	bool checkEnumType(NkMAIDEnum* e, ULONG arrayType, SWORD physicalBytes);

	bool isLoaded() {
		return instModule != nullptr;
	}

	template<typename T>
	std::pair<NkMAIDEnum, bool> fillEnum(Maid::MaidObject* maidObject, ULONG capId) {
		std::pair<NkMAIDEnum, bool> mE;
		NkMAIDEnum& e = mE.first;
		mE.second = false;

		if (!maidObject->hasCapOperation(capId, kNkMAIDCapOperation_Get)) {
			return mE;
		}

		maidObject->capGet(capId, kNkMAIDDataType_EnumPtr, (NKPARAM) &e);

		if (e.ulElements == 0) {
			return mE;
		}

		e.pData = new T[e.ulElements];
		maidObject->capGetArray(capId, kNkMAIDDataType_EnumPtr, (NKPARAM) &e);

		if (!checkEnum(&e)) {
			return mE;
		}

		mE.second = true;
		return mE;
	}

private:
	HINSTANCE instModule;
	LPMAIDEntryPointProc maidEntryPoint;

	MaidUtil();
	~MaidUtil();
	MaidUtil(MaidUtil const&);
	void operator=(MaidUtil const&);
};

}
