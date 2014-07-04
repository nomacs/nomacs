
#include "MaidObject.h"
#include "MaidUtil.h"
#include "MaidError.h"

using Maid::MaidObject;

MaidObject::MaidObject(NkMAIDObject* obj, ULONG id) 
	: obj(obj), parent(nullptr), id(id) {
}

MaidObject::~MaidObject() {
	// TODO delete obj?
	//if (obj != nullptr) {
	//	delete obj;
	//}
	closeObject();
}

#include "DkNoMacs.h"

/*!
 * Enumerate capabilities of the maid object
 * throws MaidError
 */
void MaidObject::enumCaps() {
	int result;

	do {
		result = MaidUtil::getInstance().callMAIDEntryPoint(obj, kNkMAIDCommand_GetCapCount, 0, kNkMAIDDataType_UnsignedPtr, (NKPARAM) &capCount, NULL, 0);

 		if (result == kNkMAIDResult_NoError) {
 			// allocate memory for the capability array
 			capArray.reset(new NkMAIDCapInfo[capCount]);
  
 			if (capArray) {
				result = MaidUtil::getInstance().callMAIDEntryPoint(obj, kNkMAIDCommand_GetCapInfo, capCount, kNkMAIDDataType_CapInfoPtr, (NKPARAM) capArray.get(), NULL, 0);

 				if (result == kNkMAIDResult_BufferSize)
 				{
					capArray.reset();
				}
			}
		}
	} while (result == kNkMAIDResult_BufferSize); // repeat if the number of capabilites changed between the two calls to the module

	if (result != kNkMAIDResult_NoError && result != kNkMAIDResult_Pending) {
		throw MaidError("Error enumerating capabilities");
	}
}

/*!
 * Allows easier creation of MAID objects
 * throws OpenCloseObjectError
 */
MaidObject* MaidObject::create(ULONG id, MaidObject* parent) {
	NkMAIDObject* actualMaidObject = new NkMAIDObject();
	MaidObject* object = new MaidObject(actualMaidObject, id);
	if (parent != nullptr) {
		parent->addChild(object);
		parent->openChildObjectAsParent(object);
	} else {
		object->openObject(NULL);
	}

	return object;
}

/*!
 * Returns the capabilites as a vector (does not re-enumerate)
 * throws MaidError
 */
std::vector<NkMAIDCapInfo> MaidObject::enumCapsVector() {
	if (!capArray) {
		enumCaps();
	}
	return std::vector<NkMAIDCapInfo>(capArray.get(), capArray.get() + capCount);
}

/*!
 * Returns the info of the capability with id capID
 * throws MaidError
 */
bool MaidObject::getCapInfo(ULONG capID, NkMAIDCapInfo* capInfo) {
	auto caps = enumCapsVector();
	for (NkMAIDCapInfo info : caps) {
		if (info.ulID == capID) {
			*capInfo = info;
			return true;
		}
	}

	return false;
}

/*!
 * throws MaidError
 */
void MaidObject::capGet(ULONG param, ULONG dataType, NKPARAM data) {
	int result;
	result = MaidUtil::getInstance().callMAIDEntryPoint(obj, kNkMAIDCommand_CapGet, param, dataType, data, NULL, NULL);
	// TODO idle loop for async call

	if (result != kNkMAIDResult_NoError) {
		throw MaidError("Error getting capability with ID " + param);
	}
}

/*!
 * throws MaidError
 */
void MaidObject::capGetArray(ULONG param, ULONG dataType, NKPARAM data) {
	int result;
	result = MaidUtil::getInstance().callMAIDEntryPoint(obj, kNkMAIDCommand_CapGetArray, param, dataType, data, NULL, NULL);
	// TODO idle loop for async call 

	if (result != kNkMAIDResult_NoError) {
		throw MaidError("Error getting array capability with ID " + param);
	}
}

/*!
 * throws MaidError
 */
