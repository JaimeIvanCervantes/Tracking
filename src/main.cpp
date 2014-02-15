/**
OBJECTIVE: Object recognition.
AUTHOR: Jaime I. Cervantes
LAST UPDATED: 01/17/2014
*/

#include <qapplication.h>
#include <mainwindow.h>

// Loads the main GUI
int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	MainWindow mwindow;
	mwindow.show();
	return app.exec();
}