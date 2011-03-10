#include "RecognitionEngine.h"

#include <iostream>

using namespace std;

RecognitionEngine::RecognitionEngine()
{

    trainingParams = cvSURFParams(300, 0); //1000 for iPhone
    trainingParams.nOctaves = 4; //4 default
    trainingParams.nOctaveLayers = 2; //2 default

    recognitionParams = cvSURFParams(600, 0); //900 for iPhone
    recognitionParams.nOctaves = 1; //2 for iPhone
    recognitionParams.nOctaveLayers = 2; //2 default

    win_size = 7;

    flann_index = NULL;
    m_object = NULL;

    //For Tracking
    maxNumberOfTrackedPoints = 40;

    numberMatches = 0;

    iplGray = cvCreateImage(cvSize(imgWidth, imgHeight), IPL_DEPTH_8U, 1);
    prev_grey = cvCreateImage( cvGetSize(iplGray), IPL_DEPTH_8U, 1 );
    pyramid = cvCreateImage( cvGetSize(iplGray), IPL_DEPTH_8U, 1 );
    prev_pyramid = cvCreateImage( cvGetSize(iplGray), IPL_DEPTH_8U, 1 );
    surfMask = cvCreateImage(cvSize(imgWidth, imgHeight), IPL_DEPTH_8U, 1);

    large_iplGray = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);

    tempImageForTracking = cvCreateImage( cvGetSize(iplGray), IPL_DEPTH_32F, 1 );
    secondTempImageForTracking = cvCreateImage( cvGetSize(iplGray), IPL_DEPTH_32F, 1 );

    flags = 0;

    points[0] = (CvPoint2D32f*)cvAlloc(maxNumberOfTrackedPoints*sizeof(points[0][0]));
    points[1] = (CvPoint2D32f*)cvAlloc(maxNumberOfTrackedPoints*sizeof(points[0][0]));


    status = (char*) cvAlloc(maxNumberOfTrackedPoints*sizeof(char));

    minNumberOfMatchesThr = 20;
    numberOfTrackedPoints = 0;

    maxLostPointsRatio = 0.2;
    imagePointsForTracking.resize(maxNumberOfTrackedPoints);
    templatePointsForTracking.resize(maxNumberOfTrackedPoints);

    averageRecognitionTime = 0.0;
    recognitionCount = 0;
    averageMatchesCount = 0.0;

    templateFound = false;

}

RecognitionEngine::~RecognitionEngine()
{	
    if(flann_index)
        delete flann_index;
    if(m_object)
        delete m_object;

    cvReleaseImage(&iplGray);
    cvReleaseImage(&prev_grey);
    cvReleaseImage(&pyramid);
    cvReleaseImage(&prev_pyramid);
    cvReleaseImage(&surfMask);

    cvReleaseImage(&large_iplGray);

    cvReleaseImage(&tempImageForTracking);
    cvReleaseImage(&secondTempImageForTracking);

    cvFree(&status);
    cvFree(&(points[0]));
    cvFree(&(points[1]));

}

void RecognitionEngine::saveSurfFeatures(const SurfFeatures& templateFeatures, const std::string& fileName )
{
    CvFileStorage *fileStorage = cvOpenFileStorage(fileName.c_str(), 0, CV_STORAGE_WRITE);
    cvWriteString(fileStorage, "name", templateFeatures.imageName.c_str());
    cvWriteInt(fileStorage, "width", templateFeatures.width);
    cvWriteInt(fileStorage, "height", templateFeatures.height);
    cvWrite(fileStorage, "keypoints", templateFeatures.keypoints);
    cvWrite(fileStorage, "descriptors", templateFeatures.descriptors);

    cvReleaseFileStorage(&fileStorage);

}

void RecognitionEngine::loadSurfFeatures(const char* fileName)
{
    SurfFeatures _templFeatures;

    CvFileStorage *fileStorage = cvOpenFileStorage(fileName, _templFeatures.featuresStorage, CV_STORAGE_READ);
    if (!fileStorage) {
        printf("Can't open %s \n", fileName);
        return;
    }

    _templFeatures.imageName = cvReadStringByName(fileStorage, 0, "name", 0);
    _templFeatures.width = cvReadIntByName(fileStorage, 0, "width", 0);
    _templFeatures.height	= cvReadIntByName(fileStorage, 0, "height", 0);
    _templFeatures.keypoints = (CvSeq*) cvReadByName(fileStorage, 0, "keypoints");
    _templFeatures.descriptors = (CvSeq*) cvReadByName(fileStorage, 0, "descriptors");
    this->templateFeatures.push_back(_templFeatures);

    cvReleaseFileStorage(&fileStorage);

}


