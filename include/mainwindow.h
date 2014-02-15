#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug> 

#include <qtcore>
#include <qfiledialog>
#include <qmainwindow>
#include <qpushbutton>
#include <qlabel>
#include <qimage>
#include <qpixmap>
#include <qslider>
#include <qhboxlayout>
#include <qapplication>
#include <qkeyevent>
#include <algorithm>
#include <vector>
#include <QtGui>
#include <qwidget>
#include <iostream>

#include <opencv2/video/background_segm.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <Blob.h>
#include <ImageFormat.h>

class QPushButton;
class QImage;
class QLabel;


/** Main window.
This is the main GUI that permits the user to define parameters, select, and load a video to process.
*/
class MainWindow : public QMainWindow
{
	Q_OBJECT
	
	public:
		MainWindow();
		
	signals:
	
	private slots:
	
		/** Load video to process. 
		*/
		void loadButtonClicked();

		/** Main loop to process each frame. 
		*/
		void timerLoop();
		
		/** Export frames to video for each stable blob. 
		*/
		int exportObjectVideos();
		
	private:
		unsigned int ID;
		int	frameNumber;
		QPushButton *loadVideoButton;
		QPushButton *loadObjectButton;
		QLabel *videoFrameLabel;
		QLabel *videoFrameLabelDebug;
		QLabel *fileLabel;
		QLabel *folderLabel;
		std::string folder;
		cv::Mat rawFrame,rawCopyFrame,foregroundFrame, foregroundFrameBuffer, roiFrame, roiFrameBuffer, hsvRoiFrame, roiFrameMask;
		QImage  frameImage;
		QImage  frameImageDebug;
		QTimer *timer;
		cv::VideoCapture videoCapture;
		cv::BackgroundSubtractorMOG2 mog;
		cv::vector<Blob> blobContainer;
		
		cv::Scalar getRandomColorRGB(unsigned int ID);

    protected:
};



#endif