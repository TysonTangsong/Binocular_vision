#pragma once
#include "opencv.h"
//�궨��ʼ

static void
StereoCalib(const vector<string>& imagelist, cv::Size boardSize, bool useCalibrated=true, bool showRectified=true, const string& Filename_l=0, const string& Filename_r=0)

{
    if( imagelist.size() % 2 != 0 )  //��Ƭ�ǲ���ż��
    {
        cout << "Error: the image list contains odd (non-even) number of elements\n";
        return;
    }
    
    bool displayCorners = false;//true; ��ʾ�ǵ�
    const int maxScale = 2;
    const float squareSize = 23.f;  // Set this to your actual square size
    // ARRAY AND VECTOR STORAGE:
    
    vector<vector<cv::Point2f> > imagePoints[2];
    vector<vector<cv::Point3f> > objectPoints;
    cv::Size imageSize;
    
    int i, j, k, nimages =( (int)imagelist.size()/2)-1;
    
    imagePoints[0].resize(nimages);
    imagePoints[1].resize(nimages);
    vector<string> goodImageList;
    
    for( i = j = 0; i < nimages; i++ )//��ȡ��Ӧ��Ƭ
    {
        for( k = 0; k < 2; k++ )
        {
            const string& filename = imagelist[i*2+k];
            cv::Mat img = cv::imread(filename, 0);
            if(img.empty())
                break;
            if( imageSize == cv::Size() )
                imageSize = img.size();
            else if( img.size() != imageSize )
            {
                cout << "The image " << filename << " has the size different from the first image size. Skipping the pair\n";//ͼƬ����Ӧ
                break;
            }
            bool found = false;
            vector<cv::Point2f>& corners = imagePoints[k][j];
            for( int scale = 1; scale <= maxScale; scale++ )
            {
                cv::Mat timg;
                if( scale == 1 )
                    timg = img;
                else
                    resize(img, timg, cv::Size(), scale, scale);
                found = findChessboardCorners(timg, boardSize, corners, 
                    CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE); //Ѱ�ҽǵ�
                if( found )
                {
                    if( scale > 1 )
                    {
                        cv::Mat cornersMat(corners);
                        cornersMat *= 1./scale;
                    }
                    break;
                }
            }
            if(1 /*displayCorners*/ )
            {
                cout << filename << endl;
                cv::Mat cimg, cimg1;
                cvtColor(img, cimg, CV_GRAY2BGR);
                drawChessboardCorners(cimg, boardSize, corners, found);
                double sf = 640./MAX(img.rows, img.cols);
                resize(cimg, cimg1, cv::Size(), sf, sf);
                imshow("corners", cimg1);
                char c = (char)cv::waitKey(500);
                if( c == 27 || c == 'q' || c == 'Q' ) //Allow ESC to quit
                    exit(-1);
            }
            else
                putchar('.');
            if( !found )
                break;
            cornerSubPix(img, corners, cv::Size(11,11), cv::Size(-1,-1),
                         cv::TermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS,
                                      30, 0.01));//�趨�ǵ���������;���
        }
        if( k == 2 )
        {
            goodImageList.push_back(imagelist[i*2]);
            goodImageList.push_back(imagelist[i*2+1]);
            j++;
        }
    }
	goodImageList.push_back(imagelist[j*2]);
    goodImageList.push_back(imagelist[j*2+1]);
    cout << j << " pairs have been successfully detected.\n";
    nimages = j;
    if( nimages < 2 )
    {
        cout << "Error: too little pairs to run the calibration\n";
        return;
    }
    
    imagePoints[0].resize(nimages);
    imagePoints[1].resize(nimages);
    objectPoints.resize(nimages);
    
    for( i = 0; i < nimages; i++ )
    {
        for( j = 0; j < boardSize.height; j++ )
            for( k = 0; k < boardSize.width; k++ )
                objectPoints[i].push_back(cv::Point3f(j*squareSize, k*squareSize, 0));
    }
    
    cout << "Running stereo calibration ...\n";//��������궨
    
    cv::Mat cameraMatrix[2], distCoeffs[2];//��Ӷ�ȡ����***********
    cameraMatrix[0] = cv::Mat::eye(3, 3, CV_64F);
    cameraMatrix[1] = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat R, T, E, F;
	//******************************
	cv::FileStorage fs(Filename_l, cv::FileStorage::READ);
    //fs["image_width"] >> calibratedImageSize.width;
    //fs["image_height"] >> calibratedImageSize.height;
    fs["distortion_coefficients"] >> distCoeffs[0];
    fs["camera_matrix"] >> cameraMatrix[0];
    fs.release();
   // if( distCoeffs.type() != CV_64F )
       // distCoeffs[0] = Mat_<double>(distCoeffs);
  //  if( cameraMatrix.type() != CV_64F )
       // cameraMatrix[0] = Mat_<double>(cameraMatrix);
	//******************************
	cv::FileStorage fss(Filename_r, cv::FileStorage::READ);
    //fs["image_width"] >> calibratedImageSize.width;
    //fs["image_height"] >> calibratedImageSize.height;
    fss["distortion_coefficients"] >> distCoeffs[1];
    fss["camera_matrix"] >> cameraMatrix[1];
    fss.release();
    //if( distCoeffs.type() != CV_64F )
       // distCoeffs[1] = Mat_<double>(distCoeffs);
    //if( cameraMatrix.type() != CV_64F )
       // cameraMatrix[1] = Mat_<double>(cameraMatrix);
    //*******************************
	if(!cameraMatrix)
	cout << "cameraMatrix=0\n";
    double rms = stereoCalibrate(objectPoints, imagePoints[0], imagePoints[1],
                    cameraMatrix[0], distCoeffs[0],
                    cameraMatrix[1], distCoeffs[1],
                    imageSize, R, T, E, F,
		            CV_CALIB_USE_INTRINSIC_GUESS,
                    cv::TermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 100, 1e-5)
                    /*CV_CALIB_FIX_ASPECT_RATIO + 
                    CV_CALIB_ZERO_TANGENT_DIST +
                    CV_CALIB_SAME_FOCAL_LENGTH +
                    CV_CALIB_RATIONAL_MODEL +
                    CV_CALIB_FIX_K3 + CV_CALIB_FIX_K4 + CV_CALIB_FIX_K5)*/
					);//����ƽ��ƽ����,����ͷ���� �Ĳ��� ��ǰҪ���е���궨
    cout << "done with RMS error=" << rms << endl;
    
