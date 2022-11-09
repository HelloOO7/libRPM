#include <stdio.h>
#include <cstdlib>

#include "RPM_Types.h"
#include "RPM_Module.h"
#include "Heap/exl_HeapArea.h"

//#define TEST_DUMP_SYMBOLS

#define MEMORY_MGR_HEAPSIZE 100000 //100kb heap

void Dump(void* fileBuf, rpm::Module* mod) {
	#ifdef TEST_DUMP_SYMBOLS

	rpm::Module::SymbolSection* symbols = mod->GetSymbols();

	printf("Symbol count: %d\n", symbols->SymbolCount);

	for (int i = 0; i < symbols->SymbolCount; i++) {
		rpm::Symbol* sym = &symbols->Symbols[i];
		printf("Symbol %d: %s @ %x\n", i, mod->GetString(sym->Name), sym->Addr);
	}

	#endif

	u16 modCount = mod->GetRelExternModuleCount();
	for (int i = 0; i < modCount; i++) {
		printf("Module %d: %s\n", i, mod->GetRelExternModuleName(i));
	}

	FILE* relDump = fopen("TestResult.rpm", "wb+");

	fwrite(fileBuf, 1, mod->GetModuleSize(), relDump);

	fclose(relDump);
}

void DumpMem(exl::heap::HeapArea* mgr, const char* path) {
	#ifdef DEBUG
	FILE* dump = fopen(path, "wb+");

	fwrite(mgr->GetHeapPtr(), 1, mgr->GetHeapSize(), dump);

	fclose(dump);
	#endif
}

void* ReadFile(const char* path, exl::heap::HeapArea* memMgr) {
	FILE* file = fopen(path, "rb");

	fseek(file, 0, SEEK_END);
	long len = ftell(file);

	if (len == -1) {
		return nullptr;
	}

	void* fileBuf = memMgr->Alloc(len);
	fseek(file, 0, SEEK_SET);
	size_t readres = fread(fileBuf, 1, len, file);
	int error = ferror(file);
	int eof = feof(file);
	fclose(file);

	return fileBuf;
}

int main(void) {
	void* memMgrHeap = malloc(MEMORY_MGR_HEAPSIZE);

	exl::heap::HeapArea* memMgr = new(malloc(sizeof(exl::heap::HeapArea))) exl::heap::HeapArea("RPMTests", memMgrHeap, MEMORY_MGR_HEAPSIZE);
	rpm::mgr::ModuleManager* modMgr = new(memMgr) rpm::mgr::ModuleManager(memMgr);

	void* testModule = ReadFile("D:/_REWorkspace/CTRMapProjects/PMC/vfs/data/lib/ExtLib.Media.Cinepak.dll", memMgr);

	rpm::Module* mod = modMgr->LoadModule(testModule);

	if (!mod->Verify()) {
		printf("RO verification failed.\n");
	}
	else {
		printf("RO verification success.\n");

		Dump(testModule, mod);
	}

	void* testDependency = ReadFile("D:/_REWorkspace/CTRMapProjects/PMC/vfs/data/patches/NitroKernel.dll", memMgr);
	rpm::Module* depMod = modMgr->LoadModule(testDependency);

	printf("Starting module 1\n");
	modMgr->StartModule(depMod, rpm::FixLevel::NONE);

	printf("Starting module 2\n");
	modMgr->StartModule(mod, rpm::FixLevel::NONE);

	printf("Dumping heap memory...\n");

	DumpMem(memMgr, "MemoryMgr.bin");
	
	printf("Done. Freeing.\n");

	memMgr->Free(testModule);

	free(memMgrHeap);
	free(memMgr);
}