void RecognitionEngine::createFlannIndex()
{
    //check if we have training data
    if(this->templateFeatures.size() < 1)
        return;

    //all descriptors assumed to have same length (64 or 128)
    int length = (int)(this->templateFeatures[0].descriptors->elem_size/sizeof(float));

    //total number of descriptors in the database
    int total = 0;
    for(unsigned int i = 0; i < this->templateFeatures.size(); i++)
    {
        total += this->templateFeatures[i].descriptors->total;
    }

    //why do we need to store m_object? if not, it crashes while searching the nearest neighbours
    if(m_object)
    {
        delete m_object;
        m_object = 0;
    }
    m_object = new cv::Mat(total, length, CV_32F);
    float* obj_ptr = m_object->ptr<float>(0);

    // copy descriptors
    unsigned int _total = 0;
    for(unsigned int i = 0; i < this->templateFeatures.size(); i++)
    {
        CvSeqReader obj_reader;
        cvStartReadSeq( this->templateFeatures[i].descriptors, &obj_reader );
        printf("number of features:%d \n", this->templateFeatures[i].descriptors->total);
        for(int j = 0; j < this->templateFeatures[i].descriptors->total; j++ )
        {
            const float* descriptor = (const float*)obj_reader.ptr;
            CV_NEXT_SEQ_ELEM( obj_reader.seq->elem_size, obj_reader )
                    memcpy(obj_ptr, descriptor, length*sizeof(float));
            obj_ptr += length;
        }
        _total += this->templateFeatures[i].descriptors->total;
    }
    printf("total features: %d \n", _total);

    if(flann_index)
        delete flann_index;
    flann_index = new cv::flann::Index(*m_object, /* cv::flann::KMeansIndexParams(16, 15, cv::flann::CENTERS_RANDOM,  0.2 ));*/
                                       cv::flann::KDTreeIndexParams(2)); //was 4
    
}



void RecognitionEngine::flannFindPairs( const CvSeq* imageDescriptors, std::vector<int>& ptpairs )
{
    if(flann_index == NULL) createFlannIndex();

    int length = (int)(imageDescriptors->elem_size/sizeof(float));

    cv::Mat m_image(imageDescriptors->total, length, CV_32F);
    // copy descriptors
    CvSeqReader img_reader;
    float* img_ptr = m_image.ptr<float>(0);
    cvStartReadSeq( imageDescriptors, &img_reader );
    for(int i = 0; i < imageDescriptors->total; i++ )
    {
        const float* descriptor = (const float*)img_reader.ptr;
        CV_NEXT_SEQ_ELEM( img_reader.seq->elem_size, img_reader )
                memcpy(img_ptr, descriptor, length*sizeof(float));
        img_ptr += length;
    }

    // find nearest neighbors using FLANN
    cv::Mat m_indices(imageDescriptors->total, 2, CV_32S);
    cv::Mat m_dists(imageDescriptors->total, 2, CV_32F);
    flann_index->knnSearch(m_image, m_indices, m_dists, 2, cv::flann::SearchParams(16) ); //was 64 // maximum number of leafs checked

    int* indices_ptr = m_indices.ptr<int>(0);
    float* dists_ptr = m_dists.ptr<float>(0);
    for (int i=0;i<m_indices.rows;++i) {
    	if (dists_ptr[2*i]<0.6*dists_ptr[2*i+1]) {
            ptpairs.push_back(i);
            ptpairs.push_back(indices_ptr[2*i]);
    	}
    }
}

