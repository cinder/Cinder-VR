/********************************************************************************/ /**
 \file      OVR_CAPI_Prototypes.h
 \brief     Internal CAPI prototype listing macros
 \copyright Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.
 ************************************************************************************/

#ifndef OVR_CAPI_Prototypes_h
#define OVR_CAPI_Prototypes_h

#include "OVR_CAPI.h"


//
// OVR_LIST_*_APIS - apply passed in macros to a list of API entrypoints
//
// The _ macro argument is applied for all current API versions
// The X macro argument is applied for back-compat API versions
//
// The tuple passed to either macro is (ReturnType, FunctionName, OptionalVersion, ParameterList)
//


struct ovrViewportStencilDesc_;
typedef struct ovrViewportStencilDesc_ ovrViewportStencilDesc;

// clang-format off

#define OVR_LIST_PUBLIC_APIS(_,X) \
X(ovrBool, ovr_InitializeRenderingShimVersion, , (int requestedMinorVersion)) \
_(ovrResult, ovr_Initialize, , (const ovrInitParams* params)) \
_(void, ovr_Shutdown, , (void)) \
_(const char*, ovr_GetVersionString, , (void)) \
_(void, ovr_GetLastErrorInfo, , (ovrErrorInfo* errorInfo)) \
_(ovrHmdDesc, ovr_GetHmdDesc, , (ovrSession session)) \
_(unsigned int, ovr_GetTrackerCount, , (ovrSession session)) \
_(ovrTrackerDesc, ovr_GetTrackerDesc, , (ovrSession session, unsigned int trackerDescIndex)) \
_(ovrResult, ovr_Create, , (ovrSession* pSession, ovrGraphicsLuid* pLuid)) \
_(void, ovr_Destroy, , (ovrSession session)) \
_(ovrResult, ovr_GetSessionStatus, , (ovrSession session, ovrSessionStatus* sessionStatus)) \
_(ovrResult, ovr_IsExtensionSupported, , (ovrSession session, ovrExtensions extension, ovrBool* outExtensionSupported)) \
_(ovrResult, ovr_EnableExtension, , (ovrSession session, ovrExtensions extension)) \
_(ovrResult, ovr_SetTrackingOriginType, , (ovrSession session, ovrTrackingOrigin origin)) \
_(ovrTrackingOrigin, ovr_GetTrackingOriginType, , (ovrSession session)) \
_(ovrResult, ovr_RecenterTrackingOrigin, , (ovrSession session)) \
_(ovrResult, ovr_SpecifyTrackingOrigin, , (ovrSession session, ovrPosef originPose)) \
_(void, ovr_ClearShouldRecenterFlag, , (ovrSession session)) \
_(ovrTrackingState, ovr_GetTrackingState, , (ovrSession session, double absTime, ovrBool latencyMarker)) \
_(ovrResult, ovr_GetDevicePoses, , (ovrSession session, ovrTrackedDeviceType* deviceTypes, int deviceCount, double absTime, ovrPoseStatef* outDevicePoses)) \
_(ovrTrackerPose, ovr_GetTrackerPose, , (ovrSession session, unsigned int index)) \
_(ovrResult, ovr_GetInputState, , (ovrSession session, ovrControllerType controllerType, ovrInputState*)) \
_(unsigned int, ovr_GetConnectedControllerTypes, , (ovrSession session)) \
_(ovrSizei, ovr_GetFovTextureSize, , (ovrSession session, ovrEyeType eye, ovrFovPort fov, float pixelsPerDisplayPixel)) \
X(ovrResult, ovr_GetViewportStencil, , (ovrSession session, const ovrViewportStencilDesc* viewportStencilDesc, ovrFovStencilMeshBuffer* meshBuffer)) \
_(ovrResult, ovr_GetFovStencil, , (ovrSession session, const ovrFovStencilDesc* fovStencilDesc, ovrFovStencilMeshBuffer* meshBuffer)) \
_(ovrResult, ovr_WaitToBeginFrame, , (ovrSession session, long long frameIndex)) \
_(ovrResult, ovr_BeginFrame, , (ovrSession session, long long frameIndex)) \
_(ovrResult, ovr_EndFrame, , (ovrSession session, long long frameIndex, const ovrViewScaleDesc* viewScaleDesc, ovrLayerHeader const * const * layerPtrList, unsigned int layerCount)) \
X(ovrResult, ovr_SubmitFrame, , (ovrSession session, long long frameIndex, const ovrViewScaleDescPre117* viewScaleDesc, ovrLayerHeader const * const * layerPtrList, unsigned int layerCount)) \
_(ovrResult, ovr_SubmitFrame, 2, (ovrSession session, long long frameIndex, const ovrViewScaleDesc* viewScaleDesc, ovrLayerHeader const * const * layerPtrList, unsigned int layerCount)) \
X(ovrEyeRenderDescPre117, ovr_GetRenderDesc, , (ovrSession session, ovrEyeType eyeType, ovrFovPort fov)) \
_(ovrEyeRenderDesc, ovr_GetRenderDesc, 2, (ovrSession session, ovrEyeType eyeType, ovrFovPort fov)) \
_(double, ovr_GetPredictedDisplayTime, , (ovrSession session, long long frameIndex)) \
_(double, ovr_GetTimeInSeconds, , (void)) \
_(ovrBool, ovr_GetBool, , (ovrSession session, const char* propertyName, ovrBool defaultVal)) \
_(ovrBool, ovr_SetBool, , (ovrSession session, const char* propertyName, ovrBool value)) \
_(int, ovr_GetInt, , (ovrSession session, const char* propertyName, int defaultVal)) \
_(ovrBool, ovr_SetInt, , (ovrSession session, const char* propertyName, int value)) \
_(float, ovr_GetFloat, , (ovrSession session, const char* propertyName, float defaultVal)) \
_(ovrBool, ovr_SetFloat, , (ovrSession session, const char* propertyName, float value)) \
_(unsigned int, ovr_GetFloatArray, , (ovrSession session, const char* propertyName, float values[], unsigned int arraySize)) \
_(ovrBool, ovr_SetFloatArray, , (ovrSession session, const char* propertyName, const float values[], unsigned int arraySize)) \
_(const char*, ovr_GetString, , (ovrSession session, const char* propertyName, const char* defaultVal)) \
_(ovrBool, ovr_SetString, , (ovrSession session, const char* propertyName, const char* value)) \
_(int, ovr_TraceMessage, , (int level, const char* message)) \
_(ovrResult, ovr_IdentifyClient, , (const char* identity)) \
_(ovrResult, ovr_CreateTextureSwapChainGL, , (ovrSession session, const ovrTextureSwapChainDesc* desc, ovrTextureSwapChain* outTextureChain)) \
_(ovrResult, ovr_CreateMirrorTextureGL, , (ovrSession session, const ovrMirrorTextureDesc* desc, ovrMirrorTexture* outMirrorTexture)) \
_(ovrResult, ovr_CreateMirrorTextureWithOptionsGL, , (ovrSession session, const ovrMirrorTextureDesc* desc, ovrMirrorTexture* outMirrorTexture)) \
_(ovrResult, ovr_GetTextureSwapChainBufferGL, , (ovrSession session, ovrTextureSwapChain chain, int index, unsigned int* texId)) \
_(ovrResult, ovr_GetMirrorTextureBufferGL, , (ovrSession session, ovrMirrorTexture mirror, unsigned int* texId)) \
_(ovrResult, ovr_GetTextureSwapChainLength, , (ovrSession session, ovrTextureSwapChain chain, int* length)) \
_(ovrResult, ovr_GetTextureSwapChainCurrentIndex, , (ovrSession session, ovrTextureSwapChain chain, int* currentIndex)) \
_(ovrResult, ovr_GetTextureSwapChainDesc, , (ovrSession session, ovrTextureSwapChain chain, ovrTextureSwapChainDesc* desc)) \
_(ovrResult, ovr_CommitTextureSwapChain, , (ovrSession session, ovrTextureSwapChain chain)) \
_(void, ovr_DestroyTextureSwapChain, , (ovrSession session, ovrTextureSwapChain chain)) \
_(void, ovr_DestroyMirrorTexture, , (ovrSession session, ovrMirrorTexture texture)) \
X(ovrResult, ovr_SetQueueAheadFraction, , (ovrSession session, float queueAheadFraction)) \
_(ovrResult, ovr_Lookup, , (const char* name, void** data)) \
_(ovrTouchHapticsDesc, ovr_GetTouchHapticsDesc, , (ovrSession session, ovrControllerType controllerType)) \
_(ovrResult, ovr_SetControllerVibration, , (ovrSession session, ovrControllerType controllerType, float frequency, float amplitude)) \
_(ovrResult, ovr_SubmitControllerVibration, , (ovrSession session, ovrControllerType controllerType, const ovrHapticsBuffer* buffer)) \
_(ovrResult, ovr_GetControllerVibrationState, , (ovrSession session, ovrControllerType controllerType, ovrHapticsPlaybackState* outState)) \
_(ovrResult, ovr_TestBoundary, , (ovrSession session, ovrTrackedDeviceType deviceBitmask, ovrBoundaryType singleBoundaryType, ovrBoundaryTestResult* outTestResult)) \
_(ovrResult, ovr_TestBoundaryPoint, , (ovrSession session, const ovrVector3f* point, ovrBoundaryType singleBoundaryType, ovrBoundaryTestResult* outTestResult)) \
_(ovrResult, ovr_SetBoundaryLookAndFeel, , (ovrSession session, const ovrBoundaryLookAndFeel* lookAndFeel)) \
_(ovrResult, ovr_ResetBoundaryLookAndFeel, , (ovrSession session)) \
_(ovrResult, ovr_GetBoundaryGeometry, , (ovrSession session, ovrBoundaryType singleBoundaryType, ovrVector3f* outFloorPoints, int* outFloorPointsCount)) \
_(ovrResult, ovr_GetBoundaryDimensions, , (ovrSession session, ovrBoundaryType singleBoundaryType, ovrVector3f* outDimension)) \
_(ovrResult, ovr_GetBoundaryVisible, , (ovrSession session, ovrBool* outIsVisible)) \
_(ovrResult, ovr_RequestBoundaryVisible, , (ovrSession session, ovrBool visible)) \
_(ovrResult, ovr_GetPerfStats, , (ovrSession session, ovrPerfStats* outPerfStats)) \
_(ovrResult, ovr_ResetPerfStats, , (ovrSession session))\
_(ovrResult, ovr_GetExternalCameras, , (ovrSession session, ovrExternalCamera* outCameras, unsigned int* outCameraCount))\
_(ovrResult, ovr_SetExternalCameraProperties, , (ovrSession session, const char* name, const ovrCameraIntrinsics* const intrinsics, const ovrCameraExtrinsics* const extrinsics ))

