/*****************************************************************************/
//	MAID3D1.H    Rev. 2.39
//		Extra definition of maid3.h
/*****************************************************************************/
#ifndef _MAID3D1_H
#define _MAID3D1_H

#ifndef _MAID3_
	#include "maid3.h"
#endif

///////////////////////////////////////////////////////////////////////////////
#define kNkMAIDCapability_VendorBaseD1	kNkMAIDCapability_VendorBase + 0x100
#define kNkMAIDResult_D1Origin	kNkMAIDResult_VendorBase + 129
#define kNkMAIDEvent_D1Origin	kNkMAIDEvent_CapChangeValueOnly + 0x100
///////////////////////////////////////////////////////////////////////////////
enum eNkMAIDCapabilityD1
{
	kNkMAIDCapability_ModuleMode			= kNkMAIDCapability_VendorBaseD1 + 0x01,// 0x8101
	kNkMAIDCapability_CurrentDirectory		= kNkMAIDCapability_VendorBaseD1 + 0x02,
	kNkMAIDCapability_FormatStorage			= kNkMAIDCapability_VendorBaseD1 + 0x03,
	kNkMAIDCapability_PreCapture			= kNkMAIDCapability_VendorBaseD1 + 0x04,
	kNkMAIDCapability_LockFocus				= kNkMAIDCapability_VendorBaseD1 + 0x05,
	kNkMAIDCapability_LockExposure			= kNkMAIDCapability_VendorBaseD1 + 0x06,
	kNkMAIDCapability_LifeTime				= kNkMAIDCapability_VendorBaseD1 + 0x07,
	kNkMAIDCapability_CFStatus				= kNkMAIDCapability_VendorBaseD1 + 0x08,
	kNkMAIDCapability_ClockBatteryLevel		= kNkMAIDCapability_VendorBaseD1 + 0x09,
	kNkMAIDCapability_FlashStatus			= kNkMAIDCapability_VendorBaseD1 + 0x0a,
	kNkMAIDCapability_ExposureStatus		= kNkMAIDCapability_VendorBaseD1 + 0x0b,
	kNkMAIDCapability_MediaType				= kNkMAIDCapability_VendorBaseD1 + 0x0c,

	kNkMAIDCapability_FileType				= kNkMAIDCapability_VendorBaseD1 + 0x0f,
	kNkMAIDCapability_CompressionLevel		= kNkMAIDCapability_VendorBaseD1 + 0x10,
	kNkMAIDCapability_ExposureMode			= kNkMAIDCapability_VendorBaseD1 + 0x11,
	kNkMAIDCapability_ShutterSpeed			= kNkMAIDCapability_VendorBaseD1 + 0x12,
	kNkMAIDCapability_Aperture				= kNkMAIDCapability_VendorBaseD1 + 0x13,
	kNkMAIDCapability_FlexibleProgram		= kNkMAIDCapability_VendorBaseD1 + 0x14,
	kNkMAIDCapability_ExposureComp			= kNkMAIDCapability_VendorBaseD1 + 0x15,
	kNkMAIDCapability_MeteringMode			= kNkMAIDCapability_VendorBaseD1 + 0x16,
	kNkMAIDCapability_Sensitivity			= kNkMAIDCapability_VendorBaseD1 + 0x17,
	kNkMAIDCapability_WBMode				= kNkMAIDCapability_VendorBaseD1 + 0x18,
	kNkMAIDCapability_WBTuneAuto			= kNkMAIDCapability_VendorBaseD1 + 0x19,
	kNkMAIDCapability_WBTuneIncandescent	= kNkMAIDCapability_VendorBaseD1 + 0x1a,
	kNkMAIDCapability_WBTuneFluorescent		= kNkMAIDCapability_VendorBaseD1 + 0x1b,
	kNkMAIDCapability_WBTuneSunny			= kNkMAIDCapability_VendorBaseD1 + 0x1c,
	kNkMAIDCapability_WBTuneFlash			= kNkMAIDCapability_VendorBaseD1 + 0x1d,
	kNkMAIDCapability_WBTuneShade			= kNkMAIDCapability_VendorBaseD1 + 0x1e,
	kNkMAIDCapability_WBTuneCloudy			= kNkMAIDCapability_VendorBaseD1 + 0x1f,
	kNkMAIDCapability_FocusMode				= kNkMAIDCapability_VendorBaseD1 + 0x20,
	kNkMAIDCapability_FocusAreaMode			= kNkMAIDCapability_VendorBaseD1 + 0x21,
	kNkMAIDCapability_FocusPreferredArea	= kNkMAIDCapability_VendorBaseD1 + 0x22,
	kNkMAIDCapability_FocalLength			= kNkMAIDCapability_VendorBaseD1 + 0x23,
	kNkMAIDCapability_ClockDateTime			= kNkMAIDCapability_VendorBaseD1 + 0x24,
	kNkMAIDCapability_CustomSettings		= kNkMAIDCapability_VendorBaseD1 + 0x25,
	kNkMAIDCapability_BracketingOrder		= kNkMAIDCapability_VendorBaseD1 + 0x26,
	kNkMAIDCapability_BracketingVary		= kNkMAIDCapability_VendorBaseD1 + 0x27,
	kNkMAIDCapability_AFonRelease			= kNkMAIDCapability_VendorBaseD1 + 0x28,
	kNkMAIDCapability_AFAreaSelector		= kNkMAIDCapability_VendorBaseD1 + 0x29,
	kNkMAIDCapability_AFsPriority			= kNkMAIDCapability_VendorBaseD1 + 0x2a,
	kNkMAIDCapability_AFcPriority			= kNkMAIDCapability_VendorBaseD1 + 0x2b,
	kNkMAIDCapability_EVInterval			= kNkMAIDCapability_VendorBaseD1 + 0x00,
	kNkMAIDCapability_CWMeteringDiameter	= kNkMAIDCapability_VendorBaseD1 + 0x2d,
	kNkMAIDCapability_AELockonRelease		= kNkMAIDCapability_VendorBaseD1 + 0x2e,
	kNkMAIDCapability_ExchangeDials			= kNkMAIDCapability_VendorBaseD1 + 0x2f,
	kNkMAIDCapability_EasyExposureComp		= kNkMAIDCapability_VendorBaseD1 + 0x30,
	kNkMAIDCapability_Microscope			= kNkMAIDCapability_VendorBaseD1 + 0x31,
	kNkMAIDCapability_AutoOffDelay			= kNkMAIDCapability_VendorBaseD1 + 0x32,
	kNkMAIDCapability_SelfTimerDuration		= kNkMAIDCapability_VendorBaseD1 + 0x33,
	kNkMAIDCapability_LCDBackLight			= kNkMAIDCapability_VendorBaseD1 + 0x34,
	kNkMAIDCapability_PlayBackImage			= kNkMAIDCapability_VendorBaseD1 + 0x35,
	kNkMAIDCapability_LimitImageDisplay		= kNkMAIDCapability_VendorBaseD1 + 0x36,
	kNkMAIDCapability_BlinkTimerLED			= kNkMAIDCapability_VendorBaseD1 + 0x37,
	kNkMAIDCapability_ApertureDial			= kNkMAIDCapability_VendorBaseD1 + 0x38,
	kNkMAIDCapability_ZoomAperture			= kNkMAIDCapability_VendorBaseD1 + 0x39,
	kNkMAIDCapability_AEAFLockButton		= kNkMAIDCapability_VendorBaseD1 + 0x3a,
	kNkMAIDCapability_EdgeEnhancement		= kNkMAIDCapability_VendorBaseD1 + 0x3b,
	kNkMAIDCapability_Curve					= kNkMAIDCapability_VendorBaseD1 + 0x3c,
	kNkMAIDCapability_ShootingSpeed			= kNkMAIDCapability_VendorBaseD1 + 0x3d,
	kNkMAIDCapability_ShootingLimit			= kNkMAIDCapability_VendorBaseD1 + 0x3e,
	kNkMAIDCapability_LutHeader				= kNkMAIDCapability_VendorBaseD1 + 0x3f,
	kNkMAIDCapability_FileHeader			= kNkMAIDCapability_VendorBaseD1 + 0x40,
	kNkMAIDCapability_LockCamera			= kNkMAIDCapability_VendorBaseD1 + 0x41,
	kNkMAIDCapability_LockShutterSpeed		= kNkMAIDCapability_VendorBaseD1 + 0x42,
	kNkMAIDCapability_LockAperture			= kNkMAIDCapability_VendorBaseD1 + 0x43,
	kNkMAIDCapability_LensInfo				= kNkMAIDCapability_VendorBaseD1 + 0x44,
	kNkMAIDCapability_MirrorUp				= kNkMAIDCapability_VendorBaseD1 + 0x45,
	kNkMAIDCapability_EnableNIF				= kNkMAIDCapability_VendorBaseD1 + 0x46,
	kNkMAIDCapability_PlaybackMode			= kNkMAIDCapability_VendorBaseD1 + 0x47,
	kNkMAIDCapability_UserComment			= kNkMAIDCapability_VendorBaseD1 + 0x48,
	kNkMAIDCapability_DeleteDirectory		= kNkMAIDCapability_VendorBaseD1 + 0x2c,
	kNkMAIDCapability_NumberingMode			= kNkMAIDCapability_VendorBaseD1 + 0x49,
	kNkMAIDCapability_ReadOnly				= kNkMAIDCapability_VendorBaseD1 + 0x4a,
	kNkMAIDCapability_Invisible				= kNkMAIDCapability_VendorBaseD1 + 0x4b,
	kNkMAIDCapability_DirCreatedByD1		= kNkMAIDCapability_VendorBaseD1 + 0x4c,
	kNkMAIDCapability_ContinuousInPCMode	= kNkMAIDCapability_VendorBaseD1 + 0x4d,
	kNkMAIDCapability_CurrentDirID			= kNkMAIDCapability_VendorBaseD1 + 0x4e,
	kNkMAIDCapability_SensitivityIncrease	= kNkMAIDCapability_VendorBaseD1 + 0x4f,
	kNkMAIDCapability_WritingMedia			= kNkMAIDCapability_VendorBaseD1 + 0x50,
	kNkMAIDCapability_WBPresetNumber		= kNkMAIDCapability_VendorBaseD1 + 0x51,
	kNkMAIDCapability_ThumbnailSize			= kNkMAIDCapability_VendorBaseD1 + 0x52,
	kNkMAIDCapability_SensitivityInterval	= kNkMAIDCapability_VendorBaseD1 + 0x53,
	kNkMAIDCapability_ShootNoCard 			= kNkMAIDCapability_VendorBaseD1 + 0x54,

	kNkMAIDCapability_ColorReproduct		= kNkMAIDCapability_VendorBaseD1 + 0x56,
	kNkMAIDCapability_ImageSize				= kNkMAIDCapability_VendorBaseD1 + 0x57,
	kNkMAIDCapability_CompressRAW			= kNkMAIDCapability_VendorBaseD1 + 0x58,
	kNkMAIDCapability_EnableMonitor			= kNkMAIDCapability_VendorBaseD1 + 0x59,
	kNkMAIDCapability_WBGainRed				= kNkMAIDCapability_VendorBaseD1 + 0x5a,
	kNkMAIDCapability_WBGainBlue			= kNkMAIDCapability_VendorBaseD1 + 0x5b,
	kNkMAIDCapability_MakeDirectory			= kNkMAIDCapability_VendorBaseD1 + 0x5c,
	kNkMAIDCapability_RearPanelDisplayMode	= kNkMAIDCapability_VendorBaseD1 + 0x5d,
	kNkMAIDCapability_ColorAdjustment		= kNkMAIDCapability_VendorBaseD1 + 0x5e,
	kNkMAIDCapability_SelectFUNC			= kNkMAIDCapability_VendorBaseD1 + 0x5f,
	kNkMAIDCapability_TypicalFlashMode		= kNkMAIDCapability_VendorBaseD1 + 0x60,
	kNkMAIDCapability_Converter				= kNkMAIDCapability_VendorBaseD1 + 0x61,
	kNkMAIDCapability_ElectronicZoom		= kNkMAIDCapability_VendorBaseD1 + 0x62,
	kNkMAIDCapability_DateFormat			= kNkMAIDCapability_VendorBaseD1 + 0x63,
	kNkMAIDCapability_PreviewInterval		= kNkMAIDCapability_VendorBaseD1 + 0x64,
	kNkMAIDCapability_MenuBank				= kNkMAIDCapability_VendorBaseD1 + 0x65,
	kNkMAIDCapability_FlashComp				= kNkMAIDCapability_VendorBaseD1 + 0x66,
	kNkMAIDCapability_NoAperture			= kNkMAIDCapability_VendorBaseD1 + 0x67,
	kNkMAIDCapability_AntiVibration			= kNkMAIDCapability_VendorBaseD1 + 0x68,
	kNkMAIDCapability_BatteryPack			= kNkMAIDCapability_VendorBaseD1 + 0x69,
	kNkMAIDCapability_ResetCustomSetting	= kNkMAIDCapability_VendorBaseD1 + 0x6a,
	kNkMAIDCapability_ImagePreview			= kNkMAIDCapability_VendorBaseD1 + 0x6b,
	kNkMAIDCapability_IsoControl			= kNkMAIDCapability_VendorBaseD1 + 0x6c,
	kNkMAIDCapability_NoiseReduction		= kNkMAIDCapability_VendorBaseD1 + 0x6d,
	kNkMAIDCapability_FocusAreaLed			= kNkMAIDCapability_VendorBaseD1 + 0x6e,
	kNkMAIDCapability_AfSubLight			= kNkMAIDCapability_VendorBaseD1 + 0x6f,
	kNkMAIDCapability_AfButton				= kNkMAIDCapability_VendorBaseD1 + 0x70,
	kNkMAIDCapability_SoundLevel			= kNkMAIDCapability_VendorBaseD1 + 0x71,
	kNkMAIDCapability_FinderMode			= kNkMAIDCapability_VendorBaseD1 + 0x72,
	kNkMAIDCapability_AeBracketNum			= kNkMAIDCapability_VendorBaseD1 + 0x73,
	kNkMAIDCapability_WbBracketNum			= kNkMAIDCapability_VendorBaseD1 + 0x74,
	kNkMAIDCapability_InternalSplMode		= kNkMAIDCapability_VendorBaseD1 + 0x75,
	kNkMAIDCapability_EnableComment			= kNkMAIDCapability_VendorBaseD1 + 0x76,
	kNkMAIDCapability_PresetExpMode			= kNkMAIDCapability_VendorBaseD1 + 0x77,
	kNkMAIDCapability_PossibleToShoot		= kNkMAIDCapability_VendorBaseD1 + 0x78,
	kNkMAIDCapability_ResetFileNumber		= kNkMAIDCapability_VendorBaseD1 + 0x79,

