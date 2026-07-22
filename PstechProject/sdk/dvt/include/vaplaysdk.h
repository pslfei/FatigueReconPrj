
#ifndef __VA_PLAY_SDK_H_
#define __VA_PLAY_SDK_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef VAPLAYSDK_EXPORTS
#define VAPAPI __declspec(dllexport)
#else
#define VAPAPI __declspec(dllimport)
#endif

#define VAPLAY_MAX_PLAYER_NUM				128
#define VAPLAY_MAX_DISPLAY_NUM				6

typedef enum _PLAY_STATUS_TYPE_E{
	PLAY_NONE,							//not open
	PLAY_STREAM,						//open in streaming mode
	PLAY_FILE,							//open as a file
}PLAY_STATUS_TYPE_E;

typedef enum _PLAY_MODE_TYPE_E{
	PMT_REALTIME,						//real-time
	PMT_FLUENCY,						//fluent
	PMT_MIDDLE,							//Consider both real-time and fluency
}PLAY_MODE_TYPE_E;

typedef enum _RENDER_MODEL_TYPE_E
{
	RENDER_MODEL_AUTO		=0,
	RENDER_MODEL_NORMAL		=1,
	RENDER_MODEL_FISHEYE	=2,
	RENDER_MODEL_VR720		=3,
	RENDER_MODEL_VR360		=4,
	RENDER_MODEL_RING		=5,
}RENDER_MODEL_TYPE;

typedef enum _PLAY_RATIO_TYPE_E
{
	PLAY_RATIO_WINDOW,					//Window scale			
	PLAY_RATIO_AUTO,					//Picture scale,Automatic Adaptation Window 
	PLAY_RATIO_HORIZONTAL,
	PLAY_RATIO_VERTICAL,
}PLAY_RATIO_TYPE;

typedef enum _DISPLAY_TYPE_E
{
	DISPLAY_PLANE				=0,			//normal		
	DISPLAY_ORIGINAL			=9,			//only for multi sensor
	DISPLAY_VR					=10,		
	DISPLAY_SPHERE				=11,		
	DISPLAY_PLANE180			=12,		
	DISPLAY_PLANE360			=13,		
	DISPLAY_PLANE360_FIXED		=14,		
	DISPLAY_PLANE360_FRONTBACK	=15,		
	DISPLAY_CYLINDER			=16,
	DISPLAY_CYLINDER_INSIDE		=17,		
}DISPLAY_TYPE;

typedef enum IMG_FORMAT_E
{
	IMG_FORMAT_AUTO=0,
	IMG_FORMAT_RGBA=1,
	IMG_FORMAT_RGB,
	IMG_FORMAT_BGRA,
	IMG_FORMAT_BGR,
}IMG_FORMAT;

typedef int (WINAPI * VADrawCallback)(unsigned short nPort,unsigned short nRegionIdx,RECT * pDisplayRect,RECT * pRegionRect,void *pContext);	//please use opengl draw
typedef int (WINAPI * VADrawCallback2)(unsigned short nPort,HDC hdc,unsigned short nRegionIdx,RECT * pDisplayRect,RECT * pRegionRect,void * pContext);	//use dc draw

//typedef int (WINAPI * VAAudioCaptureCallback)(HANDLE hCapture,unsigned char * pBuffer,int nBufLen,void *pContext);
typedef int (WINAPI * VAFileEndCallback)(unsigned short nPort,void *pContext);
typedef int (WINAPI * VAFileOpenInfoCallback)(unsigned short nPort,unsigned int nModelType,unsigned char * pSysInfo,unsigned int nSysInfoLen,void *pContext);
typedef int (WINAPI * VAAttachFrameCallback)(unsigned short nPort,unsigned short nChn,unsigned char * pFrame,unsigned int nFrameLen,void *pContext);
typedef int (WINAPI * VASnapCallback)(unsigned short nPort,unsigned short nRegionIdx,unsigned int nImgWidth, unsigned int nImgStride,unsigned int nImgHeight, int nImgFormat,const unsigned char* pImgData,void * pContext);
typedef int (WINAPI * VARenderStatusCallback)(unsigned short nPort,int bInit,void * pContext);

VAPAPI int WINAPI VAPLAY_InitSDK();
VAPAPI int WINAPI VAPLAY_ReleaseSDK();

//get a player 
VAPAPI int WINAPI VAPLAY_GetPort(unsigned short * nPort);
//free a player
VAPAPI int WINAPI VAPLAY_FreePort(unsigned short nPort);