void MaidObject::capSet(ULONG param, ULONG dataType, NKPARAM data) {
	int result;
	result = MaidUtil::getInstance().callMAIDEntryPoint(obj, kNkMAIDCommand_CapSet, param, dataType, data, NULL, NULL);
	// TODO idle loop for async call

	if (result != kNkMAIDResult_NoError) {
		throw MaidError("Error setting capability with ID " + param);
	}
}

/*!
 * throws MaidError
 */
bool MaidObject::hasCapOperation(ULONG capID, ULONG operations) {
	int result;
	NkMAIDCapInfo capInfo;
	bool ret = getCapInfo(capID, &capInfo);

	if (ret) {
		if(capInfo.ulOperations & operations){
			result = kNkMAIDResult_NoError;
		} else {
			result = kNkMAIDResult_NotSupported;
		}
	} else {
		result = kNkMAIDResult_NotSupported;
	}

	return (result == kNkMAIDResult_NoError);
}

/*!
 * throws MaidError
 */
int MaidObject::capStart(ULONG param, LPNKFUNC completionProc, NKREF completionData) {
	int result;
	result = MaidUtil::getInstance().callMAIDEntryPoint(obj, kNkMAIDCommand_CapStart, param, kNkMAIDDataType_Null, (NKPARAM) NULL, completionProc, completionData);

	return result;
}

/*!
 * throws MaidError
 */
void MaidObject::setProgressCallback(NKREF refProc, LPMAIDProgressProc progressProc) {
	NkMAIDCallback proc;
	proc.refProc = refProc;

	if (obj != nullptr) {
		if (hasCapOperation(kNkMAIDCapability_ProgressProc, kNkMAIDCapOperation_Set)){
			proc.pProc = (LPNKFUNC) progressProc;
			capSet(kNkMAIDCapability_ProgressProc, kNkMAIDDataType_CallbackPtr, (NKPARAM) &proc);
		}
	}
}

/*!
 * throws MaidError
 */
void MaidObject::setEventCallback(NKREF refProc, LPMAIDEventProc eventProc) {
	NkMAIDCallback proc;
	proc.refProc = refProc;

	if (obj != nullptr) {
		if (hasCapOperation(kNkMAIDCapability_EventProc, kNkMAIDCapOperation_Set)) {
			proc.pProc = (LPNKFUNC) eventProc;
			capSet(kNkMAIDCapability_EventProc, kNkMAIDDataType_CallbackPtr, (NKPARAM) &proc);
		}
	}
}

/*!
 * throws MaidError
 */
void MaidObject::setDataCallback(NKREF refProc, LPMAIDDataProc dataProc) {
	bool ret = false;
	NkMAIDCallback proc;
	proc.refProc = refProc;

	if (obj != nullptr) {
		if (obj->ulType == kNkMAIDObjectType_DataObj && 
			hasCapOperation(kNkMAIDCapability_DataProc, kNkMAIDCapOperation_Set)) {

			proc.pProc = (LPNKFUNC) dataProc;
			capSet(kNkMAIDCapability_DataProc, kNkMAIDDataType_CallbackPtr, (NKPARAM) &proc);
		}
	}
}

/*!
 * throws MaidError
 */
void MaidObject::setUIRequestCallback(NKREF refProc, LPMAIDUIRequestProc uiRequestProc) {
	bool ret = false;
	NkMAIDCallback proc;
	proc.refProc = refProc;

	if (obj != nullptr) {
		if (obj->ulType == kNkMAIDObjectType_Module && 
			hasCapOperation(kNkMAIDCapability_UIRequestProc, kNkMAIDCapOperation_Set)) {

			proc.pProc = (LPNKFUNC) uiRequestProc;
			capSet(kNkMAIDCapability_UIRequestProc, kNkMAIDDataType_CallbackPtr, (NKPARAM) &proc);
		}
	}
}

/*!
 * Open the maid module
 * throws OpenCloseObjectError
 */