	kNkMAIDCapability_ExpCompInterval			= kNkMAIDCapability_VendorBaseD1 + 0x80,
	kNkMAIDCapability_FocusGroupPreferredArea	= kNkMAIDCapability_VendorBaseD1 + 0x8a,
	kNkMAIDCapability_ResetMenuBank				= kNkMAIDCapability_VendorBaseD1 + 0x8b,
	kNkMAIDCapability_WBTuneColorTemp			= kNkMAIDCapability_VendorBaseD1 + 0x8c,
	kNkMAIDCapability_ShootingMode				= kNkMAIDCapability_VendorBaseD1 + 0x8d,
	kNkMAIDCapability_LockFV					= kNkMAIDCapability_VendorBaseD1 + 0x8e,
	kNkMAIDCapability_RemainContinuousShooting	= kNkMAIDCapability_VendorBaseD1 + 0x8f,
	kNkMAIDCapability_ShootingBankName			= kNkMAIDCapability_VendorBaseD1 + 0x90,
	kNkMAIDCapability_WBPresetName				= kNkMAIDCapability_VendorBaseD1 + 0x91,
	kNkMAIDCapability_FmmManual					= kNkMAIDCapability_VendorBaseD1 + 0x92,
	kNkMAIDCapability_F0Manual					= kNkMAIDCapability_VendorBaseD1 + 0x93,
	kNkMAIDCapability_CustomBankName			= kNkMAIDCapability_VendorBaseD1 + 0x94,
	kNkMAIDCapability_AfGroupAreaPattern		= kNkMAIDCapability_VendorBaseD1 + 0x95,
	kNkMAIDCapability_FocusFrameInMf			= kNkMAIDCapability_VendorBaseD1 + 0x96,
	kNkMAIDCapability_FocusFrameInContinuousShooting= kNkMAIDCapability_VendorBaseD1 + 0x97,
	kNkMAIDCapability_FocusFrameDisplayTime		= kNkMAIDCapability_VendorBaseD1 + 0x98,
	kNkMAIDCapability_ExposureDelay				= kNkMAIDCapability_VendorBaseD1 + 0x99,
	kNkMAIDCapability_AddIccProfile				= kNkMAIDCapability_VendorBaseD1 + 0x9a,
	kNkMAIDCapability_ShootCounterInFinder		= kNkMAIDCapability_VendorBaseD1 + 0x9b,
	kNkMAIDCapability_CenterButtonOnShooting	= kNkMAIDCapability_VendorBaseD1 + 0x9c,
	kNkMAIDCapability_CenterButtonOnPlayback	= kNkMAIDCapability_VendorBaseD1 + 0x9d,
	kNkMAIDCapability_MultiSelector				= kNkMAIDCapability_VendorBaseD1 + 0x9e,
	kNkMAIDCapability_MultiSelectorDirection	= kNkMAIDCapability_VendorBaseD1 + 0x9f,
	kNkMAIDCapability_CommandDialDirection			= kNkMAIDCapability_VendorBaseD1 + 0xa0,
	kNkMAIDCapability_EnableCommandDialOnPlayback	= kNkMAIDCapability_VendorBaseD1 + 0xa1,
	kNkMAIDCapability_UniversalMode				= kNkMAIDCapability_VendorBaseD1 + 0xa2,
	kNkMAIDCapability_VerticalAfButton			= kNkMAIDCapability_VendorBaseD1 + 0xa4,
	kNkMAIDCapability_FlashSyncTime				= kNkMAIDCapability_VendorBaseD1 + 0xa5,
	kNkMAIDCapability_FlashSlowLimit			= kNkMAIDCapability_VendorBaseD1 + 0xa6,
	kNkMAIDCapability_ExternalFlashMode			= kNkMAIDCapability_VendorBaseD1 + 0xa7,
	kNkMAIDCapability_ModelingOnPreviewButton	= kNkMAIDCapability_VendorBaseD1 + 0xa8,
	kNkMAIDCapability_BracketingFactor			= kNkMAIDCapability_VendorBaseD1 + 0xa9,
	kNkMAIDCapability_BracketingMethod			= kNkMAIDCapability_VendorBaseD1 + 0xaa,
	kNkMAIDCapability_RGBGain					= kNkMAIDCapability_VendorBaseD1 + 0xab,
	kNkMAIDCapability_USBSpeed					= kNkMAIDCapability_VendorBaseD1 + 0xac,
	kNkMAIDCapability_WBPresetData				= kNkMAIDCapability_VendorBaseD1 + 0xad,
	kNkMAIDCapability_ContinuousShootingNum		= kNkMAIDCapability_VendorBaseD1 + 0xae,
	kNkMAIDCapability_EnableBracketing			= kNkMAIDCapability_VendorBaseD1 + 0xaf,
	kNkMAIDCapability_BracketingType			= kNkMAIDCapability_VendorBaseD1 + 0xb0,
	kNkMAIDCapability_AEBracketingStep			= kNkMAIDCapability_VendorBaseD1 + 0xb1,
	kNkMAIDCapability_WBBracketingStep			= kNkMAIDCapability_VendorBaseD1 + 0xb2,
	kNkMAIDCapability_BracketingCount			= kNkMAIDCapability_VendorBaseD1 + 0xb3,
	kNkMAIDCapability_CameraInclination			= kNkMAIDCapability_VendorBaseD1 + 0xb4,
	kNkMAIDCapability_RawJpegImageStatus		= kNkMAIDCapability_VendorBaseD1 + 0xb5,
	kNkMAIDCapability_CaptureDustImage			= kNkMAIDCapability_VendorBaseD1 + 0xb6,
	kNkMAIDCapability_ZoomRateOnPlayback		= kNkMAIDCapability_VendorBaseD1 + 0xb7,
	kNkMAIDCapability_AfGroupAreaPatternType	= kNkMAIDCapability_VendorBaseD1 + 0xb8,
	kNkMAIDCapability_ExternalFlashSort			= kNkMAIDCapability_VendorBaseD1 + 0xb9,
	kNkMAIDCapability_ExternalOldTypeFlashMode	= kNkMAIDCapability_VendorBaseD1 + 0xba,
	kNkMAIDCapability_ExternalNewTypeFlashMode	= kNkMAIDCapability_VendorBaseD1 + 0xbb,
	kNkMAIDCapability_InternalFlashStatus		= kNkMAIDCapability_VendorBaseD1 + 0xbc,
	kNkMAIDCapability_ExternalFlashStatus		= kNkMAIDCapability_VendorBaseD1 + 0xbd,
	kNkMAIDCapability_InternalFlashComp			= kNkMAIDCapability_VendorBaseD1 + 0xbe,
	kNkMAIDCapability_ExternalFlashComp			= kNkMAIDCapability_VendorBaseD1 + 0xbf,
	kNkMAIDCapability_ImageSetting				= kNkMAIDCapability_VendorBaseD1 + 0xc0,
	kNkMAIDCapability_SaturationSetting			= kNkMAIDCapability_VendorBaseD1 + 0xc1,
	kNkMAIDCapability_Beep						= kNkMAIDCapability_VendorBaseD1 + 0xc2,
	kNkMAIDCapability_AFMode					= kNkMAIDCapability_VendorBaseD1 + 0xc3,
	kNkMAIDCapability_ISOAutoShutterTime		= kNkMAIDCapability_VendorBaseD1 + 0xc4,
	kNkMAIDCapability_InternalSplValue			= kNkMAIDCapability_VendorBaseD1 + 0xc5,
	kNkMAIDCapability_RecommendFlashDisp		= kNkMAIDCapability_VendorBaseD1 + 0xc7,
	kNkMAIDCapability_RemoteTimer				= kNkMAIDCapability_VendorBaseD1 + 0xc8,
	kNkMAIDCapability_CameraInclinationMode		= kNkMAIDCapability_VendorBaseD1 + 0xc9,
	kNkMAIDCapability_CCDDataMode				= kNkMAIDCapability_VendorBaseD1 + 0xd0,
	kNkMAIDCapability_JpegCompressionPolicy		= kNkMAIDCapability_VendorBaseD1 + 0xd1,
	kNkMAIDCapability_AFLockOn					= kNkMAIDCapability_VendorBaseD1 + 0xd2,
	kNkMAIDCapability_FocalLengthControl		= kNkMAIDCapability_VendorBaseD1 + 0xd3,
	kNkMAIDCapability_ExpBaseMatrix				= kNkMAIDCapability_VendorBaseD1 + 0xd4,
	kNkMAIDCapability_ExpBaseCenter				= kNkMAIDCapability_VendorBaseD1 + 0xd5,
	kNkMAIDCapability_ExpBaseSpot				= kNkMAIDCapability_VendorBaseD1 + 0xd6,
	kNkMAIDCapability_CameraType				= kNkMAIDCapability_VendorBaseD1 + 0xd7,
	kNkMAIDCapability_NoiseReductionHighISO		= kNkMAIDCapability_VendorBaseD1 + 0xd8,
	kNkMAIDCapability_EasyExposureCompMode		= kNkMAIDCapability_VendorBaseD1 + 0xd9,
	kNkMAIDCapability_ImageColorSpace			= kNkMAIDCapability_VendorBaseD1 + 0xdb,
	kNkMAIDCapability_FocusAreaFrame			= kNkMAIDCapability_VendorBaseD1 + 0xf1,
	kNkMAIDCapability_ISOAutoHiLimit			= kNkMAIDCapability_VendorBaseD1 + 0xf2,
	kNkMAIDCapability_BeepEx					= kNkMAIDCapability_VendorBaseD1 + 0xf3,
	kNkMAIDCapability_AFLockOnEx				= kNkMAIDCapability_VendorBaseD1 + 0xf4,
	kNkMAIDCapability_WarningDisp				= kNkMAIDCapability_VendorBaseD1 + 0xf5,
	kNkMAIDCapability_CellKind					= kNkMAIDCapability_VendorBaseD1 + 0xf6,
	kNkMAIDCapability_InternalSplMRPTValue		= kNkMAIDCapability_VendorBaseD1 + 0xf7,
	kNkMAIDCapability_InternalSplMRPTCount		= kNkMAIDCapability_VendorBaseD1 + 0xf8,
	kNkMAIDCapability_InternalSplMRPTInterval	= kNkMAIDCapability_VendorBaseD1 + 0xf9,
	kNkMAIDCapability_InternalSplCommandChannel	= kNkMAIDCapability_VendorBaseD1 + 0xfa,
	kNkMAIDCapability_InternalSplCmdSelfComp	= kNkMAIDCapability_VendorBaseD1 + 0xfb,
	kNkMAIDCapability_InternalSplCmdGroupAMode	= kNkMAIDCapability_VendorBaseD1 + 0xfc,
	kNkMAIDCapability_InternalSplCmdGroupAComp	= kNkMAIDCapability_VendorBaseD1 + 0xfd,
	kNkMAIDCapability_InternalSplCmdGroupAValue	= kNkMAIDCapability_VendorBaseD1 + 0xfe,
	kNkMAIDCapability_InternalSplCmdGroupBMode	= kNkMAIDCapability_VendorBaseD1 + 0xff,
	kNkMAIDCapability_InternalSplCmdGroupBComp	= kNkMAIDCapability_VendorBaseD1 + 0x100,
	kNkMAIDCapability_InternalSplCmdGroupBValue	= kNkMAIDCapability_VendorBaseD1 + 0x101,
	kNkMAIDCapability_InternalSplCmdSelfMode	= kNkMAIDCapability_VendorBaseD1 + 0x102,
	kNkMAIDCapability_InternalSplCmdSelfValue	= kNkMAIDCapability_VendorBaseD1 + 0x103,
	kNkMAIDCapability_SelectFUNC2				= kNkMAIDCapability_VendorBaseD1 + 0x104,
	kNkMAIDCapability_LutIndexNumber			= kNkMAIDCapability_VendorBaseD1 + 0x105,
	kNkMAIDCapability_MonochromeFilterEffect	= kNkMAIDCapability_VendorBaseD1 + 0x120,
	kNkMAIDCapability_MonochromeEdgeEnhancement	= kNkMAIDCapability_VendorBaseD1 + 0x121,
	kNkMAIDCapability_MonochromeCurve			= kNkMAIDCapability_VendorBaseD1 + 0x122,
	kNkMAIDCapability_AutoFPShoot				= kNkMAIDCapability_VendorBaseD1 + 0x123,
	kNkMAIDCapability_MonochromeSettingType		= kNkMAIDCapability_VendorBaseD1 + 0x124,
	kNkMAIDCapability_AFCapture					= kNkMAIDCapability_VendorBaseD1 + 0x125,
	kNkMAIDCapability_AutoOffTimer				= kNkMAIDCapability_VendorBaseD1 + 0x126,
	kNkMAIDCapability_ImageConfirmTime			= kNkMAIDCapability_VendorBaseD1 + 0x127,
	kNkMAIDCapability_InfoDisplayErrStatus		= kNkMAIDCapability_VendorBaseD1 + 0x128,
	kNkMAIDCapability_ExternalSplMode			= kNkMAIDCapability_VendorBaseD1 + 0x129,
	kNkMAIDCapability_ExternalSplValue			= kNkMAIDCapability_VendorBaseD1 + 0x12a,
	kNkMAIDCapability_Slot2ImageSaveMode		= kNkMAIDCapability_VendorBaseD1 + 0x12c,
	kNkMAIDCapability_CompressRAWBitMode		= kNkMAIDCapability_VendorBaseD1 + 0x12d,
	kNkMAIDCapability_PictureControl			= kNkMAIDCapability_VendorBaseD1 + 0x12e,
	kNkMAIDCapability_IntegratedLevel			= kNkMAIDCapability_VendorBaseD1 + 0x12f,
	kNkMAIDCapability_Brightness				= kNkMAIDCapability_VendorBaseD1 + 0x130,
	kNkMAIDCapability_MonochromeTuneColors		= kNkMAIDCapability_VendorBaseD1 + 0x131,
	kNkMAIDCapability_Active_D_Lighting			= kNkMAIDCapability_VendorBaseD1 + 0x132,
	kNkMAIDCapability_DynamicAFArea				= kNkMAIDCapability_VendorBaseD1 + 0x133,
	kNkMAIDCapability_ShootingSpeedHigh			= kNkMAIDCapability_VendorBaseD1 + 0x134,
	kNkMAIDCapability_InfoDispSetting			= kNkMAIDCapability_VendorBaseD1 + 0x135,
	kNkMAIDCapability_PreviewButton				= kNkMAIDCapability_VendorBaseD1 + 0x137,
	kNkMAIDCapability_PreviewButton2			= kNkMAIDCapability_VendorBaseD1 + 0x138,
	kNkMAIDCapability_AEAFLockButton2			= kNkMAIDCapability_VendorBaseD1 + 0x13a,
	kNkMAIDCapability_IndicatorDisplay			= kNkMAIDCapability_VendorBaseD1 + 0x13b,
	kNkMAIDCapability_LiveViewMode				= kNkMAIDCapability_VendorBaseD1 + 0x13c,
	kNkMAIDCapability_LiveViewDriveMode			= kNkMAIDCapability_VendorBaseD1 + 0x13d,
	kNkMAIDCapability_LiveViewStatus			= kNkMAIDCapability_VendorBaseD1 + 0x13e,
	kNkMAIDCapability_LiveViewImageZoomRate		= kNkMAIDCapability_VendorBaseD1 + 0x13f,
	kNkMAIDCapability_ContrastAF				= kNkMAIDCapability_VendorBaseD1 + 0x140,
	kNkMAIDCapability_DeleteDramImage			= kNkMAIDCapability_VendorBaseD1 + 0x143,
	kNkMAIDCapability_CurrentPreviewID			= kNkMAIDCapability_VendorBaseD1 + 0x144,
	kNkMAIDCapability_GetPreviewImageLow		= kNkMAIDCapability_VendorBaseD1 + 0x145,
	kNkMAIDCapability_GetPreviewImageNormal		= kNkMAIDCapability_VendorBaseD1 + 0x146,
	kNkMAIDCapability_GetLiveViewImage			= kNkMAIDCapability_VendorBaseD1 + 0x147,
	kNkMAIDCapability_MFDriveStep				= kNkMAIDCapability_VendorBaseD1 + 0x148,
	kNkMAIDCapability_MFDrive					= kNkMAIDCapability_VendorBaseD1 + 0x149,
	kNkMAIDCapability_ContrastAFArea			= kNkMAIDCapability_VendorBaseD1 + 0x14a,
	kNkMAIDCapability_CompressRAWEx				= kNkMAIDCapability_VendorBaseD1 + 0x14b,
	kNkMAIDCapability_CellKindPriority			= kNkMAIDCapability_VendorBaseD1 + 0x14c,
	kNkMAIDCapability_WBFluorescentType			= kNkMAIDCapability_VendorBaseD1 + 0x14d,
	kNkMAIDCapability_WBTuneColorAdjust			= kNkMAIDCapability_VendorBaseD1 + 0x14e,
	kNkMAIDCapability_WBTunePreset1				= kNkMAIDCapability_VendorBaseD1 + 0x14f,
	kNkMAIDCapability_WBTunePreset2				= kNkMAIDCapability_VendorBaseD1 + 0x150,
	kNkMAIDCapability_WBTunePreset3				= kNkMAIDCapability_VendorBaseD1 + 0x151,
	kNkMAIDCapability_WBTunePreset4				= kNkMAIDCapability_VendorBaseD1 + 0x152,
	kNkMAIDCapability_WBTunePreset5				= kNkMAIDCapability_VendorBaseD1 + 0x153,
	kNkMAIDCapability_AFAreaPoint				= kNkMAIDCapability_VendorBaseD1 + 0x154,
	kNkMAIDCapability_NormalAfButton			= kNkMAIDCapability_VendorBaseD1 + 0x155,
	kNkMAIDCapability_ManualSetLensNo			= kNkMAIDCapability_VendorBaseD1 + 0x156,
	kNkMAIDCapability_AutoDXCrop				= kNkMAIDCapability_VendorBaseD1 + 0x157,
	kNkMAIDCapability_PictureControlData		= kNkMAIDCapability_VendorBaseD1 + 0x158,
	kNkMAIDCapability_GetPicCtrlInfo			= kNkMAIDCapability_VendorBaseD1 + 0x159,
	kNkMAIDCapability_DeleteCustomPictureControl= kNkMAIDCapability_VendorBaseD1 + 0x15a,
	kNkMAIDCapability_LensType					= kNkMAIDCapability_VendorBaseD1 + 0x15c,
	kNkMAIDCapability_ChangedPictureControl		= kNkMAIDCapability_VendorBaseD1 + 0x15d,
	kNkMAIDCapability_LiveViewProhibit			= kNkMAIDCapability_VendorBaseD1 + 0x15e,
	kNkMAIDCapability_AutoOffPhoto				= kNkMAIDCapability_VendorBaseD1 + 0x165,
	kNkMAIDCapability_AutoOffMenu				= kNkMAIDCapability_VendorBaseD1 + 0x166,
	kNkMAIDCapability_AutoOffInfo				= kNkMAIDCapability_VendorBaseD1 + 0x167,
	kNkMAIDCapability_ScreenTips				= kNkMAIDCapability_VendorBaseD1 + 0x168,
	kNkMAIDCapability_IlluminationSetting		= kNkMAIDCapability_VendorBaseD1 + 0x169,
	kNkMAIDCapability_ShutterSpeedLockSetting	= kNkMAIDCapability_VendorBaseD1 + 0x16a,
	kNkMAIDCapability_ApertureLockSetting		= kNkMAIDCapability_VendorBaseD1 + 0x16b,
	kNkMAIDCapability_VignetteControl			= kNkMAIDCapability_VendorBaseD1 + 0x16c,
	kNkMAIDCapability_FocusPointBrightness		= kNkMAIDCapability_VendorBaseD1 + 0x16d,
	kNkMAIDCapability_EnableCopyright			= kNkMAIDCapability_VendorBaseD1 + 0x16e,
	kNkMAIDCapability_ArtistName				= kNkMAIDCapability_VendorBaseD1 + 0x16f,
	kNkMAIDCapability_CopyrightInfo				= kNkMAIDCapability_VendorBaseD1 + 0x170,
	kNkMAIDCapability_AngleLevel				= kNkMAIDCapability_VendorBaseD1 + 0x171,
	kNkMAIDCapability_MovieScreenSize			= kNkMAIDCapability_VendorBaseD1 + 0x172,
	kNkMAIDCapability_MovieVoice				= kNkMAIDCapability_VendorBaseD1 + 0x173,
	kNkMAIDCapability_LiveViewAF				= kNkMAIDCapability_VendorBaseD1 + 0x175,
	kNkMAIDCapability_SelfTimerShootNum			= kNkMAIDCapability_VendorBaseD1 + 0x176,
	kNkMAIDCapability_FinderISODisplay			= kNkMAIDCapability_VendorBaseD1 + 0x177,
	kNkMAIDCapability_EnableCommandDialOnPlaybackEx = kNkMAIDCapability_VendorBaseD1 + 0x178,
	kNkMAIDCapability_ExchangeDialsEx			= kNkMAIDCapability_VendorBaseD1 + 0x179,
	kNkMAIDCapability_CenterButtonOnLiveView    = kNkMAIDCapability_VendorBaseD1 + 0x180,
	kNkMAIDCapability_ZoomRateOnLiveView        = kNkMAIDCapability_VendorBaseD1 + 0x181,
	kNkMAIDCapability_AutoDistortion			= kNkMAIDCapability_VendorBaseD1 + 0x182,
	kNkMAIDCapability_SceneMode					= kNkMAIDCapability_VendorBaseD1 + 0x183,
	kNkMAIDCapability_MovieRecMicrophone		= kNkMAIDCapability_VendorBaseD1 + 0x185,
	kNkMAIDCapability_MovieRecDestination		= kNkMAIDCapability_VendorBaseD1 + 0x186,
	kNkMAIDCapability_PrimarySlot				= kNkMAIDCapability_VendorBaseD1 + 0x187,
	kNkMAIDCapability_ADLBracketingType		    = kNkMAIDCapability_VendorBaseD1 + 0x188,
	kNkMAIDCapability_SelectFUNC2CapAreaCrop    = kNkMAIDCapability_VendorBaseD1 + 0x191,
	kNkMAIDCapability_PreviewButton2CapAreaCrop = kNkMAIDCapability_VendorBaseD1 + 0x192,
	kNkMAIDCapability_AEAFLockButton2CapAreaCrop= kNkMAIDCapability_VendorBaseD1 + 0x193,
	kNkMAIDCapability_BracketButton             = kNkMAIDCapability_VendorBaseD1 + 0x194,
	kNkMAIDCapability_RemainCountInMedia        = kNkMAIDCapability_VendorBaseD1 + 0x195,
	kNkMAIDCapability_AngleLevelPitch           = kNkMAIDCapability_VendorBaseD1 + 0x196,
	kNkMAIDCapability_AngleLevelYaw             = kNkMAIDCapability_VendorBaseD1 + 0x197,
	kNkMAIDCapability_MovRecInCardStatus        = kNkMAIDCapability_VendorBaseD1 + 0x198,
	kNkMAIDCapability_MovRecInCardProhibit      = kNkMAIDCapability_VendorBaseD1 + 0x199,
	kNkMAIDCapability_ActiveSlot                = kNkMAIDCapability_VendorBaseD1 + 0x204,
	kNkMAIDCapability_SaveMedia                 = kNkMAIDCapability_VendorBaseD1 + 0x205,
	kNkMAIDCapability_MovieRecHiISO				= kNkMAIDCapability_VendorBaseD1 + 0x206,
	kNkMAIDCapability_UserMode1			    	= kNkMAIDCapability_VendorBaseD1 + 0x207,
	kNkMAIDCapability_UserMode2				    = kNkMAIDCapability_VendorBaseD1 + 0x208,
	kNkMAIDCapability_MovieManualSetting		= kNkMAIDCapability_VendorBaseD1 + 0x209,
	kNkMAIDCapability_AFModeAtLiveView			= kNkMAIDCapability_VendorBaseD1 + 0x210,
	kNkMAIDCapability_SelfTimerShootInterval	= kNkMAIDCapability_VendorBaseD1 + 0x211,
	kNkMAIDCapability_AutoOffLiveView			= kNkMAIDCapability_VendorBaseD1 + 0x212,
	kNkMAIDCapability_RemoteCtrlWaitTime		= kNkMAIDCapability_VendorBaseD1 + 0x213,
	kNkMAIDCapability_BeepVolume				= kNkMAIDCapability_VendorBaseD1 + 0x214,
	kNkMAIDCapability_VideoMode			    	= kNkMAIDCapability_VendorBaseD1 + 0x215,
	kNkMAIDCapability_WBAutoType	    		= kNkMAIDCapability_VendorBaseD1 + 0x216,
	kNkMAIDCapability_GetVideoImage	    		= kNkMAIDCapability_VendorBaseD1 + 0x217,
    kNkMAIDCapability_TerminateCapture          = kNkMAIDCapability_VendorBaseD1 + 0x218,
    kNkMAIDCapability_EffectMode		        = kNkMAIDCapability_VendorBaseD1 + 0x219,
    kNkMAIDCapability_HDRMode					= kNkMAIDCapability_VendorBaseD1 + 0x220,
    kNkMAIDCapability_HDRExposure				= kNkMAIDCapability_VendorBaseD1 + 0x221,
    kNkMAIDCapability_HDRSmoothing				= kNkMAIDCapability_VendorBaseD1 + 0x222,
    kNkMAIDCapability_BlinkingStatus			= kNkMAIDCapability_VendorBaseD1 + 0x223,
    kNkMAIDCapability_AutoSceneModeStatus		= kNkMAIDCapability_VendorBaseD1 + 0x224,
	kNkMAIDCapability_WBTuneColorTempEx			= kNkMAIDCapability_VendorBaseD1 + 0x225,
    kNkMAIDCapability_WBPresetProtect1			= kNkMAIDCapability_VendorBaseD1 + 0x226,
    kNkMAIDCapability_WBPresetProtect2			= kNkMAIDCapability_VendorBaseD1 + 0x227,
    kNkMAIDCapability_WBPresetProtect3			= kNkMAIDCapability_VendorBaseD1 + 0x228,
    kNkMAIDCapability_WBPresetProtect4			= kNkMAIDCapability_VendorBaseD1 + 0x229,
    kNkMAIDCapability_ISOAutoShutterTimeAutoValue = kNkMAIDCapability_VendorBaseD1 + 0x230,
    kNkMAIDCapability_MovieImageQuality			= kNkMAIDCapability_VendorBaseD1 + 0x231,
    kNkMAIDCapability_MovieRecMicrophoneValue	= kNkMAIDCapability_VendorBaseD1 + 0x232,
    kNkMAIDCapability_LiveViewExposurePreview	= kNkMAIDCapability_VendorBaseD1 + 0x233,
    kNkMAIDCapability_LiveViewSelector			= kNkMAIDCapability_VendorBaseD1 + 0x234,
    kNkMAIDCapability_LiveViewWBMode			= kNkMAIDCapability_VendorBaseD1 + 0x235,
    kNkMAIDCapability_MovieShutterSpeed			= kNkMAIDCapability_VendorBaseD1 + 0x236,
    kNkMAIDCapability_MovieAperture				= kNkMAIDCapability_VendorBaseD1 + 0x237,
    kNkMAIDCapability_MovieSensitivity			= kNkMAIDCapability_VendorBaseD1 + 0x238,
    kNkMAIDCapability_MovieExposureComp			= kNkMAIDCapability_VendorBaseD1 + 0x239,
    kNkMAIDCapability_ADLBracketingStep			= kNkMAIDCapability_VendorBaseD1 + 0x243,
    kNkMAIDCapability_ResetWBMode				= kNkMAIDCapability_VendorBaseD1 + 0x244,
    kNkMAIDCapability_ExpCompFlashUsed			= kNkMAIDCapability_VendorBaseD1 + 0x245,
    kNkMAIDCapability_LiveViewPhotoShootingMode	= kNkMAIDCapability_VendorBaseD1 + 0x246,
    kNkMAIDCapability_ExposureDelayEx			= kNkMAIDCapability_VendorBaseD1 + 0x247,
    kNkMAIDCapability_MovieISORange				= kNkMAIDCapability_VendorBaseD1 + 0x248,
	kNkMAIDCapability_MovieReleaseButton		= kNkMAIDCapability_VendorBaseD1 + 0x249,
    kNkMAIDCapability_MovieRecFrameCount		= kNkMAIDCapability_VendorBaseD1 + 0x250,
	kNkMAIDCapability_CurrentItemID				= kNkMAIDCapability_VendorBaseD1 + 0x251,
	kNkMAIDCapability_GetIPTCInfo				= kNkMAIDCapability_VendorBaseD1 + 0x252
};

