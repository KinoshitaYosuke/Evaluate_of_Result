#include <iostream>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>

using namespace std;

#define YUDO 0.8
#define IOU_OUTPUT 0.9

char GT_Text_Name[1024] = "";
char Result_Text_Name[1024] = "";

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

//IoUの計算
float calculate_IoU(cv::Mat RE,cv::Mat GT) {
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

//Result領域/(GT領域+Result領域)の計算
float calculate_Overlap(cv::Mat RE, cv::Mat GT) {
	float Overlap = 0.0, Union = 0.0;
	for (int n = 0; n < RE.rows; n++) {
		for (int m = 0; m < RE.cols; m++) {
			if (GT.at<cv::Vec3b>(n, m) == cv::Vec3b(255, 255, 255)) {
				Union += 1.0;
				if (RE.at<cv::Vec3b>(n, m) == cv::Vec3b(255, 255, 255)) {
					Overlap += 1.0;
				}
			}
		}
	}

	return Overlap / Union;
}
/*
//GTの2値画像生成
void Generate_GT() {
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

		for (int i = 0; i < num_G; i++) {
			GT_Binary[i] = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
		}

		for (int i = 0; i < num_G; i++) {
			for (int n = place_GT[i].y; n < place_GT[i].y + place_GT[i].height; n++) {
				for (int m = place_GT[i].x; m < place_GT[i].x + place_GT[i].width; m++) {
					GT_Binary[i].at<cv::Vec3b>(n, m) = cv::Vec3b(255, 255, 255);
				}
			}
		}
	}
	fclose(List);
}

//Resultの2値画像生成
void Generate_Result() {
	//テキストファイルのリスト読み込み
	char List_n[1024];
	FILE *List;
	if (fopen_s(&List, "c:/photo/text_list_4.txt", "r") != 0) {
		cout << "not found List file" << endl;
		return;
	}

	char Result_name[1024] = "";

	while (fgets(List_n, 256, List) != NULL) {
		string List_str = List_n;
		char List_name[1024];
		for (int i = 0; i < List_str.length() - 1; i++) {
			List_name[i] = List_n[i];
			List_name[i + 1] = '\0';
		}

		char Result_Text[1024];
		for (int i = 0; ; i++) {
			if(Result_Text_Name[i]=='\0')
			Result_Text[i] = Result_Text_Name[i];
		}

		strcat_s(Result_Text_Name, List_name);
		
		//Resultファイル読み込み
		char Result_n[5][1024];
		FILE *Result;
		if (fopen_s(&Result, Result_name, "r") != 0) {
			cout << "not found Result file" << endl;
			return;
		}
		Place place_Result[500];
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
			if (place_Result[num_R].yudo < YUDO) {
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

		cv::Mat Result_Binary[500];
		for (int i = 0; i < num_R; i++) {
			Result_Binary[i] = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
		}

		for (int i = 0; i < num_R; i++) {
			for (int n = place_Result[i].y; n < place_Result[i].y + place_Result[i].height; n++) {
				for (int m = place_Result[i].x; m < place_Result[i].x + place_Result[i].width; m++) {
					Result_Binary[i].at<cv::Vec3b>(n, m) = cv::Vec3b(255, 255, 255);
				}
			}
		}
	}

	fclose(List);
}

//TP,FP,FNの計算
void show_TP_FP_FN_IoU_value() {
	float TP = 0, FP = 0, FN = 0, IoU = 0;
	int RE_num = 0, GT_num = 0;
	while (1) {
		//ファイルパス
		char GT_Name[1024] = "";
		char Result_Name[1024] = "";

		strcpy_s(GT_Name, GT_Text_Name);
		strcpy_s(Result_Name, Result_Text_Name);

		//2値画像生成
		cv::Mat Result_Binary[10];
		cv::Mat GT_Binary[10];

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


			strcat_s(Result_Text_Name, List_name);
			//Resultファイル読み込み
			char Result_n[5][1024];
			FILE *Result;
			if (fopen_s(&Result, Result_Text_Name, "r") != 0) {
				cout << "not found Result file" << endl;
				return;
			}
			Place place_Result[500];
			int num_R = 0;
			while (fgets(Result_n[0], 256, Result) != NULL) {	//すべて読み込み，変数に格納
				fgets(Result_n[1], 256, Result);
				fgets(Result_n[2], 256, Result);
				fgets(Result_n[3], 256, Result);
				fgets(Result_n[4], 256, Result);

				if (flag == 1) {
					fgets(Result_n[0], 256, Result);
					fgets(Result_n[1], 256, Result);
					fgets(Result_n[2], 256, Result);
					fgets(Result_n[3], 256, Result);
					fgets(Result_n[4], 256, Result);
				}

				place_Result[num_R].yudo = atof(Result_n[0]);
				place_Result[num_R].x = atoi(Result_n[1]);
				place_Result[num_R].y = atoi(Result_n[2]);
				place_Result[num_R].width = atoi(Result_n[3]);
				place_Result[num_R].height = atoi(Result_n[4]);
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

			//バイナリ画像生成
			cv::Mat GT_Binary[10];

			for (int i = 0; i < num_G; i++) {
				GT_Binary[i] = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
			}
			cv::Mat Result_Binary[500];
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

			//TP,FP,FN計算
			for (int i = 0; i < RE_num; i++) {
				for (int j = 0; j < GT_num; j++) {
					if (calculate_Overlap(Result_Binary[i], GT_Binary[j]) >= 0.5) {
						calculate_IoU();
					}
				}
			}
		}
	}
}
*/

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
		cout << new_result_name << "," << calculate_IoU(RE, GT) << endl;

	}

	fclose(GT_data);

}

