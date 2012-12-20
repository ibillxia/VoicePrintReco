// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the VS_DLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// VS_DLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef VS_DLL_EXPORTS
#define VS_DLL_API __declspec(dllexport)
#else
#define VS_DLL_API __declspec(dllimport)
#endif

// This class is exported from the vs_dll.dll
class VS_DLL_API Cvs_dll {
public:
	Cvs_dll(void);
	// TODO: add your methods here.
};

extern VS_DLL_API int nvs_dll;

VS_DLL_API int fnvs_dll(void);