//homography should have size 9
bool RecognitionEngine::locatePlanarObject( float homography[])
{
    std::vector<int> ptpairs;
    int i, n;

    //int t_on = clock(); // timer before calling func
    flannFindPairs(this->imageFeatures.descriptors, ptpairs );
    //int t_off = clock(); // timer when func returns

    //cout << "flannFindPairs: " << (static_cast<float>(t_off - t_on))/CLOCKS_PER_SEC << " seconds" << endl;

    n = ptpairs.size()/2;
    if( n < this->minNumberOfMatchesThr )
        return false;


    //find the template with maxmimum number of matches
    std::vector< std::vector<int> > matchesIndices(this->templateFeatures.size());
    //correspinding indices from images
    std::vector< std::vector<int> > imageIndices(this->templateFeatures.size());
    //loop over n matches
    for(int i = 0; i < n; i++)
    {
        int matchIndex = ptpairs[i*2+1];
        int lowerBound = 0;
        int upperBound = 0;
        //find which template this match belong to
        for(unsigned int j = 0; j < this->templateFeatures.size(); j++)
        {
            //number of descriptors in this template
            int _count = this->templateFeatures[j].descriptors->total;
            upperBound += _count;
            if((matchIndex > lowerBound - 1) &&
               (matchIndex < upperBound))
            {
                matchesIndices[j].push_back(matchIndex - lowerBound);
                imageIndices[j].push_back(ptpairs[i*2]);
                break;
            }
            lowerBound += _count;
        }
    }
    //find the template with maximum number of matches
    std::vector<int> matchesCount(this->templateFeatures.size());
    for(unsigned int j = 0; j < this->templateFeatures.size(); j++)
    {
        matchesCount[j] = matchesIndices[j].size();
    }

    std::vector<int>::iterator iterMax = std::max_element(matchesCount.begin(), matchesCount.end());

    //Matched template
    this->matchedTemplate = iterMax - matchesCount.begin();
    int numberOfMatches = *iterMax;
    if(numberOfMatches < this->minNumberOfMatchesThr)
        return false;

    printf("Matched template: %d", this->matchedTemplate);
    printf(" file: %s \n", this->templateFeatures[this->matchedTemplate].imageName.c_str());

    this->templateFeatures[this->matchedTemplate].matchedPts.resize(numberOfMatches);
    this->imageFeatures.matchedPts.resize(numberOfMatches);

    float _average_scale = 0;
    for( i = 0; i < numberOfMatches; i++ )
    {
        CvSURFPoint* _surfPoint =  (CvSURFPoint*)cvGetSeqElem(this->templateFeatures[this->matchedTemplate].keypoints, matchesIndices[this->matchedTemplate][i]);
        this->templateFeatures[this->matchedTemplate].matchedPts[i] = _surfPoint->pt;
        this->imageFeatures.matchedPts[i] = ((CvSURFPoint*)cvGetSeqElem(this->imageFeatures.keypoints,imageIndices[this->matchedTemplate][i]))->pt;
        _average_scale += _surfPoint->size;
    }
    _average_scale /= numberOfMatches;
    //  printf("average_scale: %f \n", _average_scale);

    CvMat _pt1 = cvMat(1, n, CV_32FC2, &this->templateFeatures[this->matchedTemplate].matchedPts[0] );
    CvMat _pt2 = cvMat(1, n, CV_32FC2, &this->imageFeatures.matchedPts[0] );

    //t_on = clock(); // timer before calling func
    bool found = false;

    CvMat* _h = cvCreateMatHeader(3, 3, CV_32F);
    cvSetData(_h, homography, CV_AUTOSTEP);
    try{
        found = cvFindHomography( &_pt1, &_pt2, _h, CV_RANSAC, 1 );
    }
    catch( cv::Exception& e )
    {
        const char* err_msg = e.what();
        printf("Exception in cvExtractSURF: %s \n", err_msg );
        cvReleaseMatHeader(&_h);
        return false;
    }
    cvReleaseMatHeader(&_h);
    //t_off = clock(); // timer when func return
    //cout << "cvFindHomography: " << (static_cast<float>(t_off - t_on))/CLOCKS_PER_SEC << " seconds" << endl;
    if(!found)
        return false;

    return true;
}

void RecognitionEngine::transform(const float h[], const CvPoint2D32f& src, CvPoint2D32f& dst ) const
{

    float x = src.x, y = src.y;
    float Z = 1./(h[6]*x + h[7]*y + h[8]);
    float X = (h[0]*x + h[1]*y + h[2])*Z;
    float Y = (h[3]*x + h[4]*y + h[5])*Z;
    dst.x = cvRound(X);
    dst.y = cvRound(Y);

}

