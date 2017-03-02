//
// Created by lancelot on 2/27/17.
//

#include "../Tracker.h"
#include "opencv2/ts/ts.hpp"
#include "DataStructure/viFrame.h"
#include "DataStructure/cv/cvFrame.h"
#include "IO/camera/CameraIO.h"
#include "cv/FeatureDetector/Detector.h"
#include "DataStructure/cv/Feature.h"

TEST(Tracker, Tracker) {
    direct_tracker::Tracker tracker;
    cv::Mat pic_i = cv::imread("../testData/mav0/cam0/data/1403715273562142976.png", 0);
    cv::Mat pic_j = cv::imread("../testData/mav0/cam0/data/1403715273562142976.png", 0);
    std::string camDatafile = "../testData/mav0/cam1/data.csv";
    std::string camParamfile ="../testData/mav0/cam1/sensor.yaml";
    CameraIO camTest(camDatafile,camParamfile);
    const CameraIO::pCamereParam cam = camTest.getCamera();
    std::shared_ptr<cvFrame> cvframe_i = std::make_shared<cvFrame>(cam, pic_i);
    cvMeasure::features_t fts;
    feature_detection::Detector detector(pic_i.cols, pic_i.rows, 25, IMG_LEVEL);
    detector.detect(cvframe_i, cvframe_i->getMeasure().measurement.imgPyr, fts);
    for(auto &ft : fts)
        cvframe_i->addFeature(ft);

#ifdef SHOW_DETECT

    int levelTimes[5] = {1,2,4,8,16};
    cv::Scalar color[3] = {cv::Scalar(255,0,0),cv::Scalar(255,255,0),cv::Scalar(0,0,255)};
    cv::Mat result = cv::imread("../testData/mav0/cam0/data/1403715273562142976.png");
    const cvMeasure::features_t& fts_ = cvframe_i->getMeasure().fts_;
    printf("\t--size of feature:%u\n", fts_.size());
    for (auto &it : fts_) {
        if(it->type == Feature::EDGELET) {
            cv::Point2i point0((it)->px(0)*levelTimes[it->level]+it->grad(1)*0.75, (it)->px(1)*levelTimes[it->level]-it->grad(0)*0.75);
            cv::Point2i point1((it)->px(0)*levelTimes[it->level]-it->grad(1)*0.75, (it)->px(1)*levelTimes[it->level]+it->grad(0)*0.75);
            cv::line(result, point0, point1, color[it->level]);
        }

        if(it->type == Feature::CORNER) {
            cv::Point2i point(int((it)->px(0)), int((it)->px(1)));
            cv::circle(result, point, 4 * (it->level + 1), cv::Scalar(0, 255, 0), 1);
        }

    }

    cv::imshow("result", result);
    cv::waitKey();

#endif //SHOW_DETECT

    std::shared_ptr<viFrame> viframe_i = std::make_shared<viFrame>(1, cvframe_i);

    std::shared_ptr<cvFrame> cvframe_j = std::make_shared<cvFrame>(cam, pic_j);
    std::shared_ptr<viFrame> viframe_j = std::make_shared<viFrame>(2, cvframe_j);

    Sophus::SE3d Tij;
    //! test runing time
    printf("\t--start tracking!\n");
    bool isTracked = tracker.Tracking(viframe_i, viframe_j, Tij, 50);
    if(isTracked)
        printf("\t--successful!\n");
    else
        printf("\t--failed!\n");
    std::cout<<Tij.so3().matrix()<<"\n";
    std::cout<<Tij.translation()<<"\n";
    GTEST_ASSERT_EQ(Tij.so3().matrix(), Eigen::Matrix3d::Identity());
    GTEST_ASSERT_EQ(Tij.translation(), Eigen::Vector3d::Zero());

    pic_j = cv::imread("../testData/mav0/cam0/data/1403715273312143104.png", 0);

    if(isTracked)
        printf("successful!\n");
    else
        printf("failed!\n");
}