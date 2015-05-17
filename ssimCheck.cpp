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
		<< "            psnr comparison. Add 'true' for SSIM comparison." << endl
		<< endl
		<< "Usage:" << endl
		<< "ssimCheck <reference video> <test video> <result file> [true]" << endl
		<< endl;
}

int main(int argc, char *argv[]){
	if (argc < 4){
		help();
		cout << "Not enough parameters" << endl;
		return -1;
	}

	bool withmssim = argv[4];

	stringstream conv;

	const string referenceVideo = argv[1], testVideo = argv[2], oFile = argv[3];

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

	int totalTstFrames 	= captTest.get(CAP_PROP_FRAME_COUNT);
	int totalSrcFrames 	= captReference.get(CAP_PROP_FRAME_COUNT);
	int fourCCTstProp	= static_cast<int>(captTest.get(CAP_PROP_FOURCC));
	int fourCCSrcProp	= static_cast<int>(captReference.get(CAP_PROP_FOURCC));
	//char fourCCTst[]	= {fourCCTstProp & 0XFF , (fourCCTstProp & 0XFF00) >> 8,(fourCCTstProp & 0XFF0000) >> 16,(fourCCTstProp & 0XF000000) >> 24, 0};
	//char fourCCSrc[]	= {fourCCSrcProp & 0XFF , (fourCCSrcProp & 0XFF00) >> 8,(fourCCSrcProp & 0XFF0000) >> 16,(fourCCSrcProp & 0XF000000) >> 24, 0};
	union { int v; char c[5];} fourCCTst ;
	fourCCTst.v = fourCCTstProp;
	fourCCTst.c[4]='\0';
	union { int v; char c[5];} fourCCSrc ;
	fourCCSrc.v = fourCCSrcProp;
	fourCCSrc.c[4]='\0';

	Mat frameRReference, frameReference, frameTest;
	double psnrV;
	Scalar mssimV;

	outputfile << "{" << endl
		<< "	\"reference\":{" << endl
		<< "		\"file\": \"" << referenceVideo << "\"," << endl
		<< "		\"dimensions\":\"" << refS.width << "x" << refS.height << "\"," << endl
		<< "		\"numframes\":\"" << totalSrcFrames << "\"," << endl
		<< "		\"fourcc\":\"" << fourCCSrc.c << "\"" << endl
		<< "	}," << endl
		<< "	\"test\":{" << endl
		<< "		\"file\": \"" << testVideo << "\"," << endl
		<< "		\"dimensions\":\"" << dstS.width << "x" << dstS.height << "\"," << endl
		<< "		\"numframes\":\"" << totalTstFrames << "\"," << endl
		<< "		\"fourcc\":\"" << fourCCTst.c << "\"" << endl
		<< "	}," << endl
		<< "	\"results\":{" << endl;

	for(;;){
		captReference >> frameReference;
		captTest >> frameTest;

		if (frameReference.empty() || frameTest.empty()){
			cout << "Comparison completed... " << endl;
			cout << "Results can be found in " << oFile << endl;
			cout << endl;
			break;
		}
		++frameNum;

		double curTstMsec = captTest.get(CAP_PROP_POS_MSEC);
		double curSrcMsec = captReference.get(CAP_PROP_POS_MSEC);

		resize(frameReference, frameRReference, dstS, 0, 0);
		psnrV = getPSNR(frameRReference,frameTest);

		outputfile << "		\"frame_" << frameNum << "\":{" << endl
			<< "			\"psnr\":\"" << setiosflags(ios::fixed) << setprecision(3) << psnrV << "\"," << endl
			<< "			\"tmsec\":\"" << curTstMsec << "\"," << endl
			<< "			\"smsec\":\"" << curSrcMsec << "\"";

		if (withmssim){
			mssimV = getMSSIM(frameRReference, frameTest);
			outputfile << "," << endl
				<< "			\"mssim\": {" << endl
				<< "				\"R\":\"" << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[2] * 100 << "\"," << endl
				<< "				\"G\":\"" << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[1] * 100 << "\"," << endl
				<< "				\"B\":\"" << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[0] * 100 << "\"" << endl
				<< "			}";
		}

		outputfile << endl
			<< "		}," << endl;

	}

	outputfile << "		\"framecount\":\"" << frameNum << "\"" << endl
		<< "	}" << endl
		<< "}" << endl;
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
