#include <opencv2/core/core.hpp>

/** Blob class (Also called object).
Blob, or objects, detected in the image. A blob can represent a car or a person for example.
*/
class Blob {
	public:
	int ID;
	int	firstFrameNumber;
	int lastFrameNumber;
	int frameCount;
	int avgWidth;
	int avgHeight;
	int maxWidth;
	int maxHeight;
	int collision;
	cv::Point currentPosition;
	cv::MatND currentHist;
	cv::Rect firstRectangle;
	cv::Rect lastRectangle;
	std::vector<int> contactContours;
	cv::vector<cv::Mat> frames;
	cv::vector<cv::MatND> histograms;
	
};