cv::Mat draw_rectangle(cv::Mat ans_im, int x, int y, int width, int height, int r, int g, int b) {
	rectangle(ans_im, cvPoint(x, y), cvPoint(x + width, y + height), CV_RGB(r, g, b), 1);
	return ans_im;
}

void check_rectangle(float yudo, char* Result_file, int flag) {
	//テキストファイルのリスト読み込み
	char List_n[1024];
	FILE *List;
	if (fopen_s(&List, "c:/test_text/text_list_all_person.txt", "r") != 0) {
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

		char Img_path[1024] = "c:/test_images/";
		strcat_s(Img_path, List_name);
		printf("%s\n", Img_path);
		for (int i = 0; ; i++) {
			if (Img_path[i] == '.') {
				Img_path[i] = '.';
				Img_path[i+1] = 'b';
				Img_path[i+2] = 'm';
				Img_path[i+3] = 'p';
				break;
			}
		}
		cv::Mat img = cv::imread(Img_path, 0);


		char Result_name[1024];
		for (int i = 0;; i++) {
			Result_name[i] = Result_file[i];
			if (Result_file[i] == '\0')break;
		}

		strcat_s(Result_name, List_name);
		//Resultファイル読み込み
		char Result_n[5][1024];
		FILE *Result;
		if (fopen_s(&Result, Result_name, "r") != 0) {
			cout << "not found Result file" << endl;
			return;
		}
		int num_R = 0;
		Place place_Result[20];
		while (fgets(Result_n[0], 256, Result) != NULL) {	//すべて読み込み，変数に格納
			fgets(Result_n[1], 256, Result);
			fgets(Result_n[2], 256, Result);
			fgets(Result_n[3], 256, Result);
			fgets(Result_n[4], 256, Result);

			if (flag == 1) {
				fgets(Result_n[0], 256, Result);
				fgets(Result_n[1], 256, Result);
				fgets(Result_n[2], 256, Result);
				fgets(Result_n[3], 256, Result);
				fgets(Result_n[4], 256, Result);
			}
			place_Result[num_R].yudo = atof(Result_n[0]);
			place_Result[num_R].x = atoi(Result_n[1]);
			place_Result[num_R].y = atoi(Result_n[2]);
			place_Result[num_R].width = atoi(Result_n[3]);
			place_Result[num_R].height = atoi(Result_n[4]);
			place_Result[num_R].width -= place_Result[num_R].x;
			place_Result[num_R].height -= place_Result[num_R].y;
			num_R++;
		}
		fclose(Result);

		for (int i = 0; i < num_R; i++) {
			img = draw_rectangle(img, place_Result[i].x, place_Result[i].y, place_Result[i].width, place_Result[i].height, 0, 255, 0);
		}
		cv::imshow("", img);
		cv::waitKey(0);
	}
	fclose(List);

}

