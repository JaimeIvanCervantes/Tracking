#include <ImageFormat.h>

QImage ImageFormat::Mat2QImage(cv::Mat const& src) {
	 cv::Mat temp;
	 cvtColor(src, temp,CV_BGR2RGB); 
	 QImage dest((uchar*) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
	 QImage dest2(dest);
	 dest2.detach();
	 return dest2;
}

QImage ImageFormat::MatGray2QImage(cv::Mat const& src) {
	 cv::Mat temp;
	 QImage dest((uchar*) src.data, src.cols, src.rows, src.step, QImage::Format_Indexed8);
	 QImage dest2(dest);
	 dest2.detach();
	 return dest2;
}

cv::Mat ImageFormat::QImage2Mat(QImage const& src) {
	 cv::Mat tmp(src.height(),src.width(),CV_8UC3,(uchar*)src.bits(),src.bytesPerLine());
	 cv::Mat result;
	 cvtColor(tmp, result,CV_BGR2RGB);
	 return result;
}