
#include "Utilities.h"

const int table_width = 1434;
const int table_height = 683;

const Point2f table_corner[4] = {
	Point2f(337.0f, 35.0f), // top left
	Point2f(1570.0f, 21.0f), // top right
	Point2f(38.0f, 472.0f), // bottom left 
	Point2f(1888.0f, 451.0f) // bottom right 
};

Mat gridTable(const Mat& image) {
	Point2f flat_table[4] = {
		Point2f(0.0f, 0.0f),// top left
		Point2f(table_width - 1.0f, 0.0f), // top right 
		Point2f(0, table_height - 1.0f), // bottom left 
		Point2f(table_width - 1.0f, table_height - 1.0f) // bottom right 
	};
	Mat table_matrix = getPerspectiveTransform(table_corner, flat_table);
	Mat flat_image; 
	warpPerspective(image, flat_image, table_matrix, Size(table_width, table_height));
	return flat_image;
}
void MyApplication()
{
	string image_files[6] = {
		"Media/Pool/Table1.jpeg",
		"Media/Pool/Table2.jpeg",
		"Media/Pool/Table3.jpeg",
		"Media/Pool/Table4.jpeg",
		"Media/Pool/Table5.jpeg",
		"Media/Pool/Table6.jpeg"
	};
	Mat flat_images[6];
	for (int i = 0; i < 6; i++) {
		Mat image = imread(image_files[i]);
		if (image.empty()) {
			cout << "Could not open image" << image_files[i] << endl;
			return;
		}
		flat_images[i] = gridTable(image);
		imshow("Table ", flat_images[i]);
		waitKey(0); 
		destroyAllWindows();
	}

}

