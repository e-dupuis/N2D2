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

#ifndef N2D2_SWISHACTIVATION_FRAME_CUDA_H
#define N2D2_SWISHACTIVATION_FRAME_CUDA_H

#include "CudaContext.hpp"
#include "CudaUtils.hpp"
#include "Activation/Activation_CUDA_Kernels.hpp"
#include "Activation/SwishActivation.hpp"
#include "Cell/Cell.hpp"
#include "containers/CudaTensor.hpp"

namespace N2D2 {
template <class T>
class SwishActivation_Frame_CUDA : public SwishActivation {
public:
    static std::shared_ptr<SwishActivation> create()
    {
        return std::make_shared<SwishActivation_Frame_CUDA<T> >();
    }

    virtual void propagate(const Cell& cell,
                           const BaseTensor& input,
                           BaseTensor& output,
                           bool inference = false);
    virtual void backPropagate(const Cell& cell,
                               const BaseTensor& input,
                               const BaseTensor& output,
                               const BaseTensor& diffInput,
                               BaseTensor& diffOutput);
    virtual ~SwishActivation_Frame_CUDA() {};

protected:
    CudaTensor<T> mSigmoid;

private:
    static Registrar<SwishActivation > mRegistrar;
};
}

template <class T>
void N2D2::SwishActivation_Frame_CUDA<T>::propagate(
    const Cell& cell, 
    const BaseTensor& baseInput,
    BaseTensor& baseOutput,
    bool /*inference*/)
{
    const CudaTensor<T>& input = dynamic_cast<const CudaTensor<T>&>(baseInput);
    CudaTensor<T>& output = dynamic_cast<CudaTensor<T>&>(baseOutput);

    mScaling.propagate(cell, input, output);

    mSigmoid.resize(output.dims());
    cudaSwish_propagate(
        output.getDevicePtr(),
        output.getDevicePtr(),
        mSigmoid.getDevicePtr(),
        output.size());
}

template <class T>
void N2D2::SwishActivation_Frame_CUDA<T>::backPropagate(
    const Cell& cell, 
    const BaseTensor& /*baseInput*/,
    const BaseTensor& baseOutput,
    const BaseTensor& baseDiffInput,
    BaseTensor& baseDiffOutput)
{
    const CudaTensor<T>& output = dynamic_cast<const CudaTensor<T>&>(baseOutput);
    const CudaTensor<T>& diffInput = dynamic_cast<const CudaTensor<T>&>(baseDiffInput);
    CudaTensor<T>& diffOutput = dynamic_cast<CudaTensor<T>&>(baseDiffOutput);

    cudaSwish_backPropagate(
        output.getDevicePtr(),
        diffInput.getDevicePtr(),
        diffOutput.getDevicePtr(),
        mSigmoid.getDevicePtr(),
        output.size());
    
    mScaling.backPropagate(cell, diffOutput, diffOutput);
}

#endif // N2D2_SWISHACTIVATION_FRAME_CUDA_H