// The module will deliver one of these values to the nResult parameter of the client 
// completion callback function and return the same value from the entry point. 
// Errors will have smaller values than kNkMAIDResult_D1Origin.
// Minimun value = kNkMAIDResult_D1Origin - 128
// Maximun value = kNkMAIDResult_D1Origin + 127
enum eNkMAIDResultD1
{
	kNkMAIDResult_ApertureFEE					= kNkMAIDResult_D1Origin - 128,// = 128
	kNkMAIDResult_BufferNotReady				= 129,
	kNkMAIDResult_NormalTTL						= 130,
	kNkMAIDResult_MediaFull						= 131,
	kNkMAIDResult_InvalidMedia					= 132,
	kNkMAIDResult_EraseFailure					= 133,
	kNkMAIDResult_CameraNotFound				= 134,
	kNkMAIDResult_BatteryDontWork				= 135,
	kNkMAIDResult_ShutterBulb					= 136,
	kNkMAIDResult_OutOfFocus					= 137,
	kNkMAIDResult_Protected						= 138,
	kNkMAIDResult_FileExists					= 139,
	kNkMAIDResult_SharingViolation				= 140,
	kNkMAIDResult_DataTransFailure				= 141,
	kNkMAIDResult_SessionFailure				= 142,
	kNkMAIDResult_FileRemoved					= 143,
	kNkMAIDResult_BusReset						= 144,
	kNkMAIDResult_NonCPULens					= 145,
	kNkMAIDResult_ReleaseButtonPressed			= 146,
	kNkMAIDResult_BatteryExhausted				= 147,
	kNkMAIDResult_CaptureFailure				= 148,
	kNkMAIDResult_InvalidString					= 149,
	kNkMAIDResult_NotInitialized				= 150,
	kNkMAIDResult_CaptureDisable				= 151,
	kNkMAIDResult_DeviceBusy					= 152,
	kNkMAIDResult_CaptureDustFailure			= 153,
	kNkMAIDResult_ICADown						= 154,
	kNkMAIDResult_CpxPlaybackMode				= 158,
	kNkMAIDResult_NotLiveView					= 159,
	kNkMAIDResult_MFDriveEnd					= 160,
	kNkMAIDResult_UnformattedMedia				= 161,
	kNkMAIDResult_MediaReadOnly					= 162,
	kNkMAIDResult_DuringUpdate					= 163,
	kNkMAIDResult_BulbReleaseBusy				= 164,
	kNkMAIDResult_SilentReleaseBusy				= 165,
	kNkMAIDResult_MovieFrameReleaseBusy			= 166
};

// the verdor unique event code.
enum eNkMAIDEventDX2
{
	kNkMAIDEvent_AddPreviewImage		= kNkMAIDEvent_D1Origin + 1, //0x107
	kNkMAIDEvent_CaptureComplete		= kNkMAIDEvent_D1Origin + 2, //0x108
	kNkMAIDEvent_AddChildInCard			= kNkMAIDEvent_D1Origin + 3, //0x109
	kNkMAIDEvent_RecordingInterrupted	= kNkMAIDEvent_D1Origin + 4  //0x10A
};

///////////////////////////////////////////////////////////////////////////////
//	CONSTANT DEFINITIONS
//

// kNkMAIDCapability_ModuleMode(0x8101)
enum eNkMAIDModuleMode
{
	kNkMAIDModuleMode_Browser,
	kNkMAIDModuleMode_Controller
};

// kNkMAIDCapability_CFStatus(0x8108)
enum eNkMAIDCFStatus
{
	kNkMAIDCFStatus_Good,
	kNkMAIDCFStatus_Full,
	kNkMAIDCFStatus_NotFormated,
	kNkMAIDCFStatus_NotExist,
	kNkMAIDCFStatus_NotAvailable
};

// kNkMAIDCapability_FlashStatus(0x810a)
enum eNkMAIDFlashStatus
{
	kNkMAIDFlashStatus_Ready,
	kNkMAIDFlashStatus_NotReady,
	kNkMAIDFlashStatus_NotExist,
	kNkMAIDFlashStatus_NormalTTL
};

// kNkMAIDCapability_MediaType(0x810c)
// kNkMAIDCapability_WritingMedia(0x8150)
enum eNkMAIDMediaType
{
	kNkMAIDMediaType_CFCard,
	kNkMAIDMediaType_1394
};

// kNkMAIDCapability_ExposureMode(0x8111)
enum eNkMAIDExposureMode
{
	kNkMAIDExposureMode_Program,
	kNkMAIDExposureMode_AperturePriority,
	kNkMAIDExposureMode_SpeedPriority,
	kNkMAIDExposureMode_Manual,
	kNkMAIDExposureMode_Disable,
	kNkMAIDExposureMode_Auto,
	kNkMAIDExposureMode_Portrait,
	kNkMAIDExposureMode_Landscape,
	kNkMAIDExposureMode_Closeup,
	kNkMAIDExposureMode_Sports,
	kNkMAIDExposureMode_NightPortrait,
	kNkMAIDExposureMode_NightView,
	kNkMAIDExposureMode_Child,
	kNkMAIDExposureMode_FlashOff,
	kNkMAIDExposureMode_Scene,
	kNkMAIDExposureMode_UserMode1,
	kNkMAIDExposureMode_UserMode2,
	kNkMAIDExposureMode_Effects
};