#if defined (_WIN32)
#define OVR_LIST_WIN32_APIS(_,X) \
    _(ovrResult, ovr_CreateTextureSwapChainDX, , (ovrSession session, IUnknown* d3dPtr, const ovrTextureSwapChainDesc* desc, ovrTextureSwapChain* outTextureChain)) \
    _(ovrResult, ovr_CreateMirrorTextureDX, , (ovrSession session, IUnknown* d3dPtr, const ovrMirrorTextureDesc* desc, ovrMirrorTexture* outMirrorTexture)) \
    _(ovrResult, ovr_CreateMirrorTextureWithOptionsDX, , (ovrSession session, IUnknown* d3dPtr, const ovrMirrorTextureDesc* desc, ovrMirrorTexture* outMirrorTexture)) \
    _(ovrResult, ovr_GetTextureSwapChainBufferDX, , (ovrSession session, ovrTextureSwapChain chain, int index, IID iid, void** ppObject)) \
    _(ovrResult, ovr_GetMirrorTextureBufferDX, , (ovrSession session, ovrMirrorTexture mirror, IID iid, void** ppObject)) \
    _(ovrResult, ovr_GetAudioDeviceOutWaveId, , (UINT* deviceOutId)) \
    _(ovrResult, ovr_GetAudioDeviceInWaveId, , (UINT* deviceInId)) \
    _(ovrResult, ovr_GetAudioDeviceOutGuidStr, , (WCHAR* deviceOutStrBuffer)) \
    _(ovrResult, ovr_GetAudioDeviceOutGuid, , (GUID* deviceOutGuid)) \
    _(ovrResult, ovr_GetAudioDeviceInGuidStr, , (WCHAR* deviceInStrBuffer)) \
    _(ovrResult, ovr_GetAudioDeviceInGuid, , (GUID* deviceInGuid)) \
    _(ovrResult, ovr_GetInstanceExtensionsVk, , (ovrGraphicsLuid luid, char* extensionNames, uint32_t* inoutExtensionNamesSize)) \
    _(ovrResult, ovr_GetDeviceExtensionsVk, , (ovrGraphicsLuid luid, char* extensionNames, uint32_t* inoutExtensionNamesSize)) \
    _(ovrResult, ovr_GetSessionPhysicalDeviceVk, , (ovrSession session, ovrGraphicsLuid luid, VkInstance instance, VkPhysicalDevice* out_physicalDevice)) \
    X(ovrResult, ovr_SetSynchonizationQueueVk, , (ovrSession session, VkQueue queue)) \
    _(ovrResult, ovr_SetSynchronizationQueueVk, , (ovrSession session, VkQueue queue)) \
    _(ovrResult, ovr_CreateTextureSwapChainVk, , (ovrSession session, VkDevice device, const ovrTextureSwapChainDesc* desc, ovrTextureSwapChain* out_TextureSwapChain)) \
    _(ovrResult, ovr_GetTextureSwapChainBufferVk, , (ovrSession session, ovrTextureSwapChain chain, int index, VkImage* out_Image)) \
    _(ovrResult, ovr_CreateMirrorTextureWithOptionsVk, , (ovrSession session, VkDevice device, const ovrMirrorTextureDesc* desc, ovrMirrorTexture* out_MirrorTexture)) \
    _(ovrResult, ovr_GetMirrorTextureBufferVk, , (ovrSession session, ovrMirrorTexture mirrorTexture, VkImage* out_Image))