// CALIBRATION QUALITY CHECK
// because the output fundamental matrix implicitly
// includes all the output information,
// we can check the quality of calibration using the
// epipolar geometry constraint: m2^t*F*m1=0
    double err = 0;
    int npoints = 0;
    vector<cv::Vec3f> lines[2];
    for( i = 0; i < nimages; i++ )//ͼ�����
    {
        int npt = (int)imagePoints[0][i].size();
        cv::Mat imgpt[2];
        for( k = 0; k < 2; k++ )//��������
        {
            imgpt[k] = cv::Mat(imagePoints[k][i]);
            undistortPoints(imgpt[k], imgpt[k], cameraMatrix[k], distCoeffs[k], cv::Mat(), cameraMatrix[k]);
            computeCorrespondEpilines(imgpt[k], k+1, F, lines[k]);
        }
        for( j = 0; j < npt; j++ )
        {
            double errij = fabs(imagePoints[0][i][j].x*lines[1][j][0] +
                                imagePoints[0][i][j].y*lines[1][j][1] + lines[1][j][2]) +
                           fabs(imagePoints[1][i][j].x*lines[0][j][0] +
                                imagePoints[1][i][j].y*lines[0][j][1] + lines[0][j][2]);
            err += errij;
        }
        npoints += npt;
    }
    cout << "average reprojection err = " <<  err/npoints << endl;//��ͶӰ���
    
    // save intrinsic parameters ����
    cv::FileStorage fsss("intrinsics.yml", CV_STORAGE_WRITE);//�ڲ���
    if( fsss.isOpened() )
    {
        fsss << "M1" << cameraMatrix[0] << "D1" << distCoeffs[0] <<
            "M2" << cameraMatrix[1] << "D2" << distCoeffs[1]<< "F" << F ;
        fsss.release();
    }
    else
        cout << "Error: can not save the intrinsic parameters\n";
    
    cv::Mat R1, R2, P1, P2, Q;
    cv::Rect validRoi[2];
    
    stereoRectify(cameraMatrix[0], distCoeffs[0],
                  cameraMatrix[1], distCoeffs[1],
                  imageSize, R, T, R1, R2, P1, P2, Q,
                  cv::CALIB_ZERO_DISPARITY,1, imageSize, &validRoi[0], &validRoi[1]);//����У��
        
    fs.open("extrinsics.yml", CV_STORAGE_WRITE);//�����
    if( fs.isOpened() )
    {
        fs << "R" << R << "T" << T << "R1" << R1 << "R2" << R2 << "P1" << P1 << "P2" << P2 << "Q" << Q;
        fs.release();
    }
    else
        cout << "Error: can not save the intrinsic parameters\n";//д����
    
    // OpenCV can handle left-right
    // or up-down camera arrangements
    bool isVerticalStereo = fabs(P2.at<double>(1, 3)) > fabs(P2.at<double>(0, 3));
  