//make sure templatePointsInImagePlane has the size templateFeatures.templateKeypoints->total
void RecognitionEngine::computeObjectPointsInImagePlane(const SurfFeatures& templateFeatures,  float* h)
{

    for( int i = 0; i < templateFeatures.keypoints->total; i++ )
    {
        CvPoint2D32f pt = ((CvSURFPoint*)cvGetSeqElem(templateFeatures.keypoints,i))->pt;

        CvPoint2D32f dst = cvPoint2D32f(0, 0);
        transform(h, pt, dst);

        templatePointsInImagePlane[i] = dst;

    }


}

void RecognitionEngine::createAndLoadSurfFeaturesFromImage(const std::string& imageFileName)
{

    SurfFeatures _templFeatures;
    createSurfFeaturesFromImage(_templFeatures, imageFileName);

    std::string xmlFileName = imageFileName + std::string(".xml");
    saveSurfFeatures(_templFeatures, xmlFileName);

    this->templateFeatures.push_back(_templFeatures);

    createFlannIndex();

}

void RecognitionEngine::createAndLoadSurfFeaturesFromImage(IplImage* image, const std::string& imageFileName, const std::string& xmlFileName)
{

    SurfFeatures _templFeatures;
    createSurfFeaturesFromImage(_templFeatures, image, imageFileName);

    saveSurfFeatures(_templFeatures, xmlFileName);

    this->templateFeatures.push_back(_templFeatures);

    createFlannIndex();

}

void RecognitionEngine::createSurfFeaturesFromImage(SurfFeatures& surfFeatures, const std::string& imageFileName)
{
    IplImage* img = cvLoadImage(imageFileName.c_str(),0);
    if(!img)
    {
        printf("Could not load image.") ;
        return;
    }

    surfFeatures.width = img->width;
    surfFeatures.height = img->height;
    surfFeatures.imageName = imageFileName;

    cvExtractSURF( img, 0, &surfFeatures.keypoints, &surfFeatures.descriptors,
                   surfFeatures.featuresStorage, trainingParams, 0 );

    cvReleaseImage(&img);

}

void RecognitionEngine::createSurfFeaturesFromImage(SurfFeatures& surfFeatures, IplImage* image, const std::string& imageFileName)
{
    surfFeatures.width = image->width;
    surfFeatures.height = image->height;
    surfFeatures.imageName = imageFileName;

    cvExtractSURF( image, 0, &surfFeatures.keypoints, &surfFeatures.descriptors,
                   surfFeatures.featuresStorage, trainingParams, 0 );
}

bool RecognitionEngine::surfRecognize() {

    //check if we have a template
    if( this->templateFeatures.size() < 1)
        return false;

    /* automatic initialization */
    bool found = false;
    cvClearMemStorage(this->imageFeatures.featuresStorage);

    int t_on = clock(); // timer before calling func
    //if we already have a matched template, try to extract surf only in approximate known locations
    IplImage *mask = 0;

    try{
        cvExtractSURF(  this->iplGray, mask, & this->imageFeatures.keypoints, & this->imageFeatures.descriptors,
                        this->imageFeatures.featuresStorage,  this->recognitionParams, 0 );
    }
    catch( cv::Exception& e )
    {
        const char* err_msg = e.what();
        printf("Exception in cvExtractSURF: %s \n", err_msg );
        return false;
    }

    //if extract surf fails
    if(this->imageFeatures.keypoints == NULL || this->imageFeatures.descriptors == NULL)
    {
        return false;
    }


    found =  this->locatePlanarObject(homography);
    if(!found) {
        return false;
    }
    this->numberMatches = this->templateFeatures[this->matchedTemplate].matchedPts.size();
    //printf("N matches: %d \n", this->numberMatches);

    int t_off = clock(); // timer when func returns
    float currentRecognTime = (static_cast<float>(t_off - t_on))/CLOCKS_PER_SEC;
    //cout << "Recgn time: " << currentRecognTime << " seconds" << endl;
    averageRecognitionTime = recognitionCount * averageRecognitionTime + currentRecognTime;
    averageMatchesCount = recognitionCount * averageMatchesCount + this->numberMatches;

    recognitionCount++;
    averageRecognitionTime /= recognitionCount;
    averageMatchesCount /= recognitionCount;

    printf("averageRecognitionTime: %f \n", averageRecognitionTime);
    // printf("averageRecognitionTimeUsingPrediction: %f \n", averageRecognitionTimeUsingPrediction);
    // printf("predictionRate: %f \n", predictionRate);
    printf("averageMatchesCount: %f \n", averageMatchesCount);

    //end recognition
    return true;
}