#else
#define OVR_LIST_WIN32_APIS(_,X)
#endif

#define OVR_LIST_INTERNAL_APIS(_,X)

// We need to forward declare the ovrSensorData type here, as it won't be in a public OVR_CAPI.h header.
struct ovrSensorData_;
typedef struct ovrSensorData_ ovrSensorData;

// Hybrid Apps API forward declaration which won't be in a public OVR_CAPI.h header for now.
// --------------------------------------------------------------------------
struct ovrDesktopWindowDesc_;
typedef struct ovrDesktopWindowDesc_ ovrDesktopWindowDesc;

struct ovrKeyboardDesc_;
typedef struct ovrKeyboardDesc_ ovrKeyboardDesc;

enum ovrHybridInputFocusType_ ;
typedef enum ovrHybridInputFocusType_ ovrHybridInputFocusType;

struct ovrHybridInputFocusState_;
typedef struct ovrHybridInputFocusState_ ovrHybridInputFocusState;

typedef uint32_t ovrDesktopWindowHandle;
// --------------------------------------------------------------------------

#define OVR_LIST_PRIVATE_APIS(_,X)

// clang-format on

//
// OVR_LIST_APIS - master list of all API entrypoints
//

#define OVR_LIST_APIS(_, X)    \
  OVR_LIST_PUBLIC_APIS(_, X)   \
  OVR_LIST_WIN32_APIS(_, X)    \
  OVR_LIST_INTERNAL_APIS(_, X) \
  OVR_LIST_PRIVATE_APIS(_, X)

#endif // OVR_CAPI_Prototypes_h
