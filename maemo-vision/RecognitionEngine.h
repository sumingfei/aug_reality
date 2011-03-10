#ifndef Recognition_H
#define Recognition_H

#include <opencv/cv.h>
#include <opencv/cvaux.h>
#include <opencv/highgui.h>
#include <vector>
#include <list>


class SurfFeatures
{

public:

    SurfFeatures()
    {
        keypoints = 0;
        descriptors = 0;

        featuresStorage = cvCreateMemStorage(0);
    }

    ~SurfFeatures()
    {
       //cvReleaseMemStorage(&featuresStorage);

    }

    std::string imageName;
    CvSeq *keypoints;
    CvSeq *descriptors;
    int width;
    int height;

    CvMemStorage* featuresStorage;

    //correspondences
    std::vector<CvPoint2D32f> matchedPts;

    //input paths in image pixel coordinates
    std::vector<CvPoint2D32f> pathPointsInTemplate;
} ;



class RecognitionEngine {

public:	
    RecognitionEngine();
    ~RecognitionEngine();

    std::vector<SurfFeatures> templateFeatures;
    int matchedTemplate;

    SurfFeatures imageFeatures;
    CvSURFParams trainingParams;
    CvSURFParams recognitionParams;

    //FLANN
    cv::flann::Index* flann_index;
    cv::Mat* m_object;

    //Tracking
    int win_size;
    IplImage *iplGray;
    IplImage *prev_grey;
    IplImage *pyramid;
    IplImage *prev_pyramid;
    IplImage *swap_temp;
    IplImage *surfMask;

    IplImage *large_iplGray;

    IplImage *tempImageForTracking;
    IplImage *secondTempImageForTracking;

    //points for tracking
    CvPoint2D32f* points[2];
    CvPoint2D32f* swap_points;

    //tracking status
    char* status;
    int flags;

    unsigned int numberMatches;

    const static int imgWidth = 320;
    const static int imgHeight = 240;

    //from all matched features, pick maxNumberOfTrackedPoints random indices for tracking
    unsigned int maxNumberOfTrackedPoints;
    int minNumberOfMatchesThr;
    int numberOfTrackedPoints;

    float maxLostPointsRatio;
    std::vector<CvPoint2D32f> templatePointsForTracking;
    std::vector<CvPoint2D32f> imagePointsForTracking;
    std::list<int> randomIndices;

    //////////////////////
    std::vector<CvPoint2D32f> templatePointsInImagePlane;
    /////////////////////
    void createFlannIndex( );

    void flannFindPairs( const CvSeq* imageDescriptors, std::vector<int>& ptpairs );

    bool locatePlanarObject( float homography[]);

    void computeObjectPointsInImagePlane(const SurfFeatures& templateFeatures, float* h);

    void reset();

    void createAndLoadSurfFeaturesFromImage(const std::string& imageFileName);
    void createAndLoadSurfFeaturesFromImage(IplImage* image, const std::string& imageFileName, const std::string& xmlFileName);

    void createSurfFeaturesFromImage(SurfFeatures& surfFeatures, const std::string& imageFileName);
    void createSurfFeaturesFromImage(SurfFeatures& surfFeatures, IplImage* image, const std::string& imageFileName);

    bool surfRecognize();
    bool surfTrack();

    void transform(const float h[], const CvPoint2D32f& src, CvPoint2D32f& dst ) const;
    float homography[9];

    void saveCurrentImage(const char* fileName);

    void saveSurfFeatures(const SurfFeatures& templateFeatures, const std::string& fileName);
    void loadSurfFeatures(const char* fileName);

    void buildDatabase(const std::vector<std::string>& dbFiles);

private:


    bool templateFound;
    std::list<int> lostIndices;

    float averageRecognitionTime;
    long recognitionCount;
    float averageMatchesCount;

};

#endif
