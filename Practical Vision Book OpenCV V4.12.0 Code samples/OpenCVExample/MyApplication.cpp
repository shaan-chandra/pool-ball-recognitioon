
#include "Utilities.h"


void MyApplication()
{
	string filename = "Media/Pool/Table1.jpeg";
	Mat image = imread(filename);
	if (image.empty()) {
		cout << "Could not open image" << filename << endl;
	}
	imshow("Table 1", image); 
	waitKey(0);

}

