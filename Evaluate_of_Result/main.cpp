#include <iostream>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>

using namespace std;

#define YUDO 0.5
#define IOU_OUTPUT 0.9

class Place {
public:
	int x;
	int y;
	int width;
	int height;
	float yudo;

	Place() {
		x = y = -1;
		width = height = -1;
		yudo = 0.0;
	}
};

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

void Result_IoU() {
	//テスト画像ファイル一覧メモ帳読み込み
	char GT_name[1024];
	FILE *GT_data;
	fopen_s(&GT_data, "C:/photo/GT_binary_list2.txt", "r");

	while (fgets(GT_name, 256, GT_data) != NULL) {

		string name_GT = GT_name;
		char new_GT_name[1024];
		char new_result_name[1024];
		for (int i = 0; i < name_GT.length() - 1; i++) {
			new_GT_name[i] = GT_name[i];
			new_GT_name[i + 1] = '\0';
			new_result_name[i] = GT_name[i];
			new_result_name[i + 1] = '\0';
		}
		char GT_n[1024] = "C:/photo/GT_binary/";
		char RE_n[1024] = "C:/photo/result_data_from_demo/CD-0.99_FD-0.999/result_binary/";
		strcat_s(GT_n, new_GT_name);
		strcat_s(RE_n, new_result_name);
		cv::Mat GT = cv::imread(GT_n, 1);
		cv::Mat RE = cv::imread(RE_n, 1);
		//		cv::imshow("GT", GT);
		//		cv::imshow("RE", RE);
		//		cvWaitKey(10);
		cout << new_result_name << "," << evaluation(RE, GT) << endl;

	}

	fclose(GT_data);

}

cv::Mat draw_rectangle(cv::Mat ans_im, int x, int y, int width, int height, int r, int g, int b) {
	rectangle(ans_im, cvPoint(x, y), cvPoint(x + width, y + height), CV_RGB(r, g, b), 1);
	return ans_im;
}

