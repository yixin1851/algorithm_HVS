#include "TestDemo.h"
#include "AlpMPAlgoInterface.h"

static unsigned char imagebuffer[8000 * 8000] = { 0 };

static CAlpDVSMPAlgoInterface* gDVSInterface = nullptr;
static CAlpAPSMPAlgoInterface* gAPSInterface = nullptr;

int ImageCapture_capture(unsigned char rawDataBuf[], unsigned long rawDataBufLen, unsigned long& rawDataRealLen, void* frameInfo)
{
	return 0;
}


void CPDVSTest()
{
	gDVSInterface = CreateDVSAlgoInterface(ALP_003BA, "D:/");
	gDVSInterface->SetMultiThreadEnable(true);
	gDVSInterface->SetLogEnable(true);
	unsigned long rawDataRealLen = 0;
	void* frameInfo = nullptr;
	StationaryNoiseData StationaryNoise;
	StationaryUniformityData StationaryUniformity;
	HotpixelData HotpixelData;

	ImageCapture_capture(imagebuffer, sizeof(imagebuffer), rawDataRealLen, frameInfo); // 采集350帧数据弱光配置数据
	gDVSInterface->ImportRawData(imagebuffer, rawDataRealLen, 0, 350);
	gDVSInterface->StationaryNoise(300, 50, StationaryNoise); //丢弃300帧
	gDVSInterface->StationaryUniformity(300, 50, StationaryUniformity); //丢弃300帧
	gDVSInterface->HotPixel(300, 50, HotpixelData); //丢弃300帧

	ImageContrastSensitivityData WeakImageContrastSensitivityData, NormalImageContrastSensitivityData, StrongImageContrastSensitivityData;
	ImageCapture_capture(imagebuffer, sizeof(imagebuffer), rawDataRealLen, frameInfo); // 采集160帧低变化量数据
	gDVSInterface->ImportRawData(imagebuffer, rawDataRealLen, 0, 160);
	gDVSInterface->ImageContrastSensitivity(110, 50, nullptr, 3, On_OffEvents, WeakImageContrastSensitivityData);

	SpatialResponseUniformityData NormalSpatialResponseUniformityData;
	ImageCapture_capture(imagebuffer, sizeof(imagebuffer), rawDataRealLen, frameInfo); // 采集160帧中变化量数据
	gDVSInterface->ImportRawData(imagebuffer, rawDataRealLen, 0, 160);
	PeakInfo PeakData;
	gDVSInterface->FindPeak(110, 50, PeakData, On_OffEvents);
	gDVSInterface->ImageContrastSensitivity(110, 50, &PeakData, 3, On_OffEvents, NormalImageContrastSensitivityData); //丢弃110帧数据
	gDVSInterface->SpatialResponseUniformity(110, 50, &PeakData, 3, On_OffEvents, NormalSpatialResponseUniformityData); //丢弃110帧数据

	AccompaniedPeakAndDelayedPeakData StrongAccompaniedPeakAndDelayedPeakData;
	ImageCapture_capture(imagebuffer, sizeof(imagebuffer), rawDataRealLen, frameInfo); // 采集160帧高变化量数据
	gDVSInterface->ImportRawData(imagebuffer, rawDataRealLen, 0, 160);
	gDVSInterface->FindPeak(110, 50, PeakData, On_OffEvents);
	gDVSInterface->ImageContrastSensitivity(110, 50, &PeakData, 3, On_OffEvents, StrongImageContrastSensitivityData); //丢弃110帧数据
	gDVSInterface->AccompaniedPeakAndDelayedPeak(110, 50, &PeakData, 3, On_OffEvents, StrongAccompaniedPeakAndDelayedPeakData); //丢弃110帧数据

	DVSBadpixelData BadpixelData;
	ImageCapture_capture(imagebuffer, sizeof(imagebuffer), rawDataRealLen, frameInfo); // 采集550帧数据
	gDVSInterface->ImportRawData(imagebuffer, rawDataRealLen, 0, 550);
	gDVSInterface->BadPixel(40, 510, nullptr, 50, On_OffEvents, BadpixelData);
}