// kNkMAIDCapability_MeteringMode(0x8116)
enum eNkMAIDMeteringMode
{
	kNkMAIDMeteringMode_Matrix,
	kNkMAIDMeteringMode_CenterWeighted,
	kNkMAIDMeteringMode_Spot,
	kNkMAIDMeteringMode_AfSpot
};

// kNkMAIDCapability_FocusMode(0x8120)
enum eNkMAIDFocusMode
{
	kNkMAIDFocusMode_MF,
	kNkMAIDFocusMode_AFs,
	kNkMAIDFocusMode_AFc,
	kNkMAIDFocusMode_AFa,
	kNkMAIDFocusMode_AFf,
	kNkMAIDFocusMode_AF = 0x10,
	kNkMAIDFocusMode_Macro,
	kNkMAIDFocusMode_Infinity
};

// kNkMAIDCapability_ColorAdjustment(0x815e)
enum eNkMAIDColorAdjustment
{
	eNkMAIDColorAdjustment_Minus9,
	eNkMAIDColorAdjustment_Minus6,
	eNkMAIDColorAdjustment_Minus3,
	eNkMAIDColorAdjustment_0,
	eNkMAIDColorAdjustment_Plus3,
	eNkMAIDColorAdjustment_Plus6,
	eNkMAIDColorAdjustment_Plus9
};

// kNkMAIDCapability_TypicalFlashMode(0x8160)
enum eNkMAIDTypicalFlashMode
{
	kNkMAIDTypicalFlashMode_Auto,
	kNkMAIDTypicalFlashMode_ForcedOn,
	kNkMAIDTypicalFlashMode_Off,
	kNkMAIDTypicalFlashMode_RedEyeReduction,
	kNkMAIDTypicalFlashMode_SlowSync
};

// kNkMAIDCapability_Converter(0x8161)
enum eNkMAIDConverter
{
	kNkMAIDConverter_None,
	kNkMAIDConverter_Wide,
	kNkMAIDConverter_FishEye
};

// kNkMAIDCapability_ElectronicZoom(0x8162)
enum eNkMAIDElectronicZoom
{
	kNkMAIDElectronicZoom_100,
	kNkMAIDElectronicZoom_120,
	kNkMAIDElectronicZoom_140,
	kNkMAIDElectronicZoom_160,
	kNkMAIDElectronicZoom_180,
	kNkMAIDElectronicZoom_200,
	kNkMAIDElectronicZoom_220,
	kNkMAIDElectronicZoom_240,
	kNkMAIDElectronicZoom_260,
	kNkMAIDElectronicZoom_280,
	kNkMAIDElectronicZoom_300,
	kNkMAIDElectronicZoom_320,
	kNkMAIDElectronicZoom_340,
	kNkMAIDElectronicZoom_360,
	kNkMAIDElectronicZoom_380,
	kNkMAIDElectronicZoom_400
};

// kNkMAIDCapability_DateFormat(0x8163)
enum eNkMAIDDateFormat
{
	kNkMAIDDateFormat_Off,
	kNkMAIDDateFormat_YMD,
	kNkMAIDDateFormat_MDY,
	kNkMAIDDateFormat_DMY
};

// kNkMAIDCapability_WBTuneColorTemp(0x818c)
enum eNkMAIDWBTuneColorTemp
{
	eNkMAIDWBTuneColorTemp_Unknown = 0,
	eNkMAIDWBTuneColorTemp_2500 = 2500,
	eNkMAIDWBTuneColorTemp_2550 = 2550,
	eNkMAIDWBTuneColorTemp_2560 = 2560,
	eNkMAIDWBTuneColorTemp_2630 = 2630,
	eNkMAIDWBTuneColorTemp_2650 = 2650,
	eNkMAIDWBTuneColorTemp_2700 = 2700,
	eNkMAIDWBTuneColorTemp_2780 = 2780,
	eNkMAIDWBTuneColorTemp_2800 = 2800,
	eNkMAIDWBTuneColorTemp_2850 = 2850,
	eNkMAIDWBTuneColorTemp_2860 = 2860,
	eNkMAIDWBTuneColorTemp_2940 = 2940,
	eNkMAIDWBTuneColorTemp_2950 = 2950,
	eNkMAIDWBTuneColorTemp_3000 = 3000,
	eNkMAIDWBTuneColorTemp_3030 = 3030,
	eNkMAIDWBTuneColorTemp_3100 = 3100,
	eNkMAIDWBTuneColorTemp_3130 = 3130,
	eNkMAIDWBTuneColorTemp_3200 = 3200,
	eNkMAIDWBTuneColorTemp_3230 = 3230,
	eNkMAIDWBTuneColorTemp_3300 = 3300,
	eNkMAIDWBTuneColorTemp_3330 = 3330,
	eNkMAIDWBTuneColorTemp_3400 = 3400,
	eNkMAIDWBTuneColorTemp_3450 = 3450,
	eNkMAIDWBTuneColorTemp_3570 = 3570,
	eNkMAIDWBTuneColorTemp_3600 = 3600,
	eNkMAIDWBTuneColorTemp_3700 = 3700,
	eNkMAIDWBTuneColorTemp_3800 = 3800,
	eNkMAIDWBTuneColorTemp_3850 = 3850,
	eNkMAIDWBTuneColorTemp_4000 = 4000,
	eNkMAIDWBTuneColorTemp_4170 = 4170,
	eNkMAIDWBTuneColorTemp_4200 = 4200,
	eNkMAIDWBTuneColorTemp_4300 = 4300,
	eNkMAIDWBTuneColorTemp_4350 = 4350,
	eNkMAIDWBTuneColorTemp_4500 = 4500,
	eNkMAIDWBTuneColorTemp_4550 = 4550,
	eNkMAIDWBTuneColorTemp_4760 = 4760,
	eNkMAIDWBTuneColorTemp_4800 = 4800,
	eNkMAIDWBTuneColorTemp_5000 = 5000,
	eNkMAIDWBTuneColorTemp_5260 = 5260,
	eNkMAIDWBTuneColorTemp_5300 = 5300,
	eNkMAIDWBTuneColorTemp_5560 = 5560,
	eNkMAIDWBTuneColorTemp_5600 = 5600,
	eNkMAIDWBTuneColorTemp_5880 = 5880,
	eNkMAIDWBTuneColorTemp_5900 = 5900,
	eNkMAIDWBTuneColorTemp_6250 = 6250,
	eNkMAIDWBTuneColorTemp_6300 = 6300,
	eNkMAIDWBTuneColorTemp_6670 = 6670,
	eNkMAIDWBTuneColorTemp_6700 = 6700,
	eNkMAIDWBTuneColorTemp_7100 = 7100,
	eNkMAIDWBTuneColorTemp_7140 = 7140,
	eNkMAIDWBTuneColorTemp_7690 = 7690,
	eNkMAIDWBTuneColorTemp_7700 = 7700,
	eNkMAIDWBTuneColorTemp_8300 = 8300,
	eNkMAIDWBTuneColorTemp_8330 = 8330,
	eNkMAIDWBTuneColorTemp_9090 = 9090,
	eNkMAIDWBTuneColorTemp_9100 = 9100,
	eNkMAIDWBTuneColorTemp_9900 = 9900,
	eNkMAIDWBTuneColorTemp_10000 = 10000
};

//kNkMAIDCapability_ShootingMode(0x818d)
enum eNkMAIDShootingMode
{
	eNkMAIDShootingMode_S = 0,
	eNkMAIDShootingMode_C,
	eNkMAIDShootingMode_CH,
	eNkMAIDShootingMode_SelfTimer,
	eNkMAIDShootingMode_MirrorUp,
	eNkMAIDShootingMode_RemoteTimer_Instant,
	eNkMAIDShootingMode_RemoteTimer_2sec,
	eNkMAIDShootingMode_LiveView,
	eNkMAIDShootingMode_Quiet,
	eNkMAIDShootingMode_RemoteCtrl,
	eNkMAIDShootingMode_Unknown = 0xff
};

//kNkMAIDCapability_FmmManual(0x8192)
enum eNkMAIDFmmManual
{
	eNkMAIDFmmManual_0 = 0,
	eNkMAIDFmmManual_6 = 6,
	eNkMAIDFmmManual_8 = 8,
	eNkMAIDFmmManual_13 = 13,
	eNkMAIDFmmManual_15 = 15,
	eNkMAIDFmmManual_16 = 16,
	eNkMAIDFmmManual_18 = 18,
	eNkMAIDFmmManual_20 = 20,
	eNkMAIDFmmManual_24 = 24,
	eNkMAIDFmmManual_25 = 25,
	eNkMAIDFmmManual_28 = 28,
	eNkMAIDFmmManual_35 = 35,
	eNkMAIDFmmManual_43 = 43,
	eNkMAIDFmmManual_45 = 45,
	eNkMAIDFmmManual_50 = 50,
	eNkMAIDFmmManual_55 = 55,
	eNkMAIDFmmManual_58 = 58,
	eNkMAIDFmmManual_70 = 70,
	eNkMAIDFmmManual_80 = 80,
	eNkMAIDFmmManual_85 = 85,
	eNkMAIDFmmManual_86 = 86,
	eNkMAIDFmmManual_100 = 100,
	eNkMAIDFmmManual_105 = 105,
	eNkMAIDFmmManual_135 = 135,
	eNkMAIDFmmManual_180 = 180,
	eNkMAIDFmmManual_200 = 200,
	eNkMAIDFmmManual_300 = 300,
	eNkMAIDFmmManual_360 = 360,
	eNkMAIDFmmManual_400 = 400,
	eNkMAIDFmmManual_500 = 500,
	eNkMAIDFmmManual_600 = 600,
	eNkMAIDFmmManual_800 = 800,
	eNkMAIDFmmManual_1000 = 1000,
	eNkMAIDFmmManual_1200 = 1200,
	eNkMAIDFmmManual_1400 = 1400,
	eNkMAIDFmmManual_1600 = 1600,
	eNkMAIDFmmManual_2000 = 2000,
	eNkMAIDFmmManual_2400 = 2400,
	eNkMAIDFmmManual_2800 = 2800,
	eNkMAIDFmmManual_3200 = 3200,
	eNkMAIDFmmManual_4000 = 4000
};

//kNkMAIDCapability_USBSpeed(0x81ac)
enum eNkMAIDUSBSpeed
{
	eNkMAIDUSBSpeed_FullSpeed,
	eNkMAIDUSBSpeed_HighSpeed,
};

//kNkMAIDCapability_FocusGroupPreferredArea(0x818a)
enum eNkMAIDFocusGroupPreferred
{
	kNkMAIDFocusGroupPreferred_Center,
	kNkMAIDFocusGroupPreferred_Upper,
	kNkMAIDFocusGroupPreferred_Bottom,
	kNkMAIDFocusGroupPreferred_Left,
	kNkMAIDFocusGroupPreferred_Right
};

//kNkMAIDCapability_BracketingType(0x81b0)
enum eNkMAIDBracketingType
{
	eNkMAIDBracketingType_Minus_2,
	eNkMAIDBracketingType_Puls_2,
	eNkMAIDBracketingType_Minus_3,
	eNkMAIDBracketingType_Puls_3,
	eNkMAIDBracketingType_Both_3,
	eNkMAIDBracketingType_Both_5,
	eNkMAIDBracketingType_Both_7,
	eNkMAIDBracketingType_Both_9
};

//kNkMAIDCapability_AEBracketingStep(0x81b1)
enum eNkMAIDAEBracketingStep
{
	eNkMAIDAEBracketingStep_1_3EV,
	eNkMAIDAEBracketingStep_1_2EV,
	eNkMAIDAEBracketingStep_2_3EV,
	eNkMAIDAEBracketingStep_1EV,
	eNkMAIDAEBracketingStep_4_3EV,
	eNkMAIDAEBracketingStep_3_2EV,
	eNkMAIDAEBracketingStep_5_3EV,
	eNkMAIDAEBracketingStep_2EV,
	eNkMAIDAEBracketingStep_3EV
};

//kNkMAIDCapability_WBBracketingStep(0x81b2)
enum eNkMAIDWBBracketingStep
{
	eNkMAIDWBBracketingStep_1STEP,
	eNkMAIDWBBracketingStep_2STEP,
	eNkMAIDWBBracketingStep_3STEP
};

//kNkMAIDCapability_CameraInclination(0x81b4)
enum eNkMAIDCameraInclination
{
	eNkMAIDCameraInclination_Level = 0,
	eNkMAIDCameraInclination_GripTop,
	eNkMAIDCameraInclination_GripBottom,
	eNkMAIDCameraInclination_LevelUpDown
};

//kNkMAIDCapability_RawJpegImageStatus(0x81b5)
enum eNkMAIDRawJpegImageStatus
{
	eNkMAIDRawJpegImageStatus_Single,
	eNkMAIDRawJpegImageStatus_RawJpeg
};

// kNkMAIDCapability_CaptureDustImage(0x81b6)
enum eNkMAIDFileDataTypesDX2
{
	kNkMAIDFileDataType_VendorBaseDx2 = kNkMAIDFileDataType_UserType, //0x100
	kNkMAIDFileDataType_NDF,
	kNkMAIDFileDataType_AVI,
	kNkMAIDFileDataType_MOV
};

//	kNkMAIDCapability_AfGroupAreaPatternType(0x81b8)
enum eNkMAIDAfGroupAreaPatternType
{
	eNkMAIDAfGroupAreaPatternType_Horizontal,
	eNkMAIDAfGroupAreaPatternType_Vertical
};

//	kNkMAIDCapability_ExternalFlashSort(0x81b9)
enum eNkMAIDExternalFlashSort
{
	eNkMAIDExternalFlashSort_NoCommunicate = 0,
	eNkMAIDExternalFlashSort_OldType,
	eNkMAIDExternalFlashSort_NewType,
	eNkMAIDExternalFlashSort_NotExist,
	eNkMAIDExternalFlashSort_NewTypeControl

};

//	kNkMAIDCapability_ExternalOldTypeFlashMode(0x81ba)
enum eNkMAIDExternalOldTypeFlashMode
{
	eNkMAIDExternalOldTypeFlashMode_TTL = 0,
	eNkMAIDExternalOldTypeFlashMode_DTTL,
	eNkMAIDExternalOldTypeFlashMode_AA,
	eNkMAIDExternalOldTypeFlashMode_ExtAutoFlash,
	eNkMAIDExternalOldTypeFlashMode_Manual,
	eNkMAIDExternalOldTypeFlashMode_FP,
	eNkMAIDExternalOldTypeFlashMode_NotExist,
	eNkMAIDExternalOldTypeFlashMode_NonTTL
};

//	kNkMAIDCapability_ExternalNewTypeFlashMode(0x81bb)
enum eNkMAIDExternalNewTypeFlashMode
{
	eNkMAIDExternalNewTypeFlashMode_OFF = 0,
	eNkMAIDExternalNewTypeFlashMode_CompTTL,
	eNkMAIDExternalNewTypeFlashMode_NoCompTTL,
	eNkMAIDExternalNewTypeFlashMode_AA,
	eNkMAIDExternalNewTypeFlashMode_ExtAutoFlash,
	eNkMAIDExternalNewTypeFlashMode_ManualLengthPriority,
	eNkMAIDExternalNewTypeFlashMode_Manual,
	eNkMAIDExternalNewTypeFlashMode_Multi,
	eNkMAIDExternalNewTypeFlashMode_NotExist
};

//	kNkMAIDCapability_CCDDataMode(0x81d0)
enum eNkMAIDCCDDataMode
{
	kNkMAIDCCDDataMode_Full = 0,
	kNkMAIDCCDDataMode_HiSpeedCrop,
	kNkMAIDCCDDataMode_AutoDXCrop,
	kNkMAIDCCDDataMode_FXFormat,
	kNkMAIDCCDDataMode_DXFormat,
	kNkMAIDCCDDataMode_5_4,
	kNkMAIDCCDDataMode_12x
};