void Result_ROC(float yudo, float iou, char* Result_file,char* Save_file) {
	//変数宣言
	float miss_rate = 0;
	float fppi = 0;
	int num = 0;
	float TP = 0;
	float FP = 0;
	float FN = 0;
	float parts = 0;

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

		Place place_Result[150];
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
		cv::Mat Result_B[150];

		for (int i = 0; i < 150; i++) {
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

		float Pre_num[10][150];
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 150; j++) {
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
						if (GT_B[i].at<cv::Vec3b>(n, m) == cv::Vec3b(255, 255, 255) || Result_B[j].at<cv::Vec3b>(n,m)==cv::Vec3b(255,255,255)) {
							Union += 1.0;
							if (Result_B[j].at<cv::Vec3b>(n, m) == cv::Vec3b(255, 255, 255) && GT_B[i].at<cv::Vec3b>(n, m) == cv::Vec3b(255, 255, 255)) {
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
			if (Pre_num[0][i] >= iou || Pre_num[1][i] >= iou) {TP+=1.0;}
			else {FP+=1.0;}

			if (0.3 <= Pre_num[0][i] && Pre_num[0][i] <= 0.7 || 0.3 <= Pre_num[1][i] && Pre_num[1][i] <= 0.7) {
				parts+1.0;
			}
		}
		if ((TP - TP_tmp) < num_G) {
			FN += num_G - (TP - TP_tmp);
		}

	}

	float Precision = TP / (TP + FP);
	float Recall = TP / (TP + FN);
	float FPPI = FP / 203.0;
	fprintf_s(Save, "%.0f, %.0f, %.0f, %f, %f, %f\n", TP, FP, FN, Precision, Recall, FPPI);
	fclose(Save);
	fclose(List);
}

void Result_ROC_2(float yudo, float iou, char* Result_file, char* Save_file, int flag) {
	//変数宣言
	float miss_rate = 0;
	float fppi = 0;
	int num = 0;
	float TP = 0;
	float FP = 0;
	float FN = 0;
	float parts = 0;

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

		//	place_Result[num_R].yudo = atof(Result_n[0]);
		//	if (place_Result[num_R].yudo < 0.5) {
		//		place_Result[num_R].yudo = 0;
		//		continue;
		//	}

			if (flag == 1) {
				place_Result[num_R].yudo = atof(Result_n[0]);
				place_Result[num_R].x = atoi(Result_n[1]);
				place_Result[num_R].y = atoi(Result_n[2]);
				place_Result[num_R].width = atoi(Result_n[3]);
				place_Result[num_R].height = atoi(Result_n[4]);
			}
			else {
				place_Result[num_R].yudo = atof(Result_n[5]);
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
						if (GT_B[i].at<cv::Vec3b>(n, m) == cv::Vec3b(255, 255, 255) || Result_B[j].at<cv::Vec3b>(n, m) == cv::Vec3b(255, 255, 255)) {
							Union += 1.0;
							if (Result_B[j].at<cv::Vec3b>(n, m) == cv::Vec3b(255, 255, 255) && GT_B[i].at<cv::Vec3b>(n, m) == cv::Vec3b(255, 255, 255)) {
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
			if (Pre_num[0][i] >= iou || Pre_num[1][i] >= iou) { TP += 1.0; }
			else { FP += 1.0; }

			if (0.3 <= Pre_num[0][i] && Pre_num[0][i] <= 0.7 || 0.3 <= Pre_num[1][i] && Pre_num[1][i] <= 0.7) {
				parts+=1.0;
			}
		}
		if ((TP - TP_tmp) < num_G) {
			FN += num_G - (TP - TP_tmp);
		}

	}
	float Precision = TP / (TP + FP);
	float Recall = TP / (TP + FN);
	float FPPI = FP / 203.0;
	fprintf_s(Save, "%.0f, %.0f, %.0f, %f, %f, %f\n", TP, FP, FN, Precision, Recall, FPPI);
	fclose(Save);
	fclose(List);
}

void Result_TP(char* Result_file, char* Save_file, int flag, float yudo) {
	//テキストファイルのリスト読み込み
	char List_n[1024];
	FILE *List;
	if (fopen_s(&List, "c:/photo/text_list_4.txt", "r") != 0) {
		cout << "not found List file" << endl;
		return;
	}

	FILE* Save;
	if (fopen_s(&Save, Save_file, "a") != 0) {
		cout << "error" << endl;
		return;
	}

	float TP = 0, FP = 0, FN = 0;

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

		strcat_s(Result_name, List_name);
		//Resultファイル読み込み
		char Result_n[5][1024];
		FILE *Result;
		if (fopen_s(&Result, Result_name, "r") != 0) {
			cout << "not found Result file" << endl;
			return;
		}
		Place place_Result[500];
		int num_R = 0;
		while (fgets(Result_n[0], 256, Result) != NULL) {	//すべて読み込み，変数に格納
			fgets(Result_n[1], 256, Result);
			fgets(Result_n[2], 256, Result);
			fgets(Result_n[3], 256, Result);
			fgets(Result_n[4], 256, Result);
			if (flag == 1) {
				fgets(Result_n[0], 256, Result);
				fgets(Result_n[1], 256, Result);
				fgets(Result_n[2], 256, Result);
				fgets(Result_n[3], 256, Result);
				fgets(Result_n[4], 256, Result);
			}

			place_Result[num_R].yudo = atof(Result_n[0]);
			place_Result[num_R].x = atoi(Result_n[1]);
			place_Result[num_R].y = atoi(Result_n[2]);
			place_Result[num_R].width = atoi(Result_n[3]);
			place_Result[num_R].height = atoi(Result_n[4]);
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

		//バイナリ画像生成
		cv::Mat GT_Binary[10];

		for (int i = 0; i < num_G; i++) {
			GT_Binary[i] = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
		}
		cv::Mat Result_Binary[500];
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
		float IoU_num[500];
		for (int i = 0; i < 500; i++) {
			IoU_num[i] = -1;
		}

		for (int i = 0; i < num_G; i++) {
			for (int j = 0; j < num_R; j++) {
				float tmp = calculate_Overlap(Result_Binary[j], GT_Binary[i]);
				if (IoU_num[j] < tmp) {
					IoU_num[j] = tmp;
				}
			}
		}
		int TP_tmp = 0;
		for (int i = 0; i < num_R; i++) {
			if (IoU_num[i] > 0.7) {
				TP += 1.0;
				TP_tmp += 1;
			}
			else FP += 1.0;
		}

		if (num_G > TP_tmp) FN += (num_G - TP_tmp);

	}
	printf("%.0f,%.0f,%.0f", TP, FP, FN);
	fclose(Save);

	fclose(List);

}

void ROC_data(float yudo, char* Result_file, char* Save_file, int flag) {
	//変数宣言
	int num = 0;
	float TP = 0;
	float FP = 0;
	float FN = 0;
	float TN = 0;

	int Label[363],Result_label[363];
	for (int i = 0; i < 363; i++) {
		if (i < 150)Label[i] = -1;
		else Label[i] = 1;

		Result_label[i] = 0;
	}

	FILE* Save;
	if (fopen_s(&Save, Save_file, "a") != 0) {
		cout << "error" << endl;
		return;
	}

	//テキストファイルのリスト読み込み
	char List_n[1024];
	FILE *List;
	if (fopen_s(&List, "c:/photo/predict-nor-text.txt", "r") != 0) {
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

		char Result_name[1024];
		for (int i = 0;; i++) {
			Result_name[i] = Result_file[i];
			if (Result_file[i] == '\0')break;
		}

		strcat(Result_name, List_name);
		//Resultファイル読み込み
		char Result_n[1024];
		FILE *Result;
		if (fopen_s(&Result, Result_name, "r") != 0) {
			cout << "not found Result file" << endl;
			return;
		}

		Place place_Result[2];
		fgets(Result_n, 256, Result);

		place_Result[0].yudo = atof(Result_n);

		if (flag == 1)fgets(Result_n, 256, Result);

		place_Result[1].yudo = atof(Result_n);
		
		//尤度チェック
	//	if (place_Result[0].yudo >= 0.5) {
			if (place_Result[1].yudo < yudo)Result_label[num] = -1;
			else Result_label[num] = 1;
//		}
//		else Result_label[num] = -1;

		num++;

		fclose(Result);
	}

	for (int i = 0; i < 363; i++) {
		if (Result_label[i] == -1) {
			if (Label[i] == -1) TN += 1.0;
			else FN += 1.0;
		}
		else if(Result_label[i] == 1) {
			if (Label[i] == 1) TP += 1.0;
			else FP += 1.0;
		}
		else {
			cout << "yudo error" << endl;
			return;
		}
	}

	float TPR = TP / (TP + FN);
	float FPR = FP / (FP + TN);

	fprintf_s(Save, "%f, %f\n", TPR, FPR);
	fclose(Save);
	fclose(List);
}

//単独人物画像
void IoU_Result(float yudo, char* Result_file, int flag) {
	//テキストファイルのリスト読み込み
	char List_n[1024];
	FILE *List;
	if (fopen_s(&List, "c:/test_text/text_list_only_one_person.txt", "r") != 0) {
		cout << "not found List file" << endl;
		return;
	}

	float TP = 0, FP = 0, FN = 0;
	float tmp = 0;
	
	int point = 0;
	while (fgets(List_n, 256, List) != NULL) {
		cout << point << endl;
		point++;
		string List_str = List_n;
		char List_name[1024];
		for (int i = 0; i < List_str.length() - 1; i++) {
			List_name[i] = List_n[i];
			List_name[i + 1] = '\0';
		}

		char GT_name[1024] = "c:/GT_text/";
		strcat_s(GT_name, List_name);
		//GTファイル読み込み
		char GT_n[4][1024];
		FILE *GT;
		if (fopen_s(&GT, GT_name, "r") != 0) {
			cout << "not found GT file" << endl;
			return;
		}
		Place place_GT;
		int num_G = 0;
		while (fgets(GT_n[0], 256, GT) != NULL) {	//すべて読み込み，変数に格納
			fgets(GT_n[1], 256, GT);
			fgets(GT_n[2], 256, GT);
			fgets(GT_n[3], 256, GT);

			place_GT.x = atoi(GT_n[0]);
			place_GT.y = atoi(GT_n[1]);
			place_GT.width = atoi(GT_n[2]);
			place_GT.height = atoi(GT_n[3]);
			num_G++;
		}
		fclose(GT);

		int num_R = 0;
		float max_yudo = 0;

		char Result_name[1024];
		for (int i = 0;; i++) {
			Result_name[i] = Result_file[i];
			if (Result_file[i] == '\0')break;
		}

		strcat_s(Result_name, List_name);
		//Resultファイル読み込み
		char Result_n[5][1024];
		FILE *Result;
		if (fopen_s(&Result, Result_name, "r") != 0) {
			cout << "not found Result file" << endl;
			return;
		}
		Place place_Result;
		while (fgets(Result_n[0], 256, Result) != NULL) {	//すべて読み込み，変数に格納
			fgets(Result_n[1], 256, Result);
			fgets(Result_n[2], 256, Result);
			fgets(Result_n[3], 256, Result);
			fgets(Result_n[4], 256, Result);

			if (flag == 1) {
				fgets(Result_n[0], 256, Result);
				fgets(Result_n[1], 256, Result);
				fgets(Result_n[2], 256, Result);
				fgets(Result_n[3], 256, Result);
				fgets(Result_n[4], 256, Result);
			}
			num_R++;
			if (atof(Result_n[0]) > max_yudo) {
				place_Result.yudo = atof(Result_n[0]);
				place_Result.x = atoi(Result_n[1]);
				place_Result.y = atoi(Result_n[2]);
				place_Result.width = atoi(Result_n[3]);
				place_Result.height = atoi(Result_n[4]);
				place_Result.width -= place_Result.x;
				place_Result.height -= place_Result.y;
				
				max_yudo = place_Result.yudo;
			}
			else continue;
		}
		fclose(Result);



		//バイナリ画像生成
		cv::Mat GT_Binary = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
		cv::Mat Result_Binary = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);

		GT_Binary = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);

		Result_Binary = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);


		for (int n = place_GT.y; n < place_GT.y + place_GT.height; n++) {
			for (int m = place_GT.x; m < place_GT.x + place_GT.width; m++) {
				GT_Binary.at<cv::Vec3b>(n, m) = cv::Vec3b(255, 255, 255);
			}
		}
		if (num_R != 0) {
			for (int n = place_Result.y; n < place_Result.y + place_Result.height; n++) {//ここがおかしい
				for (int m = place_Result.x; m < place_Result.x + place_Result.width; m++) {
					Result_Binary.at<cv::Vec3b>(n, m) = cv::Vec3b(255, 255, 255);
				}
			}
			//			for (int i = 0; i < num_G; i++) {
			//				FN++;
		}
		else {
			FN++;
			continue;
		}

		if (calculate_Overlap(Result_Binary, GT_Binary) > 0.5) {
			TP += 1.0;
			tmp += calculate_IoU(Result_Binary, GT_Binary);
		}
		else {
			FP++;
			FN++;
		}
	}
	/*
	float IoU_num[10];
	for (int i = 0; i < 10; i++) {
		IoU_num[i] = -1;
	}
	int flag = 0;
	for (int i = 0; i < num_G; i++) {
		for (int j = 0; j < num_R; j++) {
			float tmp = calculate_Overlap(Result_Binary[j], GT_Binary[i]);
			if (IoU_num[j] < tmp && calculate_Overlap(Result_Binary[j], GT_Binary[i]) >= 0.5) {
				IoU_num[j] = tmp;
				flag = 1;
			}
		}
	}
	int TP_tmp = 0;
	float average = 0;
	for (int i = 0; i < num_R; i++) {
		if (IoU_num[i] < 0.5) { FP += 1.0; }
		else { TP += 1.0; TP_tmp++;}
		if (average < IoU_num[i])
			average += IoU_num[i];
	}
	if (num_G > TP_tmp) FN += num_G - TP_tmp;
	*/

	cout << TP << "," << FP << "," << FN << endl;
	cout << tmp / TP << endl;

	fclose(List);

}

