#ifndef SYSTEM_H__
#define SYSTEM_H__

#include "Tracking.h"
#include "SlamViewer.h"
#include "Optimizer.h"
#include "DenseMapping.h"

#include <thread>

class SlamViewer;
class Tracker;
class DenseMapping;
class Optimizer;

struct SysDesc {
	int cols, rows;
	float fx;
	float fy;
	float cx;
	float cy;
	float DepthCutoff;
	float DepthScale;
	bool TrackModel;
	std::string path;
	bool bUseDataset;
};

class System {

public:

	System(SysDesc * pParam);

	bool GrabImage(const cv::Mat & image, const cv::Mat & depth);

	void JoinViewer();

	void RebootSystem();

	void FilterMessage(bool finished = false);

	void WriteMeshToDisk();

	void WriteMapToDisk();

	void ReadMapFromDisk();

	void RenderTopDown(float dist = 8.0f);

	std::atomic<bool> paused;
	std::atomic<bool> requestMesh;
	std::atomic<bool> requestSaveMap;
	std::atomic<bool> requestReadMap;
	std::atomic<bool> requestSaveMesh;
	std::atomic<bool> requestReboot;
	std::atomic<bool> requestStop;
	std::atomic<bool> imageUpdated;
	std::atomic<bool> poseOptimized;

	DeviceArray2D<float4> vmap;
	DeviceArray2D<float4> nmap;
	DeviceArray2D<uchar4> renderedImage;

	cv::Mat mK;
	int nFrames;
	bool state;

protected:

	void ReIntegration();

	DenseMapping * map;
	SysDesc * param;
	SlamViewer  * viewer;
	Tracker * tracker;
	Optimizer * optimizer;

	std::thread * viewerThread;
	std::thread * optimizerThd;

	int num_frames_after_reloc;

};

#endif