void Result_ROC(float yudo,char* Result_file,char* Save_file) {
	//変数宣言
	float miss_rate = 0;
	float fppi = 0;
	int num = 0;
	float TP = 0;
	float FP = 0;
	int FN = 0;
	int parts = 0;

	FILE* Save;
	if (fopen_s(&Save, Save_file, "a") != 0) {
		cout << "error" << endl;
		return;
	}

	//テキストファイルのリスト読み込み
	char List_n[1024];
	FILE *List;
	if (fopen_s(&List, "c:/photo/text_list_4.txt", "r") != 0) {
		cout << "not found List file" << endl;
		return;
	}
	while (fgets(List_n, 256, List)!=NULL) {
		string List_str = List_n;
		char List_name[1024];
		for (int i = 0; i < List_str.length() - 1; i++) {
			List_name[i] = List_n[i];
			List_name[i + 1] = '\0';
		}


		char GT_name[1024] = "c:/photo/GT_text/";
		strcat_s(GT_name, List_name);
		//GTファイル読み込み
		char GT_n[4][1024];
		FILE *GT;
		if (fopen_s(&GT, GT_name, "r") != 0) {
			cout << "not found GT file" << endl;
			return;
		}

		Place place_GT[10];
		int num_G = 0;
		while (fgets(GT_n[0], 256, GT) != NULL) {	//すべて読み込み，変数に格納
			fgets(GT_n[1], 256, GT);
			fgets(GT_n[2], 256, GT);
			fgets(GT_n[3], 256, GT);
			
			place_GT[num_G].x = atoi(GT_n[0]);
			place_GT[num_G].y = atoi(GT_n[1]);
			place_GT[num_G].width = atoi(GT_n[2]);
			place_GT[num_G].height = atoi(GT_n[3]);
			num_G++;
		}
		fclose(GT);


		char Result_name[1024];
		for (int i = 0;; i++) {
			Result_name[i] = Result_file[i];
			if (Result_file[i] == '\0')break;
		}

		strcat(Result_name, List_name);
		//Resultファイル読み込み
		char Result_n[5][1024];
		FILE *Result;
		if (fopen_s(&Result, Result_name, "r") != 0) {
			cout << "not found Result file" << endl;
			return;
		}

		Place place_Result[100];
		int num_R = 0;
		while (fgets(Result_n[0], 256, Result) != NULL) {	//すべて読み込み，変数に格納
			fgets(Result_n[1], 256, Result);
			fgets(Result_n[2], 256, Result);
			fgets(Result_n[3], 256, Result);
			fgets(Result_n[4], 256, Result);

			place_Result[num_R].yudo = atof(Result_n[0]);
			place_Result[num_R].x = atoi(Result_n[1]);
			place_Result[num_R].y = atoi(Result_n[2]);
			place_Result[num_R].width = atoi(Result_n[3]);
			place_Result[num_R].height = atoi(Result_n[4]);

			//尤度チェック
			if (place_Result[num_R].yudo < yudo) {
				place_Result[num_R].yudo = 0;
				place_Result[num_R].x = -1;
				place_Result[num_R].y = -1;
				place_Result[num_R].width = -1;
				place_Result[num_R].height = -1;
				continue;
			}

			num_R++;
		}
		fclose(Result);

		//2値画像作成
		//GT
		cv::Mat GT_B[10] = {
			cv::Mat::zeros(480, 640, CV_8UC3),
			cv::Mat::zeros(480, 640, CV_8UC3),
			cv::Mat::zeros(480, 640, CV_8UC3),
			cv::Mat::zeros(480, 640, CV_8UC3),
			cv::Mat::zeros(480, 640, CV_8UC3),
			cv::Mat::zeros(480, 640, CV_8UC3),
			cv::Mat::zeros(480, 640, CV_8UC3),
			cv::Mat::zeros(480, 640, CV_8UC3),
			cv::Mat::zeros(480, 640, CV_8UC3),
			cv::Mat::zeros(480, 640, CV_8UC3),
		};
		for (int i = 0; i < num_G; i++) {
			for (int n = place_GT[i].y; n < place_GT[i].y + place_GT[i].height; n++) {
				for (int m = place_GT[i].x; m < place_GT[i].x + place_GT[i].width; m++) {
					GT_B[i].at<cv::Vec3b>(n, m) = cv::Vec3b(255, 255, 255);
				}
			}
		}

		//Result
		cv::Mat Result_B[100];

		for (int i = 0; i < 100; i++) {
			Result_B[i] = cv::Mat::zeros(480, 640, CV_8UC3);
		}

		for (int i = 0; i < num_R; i++) {
			if (place_Result[i].yudo < yudo) continue;
			for (int n = place_Result[i].y; n < place_Result[i].y + place_Result[i].height; n++) {
				for (int m = place_Result[i].x; m < place_Result[i].x + place_Result[i].width; m++) {
					Result_B[i].at<cv::Vec3b>(n, m) = cv::Vec3b(255, 255, 255);
				}
			}
		}

		float Pre_num[10][100];
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 100; j++) {
				Pre_num[i][j] = 0;
			}
		}

		//Precision(=(GT&RE)/GT)の計算
		for (int i = 0; i < num_G; i++) {
			for (int j = 0; j < num_R; j++) {
				//Precision計算
				float Overlap = 0.0, Union = 0.0;
				for (int n = 0; n < Result_B[j].rows; n++) {
					for (int m = 0; m < Result_B[j].cols; m++) {
						if (GT_B[i].at<cv::Vec3b>(n, m) == cv::Vec3b(255, 255, 255)) {
							Union += 1.0;
							if (Result_B[j].at<cv::Vec3b>(n, m) == cv::Vec3b(255, 255, 255)) {
								Overlap += 1.0;
							}
						}
					}
				}
				Pre_num[i][j] = Overlap / Union;
			}
		}
		float TP_tmp = TP;
		for (int i = 0; i < num_R; i++) {
			if (Pre_num[0][i] >= 0.7 || Pre_num[1][i] >= 0.7) {TP+=1.0;}
			else {FP+=1.0;}

			if (0.3 <= Pre_num[0][i] && Pre_num[0][i] <= 0.7 || 0.3 <= Pre_num[1][i] && Pre_num[1][i] <= 0.7) {
				parts++;
			}
		}
		if ((TP - TP_tmp) < num_G) {
			FN += num_G - (TP - TP_tmp);
		}

	}

	fprintf_s(Save, "%f, %.0f, %.0f, %d, %d\n", yudo, TP, FP, FN, parts);
	fclose(Save);
	fclose(List);
}

