#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>

using namespace std;
using namespace cv;

double getPSNR ( const Mat& I1, const Mat& I2);
double getRMSE ( const Mat& I1, const Mat& I2);
Scalar getMSSIM( const Mat& I1, const Mat& I2);

static void usage(){
	cout
		<< "Usage:" << endl
		<< "ssimCheck -s <reference video> -t <test video> -o <result file> -n <nth frame> [-f <outputformat>] [-a <source frame advance>] [-b <test frame advance>] [-m]" << endl;

}
static void help(){
	cout
		<< "ssimCheck - compares two videos to produce a JSON file with frame-by-frame " << endl
		<< "            psnr comparison. Add 'true' for SSIM comparison." << endl;
	usage();
}

int main(int argc, char **argv){
	extern char *optarg;
	extern int optind;
	bool withmssim = false;
	char *referenceVideo;
	char *testVideo;
	string tests;
	const char *templateHeader;
	const char *templateFrame;
	const char *templateFooter;
	string outputformat = "json";
	double psnrV;
	double rmseV;
	Scalar mssimV;
	char *oFile;
	char *nthc;
	int nth;
	int seconds;
	int minutes;
	int hours;
	int tfa = 0; // test frame advance
	int sfa = 0; // source frame advance
	int tfc = 0; // test frame counter
	int sfc = 0; // source frame counter
	int loops = 0; // loopcounter
	double sumpsnr;
	double sumrmse;
	int c;

	while ((c = getopt (argc, argv, "s:t:o:n:f:a:b:m")) != -1){
		switch (c){
			case 's':
				referenceVideo = strdup( optarg );
				break;
			case 't':
				testVideo = strdup( optarg );
				break;
			case 'o':
				oFile = strdup( optarg );
				break;
			case 'n': 
				nth = atoi ( strdup( optarg ) );
				break;
			case 'a': 
				sfa = atoi ( strdup( optarg ) );
				break;
			case 'b': 
				tfa = atoi ( strdup( optarg ) );
				break;
			case 'm':
				withmssim = true;
				break;
			case 'f':
				outputformat = strdup( optarg );
				break;
			case '?':
				help();
				break;
			default: 
				help();
				break;
		}
	}

	if (argc < 5){
		cerr << "Not enough parameters" << endl;
		usage();
		return -1;
	}

	FILE * outputfile;
	outputfile = fopen(oFile, "w");

	int frameNum = -1;

	if (outputformat == "xml") {
		templateHeader = 	"<resultset>					\n"
					"	<reference>				\n"
					"		<filename>%s</filename>		\n"
					"		<dimensions>%s</dimensions>	\n"
					"		<numframes>%d</numframes>	\n"
					"	</reference>				\n"
					"	<test>					\n"
					"		<filename>%s</filename>		\n"
					"		<dimensions>%s</dimensions>	\n"
					"		<numframes>%d</numframes>	\n"
					"	</test>					\n"
					"	<testsPerformed>%s</testsPerformed>	\n"
					"	<results>				\n";

		templateFrame = 	"		<frame count='%d'>		\n"
					"			<psnr>%f</psnr>		\n"
					"			<rmse>%f</rmse>		\n"
					"			<tmsec>%f</tmsec>	\n"
					"			<smsec>%f</smsec>	\n"
					"		</frame>			\n";

		templateFooter = 	"		<framecount>%d</framecount>	\n"
					"		<averagepsnr>%d</avaragepsnr>	\n"
					"		<averagermse>%d</avaragermse>	\n"
					"	</results>				\n"
					"</resultset>					\n";
	}

	if (outputformat == "json") {
		templateHeader = 	"{						\n"
					"	\"reference\":{				\n"
					"		\"file\":\"%s\",		\n"
					"		\"dimensions\":\"%s\",		\n"
					"		\"numframes\":\"%d\"		\n"
					"	},					\n"
					"	\"test\":{				\n"
					"		\"file\":\"%s\",		\n"
					"		\"dimensions\":\"%s\",		\n"
					"		\"numframes\":\"%d\"		\n"
					"	},					\n"
					"	\"testsPerformed\":\"%s\",		\n"
					"	\"results\":{				\n";

		templateFrame = 	"		\"frame_%d\":{			\n"
					"			\"psnr\":\"%f\",	\n"
					"			\"rmse\":\"%f\",	\n"
					"			\"tmsec\":\"%f\",	\n"
					"			\"smsec\":\"%f\"	\n"
					"		},				\n";

		templateFooter = 	"		\"framecount\":\"%d\",		\n"
					"		\"avgpsnr\":\"%d\",		\n"
					"		\"avgrmse\":\"%d\",		\n"
					"	}					\n"
					"}						";
	}


	// get the videofiles in openCV::VideoCapture
	VideoCapture captReference(referenceVideo), captTest(testVideo);

	if (!captReference.isOpened()){
		cerr  << "Could not open reference video " << referenceVideo << endl;
		usage();
		return -1;
	}
	if (!captTest.isOpened()){
		cerr  << "Could not open test video " << testVideo << endl;
		usage();
		return -1;
	}

	Size 	dstS 		= Size((int) captTest.get(CAP_PROP_FRAME_WIDTH),
					(int) captTest.get(CAP_PROP_FRAME_HEIGHT));
	Size 	refS 		= Size((int) captReference.get(CAP_PROP_FRAME_WIDTH),
					(int) captReference.get(CAP_PROP_FRAME_HEIGHT)),
		uTSi 		= Size((int) captTest.get(CAP_PROP_FRAME_WIDTH),
					(int) captTest.get(CAP_PROP_FRAME_HEIGHT));
	int totalTstFrames 	= captTest.get(CAP_PROP_FRAME_COUNT);
	int totalSrcFrames 	= captReference.get(CAP_PROP_FRAME_COUNT);
	int fourCCTstProp	= static_cast<int>(captTest.get(CAP_PROP_FOURCC));
	int fourCCSrcProp	= static_cast<int>(captReference.get(CAP_PROP_FOURCC));

	// do some union magic to make the fourCC index into readable text... doesn't work yet
	// todo: make sure no illegal characters get returned
	union { int v; char c[5];} fourCCTst ;
	fourCCTst.v = fourCCTstProp;
	fourCCTst.c[4]='\0';
	union { int v; char c[5];} fourCCSrc ;
	fourCCSrc.v = fourCCSrcProp;
	fourCCSrc.c[4]='\0';

	// prepare the Mat's for frame(image) manipulation
	Mat frameRReference, frameReference, frameTest;

	if (withmssim){
		tests = "PSNR, RMSE and MSSIM";
	} else {
		tests = "PSNR and RMSE";
	}

	string sourceDims = std::to_string(static_cast<long long>(refS.width)) + "x" + std::to_string(static_cast<long long>(refS.height));
	string testDims = std::to_string(static_cast<long long>(dstS.width)) + "x" + std::to_string(static_cast<long long>(dstS.height));

	// write json header to output file
	fprintf(outputfile, templateHeader, referenceVideo, sourceDims.c_str(), totalSrcFrames, testVideo, testDims.c_str(), totalTstFrames, tests.c_str());

	cout << "Source media: " << referenceVideo << " (" << refS.width << "x" << refS.height << " [" << totalSrcFrames << " frames])" << endl;
	cout << "Test media: " << testVideo << " (" << dstS.width << "x" << dstS.height << " [" << totalTstFrames << " frames])" << endl;
	cout << "Tests: " << tests << endl;
	cout << "Output format: " << outputformat << endl;
	cout << endl;

	// start the frame loop
	// shift the test video tfa frame ahead:
	if (tfa > tfc){
		while (tfc < tfa){
			captTest >> frameTest;
			++tfc;
		}
	} else {
		cout << "No test frame advance" << endl;
	}

	if (sfa > sfc) {
		while (sfc < sfa){
			captReference >> frameReference;
			++sfc;
		}
	} else {
		cout << "No source frame advance" << endl;
	}

	cout << "Skipped test video " << tfc << " frames ahead" << endl;
	cout << "Skipped source video " << sfc << " frames ahead" << endl;

	for(;;){
		captReference >> frameReference;
		captTest >> frameTest;

		if (frameReference.empty() || frameTest.empty()){
			// nothing more to do... clean up and exit.
			cout << "\x1B[2K";
			cout << "\x1B[0E";
			flush(cout);
			cout << "Comparison completed... " << endl;
			cout << "Results can be found in " << oFile << endl;
			break;
		}
		++frameNum;


		if ( frameNum % nth == 0 ){
			++loops;
			double curTstMsec = captTest.get(CAP_PROP_POS_MSEC);
			double curSrcMsec = captReference.get(CAP_PROP_POS_MSEC);

			resize(frameReference, frameRReference, dstS, 0, 0);
			psnrV = getPSNR(frameRReference,frameTest);
			rmseV = getRMSE(frameRReference, frameTest);

			sumpsnr += psnrV;
			sumrmse += rmseV;

			cout << "\x1B[2K";
			cout << "\x1B[0E";
			cout << "frame: " << frameNum << " -> PSNR: " << setiosflags(ios::fixed) << setprecision(3) << psnrV << " RMSE: " << setiosflags(ios::fixed) << setprecision(3) << rmseV;

			// add to output
			fprintf(outputfile, templateFrame, frameNum, psnrV, rmseV, curTstMsec, curSrcMsec);

			if (withmssim){
				mssimV = getMSSIM(frameRReference, frameTest);
				// TODO: Find a way to incorporate MSSIM values........
			//	outputfile << "," << endl
			//		<< "			\"mssim\": {" << endl
			//		<< "				\"R\":\"" << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[2] * 100 << "\"," << endl
			//		<< "				\"G\":\"" << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[1] * 100 << "\"," << endl
			//		<< "				\"B\":\"" << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[0] * 100 << "\"" << endl
			//		<< "			}";
				cout << " -> MSSIM: R: " << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[2] * 100 << ", G: " << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[1] * 100 << ", B: " << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[0] * 100;
			}
			flush(cout);
		}
	}
	double avgpsnr = sumpsnr / loops;
	double avgrmse = sumrmse / loops;

	// finish up the output
	fprintf(outputfile, templateFooter, frameNum, avgpsnr, avgrmse);
	fclose(outputfile);
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

double getRMSE( const Mat& I1, const Mat& I2 ){
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
		double rmse = sqrt(mse);
		return rmse;
	}
}

Scalar getMSSIM( const Mat& i1, const Mat& i2){
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
