#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <fstream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>

using namespace std;
using namespace cv;

double getPSNR ( const Mat& I1, const Mat& I2);
Scalar getMSSIM( const Mat& I1, const Mat& I2);

static void help(){
	cout
		<< "ssimCheck - compares two videos to produce a JSON file with frame-by-frame " << endl
		<< "            psnr comparison, and in-depth SSIM comparison." << endl
		<< endl
		<< "Usage:" << endl
		<< "ssimCheck <reference video> <test video> <ssim-trigger level> <result file> " << endl
		<< endl;
}

int main(int argc, char *argv[]){
	if (argc != 5){
		help();
		cout << "Not enough parameters" << endl;
		return -1;
	}

	stringstream conv;

	const string referenceVideo = argv[1], testVideo = argv[2], oFile = argv[4];

	int psnrTriggerValue;
	conv << argv[3];
	conv >> psnrTriggerValue;

	ofstream outputfile;
	outputfile.open(oFile);

	int frameNum = -1;

	VideoCapture captReference(referenceVideo), captTest(testVideo);

	if (!captReference.isOpened()){
		cout  << "Could not open reference video " << referenceVideo << endl;
		return -1;
	}

	if (!captTest.isOpened()){
		cout  << "Could not open test video " << testVideo << endl;
		return -1;
	}

	Size 	dstS = Size((int) captTest.get(CAP_PROP_FRAME_WIDTH),
			(int) captTest.get(CAP_PROP_FRAME_HEIGHT));

	Size 	refS = Size((int) captReference.get(CAP_PROP_FRAME_WIDTH),
			(int) captReference.get(CAP_PROP_FRAME_HEIGHT)),
		uTSi = Size((int) captTest.get(CAP_PROP_FRAME_WIDTH),
			(int) captTest.get(CAP_PROP_FRAME_HEIGHT));


	cout << "Reference resolution: " << refS.width << "x" << refS.height << "px" << endl;
	cout << "Reference framecount: " << captReference.get(CAP_PROP_FRAME_COUNT) << endl;
	cout << "Test resolution: " << dstS.width << "x" << dstS.height << "px" << endl;
	cout << "Test framecount: " << captTest.get(CAP_PROP_FRAME_COUNT) << endl;

	cout << "PSNR trigger value " << setiosflags(ios::fixed) << setprecision(3)
		 << psnrTriggerValue << endl;

	Mat frameRReference, frameReference, frameTest;
	double psnrV;
	Scalar mssimV;

	outputfile << "{\"reference\": {\"file\": \"" << referenceVideo << "\", \"dimensions\":\"" << refS.width << "x" << refS.height << "\", \"numframes\":\"" << captReference.get(CAP_PROP_FRAME_COUNT) << "\"}, \"test\": {\"file\": \"" << testVideo << "\", \"dimensions\":\"" << dstS.width << "x" << dstS.height << "\", \"numframes\":\"" << captTest.get(CAP_PROP_FRAME_COUNT) << "\"}, \"results\": {";

	for(;;){
		captReference >> frameReference;
		captTest >> frameTest;

		if (frameReference.empty() || frameTest.empty()){
			cout << "Comparison completed... " << endl;
			break;
		}

		++frameNum;

		resize(frameReference, frameRReference, dstS, 0, 0);
		psnrV = getPSNR(frameRReference,frameTest);

		outputfile << "\"frame_" << frameNum << "\":{ \"psnr\":\"" << setiosflags(ios::fixed) << setprecision(3) << psnrV << "\"";

		if (psnrV < psnrTriggerValue && psnrV){
			mssimV = getMSSIM(frameRReference, frameTest);
			outputfile << ", \"mssim\": {\"R\":\"" << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[2] * 100 << "\",\"G\":\"" << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[1] * 100 << "\",\"B\":\"" << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[0] * 100 << "\"}";
		}

		outputfile << "},";

	}

	outputfile << "\"framecount\":\"" << frameNum << "\"}";
	outputfile << "}" << endl;
	return 0;
}

double getPSNR(const Mat& I1, const Mat& I2){
	Mat s1;
	absdiff(I1, I2, s1); 
	s1.convertTo(s1, CV_32F);
	s1 = s1.mul(s1);

	Scalar s = sum(s1);

	double sse = s.val[0] + s.val[1] + s.val[2];

	if( sse <= 1e-10)
		return 0;
	else{
		double mse  = sse / (double)(I1.channels() * I1.total());
		double psnr = 10.0 * log10((255 * 255) / mse);
		return psnr;
	}
}

Scalar getMSSIM( const Mat& i1, const Mat& i2)
{
	const double C1 = 6.5025, C2 = 58.5225;
	int d = CV_32F;

	Mat I1, I2;
	i1.convertTo(I1, d);
	i2.convertTo(I2, d);

	Mat I2_2   = I2.mul(I2);
	Mat I1_2   = I1.mul(I1);
	Mat I1_I2  = I1.mul(I2);

	Mat mu1, mu2;
	GaussianBlur(I1, mu1, Size(11, 11), 1.5);
	GaussianBlur(I2, mu2, Size(11, 11), 1.5);

	Mat mu1_2   =   mu1.mul(mu1);
	Mat mu2_2   =   mu2.mul(mu2);
	Mat mu1_mu2 =   mu1.mul(mu2);

	Mat sigma1_2, sigma2_2, sigma12;

	GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
	sigma1_2 -= mu1_2;

	GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
	sigma2_2 -= mu2_2;

	GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
	sigma12 -= mu1_mu2;

	Mat t1, t2, t3;

	t1 = 2 * mu1_mu2 + C1;
	t2 = 2 * sigma12 + C2;
	t3 = t1.mul(t2);

	t1 = mu1_2 + mu2_2 + C1;
	t2 = sigma1_2 + sigma2_2 + C2;
	t1 = t1.mul(t2);

	Mat ssim_map;
	divide(t3, t1, ssim_map);

	Scalar mssim = mean(ssim_map);
	return mssim;
}