void Result_ROC_2(float yudo, char* Result_file, char* Save_file, int flag) {
	//変数宣言
	float miss_rate = 0;
	float fppi = 0;
	int num = 0;
	float TP = 0;
	float FP = 0;
	int FN = 0;
	int parts = 0;

	FILE* Save;
	if (fopen_s(&Save, Save_file, "a") != 0) {
		cout << "error" << endl;
		return;
	}

	//テキストファイルのリスト読み込み
	char List_n[1024];
	FILE *List;
	if (fopen_s(&List, "c:/photo/text_list_4.txt", "r") != 0) {
		cout << "not found List file" << endl;
		return;
	}
	while (fgets(List_n, 256, List) != NULL) {
		string List_str = List_n;
		char List_name[1024];
		for (int i = 0; i < List_str.length() - 1; i++) {
			List_name[i] = List_n[i];
			List_name[i + 1] = '\0';
		}


		char GT_name[1024] = "c:/photo/GT_text/";
		strcat_s(GT_name, List_name);
		//GTファイル読み込み
		char GT_n[4][1024];
		FILE *GT;
		if (fopen_s(&GT, GT_name, "r") != 0) {
			cout << "not found GT file" << endl;
			return;
		}

		Place place_GT[10];
		int num_G = 0;
		while (fgets(GT_n[0], 256, GT) != NULL) {	//すべて読み込み，変数に格納
			fgets(GT_n[1], 256, GT);
			fgets(GT_n[2], 256, GT);
			fgets(GT_n[3], 256, GT);

			place_GT[num_G].x = atoi(GT_n[0]);
			place_GT[num_G].y = atoi(GT_n[1]);
			place_GT[num_G].width = atoi(GT_n[2]);
			place_GT[num_G].height = atoi(GT_n[3]);
			num_G++;
		}
		fclose(GT);

		char Result_name[1024];
		for (int i = 0;; i++) {
			Result_name[i] = Result_file[i];
			if (Result_file[i] == '\0')break;
		}

		strcat(Result_name, List_name);
		//Resultファイル読み込み
		char Result_n[10][1024];
		FILE *Result;
		if (fopen_s(&Result, Result_name, "r") != 0) {
			cout << "not found Result file" << endl;
			return;
		}

		Place place_Result[100];
		int num_R = 0;
		while (fgets(Result_n[0], 256, Result) != NULL) {	//すべて読み込み，変数に格納
			fgets(Result_n[1], 256, Result);
			fgets(Result_n[2], 256, Result);
			fgets(Result_n[3], 256, Result);
			fgets(Result_n[4], 256, Result);
			fgets(Result_n[5], 256, Result);
			fgets(Result_n[6], 256, Result);
			fgets(Result_n[7], 256, Result);
			fgets(Result_n[8], 256, Result);
			fgets(Result_n[9], 256, Result);

			if (flag == 1) {
				place_Result[num_R].yudo = atof(Result_n[0]);
				place_Result[num_R].x = atoi(Result_n[1]);
				place_Result[num_R].y = atoi(Result_n[2]);
				place_Result[num_R].width = atoi(Result_n[3]);
				place_Result[num_R].height = atoi(Result_n[4]);
			}
			else {
				place_Result[num_R].yudo = atof(Result_n[0]);
				place_Result[num_R].x = atoi(Result_n[6]);
				place_Result[num_R].y = atoi(Result_n[7]);
				place_Result[num_R].width = atoi(Result_n[8]);
				place_Result[num_R].height = atoi(Result_n[9]);
			}
			

			//尤度チェック
			if (place_Result[num_R].yudo < yudo) {
				place_Result[num_R].yudo = 0;
				place_Result[num_R].x = -1;
				place_Result[num_R].y = -1;
				place_Result[num_R].width = -1;
				place_Result[num_R].height = -1;
				continue;
			}

			num_R++;
		}
		fclose(Result);

		//2値画像作成
		//GT
		cv::Mat GT_B[10] = {
			cv::Mat::zeros(480, 640, CV_8UC3),
			cv::Mat::zeros(480, 640, CV_8UC3),
			cv::Mat::zeros(480, 640, CV_8UC3),
			cv::Mat::zeros(480, 640, CV_8UC3),
			cv::Mat::zeros(480, 640, CV_8UC3),
			cv::Mat::zeros(480, 640, CV_8UC3),
			cv::Mat::zeros(480, 640, CV_8UC3),
			cv::Mat::zeros(480, 640, CV_8UC3),
			cv::Mat::zeros(480, 640, CV_8UC3),
			cv::Mat::zeros(480, 640, CV_8UC3),
		};
		for (int i = 0; i < num_G; i++) {
			for (int n = place_GT[i].y; n < place_GT[i].y + place_GT[i].height; n++) {
				for (int m = place_GT[i].x; m < place_GT[i].x + place_GT[i].width; m++) {
					GT_B[i].at<cv::Vec3b>(n, m) = cv::Vec3b(255, 255, 255);
				}
			}
		}

		//Result
		cv::Mat Result_B[100];

		for (int i = 0; i < 100; i++) {
			Result_B[i] = cv::Mat::zeros(480, 640, CV_8UC3);
		}

		for (int i = 0; i < num_R; i++) {
			if (place_Result[i].yudo < yudo) continue;
			for (int n = place_Result[i].y; n < place_Result[i].y + place_Result[i].height; n++) {
				for (int m = place_Result[i].x; m < place_Result[i].x + place_Result[i].width; m++) {
					Result_B[i].at<cv::Vec3b>(n, m) = cv::Vec3b(255, 255, 255);
				}
			}
		}

		float Pre_num[10][100];
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 100; j++) {
				Pre_num[i][j] = 0;
			}
		}

		//Precision(=(GT&RE)/GT)の計算
		for (int i = 0; i < num_G; i++) {
			for (int j = 0; j < num_R; j++) {
				//Precision計算
				float Overlap = 0.0, Union = 0.0;
				for (int n = 0; n < Result_B[j].rows; n++) {
					for (int m = 0; m < Result_B[j].cols; m++) {
						if (GT_B[i].at<cv::Vec3b>(n, m) == cv::Vec3b(255, 255, 255)) {
							Union += 1.0;
							if (Result_B[j].at<cv::Vec3b>(n, m) == cv::Vec3b(255, 255, 255)) {
								Overlap += 1.0;
							}
						}
					}
				}
				Pre_num[i][j] = Overlap / Union;
			}
		}
		float TP_tmp = TP;
		for (int i = 0; i < num_R; i++) {
			if (Pre_num[0][i] >= 0.7 || Pre_num[1][i] >= 0.7) { TP += 1.0; }
			else { FP += 1.0; }

			if (0.3 <= Pre_num[0][i] && Pre_num[0][i] <= 0.7 || 0.3 <= Pre_num[1][i] && Pre_num[1][i] <= 0.7) {
				parts++;
			}
		}
		if ((TP - TP_tmp) < num_G) {
			FN += num_G - (TP - TP_tmp);
		}

	}

	fprintf_s(Save, "%f, %.0f, %.0f, %d, %d\n", yudo, TP, FP, FN, parts);
	fclose(Save);
	fclose(List);
}

