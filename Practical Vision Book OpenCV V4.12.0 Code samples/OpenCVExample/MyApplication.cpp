
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
const Point table_answers[7][16] = {
	{ // Table1 coordinates for balls
		Point(1092, 309), Point(1212, 429), Point(1131, 383), Point(1050, 335),   // balls 1-4
		Point(1211, 285), Point(1170, 261), Point(1173, 352), Point(1132, 331),   // balls 5-8
		Point(1210, 240), Point(1170, 310), Point(1169, 407), Point(1135, 281),   // balls 9-12
		Point(1212, 374), Point(1091, 360), Point(1213, 331), Point(29, 288)      // balls 13, 14, 15, White
	},
	{ // Table2 coordinates for balls
		Point(1103, 282), Point(1224, 620), Point(1121, 391), Point(1046, 315),   // balls 1-4
		Point(1305, 310), Point(621, 202), Point(1173, 361), Point(1132, 331),    // balls 5-8
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
	},
	{ // AllBalls coordinates for balls
		Point(908, 253), Point(959, 250), Point(1013, 255), Point(1072, 256),   // balls 1-4
		Point(1132, 254), Point(1184, 262), Point(1244, 256), Point(1307, 253), // balls 5-8
		Point(901, 329), Point(961, 330), Point(1013, 335), Point(1067, 336),   // balls 9-12
		Point(1130, 334), Point(1187, 345), Point(1245, 342), Point(797, 293)   // balls 13, 14, 15, White
	}
};
Mat getTableMatrix() {
	Point2f flat_table[4] = {
		Point2f(0.0f, 0.0f),// top left
		Point2f(table_width - 1.0f, 0.0f), // top right 
		Point2f(0, table_height - 1.0f), // bottom left 
		Point2f(table_width - 1.0f, table_height - 1.0f) // bottom right 
	};
	return getPerspectiveTransform(table_corner, flat_table);
}

Mat gridTable(const Mat& image) {
	Mat flat_image; 
	warpPerspective(image, flat_image, getTableMatrix(), Size(table_width, table_height));
	return flat_image;
}
Mat convertToHLS(const Mat& image) {
	Mat hls_image; 
	cvtColor(image, hls_image, COLOR_BGR2HLS);
	return hls_image;
}

vector<Vec3f> findCircles(const Mat& channel) {
	Mat median_blurred; 
	medianBlur(channel, median_blurred, 5);

	vector<Vec3f> circles;
	HoughCircles(median_blurred, circles, HOUGH_GRADIENT, 1, 28, 80, 15, 15, 28);
	return circles;
}
vector<Vec3f> centerInTable(const vector<Vec3f>& circles) {
	// find only circles with center in table or else hough will give wrong 
	vector<Point2f> border = { table_corner[0], table_corner[1], table_corner[3], table_corner[2] }; // top left, top right, bottom left, bottom right
	vector<Vec3f> tmp;
	for (int i = 0; i < circles.size(); i++) {
		Point2f center(circles[i][0], circles[i][1]);
		if (pointPolygonTest(border, center, false) >= 0) {
			tmp.push_back(circles[i]);
		}
	}
	return tmp;
}
vector<Vec3f> toFlat(const vector<Vec3f>& circles) {
	if (circles.empty()) {
		return circles;
	}
	vector<Point2f> photo_points; 
	for (int i = 0; i < circles.size(); i++) {
		photo_points.push_back(Point2f(circles[i][0], circles[i][1]));
	}
	vector<Point2f> flat_points;
	perspectiveTransform(photo_points, flat_points, getTableMatrix());

	vector<Vec3f> rebuild_circles;
	for (int i = 0; i < circles.size(); i++) {
		rebuild_circles.push_back(Vec3f(flat_points[i].x, flat_points[i].y, circles[i][2]));
	}
	return rebuild_circles;
}
vector<Vec4f> mergeCircles(const vector<vector<Vec3f>>& lists, float min_distance) {
	vector<Vec4f> merged; 
	// assumed lists holds the channels 
	for (int i = 0; i < (int)lists.size(); i++) {
		for (int j = 0; j < lists[i].size(); j++) {
			Point2f center(lists[i][j][0], lists[i][j][1]);
			bool duplicate = false;
			for (int check = 0; check< merged.size(); check++) {
				Point2f kept_center(merged[check][0], merged[check][1]);
				if (norm(center - kept_center) < min_distance) {
					merged[check][3] += 1;
					duplicate = true; 
					break; 
				}
			}
			if (!duplicate) {
				merged.push_back(Vec4f(lists[i][j][0], lists[i][j][1], lists[i][j][2], 1));
			}
		}
	}
	return merged;
}
vector<Vec4f> findCirclesInChannels(const vector<Mat>& channels) {
	string names[3] = { "hue", "lightness", "saturation",
	};
	vector < vector<Vec3f>> lists;
	for (int channel = 0; channel < channels.size(); channel++) {
		vector<Vec3f> found = findCircles(channels[channel]);
		found = centerInTable(found);
		found = toFlat(found);
		cout << "  " << names[channel] << ": " << found.size() << " circles" << endl;
		lists.push_back(found);
	}
	return mergeCircles(lists, 20);
}