//	kNkMAIDCapability_JpegCompressionPolicy(0x81d1)
enum eNkMAIDJpegCompressionPolicy
{
	kNkMAIDJpegCompressionPolicy_Size = 0,
	kNkMAIDJpegCompressionPolicy_Quality
};

//	kNkMAIDCapability_CameraType(0x81d7)
enum eNkMAIDCameraType
{
	kNkMAIDCameraType_D2H_V1 = 0,	// D2H Ver.1
	kNkMAIDCameraType_D2H_V2,		// D2H Ver.2
	kNkMAIDCameraType_D2X,			// D2X Ver.1
	kNkMAIDCameraType_D2HS,			// D2Hs
   	kNkMAIDCameraType_D2X_V2,		// D2X Ver.2
	kNkMAIDCameraType_D2XS,			// D2XS
	kNkMAIDCameraType_D2HS_V2,		// D2HS Ver.2
	kNkMAIDCameraType_D40 = 0x10,	// D40
	kNkMAIDCameraType_D40X,			// D40X
	kNkMAIDCameraType_D3 = 0x20,	// D3
	kNkMAIDCameraType_D300,			// D300
	kNkMAIDCameraType_D700 = 0x24,	// D700
	kNkMAIDCameraType_D3_FU1,		// D3 Firmup 1
	kNkMAIDCameraType_D3_FU2,		// D3 Firmup 2
	kNkMAIDCameraType_D300_FU,		// D300 Firmup
	kNkMAIDCameraType_D300S = 0x2A,	// D300S
	kNkMAIDCameraType_D3S,		    // D3S
	kNkMAIDCameraType_D3_FU3,		// D3 Firmup 3
	kNkMAIDCameraType_D700_FU1,		// D700 Firmup 1
	kNkMAIDCameraType_D7000,	    // D7000
	kNkMAIDCameraType_D5100,		// D5100
	kNkMAIDCameraType_D4 = 0x31		// D4
};

//	kNkMAIDCapability_EasyExposureCompMode(0x81d9)
enum eNkMAIDEasyExposureCompMode
{
	kNkMAIDEasyExposureCompMode_Off = 0,
	kNkMAIDEasyExposureCompMode_On,
	kNkMAIDEasyExposureCompMode_AutoReset
};

//	kNkMAIDCapability_ImageColorSpace(0x81db)
enum eNkMAIDImageColorSpace
{
	kNkMAIDImageColorSpace_sRGB = 0,
	kNkMAIDImageColorSpace_AdobeRGB
};

//	kNkMAIDCapability_NoiseReductionHighISO(0x81d8)
enum eNkMAIDNoiseReductionHighISO
{
	kNkMAIDNoiseReductionHighISO_Off	= 0,
	kNkMAIDNoiseReductionHighISO_Normal	= 1, 
	kNkMAIDNoiseReductionHighISO_High	= 2,
	kNkMAIDNoiseReductionHighISO_Low	= 3
};

//	kNkMAIDCapability_AFLockOnEx(0x81f4)
enum eNkMAIDAFLockOnEx
{
	kNkMAIDAFLockOnEx_High = 0,
	kNkMAIDAFLockOnEx_Normal,
	kNkMAIDAFLockOnEx_Low,
	kNkMAIDAFLockOnEx_Off,
	kNkMAIDAFLockOnEx_LittleHigh,
	kNkMAIDAFLockOnEx_LittleLow
};

//	kNkMAIDCapability_ISOAutoShutterTime(0x81c4)
enum eNkMAIDISOAutoShutterTime
{
	kNkMAIDISOAutoShutterTime_0 = 0,// 1/125
	kNkMAIDISOAutoShutterTime_1,	// 1/60
	kNkMAIDISOAutoShutterTime_2,	// 1/30
	kNkMAIDISOAutoShutterTime_3,	// 1/15
	kNkMAIDISOAutoShutterTime_4,	// 1/8
	kNkMAIDISOAutoShutterTime_5,	// 1/4
	kNkMAIDISOAutoShutterTime_6,	// 1/2
	kNkMAIDISOAutoShutterTime_7,	// 1
	kNkMAIDISOAutoShutterTime_8,	// 2
	kNkMAIDISOAutoShutterTime_9,	// 4
	kNkMAIDISOAutoShutterTime_10,	// 8
	kNkMAIDISOAutoShutterTime_11,	// 15
	kNkMAIDISOAutoShutterTime_12,	// 30
	kNkMAIDISOAutoShutterTime_13,	// 1/250
	kNkMAIDISOAutoShutterTime_14,	// 1/200
	kNkMAIDISOAutoShutterTime_15,	// 1/160
	kNkMAIDISOAutoShutterTime_16,	// 1/100
	kNkMAIDISOAutoShutterTime_17,	// 1/80
	kNkMAIDISOAutoShutterTime_18,	// 1/40
	kNkMAIDISOAutoShutterTime_19,	// 1/50
	kNkMAIDISOAutoShutterTime_20,	// 1/4000
	kNkMAIDISOAutoShutterTime_21,	// 1/3200 
	kNkMAIDISOAutoShutterTime_22,	// 1/2500 
	kNkMAIDISOAutoShutterTime_23,	// 1/2000
	kNkMAIDISOAutoShutterTime_24,	// 1/1600
	kNkMAIDISOAutoShutterTime_25,	// 1/1250
	kNkMAIDISOAutoShutterTime_26,	// 1/1000
	kNkMAIDISOAutoShutterTime_27,	// 1/800
	kNkMAIDISOAutoShutterTime_28,	// 1/640
	kNkMAIDISOAutoShutterTime_29,	// 1/500
	kNkMAIDISOAutoShutterTime_30,	// 1/400
	kNkMAIDISOAutoShutterTime_31,	// 1/320
	kNkMAIDISOAutoShutterTime_32	// auto
};

//	kNkMAIDCapability_ISOAutoHiLimit(0x81f2)
enum eNkMAIDISOAutoHiLimit
{
	kNkMAIDISOAutoHiLimit_ISO200 = 0,
	kNkMAIDISOAutoHiLimit_ISO400,
	kNkMAIDISOAutoHiLimit_ISO800,
	kNkMAIDISOAutoHiLimit_ISO1600,
	kNkMAIDISOAutoHiLimit_ISO3200,
	kNkMAIDISOAutoHiLimit_ISO6400,
	kNkMAIDISOAutoHiLimit_Hi1,
	kNkMAIDISOAutoHiLimit_Hi2
};

enum eNkMAIDISOAutoHiLimit2
{
	kNkMAIDISOAutoHiLimit2_ISO400 = 0,
	kNkMAIDISOAutoHiLimit2_ISO500,
	kNkMAIDISOAutoHiLimit2_ISO560,
	kNkMAIDISOAutoHiLimit2_ISO640,
	kNkMAIDISOAutoHiLimit2_ISO800,
	kNkMAIDISOAutoHiLimit2_ISO1000,
	kNkMAIDISOAutoHiLimit2_ISO1100,
	kNkMAIDISOAutoHiLimit2_ISO1250,
	kNkMAIDISOAutoHiLimit2_ISO1600,
	kNkMAIDISOAutoHiLimit2_ISO2000,
	kNkMAIDISOAutoHiLimit2_ISO2200,
	kNkMAIDISOAutoHiLimit2_ISO2500,
	kNkMAIDISOAutoHiLimit2_ISO3200,
	kNkMAIDISOAutoHiLimit2_ISO4000,
	kNkMAIDISOAutoHiLimit2_ISO4500,
	kNkMAIDISOAutoHiLimit2_ISO5000,
	kNkMAIDISOAutoHiLimit2_ISO6400,
	kNkMAIDISOAutoHiLimit2_ISO8000,
	kNkMAIDISOAutoHiLimit2_ISO9000,
	kNkMAIDISOAutoHiLimit2_ISO10000,
	kNkMAIDISOAutoHiLimit2_ISO12800,
	kNkMAIDISOAutoHiLimit2_ISOHi03,
	kNkMAIDISOAutoHiLimit2_ISOHi05,
	kNkMAIDISOAutoHiLimit2_ISOHi07,
	kNkMAIDISOAutoHiLimit2_ISOHi10,
	kNkMAIDISOAutoHiLimit2_ISOHi20,
	kNkMAIDISOAutoHiLimit2_ISOHi30
};

enum eNkMAIDISOAutoHiLimit3
{
	kNkMAIDISOAutoHiLimit3_ISO200 = 0,
	kNkMAIDISOAutoHiLimit3_ISO250,
	kNkMAIDISOAutoHiLimit3_ISO280,
	kNkMAIDISOAutoHiLimit3_ISO320,
	kNkMAIDISOAutoHiLimit3_ISO400,
	kNkMAIDISOAutoHiLimit3_ISO500,
	kNkMAIDISOAutoHiLimit3_ISO560,
	kNkMAIDISOAutoHiLimit3_ISO640,
	kNkMAIDISOAutoHiLimit3_ISO800,
	kNkMAIDISOAutoHiLimit3_ISO1000,
	kNkMAIDISOAutoHiLimit3_ISO1100,
	kNkMAIDISOAutoHiLimit3_ISO1250,
	kNkMAIDISOAutoHiLimit3_ISO1600,
	kNkMAIDISOAutoHiLimit3_ISO2000,
	kNkMAIDISOAutoHiLimit3_ISO2200,
	kNkMAIDISOAutoHiLimit3_ISO2500,
	kNkMAIDISOAutoHiLimit3_ISO3200,
	kNkMAIDISOAutoHiLimit3_ISO4000,
	kNkMAIDISOAutoHiLimit3_ISO4500,
	kNkMAIDISOAutoHiLimit3_ISO5000,
	kNkMAIDISOAutoHiLimit3_ISO6400,
	kNkMAIDISOAutoHiLimit3_ISOHi03,
	kNkMAIDISOAutoHiLimit3_ISOHi05,
	kNkMAIDISOAutoHiLimit3_ISOHi07,
	kNkMAIDISOAutoHiLimit3_ISOHi10,
	kNkMAIDISOAutoHiLimit3_ISOHi20,
	kNkMAIDISOAutoHiLimit3_ISO8000,
	kNkMAIDISOAutoHiLimit3_ISO9000,
	kNkMAIDISOAutoHiLimit3_ISO10000,
	kNkMAIDISOAutoHiLimit3_ISO12800,
	kNkMAIDISOAutoHiLimit3_ISOHi30,
	kNkMAIDISOAutoHiLimit3_ISOHi40
};

// kNkMAIDCapability_AFMode(0x81c3)
enum eNkMAIDAFMode
{
	kNkMAIDAFMode_S = 0,
	kNkMAIDAFMode_C,
	kNkMAIDAFMode_A,
	kNkMAIDAFMode_M_FIX,
	kNkMAIDAFMode_M_SEL
};

// kNkMAIDCapability_MonochromeFilterEffect(0x8220)
enum eNkMAIDMonochromeFilterEffect
{
	kNkMAIDMonochromeFilterEffect_NoBorder = 0,
	kNkMAIDMonochromeFilterEffect_Yellow,
	kNkMAIDMonochromeFilterEffect_Orange,
	kNkMAIDMonochromeFilterEffect_Red,
	kNkMAIDMonochromeFilterEffect_Green
};


// kNkMAIDCapability_InternalSplMRPTValue(0x81f7)
enum eNkMAIDInternalSplMRPTValue
{
	kNkMAIDInternalSplMRPTValue_4 = 0,
	kNkMAIDInternalSplMRPTValue_8,
	kNkMAIDInternalSplMRPTValue_16,
	kNkMAIDInternalSplMRPTValue_32,
	kNkMAIDInternalSplMRPTValue_64,
	kNkMAIDInternalSplMRPTValue_128,
	kNkMAIDInternalSplMRPTValue_Full
};

// kNkMAIDCapability_FlashMode(0x0035)
enum eNkMAIDFlashModeDX2
{
	kNkMAIDFlashMode_VendorBaseDx2 = kNkMAIDFlashMode_SlowSyncRearCurtain + 0x100,
	kNkMAIDFlashMode_Off,
	kNkMAIDFlashMode_Auto,
	kNkMAIDFlashMode_On
};

// kNkMAIDCapability_ImageSetting(0x81c0)
enum eNkMAIDImageSetting
{
	kNkMAIDImageSetting_Normal = 0,
	kNkMAIDImageSetting_Clear,
	kNkMAIDImageSetting_Sharp,
	kNkMAIDImageSetting_Soft,
	kNkMAIDImageSetting_DirectPrint,
	kNkMAIDImageSetting_Portrait,
	kNkMAIDImageSetting_Landscape,
	kNkMAIDImageSetting_Custom,
	kNkMAIDImageSetting_Black_and_White,
	kNkMAIDImageSetting_MoreClear
};

// kNkMAIDCapability_SaturationSetting(0x81c1)
enum eNkMAIDSaturationSetting
{
	kNkMAIDSaturationSetting_Normal = 0,
	kNkMAIDSaturationSetting_Low,
	kNkMAIDSaturationSetting_High,
	kNkMAIDSaturationSetting_Auto
};

// kNkMAIDCapability_InternalFlashStatus(0x81bc)
enum eNkMAIDInternalFlashStatus
{
	kNkMAIDInternalFlashStatus_Ready = 0,
	kNkMAIDInternalFlashStatus_NotReady,
	kNkMAIDInternalFlashStatus_Close,
	kNkMAIDInternalFlashStatus_TTL
};

// kNkMAIDCapability_RemoteTimer(0x81c8)
enum eNkMAIDRemoteTimer
{
	kNkMAIDRemoteTimer_1min = 0,
	kNkMAIDRemoteTimer_5min,
	kNkMAIDRemoteTimer_10min,
	kNkMAIDRemoteTimer_15min
};

// kNkMAIDCapability_BeepEx(0x81f3)
enum eNkMAIDBeepEx
{
	kNkMAIDBeepEx_High = 0,
	kNkMAIDBeepEx_Low,
	kNkMAIDBeepEx_Off,
	kNkMAIDBeepEx_On
};

// kNkMAIDCapability_CellKind(0x81f6)
enum eNkMAIDCellKind
{
	kNkMAIDCellKind_Alkaline = 0,
	kNkMAIDCellKind_NiMH,
	kNkMAIDCellKind_Lithium,
	kNkMAIDCellKind_NiMn
};

// kNkMAIDCapability_ExternalFlashStatus(0x81bd)
enum eNkMAIDExternalFlashStatus
{
	kNkMAIDExternalFlashStatus_Ready = 0,
	kNkMAIDExternalFlashStatus_NotReady,
	kNkMAIDExternalFlashStatus_NotExist
};

// kNkMAIDCapability_FocusAreaFrame(0x81f1)
enum eNkMAIDFocusAreaFrame
{
	kNkMAIDFocusAreaFrame_Normal = 0,
	kNkMAIDFocusAreaFrame_Wide
};

// kNkMAIDCapability_InternalSplMRPTCount(0x81f8)
enum eNkMAIDInternalSplMRPTCount
{
	kNkMAIDInternalSplMRPTCount_2 = 0,
	kNkMAIDInternalSplMRPTCount_3,
	kNkMAIDInternalSplMRPTCount_4,
	kNkMAIDInternalSplMRPTCount_5,
	kNkMAIDInternalSplMRPTCount_6,
	kNkMAIDInternalSplMRPTCount_7,
	kNkMAIDInternalSplMRPTCount_8,
	kNkMAIDInternalSplMRPTCount_9,
	kNkMAIDInternalSplMRPTCount_10,
	kNkMAIDInternalSplMRPTCount_15,
	kNkMAIDInternalSplMRPTCount_20,
	kNkMAIDInternalSplMRPTCount_25,
	kNkMAIDInternalSplMRPTCount_30,
	kNkMAIDInternalSplMRPTCount_35
};

