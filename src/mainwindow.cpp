
#include <mainwindow.h>

using namespace std;

// Sorting functions
bool sortByFrameCount(const Blob &lhs, const Blob &rhs) { return lhs.frameCount > rhs.frameCount; }

// Main GUI
MainWindow::MainWindow() {
	// Initialize variables
	frameNumber = 0;
	ID = 0;
 
	// Initialize Qt widgets	
	videoFrameLabel = new QLabel();
	videoFrameLabelDebug = new QLabel();
	fileLabel = new QLabel("No video file selected");
	folderLabel = new QLabel("No folder file selected");
	QWidget *mainWidget = new QWidget();
	timer = new QTimer(this);
	loadVideoButton = new QPushButton("Load Video");
	QVBoxLayout *mainLayout = new QVBoxLayout;
	
	// Set style
	fileLabel->setStyleSheet("font-size: 20px;");
	folderLabel->setStyleSheet("font-size: 20px;");
	loadVideoButton->setStyleSheet("font-size: 20px;");

	
	// Connect slots
	connect(loadVideoButton,SIGNAL(clicked()),this,SLOT(loadButtonClicked()));
	connect(timer, SIGNAL(timeout()), this, SLOT(timerLoop()));
	
	// Set layout
	mainLayout->addWidget(videoFrameLabel);
	mainLayout->addWidget(videoFrameLabelDebug);
	mainLayout->addWidget(loadVideoButton);
	mainLayout->addWidget(fileLabel);
	mainLayout->addWidget(folderLabel);
	
	// Set mainLayout in main widget
	mainWidget->setLayout(mainLayout);	
	setCentralWidget(mainWidget);
	
	// Set initial screen
	QImage image;
	cv::Mat src = cv::imread("initialScreen.png");
	image = ImageFormat::Mat2QImage(src);
	videoFrameLabel->setPixmap(QPixmap::fromImage(image));	
}

// Load video to process.
void MainWindow::loadButtonClicked() {
	
	// Load video and start video
    if (!timer->isActive()) {
		// Open file dialog
		QString fileName = QFileDialog::getOpenFileName(this, "Open Video", ".", "Video Files (*.avi *.mpg)");
		videoCapture.open(fileName.toStdString().c_str());//fileName.toAscii().data());
		
		// Open directory dialog
		QString directoryName = QFileDialog::getExistingDirectory(this, "Select Output Folder");
		
		// Set labels
		string fileText("VIDEO FILE: ");
		fileText+=fileName.toStdString();
		string directoryText("OUTPUT FOLDER: ");
		directoryText+=directoryName.toStdString();
		
		fileLabel->setText(fileText.c_str());
		folderLabel->setText(directoryText.c_str());
		
		// Set output folder
		folder = directoryName.toStdString();
		
		// Get frame per second
		int fps=videoCapture.get(CV_CAP_PROP_FPS);
		
		// Set timer interval
		timer->setInterval(1000/fps);
		
        timer->start();
	} else {
		timer->stop();
	}
}

