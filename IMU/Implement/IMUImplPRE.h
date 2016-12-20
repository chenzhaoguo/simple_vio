#ifndef IMUIMPLPRE_H
#define IMUIMPLPRE_H

#include "IMUImpl.h"

class IMUImplPRE : public IMUImpl
{
public:
    IMUImplPRE();
    int propagation(const ImuMeasureDeque & imuMeasurements,
                    const ImuParamenters & imuParams,
                    Transformation& T_WS,
                    SpeedAndBias & speedAndBiases,
                    double & t_start,
                    double & t_end,
                    covariance_t* covariance,
                    jacobian_t* jacobian);

    int error(const viFrame &frame_i, const viFrame &frame_j, Error_t &err, void *info);
    int Jacobian(const error_t& err, const viFrame& frame_i, jacobian_t& jacobian_i, const viFrame& frame_j, jacobian_t& jacobian_j);
};

#endif // IMUIMPLPRE_H
