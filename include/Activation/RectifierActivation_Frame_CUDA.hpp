/*
    (C) Copyright 2016 CEA LIST. All Rights Reserved.
    Contributor(s): Olivier BICHLER (olivier.bichler@cea.fr)

    This software is governed by the CeCILL-C license under French law and
    abiding by the rules of distribution of free software.  You can  use,
    modify and/ or redistribute the software under the terms of the CeCILL-C
    license as circulated by CEA, CNRS and INRIA at the following URL
    "http://www.cecill.info".

    As a counterpart to the access to the source code and  rights to copy,
    modify and redistribute granted by the license, users are provided only
    with a limited warranty  and the software's author,  the holder of the
    economic rights,  and the successive licensors  have only  limited
    liability.

    The fact that you are presently reading this means that you have had
    knowledge of the CeCILL-C license and that you accept its terms.
*/

#ifndef N2D2_RECTIFIERACTIVATION_FRAME_CUDA_H
#define N2D2_RECTIFIERACTIVATION_FRAME_CUDA_H

#include "CudaContext.hpp"
#include "CudaUtils.hpp"
#include "Activation/Activation_CUDA_Kernels.hpp"
#include "Activation/RectifierActivation.hpp"
#include "Cell/Cell.hpp"
#include "containers/CudaTensor.hpp"

namespace N2D2 {
template <class T>
class RectifierActivation_Frame_CUDA : public RectifierActivation {
public:
    static std::shared_ptr<RectifierActivation> create()
    {
        return std::make_shared<RectifierActivation_Frame_CUDA<T> >();
    }

    RectifierActivation_Frame_CUDA();

    virtual void propagate(const Cell& cell, BaseTensor& data, bool inference = false);
    virtual void backPropagate(const Cell& cell, BaseTensor& data, BaseTensor& diffData);

    virtual ~RectifierActivation_Frame_CUDA();

protected:
#if CUDNN_VERSION >= 5000
    cudnnActivationDescriptor_t mActivationDesc;
#else
    cudnnActivationMode_t mActivationDesc;
#endif

private:
    static Registrar<RectifierActivation > mRegistrar;
};
}

template <class T>
N2D2::RectifierActivation_Frame_CUDA<T>::RectifierActivation_Frame_CUDA():
    RectifierActivation()
{
#if CUDNN_VERSION >= 5000
    CHECK_CUDNN_STATUS(cudnnCreateActivationDescriptor(&mActivationDesc));
    CHECK_CUDNN_STATUS(cudnnSetActivationDescriptor(
        mActivationDesc, CUDNN_ACTIVATION_RELU, CUDNN_NOT_PROPAGATE_NAN, 0.0));
#else
    mActivationDesc = CUDNN_ACTIVATION_RELU;
#endif
}

template <class T>
void N2D2::RectifierActivation_Frame_CUDA<T>::propagate(const Cell& cell, 
                                                        BaseTensor& baseData, bool /*inference*/)
{
    CudaTensor<T>& data = dynamic_cast<CudaTensor<T>&>(baseData);

    mScaling.propagate(cell, data);

    if (mLeakSlope == 0.0 && mClipping == 0.0) {
        const float alpha = 1.0f;
        const float beta = 0.0f;

        CHECK_CUDNN_STATUS(
            cudnnActivationForward(CudaContext::cudnnHandle(),
                                   mActivationDesc,
                                   &alpha,
                                   data.getCudnnTensorDesc(),
                                   data.getDevicePtr(),
                                   &beta,
                                   data.getCudnnTensorDesc(),
                                   data.getDevicePtr()));
    }
    else {
        cudaRectifier_propagate(
            data.getDevicePtr(),
            data.getDevicePtr(),
            data.size(),
            T(mLeakSlope),
            cell.isQuantized()?T(0.0):T(mClipping));
    }
}

template <class T>
void N2D2::RectifierActivation_Frame_CUDA<T>::backPropagate(const Cell& cell, 
                                                            BaseTensor& baseData, BaseTensor& baseDiffData) 
{
    CudaTensor<T>& data = dynamic_cast<CudaTensor<T>&>(baseData);
    CudaTensor<T>& diffData = dynamic_cast<CudaTensor<T>&>(baseDiffData);

    if (mLeakSlope == 0.0 && mClipping == 0.0) {
        const float alpha = 1.0f;
        const float beta = 0.0f;

        CHECK_CUDNN_STATUS(
            cudnnActivationBackward(CudaContext::cudnnHandle(),
                                    mActivationDesc,
                                    &alpha,
                                    data.getCudnnTensorDesc(),
                                    data.getDevicePtr(),
                                    diffData.getCudnnTensorDesc(),
                                    diffData.getDevicePtr(),
                                    data.getCudnnTensorDesc(),
                                    data.getDevicePtr(),
                                    &beta,
                                    diffData.getCudnnTensorDesc(),
                                    diffData.getDevicePtr()));
    }
    else {
        cudaRectifier_backPropagate(data.getDevicePtr(),
                                     diffData.getDevicePtr(),
                                     data.size(),
                                     T(mLeakSlope),
                                     cell.isQuantized()?T(0.0):
                                                        T(mClipping));
    }
    
    mScaling.backPropagate(cell, data, diffData);
}

template <class T>
N2D2::RectifierActivation_Frame_CUDA<T>::~RectifierActivation_Frame_CUDA()
{
// dtor
#if CUDNN_VERSION >= 5000
    cudnnDestroyActivationDescriptor(mActivationDesc);
#endif
}

#endif // N2D2_RECTIFIERACTIVATION_FRAME_CUDA_H