// Main loop activated with the timer.
void MainWindow::timerLoop() {

	// Increase frame number;
	frameNumber++;

	// Contour variables
	cv::vector<cv::vector<cv::Point> > contours;
	cv::vector<cv::Vec4i> hierarchy;

	// Capture frame from video
	videoCapture>>rawFrame;
	
	// Check if frame was successfully captured.
	if (rawFrame.empty()) {
		timer->stop();
		exportObjectVideos();
		cout<<"End of video reached successfully\n";
	} else {
		// Get a copy of rawFrame before processing
		rawCopyFrame=rawFrame.clone();	
	
		// Background subtraction
		mog(rawFrame,foregroundFrame,-1);
		mog.set("nmixtures", 2);
		mog.set("detectShadows",0);
		
		// Threshold and morphology operations
		cv::threshold(foregroundFrame,foregroundFrame,130,255,cv::THRESH_BINARY);
		cv::medianBlur(foregroundFrame,foregroundFrame,3);
		cv::erode(foregroundFrame,foregroundFrame,cv::Mat());
		cv::dilate(foregroundFrame,foregroundFrame,cv::Mat());
		
		// Get foreground buffer
		foregroundFrameBuffer = foregroundFrame.clone();

		//cv::findContours( foregroundFrame, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
		cv::findContours( foregroundFrame, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
		
		// Initialize bounding rectangle for contours
		cv::Rect boundingRectangle;
		
		// Draw contours
		cv::Mat contoursFrame = cv::Mat::zeros( foregroundFrame.size(), CV_8UC3 );
		
		// Object buffer
		vector<int> contourTaken(contours.size(),0);

		// Remove old objects with few frames
		for (vector<Blob>::iterator it=blobContainer.begin(); it!=blobContainer.end();) { 
			if (frameNumber - it->lastFrameNumber > 50 && it->frames.size() < 20) {
				it = blobContainer.erase(it);
			} else {
				++it;
			}
		}		
		
		// Detect collisions and append object to object containers
		for (unsigned int bli = 0; bli < blobContainer.size(); bli++) {
			// Clean contact contours
			blobContainer[bli].contactContours.clear();
			blobContainer[bli].collision = 0;
		
			// Loop contours
			for( unsigned int coi = 0; coi<contours.size(); coi++ ) {
				// Obtain ROI from bounding rectangle of contours
				boundingRectangle = cv::boundingRect(contours[coi]);
				
				// Get distance
				float distance = sqrt(pow((blobContainer[bli].lastRectangle.x + blobContainer[bli].lastRectangle.width/2.0)-(boundingRectangle.x + boundingRectangle.width/2.0),2.0)+
				pow((blobContainer[bli].lastRectangle.y + blobContainer[bli].lastRectangle.height/2.0)-(boundingRectangle.y + boundingRectangle.height/2.0),2.0));
			
				// Detect collisions
				if (distance < min(blobContainer[bli].lastRectangle.width,blobContainer[bli].lastRectangle.height) &&//(distance < fmaxf(boundingRectangle.width,boundingRectangle.height)*2.0 &&
				frameNumber - blobContainer[bli].lastFrameNumber == 1 &&
				max(boundingRectangle.width,boundingRectangle.height) > max(videoCapture.get(CV_CAP_PROP_FRAME_WIDTH),videoCapture.get(CV_CAP_PROP_FRAME_HEIGHT))/30)
				{
					blobContainer[bli].contactContours.push_back(coi);
				}
			
			}
		
		}
		
		// Sort blobContainer
		if (blobContainer.size() > 1)
			sort(blobContainer.begin(),blobContainer.end(), sortByFrameCount);
		
		// Find collision contours with biggest area
		for (unsigned int bli = 0; bli<blobContainer.size(); bli++) {
			unsigned int maxArea = 0;
			int selectedContourIndex = -1;
			for (unsigned int cni = 0; cni<blobContainer[bli].contactContours.size(); cni++) {
				int coi = blobContainer[bli].contactContours[cni];
				int contourArea = cv::boundingRect(contours[coi]).width*cv::boundingRect(contours[coi]).height;
				if (contourArea > maxArea) {
					maxArea = contourArea;
					selectedContourIndex = coi;	
				}
			}
			
			//blobContainer[bli].contactContours.clear();
			
			// Append blob with largest area
			if (selectedContourIndex != -1 && contourTaken[selectedContourIndex] == 0) {
				contourTaken[selectedContourIndex] = 1;
				blobContainer[bli].contactContours.push_back(selectedContourIndex);
				
				// Get contour properties
				boundingRectangle = cv::boundingRect(contours[selectedContourIndex]);
				roiFrameMask =  foregroundFrameBuffer(boundingRectangle).clone();
				roiFrame = foregroundFrameBuffer(boundingRectangle).clone();
				roiFrameBuffer = rawFrame(boundingRectangle).clone();			
				roiFrameBuffer.copyTo(roiFrame, roiFrameMask);
				
				// Append objects
				blobContainer[bli].frameCount++;
				blobContainer[bli].lastFrameNumber = frameNumber;
				blobContainer[bli].lastRectangle = boundingRectangle;
				blobContainer[bli].frames.push_back(roiFrameBuffer);
				blobContainer[bli].avgWidth = .8*blobContainer[bli].avgWidth + .2*roiFrame.size().width;
				blobContainer[bli].avgHeight = .8*blobContainer[bli].avgHeight + .2*roiFrame.size().height;
				blobContainer[bli].maxWidth = max(blobContainer[bli].maxWidth,roiFrame.size().width);
				blobContainer[bli].maxHeight = max(blobContainer[bli].maxHeight,roiFrame.size().height);				
			}
			
			// 
			if (blobContainer[bli].contactContours.size() > 1) {
				blobContainer[bli].collision = 1;
			}
			
		}		

		// Create objects for the rest of the contours
		for (unsigned int coi = 0; coi < contours.size(); coi++) {
			boundingRectangle = cv::boundingRect(contours[coi]);
			if (contourTaken[coi] == 0 && 
			max(boundingRectangle.width,boundingRectangle.height) > max(videoCapture.get(CV_CAP_PROP_FRAME_WIDTH),videoCapture.get(CV_CAP_PROP_FRAME_HEIGHT))/20) {
				// Get contour properties
				boundingRectangle = cv::boundingRect(contours[coi]);
				roiFrameMask =  foregroundFrameBuffer(boundingRectangle).clone();
				roiFrame = foregroundFrameBuffer(boundingRectangle).clone();
				roiFrameBuffer = rawFrame(boundingRectangle).clone();			
				roiFrameBuffer.copyTo(roiFrame, roiFrameMask);	
				
				// Create objects
				ID++;
				Blob newObject;
				newObject.ID = ID;
				newObject.frameCount = 1;
				newObject.firstFrameNumber = frameNumber;
				newObject.lastFrameNumber = frameNumber;
				newObject.firstRectangle = boundingRectangle;
				newObject.lastRectangle = boundingRectangle;
				newObject.frames.push_back(roiFrameBuffer);
				newObject.avgWidth = roiFrame.size().width;
				newObject.avgHeight = roiFrame.size().height;
				newObject.maxWidth = roiFrame.size().width;
				newObject.maxHeight = roiFrame.size().height;	
				blobContainer.push_back(newObject);
			}
		}

		// Draw rectangles
		for (unsigned int bli = 0; bli<blobContainer.size(); bli++) {
			if (blobContainer[bli].lastFrameNumber == frameNumber && blobContainer[bli].frameCount > 20) {//blobContainer[bli].frameCount > 10) {
				
				//rectangle(rawFrame, blobContainer[bli].lastRectangle, getRandomColorRGB(blobContainer[bli].ID));
				if (blobContainer[bli].collision == 1) 
					rectangle(rawFrame, blobContainer[bli].lastRectangle, cv::Scalar(0,0,255),2);	
				else
					rectangle(rawFrame, blobContainer[bli].lastRectangle, cv::Scalar(255,0,0),2);	
			}
		}
				
		// Convert from cv::Mat to Qimage. Source: StackOverflow (http://stackoverflow.com/questions/5026965/how-to-convert-an-opencv-cvmat-to-qimage)
		//frameImageDebug = QImage((uchar*) foregroundFrame.data, foregroundFrame.cols, foregroundFrame.rows, foregroundFrame.step, QImage::Format_Indexed8);
		//frameImageDebug = QImage((uchar*) roiFrame.data, roiFrame.cols, roiFrame.rows, roiFrame.step, QImage::Format_RGB888);
		//frameImageDebug = QImage((uchar*) contoursFrame.data, contoursFrame.cols, contoursFrame.rows, contoursFrame.step, QImage::Format_RGB888);

		//frameImage = QImage((uchar*) rawFrame.data, rawFrame.cols, rawFrame.rows, rawFrame.step, QImage::Format_RGB888);
		cv::resize(rawFrame, rawFrame, cv::Size(rawFrame.size().width*2, rawFrame.size().height*2), 0, 0, CV_INTER_LINEAR);
		frameImage = ImageFormat::Mat2QImage(rawFrame);
	
		// Set label pixmap to video frame
		videoFrameLabel->setPixmap(QPixmap::fromImage(frameImage));
		//videoFrameLabelDebug->setPixmap(QPixmap::fromImage(frameImageDebug));
	}

}

int MainWindow::exportObjectVideos() {
	vector<cv::VideoWriter> outputVideo; 

	// Loop though each blob and determine if it is stable enough to save as video.
	for (unsigned int i = 0; i < blobContainer.size(); i++) {
		
		// Determine stability.
		if (blobContainer[i].frames.size() > 30) {
		cout<<"Exporting subject: "<<i<<", frames: "<<blobContainer[i].frames.size()<<endl;
		outputVideo.push_back(cv::VideoWriter());
	
		// Create video files.
		stringstream si, sj;
		si<<i;
		sj<<blobContainer[i].frames.size();
		outputVideo.back().open(folder+"/subject"+si.str()+"_frames"+sj.str()+".avi", CV_FOURCC('D', 'I', 'B', ' '), 10, cv::Size(blobContainer[i].avgWidth*2,blobContainer[i].avgHeight*2), true);
		
		// Resize and export frames
		for (unsigned int j = 0; j < blobContainer[i].frames.size(); j++) {
			cv::resize(blobContainer[i].frames[j], blobContainer[i].frames[j], cv::Size(blobContainer[i].avgWidth*2,blobContainer[i].avgHeight*2), 0, 0, CV_INTER_LINEAR);
			//cv::resize(blobContainer[i].frames[j], blobContainer[i].frames[j], cv::Size(blobContainer[i].maxWidth*2,blobContainer[i].maxHeight*2), 0, 0, CV_INTER_LINEAR);
			outputVideo.back()<<blobContainer[i].frames[j];
		}
		
		// Release video files
		outputVideo.back().release();
		
		}
	}
}

// Select random color for object ID
cv::Scalar MainWindow::getRandomColorRGB(unsigned int ID) {
	vector<cv::Scalar> colors;
	colors.push_back(cv::Scalar(164,54,255));
	colors.push_back(cv::Scalar(255,0,0));
	colors.push_back(cv::Scalar(0,0,255));
	colors.push_back(cv::Scalar(0,255,0));
	colors.push_back(cv::Scalar(255,0,255));
	colors.push_back(cv::Scalar(255,255,0));
	colors.push_back(cv::Scalar(255,166,0));
	colors.push_back(cv::Scalar(0,133,0));
	colors.push_back(cv::Scalar(118,64,255));
	colors.push_back(cv::Scalar(64,245,265));
	colors.push_back(cv::Scalar(245,265,64));
	colors.push_back(cv::Scalar(255,64,150));
	colors.push_back(cv::Scalar(64,153,255));
	
	return colors[ID%colors.size()];	
}





