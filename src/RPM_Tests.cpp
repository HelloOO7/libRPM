#include <stdio.h>
#include <cstdlib>

#include "RPM_Types.h"
#include "RPM_Module.h"
#include "Heap/exl_MemoryManager.h"

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

	FILE* relDump = fopen("follow5_rel.rpm", "wb+");

	fwrite(fileBuf, 1, mod->GetModuleSize(), relDump);

	fclose(relDump);
}

void DumpMem(exl::heap::MemoryManager* mgr, const char* path) {
	#ifdef DEBUG
	FILE* dump = fopen(path, "wb+");

	fwrite(mgr->GetHeapPtr(), 1, mgr->GetHeapSize(), dump);

	fclose(dump);
	#endif
}

void* ReadFile(const char* path, exl::heap::MemoryManager* memMgr) {
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

	exl::heap::MemoryManager* memMgr = new(malloc(sizeof(exl::heap::MemoryManager))) exl::heap::MemoryManager("RPMTests", memMgrHeap, MEMORY_MGR_HEAPSIZE);
	rpm::mgr::ModuleManager* modMgr = new(memMgr) rpm::mgr::ModuleManager(memMgr);

	void* ondemandLoaderModule = ReadFile("OnDemandLibraryManager.rpm", memMgr);

	rpm::Module* mod = modMgr->LoadModule(ondemandLoaderModule);

	if (!mod->Verify()) {
		printf("RO verification failed.\n");
	}
	else {
		printf("RO verification success.\n");

		Dump(ondemandLoaderModule, mod);
	}
	modMgr->StartModule(mod, rpm::FixLevel::INTERNAL_RELOCATIONS);

	void* dummyDllModule = ReadFile("libDummyDll.rpm", memMgr);

	rpm::Module* ddllMod = modMgr->LoadModule(dummyDllModule);
	modMgr->StartModule(ddllMod, rpm::FixLevel::INTERNAL_RELOCATIONS);

	void* odlTestModule = ReadFile("OnDemandLibraryTest.rpm", memMgr);

	rpm::Module* splMod = modMgr->LoadModule(odlTestModule);
	modMgr->StartModule(splMod, rpm::FixLevel::INTERNAL_RELOCATIONS);

	printf("Dumping heap memory...\n");

	DumpMem(memMgr, "MemoryMgr.bin");
	
	printf("Done. Freeing.\n");

	memMgr->Free(ondemandLoaderModule);
	memMgr->Free(dummyDllModule);
	memMgr->Free(odlTestModule);

	free(memMgrHeap);
	free(memMgr);
}