//play function
VAPAPI int WINAPI VAPLAY_SetDisplayRegion(unsigned short nPort,unsigned short nRegionIdx, RECT *pRect,BOOL bEnable);
VAPAPI int WINAPI VAPLAY_Play(unsigned short nPort,HWND hWnd,HDC hDC,BOOL bManual);
VAPAPI int WINAPI VAPLAY_Stop(unsigned short nPort);
VAPAPI int WINAPI VAPLAY_CheckFrameNum(unsigned short nPort,BOOL bCheck);
VAPAPI int WINAPI VAPLAY_SetDrawCallback(unsigned short nPort,unsigned short nRegionIdx,VADrawCallback lpCallback,void * lpContext);		//it is for opengl draw
VAPAPI int WINAPI VAPLAY_SetDrawCallback2(unsigned short nPort,unsigned short nRegionIdx,VADrawCallback2 lpCallback,void * lpContext);		//it is for window dc draw
VAPAPI int WINAPI VAPLAY_GetPTZPosition(unsigned short nPort,unsigned short nRegionIdx,float * pfPan,float * pfTilt,float * pfZoom);
VAPAPI int WINAPI VAPLAY_SetPTZPosition(unsigned short nPort,unsigned short nRegionIdx,float fPan,float fTilt,float fZoom);
VAPAPI int WINAPI VAPLAY_GetSupportedDisplayTypeList(unsigned short nPort,int * pTypeList, int * pCount);
VAPAPI int WINAPI VAPLAY_GetCurrentDisplayType(unsigned short nPort,unsigned short nRegionIdx,int* pCurrentType);
VAPAPI int WINAPI VAPLAY_SetCurrentDisplayType(unsigned short nPort,unsigned short nRegionIdx,int nCurrentType);
VAPAPI int WINAPI VAPLAY_SetFlip(unsigned short nPort,unsigned short nRegionIdx,int nFlip);
VAPAPI int WINAPI VAPLAY_SetDisplayRatioType(unsigned short nPort,unsigned short nRegionIdx,int nRatioType);
VAPAPI int WINAPI VAPLAY_UnProject(unsigned short nPort,unsigned short nRegionIdx,float * pCoordinateX, float * pCoordinateY, float * pDirectionPan,float * pDirectionTilt,int nCount);
VAPAPI int WINAPI VAPLAY_Project(unsigned short nPort,unsigned short nRegionIdx,float * pDirectionPan,float * pDirectionTilt,float * pCoordinateX, float * pCoordinateY, int nCount);
VAPAPI int WINAPI VAPLAY_GetManualSupport(unsigned short nPort,int * pSupport);
VAPAPI int WINAPI VAPLAY_ManualInit(unsigned short nPort,int nCreateContext);
VAPAPI int WINAPI VAPLAY_ManualUnInit(unsigned short nPort);
VAPAPI int WINAPI VAPLAY_ManualRender(unsigned short nPort);
VAPAPI int WINAPI VAPLAY_ManualSwapbuffers(unsigned short nPort);
VAPAPI int WINAPI VAPLAY_SetAttachPlayer(unsigned short nPort,unsigned short nAttachPort);
VAPAPI int WINAPI VAPLAY_SetAttachCallback(unsigned short nPort,VAAttachFrameCallback lpCallback,void * lpContext);
VAPAPI int WINAPI VAPLAY_EnableBlankPTZ(unsigned short nPort,unsigned short nRegionIdx,int nEnable);
VAPAPI int WINAPI VAPLAY_SetRenderStatusCallback(unsigned short nPort,VARenderStatusCallback lpCallback,void * lpContext);
VAPAPI int WINAPI VAPLAY_SetBackgroundColor(unsigned short nPort,unsigned short nRegionIdx,unsigned int nRGBColor);

//stream mode
VAPAPI int WINAPI VAPLAY_OpenStream(unsigned short nPort,unsigned int nModelType,unsigned char * pSysInfo,unsigned int nSysLen,unsigned char * pModelData,unsigned int nDataLen);
VAPAPI int WINAPI VAPLAY_CloseStream(unsigned short nPort);
VAPAPI int WINAPI VAPLAY_InputStreamData(unsigned short nPort,unsigned char * lpFrameBuf,unsigned long nFrameSize);
VAPAPI int WINAPI VAPLAY_SetStreamOpenMode(unsigned short nPort,unsigned int nMode);