// kNkMAIDCapability_InternalSplMRPTInterval(0x81f9)
enum eNkMAIDInternalSplMRPTInterval
{
	kNkMAIDInternalSplMRPTInterval_1 = 0,
	kNkMAIDInternalSplMRPTInterval_2,
	kNkMAIDInternalSplMRPTInterval_3,
	kNkMAIDInternalSplMRPTInterval_4,
	kNkMAIDInternalSplMRPTInterval_5,
	kNkMAIDInternalSplMRPTInterval_6,
	kNkMAIDInternalSplMRPTInterval_7,
	kNkMAIDInternalSplMRPTInterval_8,
	kNkMAIDInternalSplMRPTInterval_9,
	kNkMAIDInternalSplMRPTInterval_10,
	kNkMAIDInternalSplMRPTInterval_20,
	kNkMAIDInternalSplMRPTInterval_30,
	kNkMAIDInternalSplMRPTInterval_40,
	kNkMAIDInternalSplMRPTInterval_50
};

// kNkMAIDCapability_InternalSplCommandChannel(0x81fa)
enum eNkMAIDInternalSplCommandChannel
{
	kNkMAIDInternalSplCommandChannel_1 = 0,
	kNkMAIDInternalSplCommandChannel_2,
	kNkMAIDInternalSplCommandChannel_3,
	kNkMAIDInternalSplCommandChannel_4
};

// kNkMAIDCapability_InternalSplCmdGroupAComp(0x81fd)
// kNkMAIDCapability_InternalSplCmdGroupBComp(0x8200)
enum eNkMAIDInternalSplCmdSelfComp
{
	kNkMAIDInternalSplCmdSelfComp_M30 = 0,
	kNkMAIDInternalSplCmdSelfComp_M27,
	kNkMAIDInternalSplCmdSelfComp_M23,
	kNkMAIDInternalSplCmdSelfComp_M20,
	kNkMAIDInternalSplCmdSelfComp_M17,
	kNkMAIDInternalSplCmdSelfComp_M13,
	kNkMAIDInternalSplCmdSelfComp_M10,
	kNkMAIDInternalSplCmdSelfComp_M07,
	kNkMAIDInternalSplCmdSelfComp_M03,
	kNkMAIDInternalSplCmdSelfComp_0,
	kNkMAIDInternalSplCmdSelfComp_P03,
	kNkMAIDInternalSplCmdSelfComp_P07,
	kNkMAIDInternalSplCmdSelfComp_P10,
	kNkMAIDInternalSplCmdSelfComp_P13,
	kNkMAIDInternalSplCmdSelfComp_P17,
	kNkMAIDInternalSplCmdSelfComp_P20,
	kNkMAIDInternalSplCmdSelfComp_P23,
	kNkMAIDInternalSplCmdSelfComp_P27,
	kNkMAIDInternalSplCmdSelfComp_P30
};

// kNkMAIDCapability_InternalSplCmdGroupAMode(0x81fc)
// kNkMAIDCapability_InternalSplCmdGroupBMode(0x81ff)
enum eNkMAIDInternalSplCmdGroupMode
{
	kNkMAIDInternalSplCmdGroupMode_TTL = 0,
	kNkMAIDInternalSplCmdGroupMode_AA,
	kNkMAIDInternalSplCmdGroupMode_Manual,
	kNkMAIDInternalSplCmdGroupMode_Off
};

// kNkMAIDCapability_InternalSplCmdGroupAComp(0x81fd)
// kNkMAIDCapability_InternalSplCmdGroupBComp(0x8200)
enum eNkMAIDInternalSplCmdGroupComp
{
	kNkMAIDInternalSplCmdGroupComp_M30 = 0,	// -3.0
	kNkMAIDInternalSplCmdGroupComp_M27,		// -2.7
	kNkMAIDInternalSplCmdGroupComp_M23,		// -2.3
	kNkMAIDInternalSplCmdGroupComp_M20,		// -2.0
	kNkMAIDInternalSplCmdGroupComp_M17,		// -1.7
	kNkMAIDInternalSplCmdGroupComp_M13,		// -1.3
	kNkMAIDInternalSplCmdGroupComp_M10,		// -1.0
	kNkMAIDInternalSplCmdGroupComp_M07,		// -0.7
	kNkMAIDInternalSplCmdGroupComp_M03,		// -0.3
	kNkMAIDInternalSplCmdGroupComp_0,		// 0
	kNkMAIDInternalSplCmdGroupComp_P03,		// +0.3
	kNkMAIDInternalSplCmdGroupComp_P07,		// +0.7
	kNkMAIDInternalSplCmdGroupComp_P10,		// +1.0
	kNkMAIDInternalSplCmdGroupComp_P13,		// +1.3
	kNkMAIDInternalSplCmdGroupComp_P17,		// +1.7
	kNkMAIDInternalSplCmdGroupComp_P20,		// +2.0
	kNkMAIDInternalSplCmdGroupComp_P23,		// +2.3
	kNkMAIDInternalSplCmdGroupComp_P27,		// +2.7
	kNkMAIDInternalSplCmdGroupComp_P30		// +3.0
};

// kNkMAIDCapability_InternalSplCmdGroupAValue(0x81fe)
// kNkMAIDCapability_InternalSplCmdGroupBValue(0x8201)
enum eNkMAIDInternalSplCmdGroupValue
{
	kNkMAIDInternalSplCmdGroupValue_1 = 0,	// 1/1
	kNkMAIDInternalSplCmdGroupValue_2,		// 1/2
	kNkMAIDInternalSplCmdGroupValue_4,		// 1/4
	kNkMAIDInternalSplCmdGroupValue_8,		// 1/8
	kNkMAIDInternalSplCmdGroupValue_16,		// 1/16
	kNkMAIDInternalSplCmdGroupValue_32,		// 1/32
	kNkMAIDInternalSplCmdGroupValue_64,		// 1/64
	kNkMAIDInternalSplCmdGroupValue_128,	// 1/128
	kNkMAIDInternalSplCmdGroupValue_1_3,	// 1/1.3
	kNkMAIDInternalSplCmdGroupValue_1_7,	// 1/1.7
	kNkMAIDInternalSplCmdGroupValue_2_5,	// 1/2.5
	kNkMAIDInternalSplCmdGroupValue_3_2,	// 1/3.2
	kNkMAIDInternalSplCmdGroupValue_5,		// 1/5
	kNkMAIDInternalSplCmdGroupValue_6_4,	// 1/6.4
	kNkMAIDInternalSplCmdGroupValue_10,		// 1/10
	kNkMAIDInternalSplCmdGroupValue_13,		// 1/13
	kNkMAIDInternalSplCmdGroupValue_20,		// 1/20
	kNkMAIDInternalSplCmdGroupValue_25,		// 1/25
	kNkMAIDInternalSplCmdGroupValue_40,		// 1/40
	kNkMAIDInternalSplCmdGroupValue_50,		// 1/50
	kNkMAIDInternalSplCmdGroupValue_80,		// 1/80
	kNkMAIDInternalSplCmdGroupValue_100		// 1/100
};

// kNkMAIDCapability_InternalSplCmdSelfMode(0x8202)
enum eNkMAIDInternalSplCmdSelfMode
{
	kNkMAIDInternalSplCmdSelfMode_TTL = 0,
	kNkMAIDInternalSplCmdSelfMode_Manual,
	kNkMAIDInternalSplCmdSelfMode_Off
};

// kNkMAIDCapability_InternalSplCmdSelfValue(0x8203)
enum eNkMAIDInternalSplCmdSelfValue
{
	kNkMAIDInternalSplCmdSelfValue_1 = 0,	// 1/1
	kNkMAIDInternalSplCmdSelfValue_2,		// 1/2
	kNkMAIDInternalSplCmdSelfValue_4,		// 1/4
	kNkMAIDInternalSplCmdSelfValue_8,		// 1/8
	kNkMAIDInternalSplCmdSelfValue_16,		// 1/16
	kNkMAIDInternalSplCmdSelfValue_32,		// 1/32
	kNkMAIDInternalSplCmdSelfValue_64,		// 1/64
	kNkMAIDInternalSplCmdSelfValue_128,		// 1/128
	kNkMAIDInternalSplCmdSelfValue_1_3,		// 1/1.3
	kNkMAIDInternalSplCmdSelfValue_1_7,		// 1/1.7
	kNkMAIDInternalSplCmdSelfValue_2_5,		// 1/2.5
	kNkMAIDInternalSplCmdSelfValue_3_2,		// 1/3.2
	kNkMAIDInternalSplCmdSelfValue_5,		// 1/5
	kNkMAIDInternalSplCmdSelfValue_6_4,		// 1/6.4
	kNkMAIDInternalSplCmdSelfValue_10,		// 1/10
	kNkMAIDInternalSplCmdSelfValue_13,		// 1/13
	kNkMAIDInternalSplCmdSelfValue_20,		// 1/20
	kNkMAIDInternalSplCmdSelfValue_25,		// 1/25
	kNkMAIDInternalSplCmdSelfValue_40,		// 1/40
	kNkMAIDInternalSplCmdSelfValue_50,		// 1/50
	kNkMAIDInternalSplCmdSelfValue_80,		// 1/80
	kNkMAIDInternalSplCmdSelfValue_100		// 1/100
};

// kNkMAIDCapability_InternalSplValue(0x81c5)
enum eNkMAIDInternalSplValue
{
	kNkMAIDInternalSplValue_Full =0,	// Full
	kNkMAIDInternalSplValue_2,			// 1/2
	kNkMAIDInternalSplValue_4,			// 1/4
	kNkMAIDInternalSplValue_8,			// 1/8
	kNkMAIDInternalSplValue_16,			// 1/16
	kNkMAIDInternalSplValue_32,			// 1/32
	kNkMAIDInternalSplValue_64,			// 1/64
	kNkMAIDInternalSplValue_128,		// 1/128
	kNkMAIDInternalSplValue_1_3,		// 1/1.3
	kNkMAIDInternalSplValue_1_7,		// 1/1.7
	kNkMAIDInternalSplValue_2_5,		// 1/2.5
	kNkMAIDInternalSplValue_3_2,		// 1/3.2
	kNkMAIDInternalSplValue_5,			// 1/5
	kNkMAIDInternalSplValue_6_4,		// 1/6.4
	kNkMAIDInternalSplValue_10,			// 1/10
	kNkMAIDInternalSplValue_13,			// 1/13
	kNkMAIDInternalSplValue_20,			// 1/20
	kNkMAIDInternalSplValue_25,			// 1/25
	kNkMAIDInternalSplValue_40,			// 1/40
	kNkMAIDInternalSplValue_50,			// 1/50
	kNkMAIDInternalSplValue_80,			// 1/80
	kNkMAIDInternalSplValue_100,		// 1/100
	kNkMAIDInternalSplValue_256			// 1/256
};

// kNkMAIDCapability_MonochromeSettingType(0x8224)
enum eNkMAIDMonochromeSettingType
{
	kNkMAIDMonochromeSettingType_Standard = 0,
	kNkMAIDMonochromeSettingType_Custom
};

// kNkMAIDCapability_WarningDisp(0x81f5)
enum eNkMAIDWarningDisp
{
	kNkMAIDWarningDisp_On = 0,
	kNkMAIDWarningDisp_Off
};

// kNkMAIDCapability_AutoOffTimer(0x8226)
enum eNkMAIDAutoOffTimer
{
	kNkMAIDAutoOffTimer_Short = 0,
	kNkMAIDAutoOffTimer_Normal,
	kNkMAIDAutoOffTimer_Long,
	kNkMAIDAutoOffTimer_Custom
};

// kNkMAIDCapability_ExternalSplValue(0x822a)
enum eNkMAIDExternalSplValue
{
	kNkMAIDExternalSplValue_Full =0,	// Full
	kNkMAIDExternalSplValue_2,			// 1/2
	kNkMAIDExternalSplValue_4,			// 1/4
	kNkMAIDExternalSplValue_8,			// 1/8
	kNkMAIDExternalSplValue_16,			// 1/16
	kNkMAIDExternalSplValue_32,			// 1/32
	kNkMAIDExternalSplValue_64,			// 1/64
	kNkMAIDExternalSplValue_128,		// 1/128
	kNkMAIDExternalSplValue_256			// 1/256
};

// kNkMAIDCapability_FocusPreferredArea(0x8122)
enum eNkMAIDFocusPreferred2
{
	kNkMAIDFocusPreferred2_Unknown = 0,
	kNkMAIDFocusPreferred2_C = 1,
	kNkMAIDFocusPreferred2_CT,
	kNkMAIDFocusPreferred2_CTT,
	kNkMAIDFocusPreferred2_CB,
	kNkMAIDFocusPreferred2_CBB,
	kNkMAIDFocusPreferred2_CR,
	kNkMAIDFocusPreferred2_CRT,
	kNkMAIDFocusPreferred2_CRTT,
	kNkMAIDFocusPreferred2_CRB,
	kNkMAIDFocusPreferred2_CRBB,
	kNkMAIDFocusPreferred2_CL,
	kNkMAIDFocusPreferred2_CLT,
	kNkMAIDFocusPreferred2_CLTT,
	kNkMAIDFocusPreferred2_CLB,
	kNkMAIDFocusPreferred2_CLBB,
	kNkMAIDFocusPreferred2_RA,
	kNkMAIDFocusPreferred2_RAT,
	kNkMAIDFocusPreferred2_RATT,
	kNkMAIDFocusPreferred2_RAB,
	kNkMAIDFocusPreferred2_RABB,
	kNkMAIDFocusPreferred2_RB,
	kNkMAIDFocusPreferred2_RBT,
	kNkMAIDFocusPreferred2_RBTT,
	kNkMAIDFocusPreferred2_RBB,
	kNkMAIDFocusPreferred2_RBBB,
	kNkMAIDFocusPreferred2_RC,
	kNkMAIDFocusPreferred2_RCT,
	kNkMAIDFocusPreferred2_RCTT,
	kNkMAIDFocusPreferred2_RCB,
	kNkMAIDFocusPreferred2_RCBB,
	kNkMAIDFocusPreferred2_RD,
	kNkMAIDFocusPreferred2_RDT,
	kNkMAIDFocusPreferred2_RDB,
	kNkMAIDFocusPreferred2_LA,
	kNkMAIDFocusPreferred2_LAT,
	kNkMAIDFocusPreferred2_LATT,
	kNkMAIDFocusPreferred2_LAB,
	kNkMAIDFocusPreferred2_LABB,
	kNkMAIDFocusPreferred2_LB,
	kNkMAIDFocusPreferred2_LBT,
	kNkMAIDFocusPreferred2_LBTT,
	kNkMAIDFocusPreferred2_LBB,
	kNkMAIDFocusPreferred2_LBBB,
	kNkMAIDFocusPreferred2_LC,
	kNkMAIDFocusPreferred2_LCT,
	kNkMAIDFocusPreferred2_LCTT,
	kNkMAIDFocusPreferred2_LCB,
	kNkMAIDFocusPreferred2_LCBB,
	kNkMAIDFocusPreferred2_LD,
	kNkMAIDFocusPreferred2_LDT,
	kNkMAIDFocusPreferred2_LDB
};

// kNkMAIDCapability_FocusPreferredArea(0x8122)
enum eNkMAIDFocusPreferred3
{
	kNkMAIDFocusPreferred3_Unknown = 0,
	kNkMAIDFocusPreferred3_Center,
	kNkMAIDFocusPreferred3_Upper,
	kNkMAIDFocusPreferred3_Bottom,
	kNkMAIDFocusPreferred3_Left,
	kNkMAIDFocusPreferred3_UpperLeft,
	kNkMAIDFocusPreferred3_BottomLeft,
	kNkMAIDFocusPreferred3_LeftEnd,
	kNkMAIDFocusPreferred3_Right,
	kNkMAIDFocusPreferred3_UpperRight,
	kNkMAIDFocusPreferred3_BottomRight,
	kNkMAIDFocusPreferred3_RightEnd
};