void IoU_Result() {
	//テキストファイルのリスト読み込み
	char List_n[1024];
	FILE *List;
	if (fopen_s(&List, "c:/photo/text_list_4.txt", "r") != 0) {
		cout << "not found List file" << endl;
		return;
	}
	while (fgets(List_n, 256, List) != NULL) {
		string List_str = List_n;
		char List_name[1024];
		for (int i = 0; i < List_str.length() - 1; i++) {
			List_name[i] = List_n[i];
			List_name[i + 1] = '\0';
		}
		char GT_name[1024] = "c:/photo/GT_text/";
		strcat_s(GT_name, List_name);
		//GTファイル読み込み
		char GT_n[4][1024];
		FILE *GT;
		if (fopen_s(&GT, GT_name, "r") != 0) {
			cout << "not found GT file" << endl;
			return;
		}
		Place place_GT[10];
		int num_G = 0;
		while (fgets(GT_n[0], 256, GT) != NULL) {	//すべて読み込み，変数に格納
			fgets(GT_n[1], 256, GT);
			fgets(GT_n[2], 256, GT);
			fgets(GT_n[3], 256, GT);

			place_GT[num_G].x = atoi(GT_n[0]);
			place_GT[num_G].y = atoi(GT_n[1]);
			place_GT[num_G].width = atoi(GT_n[2]);
			place_GT[num_G].height = atoi(GT_n[3]);
			num_G++;
		}
		fclose(GT);

		char Result_name[1024] = "C:/photo/result_data_from_demo/2018_01_15_AP/save_data/0.7_40/";
		strcat_s(Result_name, List_name);
		//Resultファイル読み込み
		char Result_n[5][1024];
		FILE *Result;
		if (fopen_s(&Result, Result_name, "r") != 0) {
			cout << "not found Result file" << endl;
			return;
		}
		Place place_Result[100];
		int num_R = 0;
		while (fgets(Result_n[0], 256, Result) != NULL) {	//すべて読み込み，変数に格納
			fgets(Result_n[1], 256, Result);
			fgets(Result_n[2], 256, Result);
			fgets(Result_n[3], 256, Result);
			fgets(Result_n[4], 256, Result);

			fgets(Result_n[0], 256, Result);
			fgets(Result_n[1], 256, Result);
			fgets(Result_n[2], 256, Result);
			fgets(Result_n[3], 256, Result);
			fgets(Result_n[4], 256, Result);


			place_Result[num_R].yudo = atof(Result_n[0]);
			place_Result[num_R].x = atoi(Result_n[1]);
			place_Result[num_R].y = atoi(Result_n[2]);
			place_Result[num_R].width = atoi(Result_n[3]);
			place_Result[num_R].height = atoi(Result_n[4]);
			num_R++;
		}
		fclose(Result);

		//バイナリ画像生成
		cv::Mat GT_Binary[10];

		for (int i = 0; i < num_G; i++) {
			GT_Binary[i] = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
		}
		cv::Mat Result_Binary[100];
		for (int i = 0; i < num_R; i++) {
			Result_Binary[i] = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
		}

		for (int i = 0; i < num_G; i++) {
			for (int n = place_GT[i].y; n < place_GT[i].y + place_GT[i].height; n++) {
				for (int m = place_GT[i].x; m < place_GT[i].x + place_GT[i].width; m++) {
					GT_Binary[i].at<cv::Vec3b>(n, m) = cv::Vec3b(255, 255, 255);
				}
			}
		}

		for (int i = 0; i < num_R; i++) {
			for (int n = place_Result[i].y; n < place_Result[i].y + place_Result[i].height; n++) {
				for (int m = place_Result[i].x; m < place_Result[i].x + place_Result[i].width; m++) {
					Result_Binary[i].at<cv::Vec3b>(n, m) = cv::Vec3b(255, 255, 255);
				}
			}
		}
		float IoU_num[100];
		for (int i = 0; i < 100; i++) {
			IoU_num[i] = -1;
		}

		for (int i = 0; i < num_G; i++) {
			for (int j = 0; j < num_R; j++) {
				float tmp = evaluation(Result_Binary[j], GT_Binary[i]);
				if (IoU_num[j] < tmp) {
					IoU_num[j] = tmp;
				}
			}
		}

		cout << List_name << ",";
		for (int i = 0; i < num_R; i++) {
			cout << IoU_num[i] << ",";
		}
		cout << endl;
	}

	fclose(List);
}

