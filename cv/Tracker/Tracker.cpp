//
// Created by lancelot on 2/20/17.
//


#include <memory>
#include "Tracker.h"
#include "DataStructure/cv/cvFrame.h"
#include "DataStructure/viFrame.h"
#include "DataStructure/cv/Feature.h"
#include "DataStructure/cv/Point.h"

namespace direct_tracker {

typedef Eigen::Matrix<double, 1, 6> Jac_t;
bool compute_EdgeJac(std::shared_ptr<viFrame> &viframe_j, std::shared_ptr<Feature> &ft, const Sophus::SE3d T_SB, const Sophus::SE3d& Ti,  Sophus::SE3d& T_ji, Jac_t &jac, double &w) {
    std::shared_ptr<Point>& p = ft->point;
    Eigen::Vector2d& dir = ft->grad;
    Eigen::Vector2d grad;
    Eigen::Vector3d P = T_ji * Ti * p->pos_;
    const viFrame::cam_t & cam = viframe_j->getCam();
    Eigen::Vector3d Pj = T_SB * P;
    if(Pj(2) < 0.0000001)
        return false;

    double u = cam->fx() * Pj(0) / Pj(2) + cam->cx();
    if(u < 0 || u >= viframe_j->getCVFrame()->getWidth(ft->level))
        return false;

    double v = cam->fy() * Pj(1) / Pj(2) + cam->cy();

    if(v < 0 || v >= viframe_j->getCVFrame()->getHeight(ft->level))
        return false;

    if(!viframe_j->getCVFrame()->getGrad(u, v, grad, ft->level))
        return false;

    w = 1.0 / viframe_j->getCVFrame()->getGradNorm(u, v, ft->level);
    if(w < 0.00000001 || std::isinf(w))
        return false;

    double Ix = dir(1) * dir(0);
    double Iy = Ix * grad(0) + dir(1) * dir(1) * grad(1);
    Ix *= grad(1);
    Ix += dir(0) * dir(0) * grad(0);
    jac(0, 0) =  cam->fx() /  Pj(2);
    jac(0, 1) = cam->fy() / Pj(2);
    jac(0, 2) = -jac(0, 0) * Pj(0) / Pj(2);
    jac(0, 3) = -jac(0, 1) * Pj(1) / Pj(2);
    jac(0, 0) *= Ix;
    jac(0, 1) *= Iy;
    jac(0, 2) = jac(0, 2) * Ix + jac(0, 3) * Iy;
    Eigen::Matrix<double, 3, 6> JacR;
    JacR.block<3, 3>(0, 0) = T_SB.so3().matrix();
    JacR.block<3, 3>(0, 3) = - JacR.block<3, 3>(0, 0) * Sophus::SO3d::hat(P);
    jac = jac.block<1, 3>(0, 0) * JacR;

    return true;
}


bool compute_PointJac(std::shared_ptr<viFrame> &viframe_i,std::shared_ptr<viFrame> &viframe_j,
                      std::shared_ptr<Feature> &ft,const Sophus::SE3d T_SB,
                      const Sophus::SE3d& Ti,  Sophus::SE3d& T_ji,
                      Jac_t &jac, double &w, double &err) {
    typedef Eigen::Vector3d Point3d;

    cvMeasure::cam_t camera = viframe_j->getCam();
    double fx = camera->fx(), fy = camera->fy(), cx = camera->cx(), cy = camera->cy();
    Point3d pointj = T_SB * T_ji * Ti * ft->point->pos_;
    if(pointj(2) < 0.0000001)
        return false;

    double X_ = pointj(0), Y_ = pointj(1), Z_ = pointj(2);
    double u = fx*X_/Z_ + cx;
    double v = fy*Y_/Z_ + cy;

    Point3d pointI =  Ti * ft->point->pos_;

    if(u < 0 || u >= viframe_j->getCVFrame()->getWidth() ||
       v < 0 || v >= viframe_j->getCVFrame()->getHeight())
        return false;

    w = 1.0 / viframe_j->getCVFrame()->getGradNorm(u, v, ft->level);
    if(w < 0.00000001 || std::isinf(w))
        return false;

    err = viframe_i->getCVFrame()->getIntensity(fx*pointI(0)/pointI(2)+cx, fy*pointI(1)/pointI(2)+cy)
         -viframe_j->getCVFrame()->getIntensity(u,v);
    err = err>0? err:-err;
    Eigen::Vector2d grad;viframe_j->getCVFrame()->getGrad(u,v,grad);
    double dI_du = grad(0) ,dI_dv = grad(1);

    double zInverse = 1.0/Z_,  fx_z = fx*zInverse, fy_z = fy*zInverse, fx_zz = fx_z*zInverse, fy_zz = fy_z*zInverse;
    Eigen::Matrix<double, 3, 6> JacR;
    JacR.block<3, 3>(0, 0) = T_SB.so3().matrix();
    JacR.block<3, 3>(0, 3) = - JacR.block<3, 3>(0, 0) * Sophus::SO3d::hat(pointj);
    Eigen::Matrix<double,2,3> a;
    a<<fx_z,0,-fx_zz*Z_,0,fy_z,-fy_zz*Y_;
    jac.block<1,6>(0,0) = Eigen::Matrix<double,1,2>(dI_du,dI_dv) *
            a * JacR;
}


Tracker::Tracker() {}
}

