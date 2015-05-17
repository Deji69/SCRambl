#ifdef __cplusplus
	#ifdef SCRAMBL_EXPORTS
		#define SCRAMBLAPI extern "C" __declspec(dllexport)
		struct SCRamblInstance;
	#else
		#define SCRAMBLAPI extern "C" __declspec(dllimport)
		typedef void SCRamblInstance;
	#endif
#else
	#ifdef SCRAMBL_EXPORTS
		#error "C++ required to build library"
	#endif
	#define SCRAMBLAPI extern "C"
	typedef void SCRamblInstance;
#endif

// Result codes for SCRambl
enum SCRamblResultCode {
	SCRAMBLRC_OK,

	// errors
	SCRAMBLRC_INIT_FAILED,
	SCRAMBLRC_BUILD_FILE_NOT_FOUND,
};

struct SCRamblStatus {
	SCRamblResultCode RC;
};
struct SCRamblInst {
	SCRamblInstance* Inst;
	SCRamblStatus Status;
};

/*/ SCRambl_Init
*/
SCRAMBLAPI bool SCRambl_Init(SCRamblInst**);

/*/ SCRambl_Free
*/
SCRAMBLAPI bool SCRambl_Free(SCRamblInst**);

/*/ SCRambl_AddInputFile
*/
SCRAMBLAPI bool SCRambl_AddInputFile(SCRamblInst*, const char* path);

/*/ SCRambl_ClearInputFiles
*/
SCRAMBLAPI void SCRambl_ClearInputFiles(SCRamblInst*);

/*/ SCRambl_LoadBuildConfig - loads build configuration from specified XML file
*/
SCRAMBLAPI bool SCRambl_LoadBuildConfig(SCRamblInst*, const char* path, const char* config);

/*/
*/
SCRAMBLAPI bool SCRambl_Build(SCRamblInst*);