bool RecognitionEngine::surfTrack() {

    //check if we have a template
    if( this->templateFeatures.size() < 1)
        return false;

    if( !templateFound )
    {
        if(!surfRecognize())
            return false;

        //end recognition

        //Compute points for tracking

        int t_on = clock(); // timer before calling func

        //create mask
        cvSet(surfMask, cvScalar(0));
        //find bounding box
        int _h = this->templateFeatures[this->matchedTemplate].height;
        int _w = this->templateFeatures[this->matchedTemplate].width;
        std::vector<CvPoint2D32f> _srcPoints;
        _srcPoints.push_back(cvPoint2D32f(0,0));
        _srcPoints.push_back(cvPoint2D32f(0,_h));
        _srcPoints.push_back(cvPoint2D32f(_w,_h));
        _srcPoints.push_back(cvPoint2D32f(_w,0));

        float minX = 100000;
        float minY = 100000;
        float maxX = -100000;
        float maxY = -100000;
        for(int i = 0; i < 4; i++)
        {
            CvPoint2D32f dst;
            this->transform(this->homography, _srcPoints[i], dst);
            if(dst.x < minX)
                minX = dst.x;
            else  if(dst.x > maxX)
                maxX = dst.x;

            if(dst.y < minY)
                minY = dst.y;
            else if(dst.y > maxY)
                maxY = dst.y;
        }
        if(minX < 0)
            minX = 0;
        else if(minX > this->surfMask->width - 1)
            minX = this->surfMask->width;

        if(minY < 0)
            minY = 0;
        else if(minY > this->surfMask->height - 1)
            minY = this->surfMask->height - 1;

        if(maxX < 0)
            maxX = 0;
        else if(maxX > this->surfMask->width - 1)
            maxX = this->surfMask->width;

        if(maxY < 0)
            maxY = 0;
        else if(maxY > this->surfMask->height - 1)
            maxY = this->surfMask->height - 1;

        //      printf("%f %f %f %f \n", minX, maxX, minY, maxY);

        for (int y = minY; y < maxY; ++y) {
            for (int x = minX; x < maxX; ++x) {
                ((uchar*)(this->surfMask->imageData + this->surfMask->widthStep*y))[x] = 1;
            }
        }

        this->numberOfTrackedPoints = this->maxNumberOfTrackedPoints;
        CvPoint2D32f* corners = new CvPoint2D32f[this->numberOfTrackedPoints];
        cvGoodFeaturesToTrack(this->iplGray, this->tempImageForTracking, this->secondTempImageForTracking, corners, &(this->numberOfTrackedPoints),
                              0.01, 1.5 * this->win_size, this->surfMask);
        cvFindCornerSubPix(this->iplGray,  corners, this->numberOfTrackedPoints,
                           cvSize(this->win_size, this->win_size), cvSize(-1,-1), cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
        int t_off = clock(); // timer when func returns
        float currentRecognTime = (static_cast<float>(t_off - t_on))/CLOCKS_PER_SEC;
        printf("Recgn time for Harris: %f \n",currentRecognTime);

        CvMat _hom = cvMat(3, 3, CV_32F, homography);
        CvMat* _h_inv = cvCreateMat(3, 3, CV_32F);
        cvInvert(&_hom, _h_inv);

        int _trackedPointsInTemplateCount = 0;
        for(int i = 0; i <  this->numberOfTrackedPoints; i++)
        {
            CvPoint2D32f _pt;
            this->transform( _h_inv->data.fl,  corners[i], _pt);
            if((_pt.x < 0) ||
               (_pt.x > this->templateFeatures[this->matchedTemplate].width - 1) ||
               (_pt.y < 0) ||
               (_pt.y > this->templateFeatures[this->matchedTemplate].height - 1))
                continue;
            templatePointsForTracking[_trackedPointsInTemplateCount] =  _pt;
            points[1][_trackedPointsInTemplateCount] =  corners[i];
            _trackedPointsInTemplateCount++;
        }
        this->numberOfTrackedPoints = _trackedPointsInTemplateCount;

        cvReleaseMat(&_h_inv);
        delete [] corners;

        templateFound = true;
    } //end init
    else {
        //track features
        cvCalcOpticalFlowPyrLK(  prev_grey,  iplGray,  prev_pyramid,  pyramid,
                                 points[0],  points[1],   this->numberOfTrackedPoints, cvSize( win_size,
                                                                                               win_size), 3,  status, 0,
                                 cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03),  flags ); //was 20, 0.03

        //use only well tracked points for homography
        std::vector<CvPoint2D32f> _objectPointsForHomography;
        std::vector<CvPoint2D32f> _imagePointsForHomography;

        for(int i = 0; i <   this->numberOfTrackedPoints; i++ )
        {
            if( status[i] == 0) {
                lostIndices.push_back(i);
                continue;
            }
            std::list<int>::iterator result = std::find( lostIndices.begin(),  lostIndices.end(), i);
            if(result !=  lostIndices.end())
                continue;
            _objectPointsForHomography.push_back( templatePointsForTracking[i]);
            _imagePointsForHomography.push_back( points[1][i]);
        }

        //check tracking
        if(lostIndices.size() >   this->numberOfTrackedPoints *  maxLostPointsRatio)
        {
            //lost tracking, reset for recognition
            templateFound = false;
            flags = 0;
            lostIndices.clear();
            return false;
        }

        flags |= CV_LKFLOW_PYR_A_READY;
        //compute new homography
        //CvMat _h = cvMat(3, 3, CV_32F, homography);
        CvMat* _h = cvCreateMatHeader(3, 3, CV_32F);
        cvSetData(_h, homography, CV_AUTOSTEP);

        CvMat _pt1 = cvMat(1, _objectPointsForHomography.size(), CV_32FC2, &_objectPointsForHomography[0] );
        CvMat _pt2 = cvMat(1, _imagePointsForHomography.size(), CV_32FC2, &_imagePointsForHomography[0] );

        bool OK = false;
        try{
            OK = cvFindHomography( &_pt1, &_pt2, _h );//, CV_RANSAC, 2 );
        }
        catch( cv::Exception& e )
        {
            const char* err_msg = e.what();
            printf("Exception in cvFindHomography: %s \n", err_msg );
            templateFound = false;
            flags = 0;
            lostIndices.clear();
            cvReleaseMatHeader(&_h);
            return false;
        }
        cvReleaseMatHeader(&_h);

        if(!OK) {
            //lost homogrophy, reset for recognition
            templateFound = false;
            flags = 0;
            lostIndices.clear();
            return false;
        }
    }//end tracking
    CV_SWAP(  prev_grey,  iplGray,  swap_temp );
    CV_SWAP(  prev_pyramid,  pyramid,  swap_temp );
    CV_SWAP(  points[0],  points[1],  swap_points );
    return true;
}

void RecognitionEngine::saveCurrentImage(const char* fileName)
{
    cvSaveImage(fileName, iplGray);
}

void RecognitionEngine::reset()
{
    if(flann_index != NULL)
    {
        delete flann_index;
        flann_index = NULL;
    }
    if(m_object != NULL)
    {
        delete m_object;
        m_object = NULL;
    }

    this->templateFeatures.clear();

}

void RecognitionEngine::buildDatabase(const std::vector<std::string>& dbFiles)
{
    for(unsigned int i = 0; i < dbFiles.size(); i++)
    {
        this->loadSurfFeatures(dbFiles[i].c_str());

    }


    int t_on = clock();
    createFlannIndex();
    int t_off = clock(); // timer when func returns
    float currentRecognTime = (static_cast<float>(t_off - t_on))/CLOCKS_PER_SEC;
    printf("time to create flann index: %f \n", currentRecognTime);

}