void IoU_Result_2(float yudo, char* Result_file, int flag) {
	//テキストファイルのリスト読み込み
	char List_n[1024];
	FILE *List;
	if (fopen_s(&List, "c:/test_text/text_list_all_person.txt", "r") != 0) {
		cout << "not found List file" << endl;
		return;
	}

	float TP = 0, FP = 0, FN = 0;
	float tmp = 0;

	while (fgets(List_n, 256, List) != NULL) {
		string List_str = List_n;
		char List_name[1024];
		for (int i = 0; i < List_str.length() - 1; i++) {
			List_name[i] = List_n[i];
			List_name[i + 1] = '\0';
		}
		char GT_name[1024] = "c:/GT_text/";
		strcat_s(GT_name, List_name);
		//GTファイル読み込み
		char GT_n[4][1024];
		FILE *GT;
		if (fopen_s(&GT, GT_name, "r") != 0) {
			cout << "not found GT file" << endl;
			return;
		}
		int num_G = 0;
		Place place_GT[2];
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

		strcat_s(Result_name, List_name);
		//Resultファイル読み込み
		char Result_n[5][1024];
		FILE *Result;
		if (fopen_s(&Result, Result_name, "r") != 0) {
			cout << "not found Result file" << endl;
			return;
		}
		int num_R = 0;
		Place place_Result[20];
		while (fgets(Result_n[0], 256, Result) != NULL) {	//すべて読み込み，変数に格納
			fgets(Result_n[1], 256, Result);
			fgets(Result_n[2], 256, Result);
			fgets(Result_n[3], 256, Result);
			fgets(Result_n[4], 256, Result);

			if (flag == 1) {
				fgets(Result_n[0], 256, Result);
				fgets(Result_n[1], 256, Result);
				fgets(Result_n[2], 256, Result);
				fgets(Result_n[3], 256, Result);
				fgets(Result_n[4], 256, Result);
			}
//			float like = atof(Result_n[0]);
	//		if (like > place_Result[num_R].yudo) {
				place_Result[num_R].yudo = atof(Result_n[0]);
				place_Result[num_R].x = atoi(Result_n[1]);
				place_Result[num_R].y = atoi(Result_n[2]);
				place_Result[num_R].width = atoi(Result_n[3]);
				place_Result[num_R].height = atoi(Result_n[4]);
				place_Result[num_R].width -= place_Result[num_R].x;
				place_Result[num_R].height -= place_Result[num_R].y;
				num_R++;
	//		}
		}
		fclose(Result);


		//バイナリ画像生成
		cv::Mat GT_Binary[2] = { cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),cv::Mat::zeros(cv::Size(640, 480), CV_8UC3) };
		cv::Mat Result_Binary[20] = {
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
			cv::Mat::zeros(cv::Size(640, 480), CV_8UC3),
		};

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
		float IoU_num[20] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, };
		float like[20] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, };
		int max_num[2] = { -1,-1 };
		int flag = 0;
		
		float like_tmp = 0;
		
		for (int i = 0; i < num_G; i++) {
			for (int k = 0; k < num_R; k++) {
				if (calculate_Overlap(Result_Binary[k], GT_Binary[i]) >= 0.5) {
					if (like_tmp < place_Result[k].yudo) {
						like_tmp = place_Result[k].yudo;
						max_num[i] = k;
					}
				}
//				if (IoU_num[k] > evaluation(Result_Binary[k], GT_Binary[i])) {
//					IoU_num[k];
//				}
			}
		}

	//	if (max_num[0] != -1) {
	//		tmp += calculate_IoU(Result_Binary[max_num[0]], GT_Binary[0]);
	//		TP++;
	//	}
	//	if (max_num[1] != -1) {
	//		tmp += calculate_IoU(Result_Binary[max_num[1]], GT_Binary[1]);
	//		TP++;
	//	}

	//	cout << tmp << endl;
		
		for (int i = 0; i < num_G; i++) {
			for (int k = 0; k < num_R; k++) {
				if (calculate_Overlap(Result_Binary[k], GT_Binary[i]) >= 0.5) {
					TP += 1.0;
					tmp += calculate_IoU(Result_Binary[k], GT_Binary[i]);
				}
				else {
					FP += 1;
					FN += 1;
				}
				if (place_Result[k].yudo < 0.9) {
					FN += 1;
				}
			}
		}
		
	}
	cout << tmp / TP << endl;
	cout << TP << "," << FP << "," << FN << endl;

	fclose(List);

}