// kNkMAIDCapability_FocusPreferredArea(0x8122)
enum eNkMAIDFocusPreferred4
{
	kNkMAIDFocusPreferred4_Unknown = 0,
	kNkMAIDFocusPreferred4_C = 1,
	kNkMAIDFocusPreferred4_CT,
	kNkMAIDFocusPreferred4_CTT,
	kNkMAIDFocusPreferred4_CB,
	kNkMAIDFocusPreferred4_CBB,
	kNkMAIDFocusPreferred4_CR,
	kNkMAIDFocusPreferred4_CRT,
	kNkMAIDFocusPreferred4_CRTT,
	kNkMAIDFocusPreferred4_CRB,
	kNkMAIDFocusPreferred4_CRBB,
	kNkMAIDFocusPreferred4_CL,
	kNkMAIDFocusPreferred4_CLT,
	kNkMAIDFocusPreferred4_CLTT,
	kNkMAIDFocusPreferred4_CLB,
	kNkMAIDFocusPreferred4_CLBB,
	kNkMAIDFocusPreferred4_RA,
	kNkMAIDFocusPreferred4_RAT,
	kNkMAIDFocusPreferred4_RAB,
	kNkMAIDFocusPreferred4_RB,
	kNkMAIDFocusPreferred4_RBT,
	kNkMAIDFocusPreferred4_RBB,
	kNkMAIDFocusPreferred4_RC,
	kNkMAIDFocusPreferred4_RCT,
	kNkMAIDFocusPreferred4_RCB,
	kNkMAIDFocusPreferred4_RD,
	kNkMAIDFocusPreferred4_RDT,
	kNkMAIDFocusPreferred4_RDB,
	kNkMAIDFocusPreferred4_LA,
	kNkMAIDFocusPreferred4_LAT,
	kNkMAIDFocusPreferred4_LAB,
	kNkMAIDFocusPreferred4_LB,
	kNkMAIDFocusPreferred4_LBT,
	kNkMAIDFocusPreferred4_LBB,
	kNkMAIDFocusPreferred4_LC,
	kNkMAIDFocusPreferred4_LCT,
	kNkMAIDFocusPreferred4_LCB,
	kNkMAIDFocusPreferred4_LD,
	kNkMAIDFocusPreferred4_LDT,
	kNkMAIDFocusPreferred4_LDB
};

// kNkMAIDCapability_CompressRAWEx(0x824b)
enum eNkMAIDCompressRAWEx
{
	kNkMAIDCompressRAWEx_Uncompressed		= 0,
	kNkMAIDCompressRAWEx_Compressed			= 1,
	kNkMAIDCompressRAWEx_LosslessCompressed = 2
};

// kNkMAIDCapability_Slot2ImageSaveMode(0x822c)
enum eNkMAIDSlot2ImageSaveMode
{
	kNkMAIDSlot2ImageSaveMode_Overflow = 0,
	kNkMAIDSlot2ImageSaveMode_Backup = 1,
	kNkMAIDSlot2ImageSaveMode_Jpeg = 2
};

// kNkMAIDCapability_CompressRAWBitMode(0x822d)
enum eNkMAIDCompressRAWBitMode
{
	kNkMAIDCompressRAWBitMode_12bit = 0,
	kNkMAIDCompressRAWBitMode_14bit = 1,
};

// kNkMAIDCapability_PictureControl(0x822e)
enum eNkMAIDPictureControl
{
	kNkMAIDPictureControl_Undefined  = 0,
	kNkMAIDPictureControl_Standard   = 1,
	kNkMAIDPictureControl_Neutral    = 2,
	kNkMAIDPictureControl_Vivid      = 3,
	kNkMAIDPictureControl_Monochrome = 4,
	kNkMAIDPictureControl_Portrait   = 5,
	kNkMAIDPictureControl_Landscape  = 6,
	kNkMAIDPictureControl_Option1	 = 101,
	kNkMAIDPictureControl_Option2	 = 102,
	kNkMAIDPictureControl_Option3	 = 103,
	kNkMAIDPictureControl_Option4	 = 104,
	kNkMAIDPictureControl_Custom1	 = 201,
	kNkMAIDPictureControl_Custom2	 = 202,
	kNkMAIDPictureControl_Custom3	 = 203,
	kNkMAIDPictureControl_Custom4	 = 204,
	kNkMAIDPictureControl_Custom5	 = 205,
	kNkMAIDPictureControl_Custom6	 = 206,
	kNkMAIDPictureControl_Custom7	 = 207,
	kNkMAIDPictureControl_Custom8	 = 208,
	kNkMAIDPictureControl_Custom9	 = 209
};

// kNkMAIDCapability_Active_D_Lighting(0x8232)
enum eNkMAIDActive_D_Lighting
{
	kNkMAIDActive_D_Lighting_High	    = 0,
	kNkMAIDActive_D_Lighting_Normal     = 1,
	kNkMAIDActive_D_Lighting_Low	    = 2,
	kNkMAIDActive_D_Lighting_Off	    = 3,
	kNkMAIDActive_D_Lighting_On		    = 4,
	kNkMAIDActive_D_Lighting_ExtraHigh  = 5,
	kNkMAIDActive_D_Lighting_Auto	    = 6,
	kNkMAIDActive_D_Lighting_ExtraHigh2	= 7
};

// kNkMAIDCapability_LiveViewMode(0x823c)
enum eNkMAIDLiveViewMode
{
	kNkMAIDLiveViewMode_Handheld	= 0,
	kNkMAIDLiveViewMode_Tripod		= 1
};

// kNkMAIDCapability_LiveViewDriveMode(0x823d)
enum eNkMAIDLiveViewDriveMode
{
	kNkMAIDLiveViewDriveMode_Single			= 0,
	kNkMAIDLiveViewDriveMode_ContinuousLow	= 1,
	kNkMAIDLiveViewDriveMode_ContinuousHigh = 2
};

// kNkMAIDCapability_DynamicAFArea(0x8233)
enum eNkMAIDDynamicAFArea
{
	kNkMAIDDynamicAFArea_9				= 0,
	kNkMAIDDynamicAFArea_21				= 1,
	kNkMAIDDynamicAFArea_51				= 2,
	kNkMAIDDynamicAFArea_51_3DTtracking = 3
};

// kNkMAIDCapability_InfoDispSetting(0x8235)
enum eNkMAIDInfoDispSetting
{
	kNkMAIDInfoDispSetting_Auto				= 0,
	kNkMAIDInfoDispSetting_M_DarkOnLight	= 1,
	kNkMAIDInfoDispSetting_M_LightOnDark	= 2
};

// kNkMAIDCapability_IndicatorDisplay(0x823b)
enum eNkMAIDIndicatorDisplay
{
	kNkMAIDIndicatorDisplay_Plus	= 0,
	kNkMAIDIndicatorDisplay_Minus	= 1
};

// kNkMAIDCapability_LiveViewImageZoomRate(0x823f)
enum eNkMAIDLiveViewImageZoomRate
{
	kNkMAIDLiveViewImageZoomRate_All	= 0, //Full
	kNkMAIDLiveViewImageZoomRate_25		= 1, //25%
	kNkMAIDLiveViewImageZoomRate_33		= 2, //33%
	kNkMAIDLiveViewImageZoomRate_50		= 3, //50%
	kNkMAIDLiveViewImageZoomRate_66		= 4, //66.7%
	kNkMAIDLiveViewImageZoomRate_100	= 5, //100%
	kNkMAIDLiveViewImageZoomRate_200	= 6  //200%
};

// kNkMAIDCapability_MFDrive(0x8249)
enum eNkMAIDMFDrive
{
	kNkMAIDMFDrive_InfinityToClosest = 0,
	kNkMAIDMFDrive_ClosestToInfinity = 1
};

// kNkMAIDCapability_LiveViewStatus(0x823e)
enum eNkMAIDLiveViewStatus
{
	kNkMAIDLiveViewStatus_OFF = 0,
	kNkMAIDLiveViewStatus_ON
};

// kNkMAIDCapability_CellKindPriority(0x824c)
enum eNkMAIDCellKindPriority
{
	kNkMAIDCellKindPriority_MB_D10 = 0,
	kNkMAIDCellKindPriority_Camera,
	kNkMAIDCellKindPriority_MB_D11 = 0
};

// kNkMAIDCapability_AFAreaPoint(0x8254)
enum eNkMAIDAFAreaPoint
{
	kNkMAIDAFAreaPoint_51 = 0,
	kNkMAIDAFAreaPoint_11,
	kNkMAIDAFAreaPoint_39
};

// kNkMAIDCapability_WBFluorescentType(0x824d)
enum eNkWBFluorescentType
{
	kNkWBFluorescentType_SodiumVapor = 0,
	kNkWBFluorescentType_WarmWhite, 
	kNkWBFluorescentType_White, 
	kNkWBFluorescentType_CoolWhite, 
	kNkWBFluorescentType_DayWhite, 
	kNkWBFluorescentType_Daylight, 
	kNkWBFluorescentType_HiTempMercuryVapor
};

// kNkMAIDCapability_ContrastAF(0x8240)
enum eNkMAIDContrastAF
{
	kNkMAIDContrastAF_Start			= 0x00, 
	kNkMAIDContrastAF_Stop			= 0x01,
	kNkMAIDContrastAF_OK			= 0x10,
	kNkMAIDContrastAF_OutOfFocus	= 0x11,
	kNkMAIDContrastAF_Busy			= 0x12
};

// kNkMAIDCapability_LensType(0x825c)
enum eNkMAIDLensType
{
	kNkMAIDLensType_D		= 0x00000001,	//D type
	kNkMAIDLensType_G		= 0x00000010,	//G type
	kNkMAIDLensType_VR		= 0x00000100,	//VR
	kNkMAIDLensType_DX		= 0x00001000,	//DX
	kNkMAIDLensType_AFS		= 0x00010000,	//AF-S
	kNkMAIDLensType_AD		= 0x00100000	//Auto Distortion
};

// kNkMAIDCapability_LiveViewProhibit(0x825e)
enum eNkMAIDLiveViewProhibit
{
	kNkMAIDLiveViewProhibit_DuringMirrorup = 0x400000,
	kNkMAIDLiveViewProhibit_BulbWarning	= 0x200000,
	kNkMAIDLiveViewProhibit_CardUnformat= 0x100000,
	kNkMAIDLiveViewProhibit_CardError   = 0x80000,
	kNkMAIDLiveViewProhibit_CardProtect = 0x40000,
	kNkMAIDLiveViewProhibit_TempRise    = 0x20000,
	kNkMAIDLiveViewProhibit_EffectMode  = 0x10000,
	kNkMAIDLiveViewProhibit_Capture     = 0x8000,
	kNkMAIDLiveViewProhibit_NoCardLock	= 0x4000,
	kNkMAIDLiveViewProhibit_MirrorMode	= 0x2000,
	kNkMAIDLiveViewProhibit_SdramImg	= 0x1000,
	kNkMAIDLiveViewProhibit_NonCPU		= 0x800,
	kNkMAIDLiveViewProhibit_ApertureRing= 0x400,
	kNkMAIDLiveViewProhibit_TTL			= 0x200,
	kNkMAIDLiveViewProhibit_Battery		= 0x100,
	kNkMAIDLiveViewProhibit_Mirrorup	= 0x80,
	kNkMAIDLiveViewProhibit_Bulb		= 0x40,
	kNkMAIDLiveViewProhibit_FEE			= 0x20,
	kNkMAIDLiveViewProhibit_Button		= 0x10,
	kNkMAIDLiveViewProhibit_Sequence	= 0x04,
	kNkMAIDLiveViewProhibit_CF			= 0x01
};

// kNkMAIDCapability_ScreenTips(0x8268)
enum eNkMAIDScreenTips
{
	kNkMAIDScreenTips_On = 0,
	kNkMAIDScreenTips_Off
};

// kNkMAIDCapability_IlluminationSetting(0x8269)
enum eNkMAIDIlluminationSetting
{
	kNkMAIDIlluminationSetting_LCDBacklight = 0,
	kNkMAIDIlluminationSetting_Both
};

// kNkMAIDCapability_ShutterSpeedLockSetting(0x826a)
enum eNkMAIDShutterSpeedLockSetting
{
	kNkMAIDShutterSpeedLockSetting_Off = 0,
	kNkMAIDShutterSpeedLockSetting_On
};

// kNkMAIDCapability_ApertureLockSetting(0x826b)
enum eNkMAIDApertureLockSetting
{
	kNkMAIDApertureLockSetting_Off = 0,
	kNkMAIDApertureLockSetting_On
};

// kNkMAIDCapability_VignetteControl(0x826c)
enum eNkMAIDVignetteControl
{
	kNkMAIDVignetteControl_High	  = 0,
	kNkMAIDVignetteControl_Normal = 1,
	kNkMAIDVignetteControl_Low	  = 2,
	kNkMAIDVignetteControl_Off	  = 3,
};

// kNkMAIDCapability_FocusPointBrightness(0x826d)
enum eNkMAIDFocusPointBrightness
{
	kNkMAIDFocusPointBrightness_Low       = 0,
	kNkMAIDFocusPointBrightness_Normal    = 1,
	kNkMAIDFocusPointBrightness_High      = 2,
	kNkMAIDFocusPointBrightness_ExtraHigh = 3
};

// kNkMAIDCapability_EnableCommandDialOnPlaybackEx(0x8278)
enum eNkMAIDEnableCommandDialOnPlaybackEx
{
	kNkMAIDEnableCommandDialOnPlaybackEx_Off = 0,
	kNkMAIDEnableCommandDialOnPlaybackEx_On  = 1,
};

// kNkMAIDCapability_MovieScreenSize(0x8272)
enum eNkMAIDMovieScreenSize
{
	kNkMAIDMovieScreenSize_QVGA = 0,
	kNkMAIDMovieScreenSize_VGA,
	kNkMAIDMovieScreenSize_720p,
	kNkMAIDMovieScreenSize_VGA_Normal,
	kNkMAIDMovieScreenSize_VGA_Fine,
	kNkMAIDMovieScreenSize_720p_Normal_Lowfps,
	kNkMAIDMovieScreenSize_720p_Fine_Lowfps,
	kNkMAIDMovieScreenSize_720p_Normal_Highfps,
	kNkMAIDMovieScreenSize_720p_Fine_Highfps,
	kNkMAIDMovieScreenSize_FullHD_Normal,
	kNkMAIDMovieScreenSize_FullHD_Fine,
	kNkMAIDMovieScreenSize_FullHD_Normal_Highfps,
	kNkMAIDMovieScreenSize_FullHD_Fine_Highfps
};

// kNkMAIDCapability_MovieScreenSize(0x8272)
enum eNkMAIDMovieScreenSize2
{
	kNkMAIDMovieScreenSize2_FullHD_30fps = 0,
	kNkMAIDMovieScreenSize2_FullHD_25fps,
	kNkMAIDMovieScreenSize2_FullHD_24fps,
	kNkMAIDMovieScreenSize2_720p_60fps,
	kNkMAIDMovieScreenSize2_720p_50fps,
	kNkMAIDMovieScreenSize2_720p_30fps,
	kNkMAIDMovieScreenSize2_720p_25fps,
	kNkMAIDMovieScreenSize_VGA_30fps,
	kNkMAIDMovieScreenSize_VGA_25fps,
	kNkMAIDMovieScreenSize2_FullHD_30fps_Crop,
	kNkMAIDMovieScreenSize2_FullHD_25fps_Crop,
	kNkMAIDMovieScreenSize2_FullHD_24fps_Crop
};

// kNkMAIDCapability_MovieVoice(0x8273)
enum eNkMAIDMovieVoice
{
	kNkMAIDMovieVoice_Off = 0,
	kNkMAIDMovieVoice_On
};

// kNkMAIDCapability_LiveViewAF(0x8275)
enum eNkMAIDLiveViewAF
{
	kNkMAIDLiveViewAF_Face = 0,
	kNkMAIDLiveViewAF_Wide,
	kNkMAIDLiveViewAF_Normal,
	kNkMAIDLiveViewAF_SubjectTracking
};

// kNkMAIDCapability_SelfTimerShootNum(0x8276)
enum eNkMAIDSelfTimerShootNum
{
	kNkMAIDSelfTimerShootNum_1 = 0,
	kNkMAIDSelfTimerShootNum_2,
	kNkMAIDSelfTimerShootNum_3,
	kNkMAIDSelfTimerShootNum_4,
	kNkMAIDSelfTimerShootNum_5,
	kNkMAIDSelfTimerShootNum_6,
	kNkMAIDSelfTimerShootNum_7,
	kNkMAIDSelfTimerShootNum_8,
	kNkMAIDSelfTimerShootNum_9
};

// kNkMAIDCapability_FinderISODisplay(0x8277)
enum eNkMAIDFinderISODisplay
{
	kNkMAIDFinderISODisplay_ISO = 0,
	kNkMAIDFinderISODisplay_EasyISO,
	kNkMAIDFinderISODisplay_FrameCount
};