bool isPocket(Point2f center, float zone) {
	Point2f pockets[6] = {
		Point2f(0,0),
		Point2f((table_width - 1) / 2.0f, 0.0f),
		Point2f(table_width - 1, 0.0f),
		Point2f(0.0f, table_height - 1),
		Point2f((table_width - 1) / 2.0f, table_height - 1),
		Point2f(table_width - 1, table_height - 1)
	};
	for (int p = 0; p < 6; p++) {
		if (norm(center - pockets[p]) < zone) {
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
vector<Vec4f> removePocketsAndEdges(const vector<Vec4f>& circle) {
	vector<Vec4f> kept;
	for (int i = 0; i < circle.size(); i++) {
		// get circle's center as a point2f
		Point2f center = Point2f(circle[i][0], circle[i][1]);
		// call isPocket and isNearEdge to see if it is if not add to kept 
		bool at_pocket = isPocket(center, 60);
		bool near_pocket = isPocket(center, 110);
		bool confirmed = circle[i][3] >= 2;
		if (!at_pocket && !isNearEdge(center) && (!near_pocket || confirmed)) {
			kept.push_back(circle[i]);
		}
	}
	return kept;
}

struct Score {
	int found = 0;
	int false_detection = 0;
	int missed = 0;
};

Score scoreTable(const vector<Vec4f>& detections, int table_number, float range) {
	// basically just compare the balls found to the ground truth described at the start of the table 
	// < 16 because that is table_answer size 
	Score s;
	vector<bool> flag(detections.size(), false);
	int actual_balls = 0; // actual balls in the photo as per the ground truth
	for (int i = 0; i < 16; i++) {
		Point ball_position = table_answers[table_number][i]; 
		// check for missing balls 
		if (ball_position.x == -1) {
			continue;
		}
		actual_balls++;
		for (int j = 0; j < detections.size(); j++) {
			if (flag[j]){
				continue;
			}
			Point2f position(detections[j][0], detections[j][1]);
			// compare distance 
			float distance = norm(position - Point2f(ball_position.x, ball_position.y));
			if (distance <= range) {
				// found ball add it 
				s.found++;
				flag[j] = true;
				// since it is comparing each ball with table answer break to get next i value
				break;
			}
		}
	}
	// calculate false_detection and missed here 
	// false detection 
	s.false_detection = detections.size() - s.found;
	s.missed = actual_balls - s.found;
	return s;
}
vector<Vec4f> locateBalls(const Mat& image) {
	// wrapping whole of stage 1 into one function that calls all stage 1 functions so that i find it easier to implement stage 2.
	Mat hls = convertToHLS(image); 
	vector<Mat> channels;
	split(hls, channels);
	vector<Vec4f> circles = findCirclesInChannels(channels);
	return removePocketsAndEdges(circles);
}

vector<Mat> ballMasks(Size size, const vector<Vec4f>& circles, int radius) {
	vector<Mat> masks;
	for (int i = 0; i < circles.size(); i++) {
		Mat mask(size, CV_8UC1, Scalar(0));
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		// mark each detected ball in the circle 
		circle(mask, center, radius, Scalar(255, 255, 255), FILLED);
		masks.push_back(mask);
	}
	return masks;
}
vector<Mat> describeBalls(const Mat channel, const vector<Mat>& masks, int bins, float max_value) {
	// basically get histogram from the detected circles mask that we previously built
	// compare against all balls to mark balls
	vector<Mat> histogram;
	int hist_channels[] = {0};
	int hist_size[] = {bins};
	float range[] = {0.0f, max_value};
	const float* ranges[] = { range };
	for (int i = 0; i < masks.size(); i++) {
		Mat hist;
		calcHist(&channel, 1, hist_channels, masks[i], hist, 1, hist_size, ranges);
		normalize(hist, hist, 1, 0, NORM_L1);
		histogram.push_back(hist);
	}
	return histogram;
}

// to draw histogram code copied from histogram.cpp
void Draw1DHistogram(MatND histograms[], int number_of_histograms, Mat& display_image)
{
	int number_of_bins = histograms[0].size[0];
	double max_value = 0, min_value = 0;
	double channel_max_value = 0, channel_min_value = 0;
	for (int channel = 0; (channel < number_of_histograms); channel++)
	{
		minMaxLoc(histograms[channel], &channel_min_value, &channel_max_value, 0, 0);
		max_value = ((max_value > channel_max_value) && (channel > 0)) ? max_value : channel_max_value;
		min_value = ((min_value < channel_min_value) && (channel > 0)) ? min_value : channel_min_value;
	}
	float scaling_factor = ((float)256.0) / ((float)number_of_bins);

	Mat histogram_image((int)(((float)number_of_bins) * scaling_factor) + 1, (int)(((float)number_of_bins) * scaling_factor) + 1, CV_8UC3, Scalar(255, 255, 255));
	display_image = histogram_image;
	line(histogram_image, Point(0, 0), Point(0, histogram_image.rows - 1), Scalar(0, 0, 0));
	line(histogram_image, Point(histogram_image.cols - 1, histogram_image.rows - 1), Point(0, histogram_image.rows - 1), Scalar(0, 0, 0));
	int highest_point = static_cast<int>(0.9 * ((float)number_of_bins) * scaling_factor);
	for (int channel = 0; (channel < number_of_histograms); channel++)
	{
		int last_height;
		for (int h = 0; h < number_of_bins; h++)
		{
			float value = histograms[channel].at<float>(h);
			int height = static_cast<int>(value * highest_point / max_value);
			int where = (int)(((float)h) * scaling_factor);
			if (h > 0)
				line(histogram_image, Point((int)(((float)(h - 1)) * scaling_factor) + 1, (int)(((float)number_of_bins) * scaling_factor) - last_height),
					Point((int)(((float)h) * scaling_factor) + 1, (int)(((float)number_of_bins) * scaling_factor) - height),
					Scalar(channel == 0 ? 255 : 0, channel == 1 ? 255 : 0, channel == 2 ? 255 : 0));
			last_height = height;
		}
	}
}

void printScore(string label, Score s) {
	int detections = s.found + s.false_detection;
	int all_balls = s.found + s.missed; 

	double precision = (detections > 0)? (double)s.found / detections : 0; 
	double recall = (all_balls > 0) ? (double)s.found / all_balls: 0; 
	double F1 = (precision + recall > 0) ? 2 * precision * recall / (precision + recall) : 0;
	cout << "label: " << label << endl;
	cout << "found: " << s.found << endl;
	cout << "false positives: " << s.false_detection << endl;
	cout << "missed detections: " << s.missed << endl;
	cout << "precision: " << precision << endl;
	cout << "recall: " << recall << endl;
	cout << "F1: " << F1 << endl;
}
void analyseImage(const Mat& image, vector<Vec4f>& circles, vector<Mat>& hue_histograms, vector<Mat>& light_histogram) {
	// this function fills the data needed for stage 2 and is reusable. 
	circles = locateBalls(image);
	Mat flat_image = gridTable(image);
	Mat hls = convertToHLS(flat_image);
	vector<Mat> channels;
	split(hls, channels);
	vector<Mat> masks = ballMasks(flat_image.size(), circles, 16);
	hue_histograms = describeBalls(channels[0], masks, 18, 180);
	light_histogram = describeBalls(channels[1], masks, 16, 256);
}
vector<int> identifyBall(const vector<Vec4f>& circles, const Point tableAnswers[16]) {
	vector<int> results;
	for (int i = 0; i < circles.size(); i++) {
		Point2f center(circles[i][0], circles[i][1]);
		// compare to ground truth from table_answers above to see what ball it is resubale function will be used for comparing
		// all balls and detected balls
		double best = 1e9;
		int best_ball = -1;
		for (int j = 0; j < 16; j++) {
			if (tableAnswers[j].x < 0) {
				continue;
			}
			double distance = norm(center - Point2f(tableAnswers[j]));
			if (distance < best) {
				best = distance;
				best_ball = j;
			}
		}
		results.push_back(best_ball);
	}
	return results;
}
int nameBall(const Mat& hue, const Mat& light, const vector<Mat>& hue_ref, const vector<Mat>& light_ref) {
	double best = 1e9;
	int best_ball = -1;
	for (int i = 0; i < 16; i++) {
		double distance = compareHist(hue, hue_ref[i], HISTCMP_BHATTACHARYYA) + compareHist(light, light_ref[i], HISTCMP_BHATTACHARYYA);
		if (distance < best) {
			best = distance;
			best_ball = i;
		}
	}
	return best_ball;
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
	Score total;
	Mat allBalls_image = imread("Media/Pool/AllBalls.jpeg");
	vector<Vec4f> allBalls_circles;
	vector<Mat> allBalls_hueHistogram; vector<Mat> allBalls_lightHistogram;
	analyseImage(allBalls_image, allBalls_circles, allBalls_hueHistogram, allBalls_lightHistogram);
	vector<int> allBalls_numbers = identifyBall(allBalls_circles, table_answers[6]);
	vector<Mat> reference_hue(16); vector<Mat> reference_light(16);
	for (int i = 0; i < allBalls_circles.size(); i++) {
		int n = allBalls_numbers[i];
		reference_hue[n] = allBalls_hueHistogram[i];
		reference_light[n] = allBalls_lightHistogram[i];
		cout << "circle " << i << " -> ball " << n + 1 << endl;
	}
	for (int i = 0; i < 6; i++) {
		Mat image = imread(image_files[i]);
		if (image.empty()) {
			cout << "Could not open image" << image_files[i] << endl;
			return;
		}
		flat_images[i] = gridTable(image);

		Mat hls = convertToHLS(image);
		vector<Mat> channels;
		split(hls, channels);
		imshow("Hue", channels[0]);
		imshow("Lightness", channels[1]);
		imshow("Saturation", channels[2]);
		imshow("Table ", flat_images[i]);
		cout << "Table " << i + 1 << ":" << endl;
		vector<Vec4f> circles = findCirclesInChannels(channels);
		circles = removePocketsAndEdges(circles);
		Score s = scoreTable(circles, i, 21);
		printScore("Table " + to_string(i + 1), s);
		if (i < 6) {
			total.found += s.found;
			total.false_detection += s.false_detection;
			total.missed += s.missed;
			cout << "Table " << i + 1 << ": " << circles.size() << " circles found" << endl;
		}
		Mat flat_hls = convertToHLS(flat_images[i]);
		vector<Mat> flat_channels;
		split(flat_hls, flat_channels);
		vector<Mat> masks = ballMasks(flat_images[i].size(), circles, 16);
		vector<Mat> hue_hists = describeBalls(flat_channels[0], masks, 18, 180);
		vector<Mat> light_hists = describeBalls(flat_channels[1], masks, 16, 256);
		for (size_t k = 0; k < circles.size(); k++) {
			Point hue_peak, light_peak;
			minMaxLoc(hue_hists[k], 0, 0, 0, &hue_peak);
			minMaxLoc(light_hists[k], 0, 0, 0, &light_peak);
			cout << "  (" << cvRound(circles[k][0]) << ", " << cvRound(circles[k][1]) << ")"
				<< "  hue bin " << hue_peak.y << "  light bin " << light_peak.y << endl;
		}
		Mat found = flat_images[i].clone();
		for (size_t c = 0; c < circles.size(); c++) {
			Point centre(cvRound(circles[c][0]), cvRound(circles[c][1]));
			circle(found, centre, 21, Scalar(0, 255, 0), 2);

			int name = nameBall(hue_hists[c], light_hists[c], reference_hue, reference_light);
			string label = (name == 15) ? "W" : to_string(name + 1);
			putText(found, label, centre + Point(-10, -25), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 0, 255), 2);
		}
		imshow("Hough circles", found);
		waitKey(0); 
		destroyAllWindows();
	}
	printScore("overall: ", total);
}

