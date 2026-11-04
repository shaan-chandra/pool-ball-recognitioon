
#include "Utilities.h"

const int table_width = 1434;
const int table_height = 683;

const Point2f table_corner[4] = {
	Point2f(337.0f, 35.0f), // top left
	Point2f(1570.0f, 21.0f), // top right
	Point2f(38.0f, 472.0f), // bottom left 
	Point2f(1888.0f, 451.0f) // bottom right 
};

// ground truth values of the ball positions will be later used for comparision 
const Point table_answers[6][16] = {
	{ // Table1 coordinates for balls
		Point(1092, 309), Point(1212, 429), Point(1174, 353), Point(1050, 335),   // balls 1-4  (2, 4 corrected)
		Point(1211, 285), Point(1170, 261), Point(1173, 352), Point(1132, 331),   // balls 5-8
		Point(1210, 240), Point(1170, 310), Point(1169, 407), Point(1135, 281),   // balls 9-12 (11 corrected)
		Point(1212, 374), Point(1091, 360), Point(1213, 331), Point(29, 288)      // balls 13, 14, 15, White
	},
	{ // Table2 coordinates for balls
		Point(1103, 282), Point(1224, 620), Point(1121, 391), Point(1305, 310),   // balls 1-4
		Point(1046, 315), Point(621, 202), Point(1173, 361), Point(1132, 331),    // balls 5-8
		Point(725, 177), Point(1269, 185), Point(798, 665), Point(438, 296),      // balls 9-12
		Point(1235, 53), Point(1090, 362), Point(1205, 399), Point(1169, 507)     // balls 13, 14, 15, White
	},
	{ // Table3 coordinates for balls
		Point(908, 23), Point(1224, 616), Point(1199, 447), Point(29, 336),       // balls 1-4
		Point(1406, 305), Point(621, 210), Point(1294, 260), Point(1252, 226),    // balls 5-8
		Point(725, 177), Point(1270, 171), Point(798, 665), Point(351, 249),      // balls 9-12
		Point(1233, 53), Point(901, 169), Point(1201, 399), Point(29, 434)        // balls 13, 14, 15, White
	},
	{ // Table4 coordinates for balls
		Point(908, 23), Point(1407, 546), Point(1204, 444), Point(-1, -1),        // balls 1-4
		Point(1406, 305), Point(621, 210), Point(1294, 260), Point(1252, 226),    // balls 5-8
		Point(724, 181), Point(1270, 171), Point(798, 665), Point(351, 249),      // balls 9-12
		Point(1231, 58), Point(901, 169), Point(1201, 399), Point(1392, 88)       // balls 13, 14, 15, White
	},
	{ // Table5 coordinates for balls
		Point(23, 535), Point(1407, 546), Point(1204, 444), Point(-1, -1),        // balls 1-4
		Point(1406, 305), Point(25, 131), Point(1294, 260), Point(1252, 226),     // balls 5-8
		Point(72, 538), Point(1270, 171), Point(798, 665), Point(351, 249),       // balls 9-12
		Point(-1, -1), Point(901, 169), Point(1201, 399), Point(26, 608)          // balls 13, 14, 15, White
	},
	{ // Table6 coordinates for balls
		Point(23, 535), Point(1364, 477), Point(1204, 444), Point(-1, -1),        // balls 1-4
		Point(1406, 305), Point(25, 131), Point(1294, 260), Point(1252, 226),     // balls 5-8
		Point(72, 538), Point(1270, 171), Point(-1, -1), Point(351, 249),         // balls 9-12
		Point(-1, -1), Point(901, 169), Point(1201, 399), Point(947, 629)         // balls 13, 14, 15, White
	}
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

void drawCircle(Mat& image, int table_number) {
	for (int i = 0; i < 16; i++) {
		Point point_ball = table_answers[table_number][i];
		if (point_ball.x != -1) {
			circle(image, point_ball, 21, Scalar(0, 0, 255), 2);
		}
	}
}

Mat convertToHLS(Mat& image) {
	Mat hls_image; 
	cvtColor(image, hls_image, COLOR_BGR2HLS);
	return hls_image;
}

vector<Vec3f> findCircles(const Mat& channel) {
	Mat median_blurred; 
	medianBlur(channel, median_blurred, 5);

	vector<Vec3f> circles;
	HoughCircles(median_blurred, circles, HOUGH_GRADIENT, 1, 40, 50, 15, 18, 32);
	return circles;
}
vector<Vec3f> mergeCircles(const vector<vector<Vec3f>>& lists, float min_distance) {
	vector<Vec3f> merged; 
	// assumed lists holds the channels 
	for (int i = 0; i < (int)lists.size(); i++) {
		for (int j = 0; j < lists[i].size(); j++) {
			Point2f center(lists[i][j][0], lists[i][j][1]);
			bool duplicate = false;
			for (int check = 0; check< merged.size(); check++) {
				Point2f kept_center(merged[check][0], merged[check][1]);
				if (norm(center - kept_center) < min_distance) {
					duplicate = true; 
					break; 
				}
			}
			if (!duplicate) {
				merged.push_back(lists[i][j]);
			}
		}
	}
	return merged;
}
vector<Vec3f> findCirclesInChannels(const vector<Mat>& channels) {
	string names[3] = { "hue", "lightness", "saturation",
	};
	vector < vector<Vec3f>> lists;
	for (int channel = 0; channel < channels.size(); channel++) {
		vector<Vec3f> found = findCircles(channels[channel]);
		cout << "  " << names[channel] << ": " << found.size() << " circles" << endl;
		lists.push_back(found);
	}
	return mergeCircles(lists, 30);
}

bool isPocket(Point2f center) {
	Point2f pockets[6] = {
		Point2f(0,0),
		Point2f((table_width - 1) / 2.0f, 0.0f),
		Point2f(table_width - 1, 0.0f),
		Point2f(0.0f, table_height - 1),
		Point2f((table_width - 1) / 2.0f, table_height - 1),
		Point2f(table_width - 1, table_height - 1)
	};
	for (int p = 0; p < 6; p++) {
		if (norm(center - pockets[p]) < 60) {
			return true;
		}
	}
	return false;
}

bool isNearEdge(Point2f center) {
	const float edge_distance = 18.0f;
	if (center.x < edge_distance ||
		center.x >(table_width - 1) - edge_distance ||
		center.y < edge_distance ||
		center.y >(table_height - 1) - edge_distance) {
		return true;
	}
	return false;
}
vector<Vec3f> removePocketsAndEdges(const vector<Vec3f>& circle) {
	vector<Vec3f> kept;
	for (int i = 0; i < circle.size(); i++) {
		// get circle's center as a point2f
		Point2f center = Point2f(circle[i][0], circle[i][1]);
		// call isPocket and isNearEdge to see if it is if not add to kept 
		if (!isPocket(center) && !isNearEdge(center)) {
			kept.push_back(circle[i]);
		}
	}
	return kept;
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

		// Step 5: HLS channels
		Mat hls = convertToHLS(flat_images[i]);
		vector<Mat> channels;
		split(hls, channels);
		imshow("Hue", channels[0]);
		imshow("Lightness", channels[1]);
		imshow("Saturation", channels[2]);
		imshow("Table ", flat_images[i]);
		cout << "Table " << i + 1 << ":" << endl;
		vector<Vec3f> circles = findCirclesInChannels(channels);
		circles = removePocketsAndEdges(circles);
		cout << "Table " << i + 1 << ": " << circles.size() << " circles found" << endl;
		Mat found = flat_images[i].clone();
		for (size_t c = 0; c < circles.size(); c++) {
			Point centre(cvRound(circles[c][0]), cvRound(circles[c][1]));
			circle(found, centre, 21, Scalar(0, 255, 0), 2);
		}
		imshow("Hough circles", found);
		waitKey(0); 
		destroyAllWindows();
	}
}