int main(int argc, char** argv) {
	char Result_file_AP[10][1024] = {
		"C:/photo/result_data_from_demo/2018_01_15_AP/save_data/0.5_10/",
		"C:/photo/result_data_from_demo/2018_01_15_AP/save_data/0.5_20/",
		"C:/photo/result_data_from_demo/2018_01_15_AP/save_data/0.5_30/",
		"C:/photo/result_data_from_demo/2018_01_15_AP/save_data/0.5_40/", 
		"C:/photo/result_data_from_demo/2018_01_15_AP/save_data/0.5_50/",
		"C:/photo/result_data_from_demo/2018_01_15_AP/save_data/0.5_60/", 
		"C:/photo/result_data_from_demo/2018_01_15_AP/save_data/0.5_70/",
		"C:/photo/result_data_from_demo/2018_01_15_AP/save_data/0.5_80/", 
		"C:/photo/result_data_from_demo/2018_01_15_AP/save_data/0.5_90/",
		"C:/photo/result_data_from_demo/2018_01_15_AP/save_data/0.5_100/", 
	};
	char Result_file_OOP[10][1024] = {
		"C:/photo/result_data_from_demo/2018_01_13_OOP/save_data/0.5_10/",
		"C:/photo/result_data_from_demo/2018_01_13_OOP/save_data/0.5_20/",
		"C:/photo/result_data_from_demo/2018_01_13_OOP/save_data/0.5_30/",
		"C:/photo/result_data_from_demo/2018_01_13_OOP/save_data/0.5_40/",
		"C:/photo/result_data_from_demo/2018_01_13_OOP/save_data/0.5_50/",
		"C:/photo/result_data_from_demo/2018_01_13_OOP/save_data/0.5_60/",
		"C:/photo/result_data_from_demo/2018_01_13_OOP/save_data/0.5_70/",
		"C:/photo/result_data_from_demo/2018_01_13_OOP/save_data/0.5_80/",
		"C:/photo/result_data_from_demo/2018_01_13_OOP/save_data/0.5_90/",
		"C:/photo/result_data_from_demo/2018_01_13_OOP/save_data/0.5_100/",
	};
	char Result_file_EP[10][1024] = {
		"C:/photo/result_data_from_demo/2018_01_13_EP/save_data/0.5_10/",
		"C:/photo/result_data_from_demo/2018_01_13_EP/save_data/0.5_20/",
		"C:/photo/result_data_from_demo/2018_01_13_EP/save_data/0.5_30/",
		"C:/photo/result_data_from_demo/2018_01_13_EP/save_data/0.5_40/",
		"C:/photo/result_data_from_demo/2018_01_13_EP/save_data/0.5_50/",
		"C:/photo/result_data_from_demo/2018_01_13_EP/save_data/0.5_60/",
		"C:/photo/result_data_from_demo/2018_01_13_EP/save_data/0.5_70/",
		"C:/photo/result_data_from_demo/2018_01_13_EP/save_data/0.5_80/",
		"C:/photo/result_data_from_demo/2018_01_13_EP/save_data/0.5_90/",
		"C:/photo/result_data_from_demo/2018_01_13_EP/save_data/0.5_100/",
	};

	char Save_file_AP_OOP[10][1024] = {
		"AP_OOP_Save_10_2.txt",
		"AP_OOP_Save_20_2.txt",
		"AP_OOP_Save_30_2.txt",
		"AP_OOP_Save_40_2.txt",
		"AP_OOP_Save_50_2.txt",
		"AP_OOP_Save_60_2.txt",
		"AP_OOP_Save_70_2.txt",
		"AP_OOP_Save_80_2.txt",
		"AP_OOP_Save_90_2.txt",
		"AP_OOP_Save_100_2.txt",
	};
	char Save_file_AP_AP[10][1024] = {
		"AP_AP_Save_10.txt",
		"AP_AP_Save_20.txt",
		"AP_AP_Save_30.txt",
		"AP_AP_Save_40.txt",
		"AP_AP_Save_50.txt",
		"AP_AP_Save_60.txt",
		"AP_AP_Save_70.txt",
		"AP_AP_Save_80.txt",
		"AP_AP_Save_90.txt",
		"AP_AP_Save_100.txt", 
	};
	char Save_file_OOP[10][1024] = {
		"OOP_Save_10.txt",
		"OOP_Save_20.txt",
		"OOP_Save_30.txt",
		"OOP_Save_40.txt",
		"OOP_Save_50.txt",
		"OOP_Save_60.txt",
		"OOP_Save_70.txt",
		"OOP_Save_80.txt",
		"OOP_Save_90.txt",
		"OOP_Save_100.txt", 
	};
	char Save_file_EP[10][1024] = {
		"EP_Save_10.txt",
		"EP_Save_20.txt",
		"EP_Save_30.txt",
		"EP_Save_40.txt",
		"EP_Save_50.txt",
		"EP_Save_60.txt",
		"EP_Save_70.txt",
		"EP_Save_80.txt",
		"EP_Save_90.txt",
		"EP_Save_100.txt",
	};

	for (int num = 0; num < 10; num++) {
		double i = 0.5;
		while (i <= 1.0) {
			cout << i << ",";
		//	Result_ROC(i, Result_file_OOP[num], Save_file_OOP[num]);
		//	Result_ROC(i, Result_file_EP[num], Save_file_EP[num]);
			Result_ROC_2(i, Result_file_AP[num], Save_file_AP_OOP[num], 1);
		//	Result_ROC_2(i, Result_file_AP[num], Save_file_AP_AP[num], 0);

			if (i < 0.8) i += 0.1;
			else if (i < 0.99)i += 0.01;
			else i += 0.0001;
		}
		cout << endl;
	}

	return 0;
}