int main(int argc, char** argv) {
	char Result_File_Path[1024] = "c:/result_data/result_text_openpose/";
	/*
	//テキストファイルのリスト読み込み
	char List_n[1024];
	FILE *List;
	if (fopen_s(&List, "c:/test_text/text_list_all_person.txt", "r") != 0) {
		cout << "not found List file" << endl;
		return 0;
	}
	while (fgets(List_n, 256, List) != NULL) {
		string List_str = List_n;
		char List_name[1024];
		for (int i = 0; i < List_str.length() - 1; i++) {
			List_name[i] = List_n[i];
			List_name[i + 1] = '\0';
		}

		char Result_name[1024];
		for (int i = 0;; i++) {
			Result_name[i] = Result_File_Path[i];
			if (Result_File_Path[i] == '\0')break;
		}

		strcat_s(Result_name, List_name);
		//Resultファイル読み込み
		char Result_n[5][1024];
		FILE *Result;
		if (fopen_s(&Result, Result_name, "r") != 0) {
			cout << "not found Result file" << endl;
			return 0;
		}
		int num_R = 0;
		Place place_Result[20];
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
			place_Result[num_R].width -= place_Result[num_R].x;
			place_Result[num_R].height -= place_Result[num_R].y;
			num_R++;
		}
		fclose(Result);

		//文字列入れ替えてテスト画像読み込み
		char Img_Name[1024] = "c:/test_images/";
		for (int i = 0; List_name[i] != '\0'; i++) {
			if (List_name[i] == '.') {
				List_name[i + 1] = 'b';
				List_name[i + 2] = 'm';
				List_name[i + 3] = 'p';
			}
		}
		strcat_s(Img_Name, List_name);
		cout << Img_Name << endl;
		cv::Mat Test_img = cv::imread(Img_Name);
		for (int i = 0; i < num_R; i++) {
			draw_rectangle(Test_img, place_Result[i].x, place_Result[i].y, place_Result[i].width, place_Result[i].height, 255, 0, 0);
		}
		cv::imshow("test", Test_img);
		cvWaitKey(0);
	}
	fclose(List);

	*/
	
	IoU_Result_2(0.8, Result_File_Path, 0);

	return 0;
}