// kNkMAIDCapability_MovieRecMicrophone(0x8285)
enum eNkMAIDMovRecMicrophone
{
	kNkMAIDMovRecMicrophone_Auto = 0,
	kNkMAIDMovRecMicrophone_High,
	kNkMAIDMovRecMicrophone_Medium,
	kNkMAIDMovRecMicrophone_Low,
	kNkMAIDMovRecMicrophone_Off,
	kNkMAIDMovRecMicrophone_Manual
};

// kNkMAIDCapability_MovieRecDestination(0x8286)
enum eNkMAIDMovRecDestination
{
	kNkMAIDMovRecDestination_CF = 0,
	kNkMAIDMovRecDestination_SD,
	kNkMAIDMovRecDestination_XQD
};

// kNkMAIDCapability_PrimarySlot(0x8287)
enum eNkMAIDPrimarySlot
{
	kNkMAIDPrimarySlot_CF = 0,
	kNkMAIDPrimarySlot_SD,
	kNkMAIDPrimarySlot_XQD
};

// kNkMAIDCapability_ADLBracketingType(0x8288)
enum eNkMAIDADLBracketingType
{
	kNkMAIDADLBracketingType_2 = 0,
	kNkMAIDADLBracketingType_3,
	kNkMAIDADLBracketingType_4,
	kNkMAIDADLBracketingType_5,
	kNkMAIDADLBracketingType_5_Max_ExHi2
};

// kNkMAIDCapability_MovieRecHiISO(0x8306)
enum eNkMAIDMovRecHiISO
{
	kNkMAIDMovRecHiISO_Off = 0,
	kNkMAIDMovRecHiISO_On
};

// kNkMAIDCapability_SelectFUNC2CapAreaCrop(0x8291)
// kNkMAIDCapability_PreviewButton2CapAreaCrop(0x8292)
// kNkMAIDCapability_AEAFLockButton2CapAreaCrop(0x8293)
enum eNkMAIDCapAreaCrop
{
	kNkMAIDCapAreaCrop_12x = 0x00000008,
	kNkMAIDCapAreaCrop_DX  = 0x00000002,
	kNkMAIDCapAreaCrop_54  = 0x00000004,
	kNkMAIDCapAreaCrop_FX  = 0x00000001
};

// kNkMAIDCapability_BracketButton(0x8294)
enum eNkMAIDBracketButton
{
	kNkMAIDBracketButton_AutoBracketing = 0,
	kNkMAIDBracketButton_MultipleExposur
};

// kNkMAIDCapability_ActiveSlot(0x8304)
enum eNkMAIDActiveSlot
{
	kNkMAIDActiveSlot_NoCard = 0,
	kNkMAIDActiveSlot_Slot1,
	kNkMAIDActiveSlot_Slot2,
	kNkMAIDActiveSlot_Slot1_Slot2
};

// kNkMAIDCapability_SaveMedia(0x8305)
enum eNkMAIDSaveMedia
{
	kNkMAIDSaveMedia_Card = 0,
	kNkMAIDSaveMedia_SDRAM,
	kNkMAIDSaveMedia_Card_SDRAM
};

// kNkMAIDCapability_ExchangeDialsEx(0x8279)
enum eNkMAIDExchangeDialsEx
{
	kNkMAIDExchangeDialsEx_Off = 0,
	kNkMAIDExchangeDialsEx_On,
	kNkMAIDExchangeDialsEx_On_Amode
};

// kNkMAIDCapability_AutoDistortion(0x8282)
enum eNkMAIDAutoDistortion
{
	kNkMAIDAutoDistortion_Off = 0,
	kNkMAIDAutoDistortion_On,
};

// kNkMAIDCapability_SceneMode(0x8283)
enum eNkMAIDSceneMode
{
	kNkMAIDSceneMode_NightLandscape = 0,
	kNkMAIDSceneMode_PartyIndoor,
	kNkMAIDSceneMode_BeachSnow,
	kNkMAIDSceneMode_Sunset,
	kNkMAIDSceneMode_Duskdawn,
	kNkMAIDSceneMode_Petportrait,
	kNkMAIDSceneMode_Candlelight,
	kNkMAIDSceneMode_Blossom,
	kNkMAIDSceneMode_AutumnColors,
	kNkMAIDSceneMode_Food,
	kNkMAIDSceneMode_Silhouette,
	kNkMAIDSceneMode_Highkey,
	kNkMAIDSceneMode_Lowkey,
	kNkMAIDSceneMode_Portrait,
	kNkMAIDSceneMode_Landscape,
	kNkMAIDSceneMode_Child,
	kNkMAIDSceneMode_Sports,
	kNkMAIDSceneMode_Closeup,
	kNkMAIDSceneMode_NightPortrait
};

// kNkMAIDCapability_MovRecInCardStatus(0x8298)
enum eNkMAIDMovRecInCardStatus
{
	kNkMAIDMovRecInCardStatus_Off = 0,
	kNkMAIDMovRecInCardStatus_On
};

// kNkMAIDCapability_MovRecInCardProhibit(0x8299)
enum eNkMAIDMovRecInCardProhibit
{
	kNkMAIDMovRecInCardProhibit_LVPhoto	   = 0x00002000,
	kNkMAIDMovRecInCardProhibit_LVImageZoom= 0x00001000,
	kNkMAIDMovRecInCardProhibit_CardProtect= 0x00000800,
	kNkMAIDMovRecInCardProhibit_RecMov     = 0x00000400,
	kNkMAIDMovRecInCardProhibit_MovInBuf   = 0x00000200,
	kNkMAIDMovRecInCardProhibit_PCInBuf    = 0x00000100,
	kNkMAIDMovRecInCardProhibit_CardInBuf  = 0x00000080,
	kNkMAIDMovRecInCardProhibit_CardFull   = 0x00000008,
	kNkMAIDMovRecInCardProhibit_NoFormat   = 0x00000004,
	kNkMAIDMovRecInCardProhibit_CardErr    = 0x00000002,
	kNkMAIDMovRecInCardProhibit_NoCard     = 0x00000001
};

// kNkMAIDCapability_UserMode1(0x8307)
// kNkMAIDCapability_UserMode2(0x8308)
enum eNkMAIDUserMode
{
	kNkMAIDUserMode_NightLandscape = 0,
	kNkMAIDUserMode_PartyIndoor,
	kNkMAIDUserMode_BeachSnow,
	kNkMAIDUserMode_Sunset,
	kNkMAIDUserMode_Duskdawn,
	kNkMAIDUserMode_Petportrait,
	kNkMAIDUserMode_Candlelight,
	kNkMAIDUserMode_Blossom,
	kNkMAIDUserMode_AutumnColors,
	kNkMAIDUserMode_Food,
	kNkMAIDUserMode_Silhouette,
	kNkMAIDUserMode_Highkey,
	kNkMAIDUserMode_Lowkey,
	kNkMAIDUserMode_Portrait,
	kNkMAIDUserMode_Landscape,
	kNkMAIDUserMode_Child,
	kNkMAIDUserMode_Sports,
	kNkMAIDUserMode_Closeup,
	kNkMAIDUserMode_NightPortrait,
	kNkMAIDUserMode_Program,
	kNkMAIDUserMode_SpeedPriority,
	kNkMAIDUserMode_AperturePriority,
	kNkMAIDUserMode_Manual,
	kNkMAIDUserMode_Auto,
	kNkMAIDUserMode_FlashOff
};

// kNkMAIDCapability_MovieManualSetting(0x8309)
enum eNkMAIDMovManualSetting
{
	kNkMAIDMovManualSetting_OFF = 0,
	kNkMAIDMovManualSetting_ON
};

// kNkMAIDCapability_AFModeAtLiveView(0x830A)
enum eNkMAIDAFModeAtLiveView
{
	kNkMAIDAFModeAtLiveView_S = 0,
	kNkMAIDAFModeAtLiveView_C,
	kNkMAIDAFModeAtLiveView_F,
	kNkMAIDAFModeAtLiveView_M_FIX,
	kNkMAIDAFModeAtLiveView_M_SEL
};

// kNkMAIDCapability_BeepVolume(0x8314)
enum eNkMAIDBeepVolume
{
	kNkMAIDBeepVolume_Off = 0,
	kNkMAIDBeepVolume_1,
	kNkMAIDBeepVolume_2,
	kNkMAIDBeepVolume_3
};

// kNkMAIDCapability_VideoMode(0x8315)
enum eNkMAIDVideoMode
{
	kNkMAIDVideoMode_NTSC = 0,
	kNkMAIDVideoMode_PAL
};

// kNkMAIDCapability_WBAutoType(0x8316)
enum eNkWBAutoType
{
	kNkWBAutoType_Normal = 0,
	kNkWBAutoType_WarmWhite
};

// kNkMAIDCapability_EffectMode(0x8319)
enum eNkMAIDEffectMode
{
	kNkMAIDEffectMode_NightVision = 0,
	kNkMAIDEffectMode_ColorSketch,
	kNkMAIDEffectMode_Miniature,
	kNkMAIDEffectMode_SelectColor,
	kNkMAIDEffectMode_Silhouette,
	kNkMAIDEffectMode_Highkey,
	kNkMAIDEffectMode_Lowkey
};

// kNkMAIDCapability_HDRMode(0x8320)
enum eNkMAIDHDRMode
{
	kNkMAIDHDRMode_Off = 0,
	kNkMAIDHDRMode_On,
	kNkMAIDHDRMode_Continuous
};

// kNkMAIDCapability_HDRExposure(0x8321)
enum eNkMAIDHDRExposure
{
	kNkMAIDHDRExposure_Auto = 0,
	kNkMAIDHDRExposure_1EV,
	kNkMAIDHDRExposure_2EV,
	kNkMAIDHDRExposure_3EV
};

// kNkMAIDCapability_HDRSmoothing(0x8322)
enum eNkMAIDHDRSmoothing
{
	kNkMAIDHDRSmoothing_High = 0,
	kNkMAIDHDRSmoothing_Normal,
	kNkMAIDHDRSmoothing_Low
};

// kNkMAIDCapability_BlinkingStatus(0x8323)
enum eNkMAIDBlinkingStatus
{
	kNkMAIDBlinkingStatus_None = 0,
	kNkMAIDBlinkingStatus_Sp_Blink,
	kNkMAIDBlinkingStatus_Ap_Blink,
	kNkMAIDBlinkingStatus_Both
};

// kNkMAIDCapability_WBPresetProtect1(0x8326)
enum eNkMAIDWBPresetProtect1
{
	kNkMAIDWBPresetProtect1_Off = 0,
	kNkMAIDWBPresetProtect1_On
};

// kNkMAIDCapability_WBPresetProtect2(0x8327)
enum eNkMAIDWBPresetProtect2
{
	kNkMAIDWBPresetProtect2_Off = 0,
	kNkMAIDWBPresetProtect2_On
};

// kNkMAIDCapability_WBPresetProtect3(0x8328)
enum eNkMAIDWBPresetProtect3
{
	kNkMAIDWBPresetProtect3_Off = 0,
	kNkMAIDWBPresetProtect3_On
};

// kNkMAIDCapability_WBPresetProtect4(0x8329)
enum eNkMAIDWBPresetProtect4
{
	kNkMAIDWBPresetProtect4_Off = 0,
	kNkMAIDWBPresetProtect4_On
};

// kNkMAIDCapability_MovieImageQuality(0x8331)
enum eNkMAIDMovieImageQuality
{
	kNkMAIDMovieImageQuality_Normal = 0,
	kNkMAIDMovieImageQuality_Fine
};

// kNkMAIDCapability_LiveViewExposurePreview(0x8333)
enum eNkMAIDLiveViewExposurePreview
{
	kNkMAIDLiveViewExposurePreview_Off = 0,
	kNkMAIDLiveViewExposurePreview_On
};

// kNkMAIDCapability_LiveViewSelector(0x8334)
enum eNkMAIDLiveViewSelector
{
	kNkMAIDLiveViewSelector_Photo = 0,
	kNkMAIDLiveViewSelector_Movie
};

// kNkMAIDCapability_ADLBracketingStep(0x8340)
enum eNkMAIDADLBracketingStep
{
	kNkMAIDADLBracketingStep_Auto	     = 0,
	kNkMAIDADLBracketingStep_Low         = 1,
	kNkMAIDADLBracketingStep_Normal	     = 2,
	kNkMAIDADLBracketingStep_High	     = 3,
	kNkMAIDADLBracketingStep_ExtraHigh_1 = 4,
	kNkMAIDADLBracketingStep_ExtraHigh_2 = 5
};

// kNkMAIDCapability_LiveViewPhotoShootingMode(0x8346)
enum eNkMAIDLiveViewPhotoShootingMode
{
	kNkMAIDLiveViewPhotoShootingMode_Quiet = 0,
	kNkMAIDLiveViewPhotoShootingMode_Silent
};

// kNkMAIDCapability_ExposureDelayEx(0x8347)
enum eNkMAIDExposureDelayEx
{
	kNkMAIDExposureDelayEx_3sec = 0,
	kNkMAIDExposureDelayEx_2sec,
	kNkMAIDExposureDelayEx_1sec,
	kNkMAIDExposureDelayEx_Off
};

// kNkMAIDCapability_MovieISORange(0x8348)
enum eNkMAIDMovieISORange
{
	kNkMAIDMovieISORange_200to12800 = 0,
	kNkMAIDMovieISORange_200toHi40
};

// kNkMAIDCapability_MovieReleaseButton(0x8349)
enum eNkMAIDMovieReleaseButton
{
	kNkMAIDMovieReleaseButton_Photo = 0,
	kNkMAIDMovieReleaseButton_Movie,
	kNkMAIDMovieReleaseButton_SaveFrame
};

// kNkMAIDCapability_GetIPTCInfo(0x8352)
enum eNkMAIDGetIPTCInfo
{
	kNkMAIDGetIPTCInfo_None		= 0,
	kNkMAIDGetIPTCInfo_Attached
};

///////////////////////////////////////////////////////////////////////////////
// Definition of structure

// kNkMAIDCapability_WBPresetData(0x81ad)
typedef struct tagNkMAIDWBPresetData
{
	ULONG  ulPresetNumber;		// Preset Number
	ULONG  ulPresetGain;		// Preset Gain
	ULONG  ulThumbnailSize;		// Thumbnail size of pThumbnailData
	ULONG  ulThumbnailRotate;	// One of eNkMAIDThumbnailRotate
	void*  pThumbnailData;		// The pointer to Thumbnail Data
} NkMAIDWBPresetData, FAR* LPNkMAIDWBPresetData;

// kNkMAIDCapability_PictureControlData(0x8258)
typedef struct tagNkMAIDPicCtrlData
{
	ULONG  ulPicCtrlItem;	// picture control item
	ULONG  ulSize;			// the data sizer of pData
	bool   bModifiedFlag;	// Flag to set New or current 
	void*  pData;			// The pointer to picture control data
} NkMAIDPicCtrlData, FAR* LPNkMAIDPicCtrlData;

// kNkMAIDCapability_GetPicCtrlInfo(0x8259)
typedef struct tagNkMAIDGetPicCtrlInfo
{
	ULONG  ulPicCtrlItem;	// picture control item
	ULONG  ulSize;			// the data sizer of pData
	void*  pData;			// The pointer to Quick Adjust Param
} NkMAIDGetPicCtrlInfo, FAR* LPNkMAIDGetPicCtrlInfo;

// kNkMAIDCapability_GetVideoImage(0x8317)
typedef struct tagNkMAIDGetVideoImage
{
	ULONG  ulType;			// one of eNkMAIDArrayType
	ULONG  ulOffset;		// Offset
	ULONG  ulReadSize;		// size of get data
	ULONG  ulDataSize;		// size of "pData" 
	LPVOID pData;			// allocated by the client
} NkMAIDGetVideoImage, FAR* LPNkMAIDGetVideoImage;

// kNkMAIDCapability_TerminateCapture(0x8318)
typedef struct tagNkMAIDTerminateCapture
{
	ULONG  ulParameter1;
	ULONG  ulParameter2;
} NkMAIDTerminateCapture, FAR* LPNkMAIDTerminateCapture;

///////////////////////////////////////////////////////////////////////////////
#endif