void CPAPSTest()
{
	gAPSInterface = CreateAPSAlgoInterface(ALP_003BA, RAW10, "D:/");
	gAPSInterface->SetMultiThreadEnable(true);
	gAPSInterface->SetLogEnable(true);
	unsigned long rawDataRealLen = 0;
	void* frameInfo = nullptr;

	ImageCapture_capture(imagebuffer, sizeof(imagebuffer), rawDataRealLen, frameInfo); // 采集5帧dark 50ms数据
	gAPSInterface->ImportRawData(imagebuffer, rawDataRealLen, 0, 5);

	ImageCapture_capture(imagebuffer, sizeof(imagebuffer), rawDataRealLen, frameInfo); // 采集5帧dark 150ms数据
	gAPSInterface->ImportRawData(imagebuffer, rawDataRealLen, 5, 5);

	ImageCapture_capture(imagebuffer, sizeof(imagebuffer), rawDataRealLen, frameInfo); // 采集5帧Light 10ms数据
	gAPSInterface->ImportRawData(imagebuffer, rawDataRealLen, 10, 5);

	ImageCapture_capture(imagebuffer, sizeof(imagebuffer), rawDataRealLen, frameInfo); // 采集5帧Light 25ms数据
	gAPSInterface->ImportRawData(imagebuffer, rawDataRealLen, 15, 5);

	ImageCapture_capture(imagebuffer, sizeof(imagebuffer), rawDataRealLen, frameInfo); // 采集5帧Light 50ms数据
	gAPSInterface->ImportRawData(imagebuffer, rawDataRealLen, 20, 5);

	ImageCapture_capture(imagebuffer, sizeof(imagebuffer), rawDataRealLen, frameInfo); // 采集5帧Light 75ms数据
	gAPSInterface->ImportRawData(imagebuffer, rawDataRealLen, 25, 5);

	std::vector<double> DarkMean, DarkTNoise, DarkRowTNoise, DarkColTNoise, DarkSNoise, DarkRowSNoise, DarkColSNoise;
	gAPSInterface->DataMean(0, 5, nullptr, DarkMean);//计算dark均值(50ms)

	std::vector<HotpixelData> DarkHotPixel;
	gAPSInterface->HotPixel(5, 5, nullptr, DarkHotPixel);//计算dark HotPixel(150ms)

	std::vector<BadPixelMaskData> DarkHotPixelMask;

	for (uint32_t nChannel = 0; nChannel < APSSubFrameIndex::SubFrameNum; nChannel++)
	{
		DarkHotPixelMask.push_back(DarkHotPixel[nChannel].HotPixelMask);
	}
	gAPSInterface->DPC(0, 10, nullptr, DarkHotPixelMask);//DPC

	gAPSInterface->TNoise(0, 5, nullptr, DarkTNoise);
	gAPSInterface->RowTNoise(0, 5, nullptr, DarkRowTNoise);
	gAPSInterface->ColTNoise(0, 5, nullptr, DarkColTNoise);

	gAPSInterface->SNoise(0, 5, nullptr, DarkSNoise);
	gAPSInterface->RowSNoise(0, 5, nullptr, DarkRowSNoise);
	gAPSInterface->ColSNoise(0, 5, nullptr, DarkColSNoise);

	std::vector<double> Dark150msMean, Dark150msTNoise;
	gAPSInterface->TNoise(5, 5, nullptr, DarkTNoise);
	gAPSInterface->DataMean(5, 5, nullptr, DarkMean);

	std::vector<double> DarkExpTime;
	DarkExpTime.push_back(50);
	DarkExpTime.push_back(150);

	std::vector<std::vector<double>> DarkMeanMethod;
	DarkMeanMethod.push_back(DarkMean);
	DarkMeanMethod.push_back(Dark150msMean);

	std::vector<std::vector<double>> DarkTNoiseMethod;
	DarkTNoiseMethod.push_back(DarkTNoise);
	DarkTNoiseMethod.push_back(Dark150msTNoise);

	std::vector<double> DarkCurrentMeanMethod, DarkCurrentTNoiseMethod;
	gAPSInterface->DarkCurrent(DarkMeanMethod, DarkExpTime, true, DarkCurrentMeanMethod); //均值法
	gAPSInterface->DarkCurrent(DarkTNoiseMethod, DarkExpTime, false, DarkCurrentTNoiseMethod); //方差法

	DSNUData DSNU;
	gAPSInterface->DSNU(0, 5, nullptr, DSNU);

	gAPSInterface->BLC(10, 20, DarkMean); //全局Base做BLC
	gAPSInterface->BLC(10, 20, 0, 5); //列均值Base做BLC

	std::vector<BadpixelData> LightBadPixel;
	gAPSInterface->BadPixel(20, 5, nullptr, LightBadPixel);

	std::vector<BadPixelMaskData> LightBadPixelMask;

	for (uint32_t nChannel = 0; nChannel < APSSubFrameIndex::SubFrameNum; nChannel++)
	{
		LightBadPixelMask.push_back(LightBadPixel[nChannel].BadPixelMask);
	}

	gAPSInterface->DPC(10, 20, nullptr, LightBadPixelMask);

	ShadingData Shading;
	gAPSInterface->Shading(20, 5, nullptr, Shading);

	ROIArea OpticalArea = { Shading.CenterRow - 99, Shading.CenterRow + 100,  Shading.CenterCol - 99, Shading.CenterCol + 100 };

	std::vector<double> LightMean, LightTNoise, LightRowTNoise, LightColTNoise, LightSNoise, LightRowSNoise, LightColSNoise;

	gAPSInterface->DataMean(20, 5, &OpticalArea, LightMean);
	gAPSInterface->TNoise(20, 5, &OpticalArea, LightTNoise);
	gAPSInterface->RowTNoise(20, 5, &OpticalArea, LightRowTNoise);
	gAPSInterface->ColTNoise(20, 5, &OpticalArea, LightColTNoise);

	gAPSInterface->SNoise(20, 5, &OpticalArea, LightSNoise);
	gAPSInterface->RowSNoise(20, 5, &OpticalArea, LightRowSNoise);
	gAPSInterface->ColSNoise(20, 5, &OpticalArea, LightColSNoise);

	std::vector<double> Light10Mean, Light25Mean, Light75Mean;
	gAPSInterface->DataMean(10, 5, &OpticalArea, Light10Mean);
	gAPSInterface->DataMean(15, 5, &OpticalArea, Light25Mean);
	gAPSInterface->DataMean(25, 5, &OpticalArea, Light75Mean);

	std::vector<std::vector<double>> LightAllMean;
	LightAllMean.push_back(Light10Mean);
	LightAllMean.push_back(Light25Mean);
	LightAllMean.push_back(LightMean);
	LightAllMean.push_back(Light75Mean);

	std::vector<double> LightExpTime;
	LightExpTime.push_back(10);
	LightExpTime.push_back(25);
	LightExpTime.push_back(50);
	LightExpTime.push_back(75);

	std::vector<LinearityData> LinearityRes;
	gAPSInterface->Linearity(LightAllMean, LightExpTime, LinearityRes);

	std::vector<double> Light10TNoise, Light25TNoise, Light75TNoise;
	gAPSInterface->TNoise(10, 5, &OpticalArea, Light10TNoise);
	gAPSInterface->TNoise(15, 5, &OpticalArea, Light25TNoise);
	gAPSInterface->TNoise(25, 5, &OpticalArea, Light75TNoise);

	std::vector<std::vector<double>> LightAllTNoise;
	LightAllTNoise.push_back(Light10TNoise);
	LightAllTNoise.push_back(Light25TNoise);
	LightAllTNoise.push_back(LightTNoise);
	LightAllTNoise.push_back(Light75TNoise);

	std::vector<double> GainK;
	gAPSInterface->OverallSystemGain(LightAllTNoise, LightAllMean, DarkTNoise, GainK);

	ImageCapture_capture(imagebuffer, sizeof(imagebuffer), rawDataRealLen, frameInfo); // 采集5帧Saturation数据
	gAPSInterface->ImportRawData(imagebuffer, rawDataRealLen, 30, 5);

	SaturationData SaturationDataRes;
	gAPSInterface->Saturation(30, 5, &OpticalArea, APSSubFrameIndex::Gb1, SaturationDataRes); //计算Saturation

}