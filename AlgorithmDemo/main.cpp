#include "AlpMPAlgoInterface.h"
#include <fstream>
#include <iostream>

#pragma comment(lib, "AlgorithmLibrary.lib")

static unsigned char imagebuffer[4000 * 3000] = { 0 };

//ATE采图接口
int ImageCapture_capture1(unsigned char rawDataBuf[], unsigned long rawDataBufLen, unsigned long& rawDataRealLen, void* frameInfo)
{
	std::string rawFilePath = "D:/Data/image_raw_site0/2.5g_aps_12bit/FrameID55_W1632_H2340P12.raw";
	std::ifstream infile;
	infile.open(rawFilePath, std::ios::binary | std::ios::in);
	if (!infile.fail())
	{
		infile.seekg(0, std::ios::end);
		rawDataRealLen = infile.tellg();
		infile.seekg(0, std::ios::beg);
		infile.read((char*)rawDataBuf, rawDataRealLen);
		infile.close();
		return 0;
	}
	else
	{
		return -1;
	}
}

int ImageCapture_capture2(unsigned char rawDataBuf[], unsigned long rawDataBufLen, unsigned long& rawDataRealLen, void* frameInfo)
{
	std::string rawFilePath = "D:/Data/image_raw_site0/2.5g_aps_12bit/FrameID56_W1632_H2340P12.raw";
	std::ifstream infile;
	infile.open(rawFilePath, std::ios::binary | std::ios::in);
	if (!infile.fail())
	{
		infile.seekg(0, std::ios::end);
		rawDataRealLen = infile.tellg();
		infile.seekg(0, std::ios::beg);
		infile.read((char*)rawDataBuf, rawDataRealLen);
		infile.close();
		return 0;
	}
	else
	{
		return -1;
	}
}

static CAlpDVSMPAlgoInterface* gDVSInterface = nullptr;
static CAlpAPSMPAlgoInterface* gAPSInterface = nullptr;

//初始化算法库接口，在程序启动时调用一次
void Init()
{
	//传入参数：芯片类型，RAW类型，算法log存储地址
	gAPSInterface = CreateAPSAlgoInterface(ALP_003BA, RAW12, "D:/");
	//启动算法log存储
	gAPSInterface->SetLogEnable(true);

	//获取算法库版本号
	std::cout << "Algo Ver: " << gAPSInterface->GetVersion() << std::endl;
}

//卸载算法库，在程序结束时调用
void UnInit()
{
	delete gAPSInterface;
}

//一个测试项中算法库调用方法
void OneTestItem()
{
	unsigned long rawDataRealLen = 0;
	ImageCapture_capture1(imagebuffer, sizeof(imagebuffer), rawDataRealLen, nullptr);

	//导入数据接口，参数为RAW数据指针，数据长度，首帧存储在算法中的位置，导入的帧数，RAW数据是否带帧头帧尾
	bool bRet = gAPSInterface->ImportRawData(imagebuffer, rawDataRealLen, 0, 1, false);

	if (!bRet)
	{
		std::cout << "ImportData Fail!" << std::endl;
		return;
	}

	ImageCapture_capture2(imagebuffer, sizeof(imagebuffer), rawDataRealLen, nullptr);

	//导入数据接口，参数为RAW数据指针，数据长度，首帧存储在算法中的位置，导入的帧数，RAW数据是否带帧头帧尾
	bRet = gAPSInterface->ImportRawData(imagebuffer, rawDataRealLen, 1, 1, false);

	if (!bRet)
	{
		std::cout << "ImportData Fail!" << std::endl;
		return;
	}

	std::vector<double> DataMean;
	//计算均值接口，参数为首帧存储在算法中的位置，导入的帧数，ROI(传入nullptr使用默认ROI)，数据结果
	bRet = gAPSInterface->DataMean(0, 2, nullptr, DataMean);

	if (!bRet)
	{
		std::cout << "Mean Func Fail!" << std::endl;
	}
	else
	{
		std::cout << "DataMean:" << std::endl;
		//子图顺序Gb1,Gb2,B1,B2,R1,R2,Gr1,Gr2
		for (int n = 0; n < APSSubFrameIndex::SubFrameNum; n++)
		{
			std::cout << DataMean[n] << std::endl;
		}
	}

	std::vector<double> DataTNoise;
	//计算时域噪声接口，参数为首帧存储在算法中的位置，导入的帧数，ROI(传入nullptr使用默认ROI)，数据结果
	bRet = gAPSInterface->TNoise(0, 2, nullptr, DataTNoise);

	if (!bRet)
	{
		std::cout << "TNoise Func Fail!" << std::endl;
	}
	else
	{
		std::cout << "DataTNoise:" << std::endl;
		//子图顺序Gb1,Gb2,B1,B2,R1,R2,Gr1,Gr2
		for (int n = 0; n < APSSubFrameIndex::SubFrameNum; n++)
		{
			std::cout << DataTNoise[n] << std::endl;
		}
	}
}

int main()
{
	Init();
	OneTestItem();
	UnInit();
	return 0;
}