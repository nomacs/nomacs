#pragma once

#include <vector>
#include <memory>
#include <set>
#include "Maid3.h"
#include "Maid3d1.h"

namespace Maid {

class MaidObject {
public:
	MaidObject(NkMAIDObject* obj, ULONG id);
	~MaidObject();
	static MaidObject* create(ULONG id, MaidObject* parent);

	void enumCaps();
	std::vector<NkMAIDCapInfo> enumCapsVector();
	bool getCapInfo(ULONG capID, NkMAIDCapInfo* capInfo);
	void capGet(ULONG param, ULONG dataType, NKPARAM data);
	void capGetArray(ULONG param, ULONG dataType, NKPARAM data);
	void capSet(ULONG param, ULONG dataType, NKPARAM data);
	bool hasCapOperation(ULONG capID, ULONG operations);
	int capStart(ULONG param, LPNKFUNC completionProc = (LPNKFUNC) NULL, NKREF completionData = (NKREF) NULL);
	void setProgressCallback(NKREF refProc, LPMAIDProgressProc progressProc);
	void setEventCallback(NKREF refProc, LPMAIDEventProc eventProc);
	void setDataCallback(NKREF refProc, LPMAIDDataProc dataProc);
	void setUIRequestCallback(NKREF refProc, LPMAIDUIRequestProc uiRequestProc);
	void openObject(NkMAIDObject* parent);
	void openChildObjectAsParent(MaidObject* child);
	bool closeObject();
	void addChild(MaidObject* child);
	bool removeChild(MaidObject* child);
	MaidObject* getParent();
	void setParent(MaidObject* newParent);
	std::vector<ULONG> getChildren();
	bool async();
	bool isAlive();

	ULONG getID() {
		return id;
	}

private:
	MaidObject* parent;
	std::set<MaidObject*> children;
	NkMAIDObject* obj;
	ULONG capCount; // capabilites count
	std::unique_ptr<NkMAIDCapInfo> capArray; // capabilites array
	ULONG id;
};

}