//file mode
VAPAPI int WINAPI VAPLAY_OpenFile(unsigned short nPort,const char * lpFileName);
VAPAPI int WINAPI VAPLAY_CloseFile(unsigned short nPort);
VAPAPI int WINAPI VAPLAY_Pause(unsigned short nPort,BOOL bPause);
VAPAPI int WINAPI VAPLAY_Fast(unsigned short nPort);
VAPAPI int WINAPI VAPLAY_Slow(unsigned short nPort);
VAPAPI int WINAPI VAPLAY_SetPlaySpeed(unsigned short nPort,int nSpeed);
VAPAPI int WINAPI VAPLAY_GetPlaySpeed(unsigned short nPort,int * nSpeed);
VAPAPI int WINAPI VAPLAY_OneByOne(unsigned short nPort);
VAPAPI int WINAPI VAPLAY_OneByOneBack(unsigned short nPort);
VAPAPI int WINAPI VAPLAY_SetPlayedTime(unsigned short nPort,unsigned int nTime);		//nTime is relative time
VAPAPI unsigned int WINAPI VAPLAY_GetPlayedTime(unsigned short nPort);					//return value is relative time
VAPAPI unsigned int WINAPI VAPLAY_GetCurrentVideoFrameStamp(unsigned short nPort);		
VAPAPI unsigned int WINAPI VAPLAY_GetCurrentVideoFrameClock(unsigned short nPort);		
VAPAPI int WINAPI VAPLAY_SetPlayedPos(unsigned short nPort,float fRelativePos);
VAPAPI float WINAPI VAPLAY_GetPlayedPos(unsigned short nPort);
VAPAPI unsigned int WINAPI VAPLAY_GetFileTotleTimes(unsigned short nPort);				//return value is relative time
VAPAPI unsigned int WINAPI VAPLAY_GetFileTotleFrames(unsigned short nPort);
VAPAPI int WINAPI VAPLAY_SetFileEndCallback(unsigned short nPort,VAFileEndCallback lpCallback,void * lpContext);
VAPAPI int WINAPI VAPLAY_SetFileOpenInfoCallback(unsigned short nPort,VAFileOpenInfoCallback lpCallback,void * lpContext);
VAPAPI int WINAPI VAPLAY_GetFileInfo(const char * szRecFileName,double * dtBegin,double * dtEnd);
VAPAPI int WINAPI VAPLAY_FilmEditing(unsigned short nPort,const char * szFileName,unsigned int nBeginValue,unsigned int nEndValue,int nByStamp,int nHasAudio);
VAPAPI int WINAPI VAPLAY_IsPlaying(unsigned short nPort,int * pPlaying);

//record
VAPAPI int WINAPI VAPLAY_StartLocalRecord(unsigned short nPort,const char * lpFileName);
VAPAPI int WINAPI VAPLAY_StopLocalRecord(unsigned short nPort);

//audio
VAPAPI int WINAPI VAPLAY_PlayAudio(unsigned short nPort);
VAPAPI int WINAPI VAPLAY_StopAudio();
VAPAPI int WINAPI VAPLAY_SetVolume(unsigned short nPort,long nVolume);
VAPAPI unsigned int WINAPI VAPLAY_GetVolume(unsigned short nPort);

VAPAPI int WINAPI VAPLAY_CaptureOriginalBmp(unsigned short nPort,const char * lpFileName);
VAPAPI int WINAPI VAPLAY_CaptureOriginalJpeg(unsigned short nPort,const char * lpFileName);
VAPAPI int WINAPI VAPLAY_CaptureBmp(unsigned short nPort,unsigned short nRegionIdx,const char * lpFileName);
VAPAPI int WINAPI VAPLAY_CaptureJpeg(unsigned short nPort,unsigned short nRegionIdx,const char * lpFileName);
VAPAPI int WINAPI VAPLAY_SetJpegQuality(unsigned short nQuality);

VAPAPI int WINAPI VAPLAY_SetSnapCallback(unsigned short nPort,unsigned short nRegionIdx,VASnapCallback lpCallback,void * lpContext,int nFormat,int nAlignment);
VAPAPI int WINAPI VAPLAY_SetScreenOffSnapCallback(unsigned short nPort,VASnapCallback lpCallback,void * lpContext,float fScale,int nFormat,int nAlignment);
VAPAPI int WINAPI VAPLAY_SnapDataSaveToFile(const char * szFileName,unsigned short nQuality,unsigned int nPictureType,unsigned int nImgWidth, unsigned int nImgStride,unsigned int nImgHeight, int nImgFormat,unsigned char* pImgData);
VAPAPI int WINAPI VAPLAY_SetSnapRect(unsigned short nPort,unsigned short nRegionIdx,float fDistX,float fDistY,float fDistWidth,float fDistHeight);
VAPAPI int WINAPI VAPLAY_SetScreenOffSnapRect(unsigned short nPort,float fDistX,float fDistY,float fDistWidth,float fDistHeight);

VAPAPI void WINAPI VAPLAY_SetLogAttribute(int nEnable,int nLevel,const char * pLogFileName,const char * pLogPostfix);
VAPAPI void WINAPI VAPLAY_EnableForceDX(int nEnable);

#ifdef __cplusplus
}
#endif

#endif