void MaidObject::openObject(NkMAIDObject* parent) {
	obj->refClient = (NKREF) this; // TODO this could be wrong
	int ret;
	ret = MaidUtil::getInstance().callMAIDEntryPoint(parent, kNkMAIDCommand_Open, id, kNkMAIDDataType_ObjectPtr, (NKPARAM) obj, NULL, NULL);

	if (ret != kNkMAIDResult_NoError) {
		throw OpenCloseObjectError();
	}
}

/*!
 * Open the child object with this object as parent
 * throws OpenCloseObjectError
 */
void MaidObject::openChildObjectAsParent(MaidObject* child) {
	return child->openObject(obj);
}

/*!
 * Close the maid module
 * throws MaidError
 */
bool MaidObject::closeObject() {
	if (parent != nullptr) {
		parent->removeChild(this);
	}

	for (auto child : children) {
		child->setParent(nullptr);
		child->closeObject();
	}
	children.clear();

	int ret = MaidUtil::getInstance().callMAIDEntryPoint(obj, kNkMAIDCommand_Close, 0, 0, 0, NULL, NULL);

	return ret == kNkMAIDResult_NoError;
	//if (ret != kNkMAIDResult_NoError) {
	//	throw OpenCloseObjectError();
	//}
}

MaidObject* MaidObject::getParent() {
	return parent;
}

void MaidObject::setParent(MaidObject* newParent) {
	parent = newParent;
}

void MaidObject::addChild(MaidObject* child) {
	child->setParent(this);
	children.insert(child);
}

bool MaidObject::removeChild(MaidObject* child) {
	int ret = children.erase(child);
	if (ret > 0) {
		child->setParent(nullptr);
		return true;
	}

	return false;
}

/*!
 * throws MaidError
 */
std::vector<ULONG> MaidObject::getChildren() {
	bool ret;
	NkMAIDCapInfo capInfo;
	NkMAIDEnum childrenEnum;
	std::vector<ULONG> childIDs;

	ret = getCapInfo(kNkMAIDCapability_Children, &capInfo);
	if (!ret) {
		throw MaidError("Enumerating child objects failed");
	}

	if (capInfo.ulType != kNkMAIDCapType_Enum) {
		throw MaidError("Object has invalid capability type for kNkMAIDCapability_Children");
	}

	if (!hasCapOperation(kNkMAIDCapability_Children, kNkMAIDCapOperation_Get)) {
		throw MaidError("Enumerating child objects failed");
	}

	capGet(kNkMAIDCapability_Children, kNkMAIDDataType_EnumPtr, (NKPARAM) &childrenEnum);

	if (childrenEnum.wPhysicalBytes != 4) {
		throw MaidError("Enum of child objects has the wrong element size");
	}

	if (childrenEnum.ulElements == 0) {
		return childIDs; // there are no children
	}

	childrenEnum.pData = new uint32_t[childrenEnum.ulElements];
	std::unique_ptr<uint32_t[]> data((uint32_t*) childrenEnum.pData); // just for RAII
	try {
		capGetArray(kNkMAIDCapability_Children, kNkMAIDDataType_EnumPtr, (NKPARAM) &childrenEnum);

		for (size_t i = 0; i < childrenEnum.ulElements; ++i) {
			childIDs.push_back(((uint32_t*) childrenEnum.pData)[i]);
		}
	} catch (MaidError e) {
		throw e;
	}
	
	return childIDs;
}

/*!
 * throws MaidError
 */
bool MaidObject::async() {
	int result;
	result = MaidUtil::getInstance().callMAIDEntryPoint(obj, kNkMAIDCommand_Async, 0, kNkMAIDDataType_Null, (NKPARAM) NULL, (LPNKFUNC) NULL, (NKREF) NULL);
	return (result == kNkMAIDResult_NoError || result == kNkMAIDResult_Pending);
}

/*!
 * throws MaidError
 */
bool MaidObject::isAlive() {
	BYTE isAlive;
	try {
		capGet(kNkMAIDCapability_IsAlive, kNkMAIDDataType_BooleanPtr, (NKPARAM) &isAlive);
	} catch (MaidError) {
		return false;
	}
	return isAlive != 0;
}