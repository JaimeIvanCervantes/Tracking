#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <qimage>

/** Image format operations.
This class contains static functions to convert between OpenCV Mat and Qt QImage formats.
*/
class ImageFormat {
	public:
	/** Converts color Mat to QImage.
	@param src OpenCV Mat color image.
	@return Qt QImage.
	*/
	static QImage Mat2QImage(cv::Mat const& src);
	
	/** Converts grayscale Mat to QImage.
	@param src OpenCV Mat grayscale image.
	@return Qt QImage.
	*/
	static QImage MatGray2QImage(cv::Mat const& src);
	
	/** Converts color QImage to Mat.
	@param src Qt color QImage.
	@return Qt OpenCV Mat color image.
	*/
	static cv::Mat QImage2Mat(QImage const& src);
};
	