// COMPUTE AND DISPLAY RECTIFICATION
    if( !showRectified )
        return;
    
    cv::Mat rmap[2][2];
// IF BY CALIBRATED        ʹ�� (BOUGUET'S METHOD)
    if( useCalibrated )
    {
        // we already computed everything
    }
// OR ELSE ʹ�� HARTLEY'S METHOD
    else
 // use intrinsic parameters of each camera, but
 // compute the rectification transformation directly
 // from the fundamental matrix
    {
        vector<cv::Point2f> allimgpt[2];
        for( k = 0; k < 2; k++ )
        {
            for( i = 0; i < nimages; i++ )
                std::copy(imagePoints[k][i].begin(), imagePoints[k][i].end(), back_inserter(allimgpt[k]));
        }
        F = cv::findFundamentalMat(cv::Mat(allimgpt[0]), cv::Mat(allimgpt[1]), cv::FM_8POINT, 0, 0);
        cv::Mat H1, H2;
        stereoRectifyUncalibrated(cv::Mat(allimgpt[0]), cv::Mat(allimgpt[1]), F, imageSize, H1, H2, 3);
        
        R1 = cameraMatrix[0].inv()*H1*cameraMatrix[0];
        R2 = cameraMatrix[1].inv()*H2*cameraMatrix[1];
        P1 = cameraMatrix[0];
        P2 = cameraMatrix[1];
    }

    //Precompute maps for cv::remap()
    initUndistortRectifyMap(cameraMatrix[0], distCoeffs[0], R1, P1, imageSize, CV_16SC2, rmap[0][0], rmap[0][1]);
    initUndistortRectifyMap(cameraMatrix[1], distCoeffs[1], R2, P2, imageSize, CV_16SC2, rmap[1][0], rmap[1][1]);//У��ӳ��
  
    cv::Mat canvas;
    double sf;
    int w, h;
    if( !isVerticalStereo )//Ϊ��������
    {
        sf = 600./MAX(imageSize.width, imageSize.height);
        w = cvRound(imageSize.width*sf);
        h = cvRound(imageSize.height*sf);
        canvas.create(h, w*2, CV_8UC3);
    }
    else
    {
        sf = 300./MAX(imageSize.width, imageSize.height);
        w = cvRound(imageSize.width*sf);
        h = cvRound(imageSize.height*sf);
        canvas.create(h*2, w, CV_8UC3);
    }
    
	  //change
	nimages=nimages+1;
    for( i = 0; i < nimages; i++ )//nimangesͼƬ����
    {
        for( k = 0; k < 2; k++ )
        {
            cv::Mat img = cv::imread(goodImageList[i*2+k], 0), rimg, cimg;
            remap(img, rimg, rmap[k][0], rmap[k][1], CV_INTER_LINEAR);
            cvtColor(rimg, cimg, CV_GRAY2BGR);
            cv::Mat canvasPart = !isVerticalStereo ? canvas(cv::Rect(w*k, 0, w, h)) : canvas(cv::Rect(0, h*k, w, h));
            resize(cimg, canvasPart, canvasPart.size(), 0, 0, CV_INTER_AREA);
            if( useCalibrated )
            {
                cv::Rect vroi(cvRound(validRoi[k].x*sf), cvRound(validRoi[k].y*sf),
                          cvRound(validRoi[k].width*sf), cvRound(validRoi[k].height*sf)); 
                rectangle(canvasPart, vroi, cv::Scalar(0,0,255), 3, 8);
            }
        }
        
        if( !isVerticalStereo )
            for( j = 0; j < canvas.rows; j += 16 )
                line(canvas, cv::Point(0, j), cv::Point(canvas.cols, j), cv::Scalar(0, 255, 0), 1, 8);
        else
            for( j = 0; j < canvas.cols; j += 16 )
                line(canvas, cv::Point(j, 0), cv::Point(j, canvas.rows), cv::Scalar(0, 255, 0), 1, 8);
        imshow("avic", canvas);
        char c = cvWaitKey(2000);
        if (c=='w')
        {
        cvWaitKey(0);
        }
        cvDestroyWindow("avic");
        }
}
