#include <iostream>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>

using namespace std;

float evaluation(cv::Mat RE,cv::Mat GT) {
	float Overlap = 0.0, Union = 0.0;
	for (int n = 0; n < RE.rows; n++) {
		for (int m = 0; m < RE.cols; m++) {
			if (RE.at<cv::Vec3b>(n, m) == cv::Vec3b(255, 255, 255) || GT.at<cv::Vec3b>(n, m) == cv::Vec3b(255, 255, 255)) {
				Union += 1.0;
				if (RE.at<cv::Vec3b>(n, m) == cv::Vec3b(255, 255, 255) && GT.at<cv::Vec3b>(n, m) == cv::Vec3b(255, 255, 255)) {
					Overlap += 1.0;
				}
			}
		}
	}

	return Overlap / Union;
}

int main(int argc, char** argv) {

	//テスト画像ファイル一覧メモ帳読み込み
	char GT_name[1024], result_name[1024];
	FILE *GT_data, *result_data;
	fopen_s(&GT_data, "", "r");
	fopen_s(&result_data, "", "r");

	while (fgets(GT_name, 256, GT_data) != NULL && fgets(result_name, 256, result_data) != NULL) {
		
		string name_GT = GT_name;
		char new_GT_name[1024];
		for (int i = 0; i < name_GT.length() - 1; i++) {
			new_GT_name[i] = GT_name[i];
			new_GT_name[i + 1] = '\0';
		}

		string name_res = result_name;
		char new_result_name[1024];
		for (int i = 0; i < name_res.length() - 1; i++) {
			new_result_name[i] = result_name[i];
			new_result_name[i + 1] = '\0';
		}

		cv::Mat GT = cv::imread(new_GT_name, 1);
		cv::Mat RE = cv::imread(new_result_name, 1);
		cout << new_result_name << ":" << evaluation(RE, GT) << endl;

	}

	fclose(GT_data);
	fclose(result_data);

	return 0;
}
