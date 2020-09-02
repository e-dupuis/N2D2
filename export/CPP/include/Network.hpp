/*
    (C) Copyright 2019 CEA LIST. All Rights Reserved.
    Contributor(s): Olivier BICHLER (olivier.bichler@cea.fr)

    This file is not part of the open source version of N2D2 and is NOT under
    the CeCILL-C license. This code is the property of the CEA. It can not be
    copied or disseminated without its authorization.
*/

#ifndef N2D2_NETWORK_HPP
#define N2D2_NETWORK_HPP

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <iostream>
#include <chrono>

#include "typedefs.h"

#define N2D2_THROW_OR_ABORT(ex, msg) throw ex(msg)
#define N2D2_ALWAYS_INLINE __attribute__((always_inline))


namespace N2D2 {

class Network {
public:
    enum class Format {
        HWC,
        CHW
    };

    enum ElemWiseOp {
        Sum
    };

    typedef std::chrono::time_point<std::chrono::high_resolution_clock> Tick_T;
    typedef struct {
        double mean;
        unsigned long long int count;
    } RunningMean_T;

    template<typename Input_T>
    void propagate(const Input_T* inputs, std::int32_t* outputs) const;

    std::size_t inputHeight() const;
    std::size_t inputWidth() const;
    std::size_t inputNbChannels() const;
    std::size_t inputSize() const;

    std::size_t outputHeight() const;
    std::size_t outputWidth() const;
    std::size_t outputNbOutputs() const;
    std::size_t outputSize() const;

private:
    template<// For all inputs
            int NB_INPUTS,
            int CHANNELS_HEIGHT, int CHANNELS_WIDTH,
            int NB_OUTPUTS,
            int OUTPUTS_HEIGHT, int OUTPUTS_WIDTH,
            int OUTPUT_MEM_CONT_OFFSET,
            int OUTPUT_MEM_CONT_SIZE,
            int OUTPUT_MEM_WRAP_OFFSET,
            int OUTPUT_MEM_WRAP_SIZE,
            int OUTPUT_MEM_STRIDE,
            // For first input
            int INPUT_NB_CHANNELS,
            int INPUT_MEM_CONT_OFFSET,
            int INPUT_MEM_CONT_SIZE,
            int INPUT_MEM_WRAP_OFFSET,
            int INPUT_MEM_WRAP_SIZE,
            int INPUT_MEM_STRIDE,
            // For next inputs
            int... ARGS,
            typename... INPUTS,
            // Types
            typename Input_T, typename Output_T>
    N2D2_ALWAYS_INLINE void concatenatePropagate(
        Output_T* __restrict outputs,
        const Input_T* __restrict firstInputs,
        INPUTS... inputs);

    template<// For all inputs
            int NB_INPUTS,
            int CHANNELS_HEIGHT, int CHANNELS_WIDTH,
            int NB_OUTPUTS,
            int OUTPUTS_HEIGHT, int OUTPUTS_WIDTH,
            N2D2::Network::ElemWiseOp ELEM_OP,
            ActivationFunction_T ACTIVATION,
            int OUTPUT_MEM_CONT_OFFSET,
            int OUTPUT_MEM_CONT_SIZE,
            int OUTPUT_MEM_WRAP_OFFSET,
            int OUTPUT_MEM_WRAP_SIZE,
            int OUTPUT_MEM_STRIDE,
            // For first input
            int INPUT_NB_CHANNELS,
            int INPUT_MEM_CONT_OFFSET,
            int INPUT_MEM_CONT_SIZE,
            int INPUT_MEM_WRAP_OFFSET,
            int INPUT_MEM_WRAP_SIZE,
            int INPUT_MEM_STRIDE,
            // For next inputs
            int... ARGS,
            typename... INPUTS,
            // Types
            typename Input_T, typename Output_T,
            typename Rescaling_T>
    N2D2_ALWAYS_INLINE void elemWisePropagate(
        Output_T* __restrict outputs,
        const Input_T* __restrict firstInputs,
        INPUTS... inputs,
        const Rescaling_T& __restrict rescaling);

    /**
     * inputs[CHANNELS_HEIGHT*CHANNELS_WIDTH*NB_CHANNELS]
     * outputs[OUTPUTS_HEIGHT*OUTPUTS_WIDTH*NB_OUTPUTS]
     * biasses[NB_OUTPUTS]
     * weights[NB_OUTPUTS*KERNEL_HEIGHT*KERNEL_WIDTH*NB_CHANNELS]
     */
    template<int NB_CHANNELS, 
            int CHANNELS_HEIGHT, int CHANNELS_WIDTH,
            int NB_OUTPUTS,
            int OUTPUTS_HEIGHT, int OUTPUTS_WIDTH,
            int PADDING_Y, int PADDING_X,
            int STRIDE_Y, int STRIDE_X,
            int KERNEL_HEIGHT, int KERNEL_WIDTH,
            ActivationFunction_T ACTIVATION,
            // Memory mapping: inputs
            int INPUT_MEM_CONT_OFFSET,
            int INPUT_MEM_CONT_SIZE,
            int INPUT_MEM_WRAP_OFFSET,
            int INPUT_MEM_WRAP_SIZE,
            int INPUT_MEM_STRIDE,
            // Memory mapping: outputs
            int OUTPUT_MEM_CONT_OFFSET,
            int OUTPUT_MEM_CONT_SIZE,
            int OUTPUT_MEM_WRAP_OFFSET,
            int OUTPUT_MEM_WRAP_SIZE,
            int OUTPUT_MEM_STRIDE,
            typename Input_T, typename Output_T,
            typename Rescaling_T>
    N2D2_ALWAYS_INLINE void convcellPropagate(
        const Input_T* __restrict inputs,
        Output_T* __restrict outputs,
        const BDATA_T* __restrict biasses,
        const WDATA_T* __restrict weights,
        const Rescaling_T& __restrict rescaling) const;

    /*
     * inputs[CHANNELS_HEIGHT*CHANNELS_WIDTH*NB_CHANNELS]
     * outputs[OUTPUTS_HEIGHT*OUTPUTS_WIDTH*NB_OUTPUTS]
     * biasses[NB_OUTPUTS]
     * weights[NB_OUTPUTS*KERNEL_HEIGHT*KERNEL_WIDTH]
     */
    template<int NB_CHANNELS, 
            int CHANNELS_HEIGHT, int CHANNELS_WIDTH,
            int NB_OUTPUTS,
            int OUTPUTS_HEIGHT, int OUTPUTS_WIDTH,
            int PADDING_Y, int PADDING_X,
            int STRIDE_Y, int STRIDE_X,
            int KERNEL_HEIGHT, int KERNEL_WIDTH,
            ActivationFunction_T ACTIVATION,
            // Memory mapping: inputs
            int INPUT_MEM_CONT_OFFSET,
            int INPUT_MEM_CONT_SIZE,
            int INPUT_MEM_WRAP_OFFSET,
            int INPUT_MEM_WRAP_SIZE,
            int INPUT_MEM_STRIDE,
            // Memory mapping: outputs
            int OUTPUT_MEM_CONT_OFFSET,
            int OUTPUT_MEM_CONT_SIZE,
            int OUTPUT_MEM_WRAP_OFFSET,
            int OUTPUT_MEM_WRAP_SIZE,
            int OUTPUT_MEM_STRIDE,
            typename Input_T, typename Output_T,
            typename Rescaling_T>
    N2D2_ALWAYS_INLINE void convcellDWPropagate(
        const Input_T* __restrict inputs,
        Output_T* __restrict outputs,
        const BDATA_T* __restrict biasses,
        const WDATA_T* __restrict weights,
        const Rescaling_T& __restrict rescaling) const;

    /**
     * inputs[CHANNELS_HEIGHT*CHANNELS_WIDTH*NB_CHANNELS]
     * outputs[OUTPUTS_HEIGHT*OUTPUTS_WIDTH*NB_OUTPUTS]
     */
    template<int NB_CHANNELS, 
            int CHANNELS_HEIGHT, int CHANNELS_WIDTH,
            int NB_OUTPUTS,
            int OUTPUTS_HEIGHT, int OUTPUTS_WIDTH,
            int PADDING_Y, int PADDING_X,
            int STRIDE_Y, int STRIDE_X,
            int POOL_HEIGHT, int POOL_WIDTH,
            Pooling_T POOLING_TYPE,
            ActivationFunction_T ACTIVATION,
            // Memory mapping: inputs
            int INPUT_MEM_CONT_OFFSET,
            int INPUT_MEM_CONT_SIZE,
            int INPUT_MEM_WRAP_OFFSET,
            int INPUT_MEM_WRAP_SIZE,
            int INPUT_MEM_STRIDE,
            // Memory mapping: outputs
            int OUTPUT_MEM_CONT_OFFSET,
            int OUTPUT_MEM_CONT_SIZE,
            int OUTPUT_MEM_WRAP_OFFSET,
            int OUTPUT_MEM_WRAP_SIZE,
            int OUTPUT_MEM_STRIDE,
            typename Input_T, typename Output_T>
    N2D2_ALWAYS_INLINE void poolcellPropagate(
        const Input_T* __restrict inputs,
        Output_T* __restrict outputs) const;

    template<int NB_CHANNELS, 
            int CHANNELS_HEIGHT, int CHANNELS_WIDTH,
            int NB_OUTPUTS,
            int OUTPUTS_HEIGHT, int OUTPUTS_WIDTH,
            ActivationFunction_T ACTIVATION,
            // Memory mapping: inputs
            int INPUT_MEM_CONT_OFFSET,
            int INPUT_MEM_CONT_SIZE,
            int INPUT_MEM_WRAP_OFFSET,
            int INPUT_MEM_WRAP_SIZE,
            int INPUT_MEM_STRIDE,
            // Memory mapping: outputs
            int OUTPUT_MEM_CONT_OFFSET,
            int OUTPUT_MEM_CONT_SIZE,
            int OUTPUT_MEM_WRAP_OFFSET,
            int OUTPUT_MEM_WRAP_SIZE,
            int OUTPUT_MEM_STRIDE,
            typename Input_T, typename Output_T,
            typename Rescaling_T>
    N2D2_ALWAYS_INLINE void fccellPropagate(
        const Input_T* __restrict inputs,
        Output_T* __restrict outputs,
        const BDATA_T* __restrict biasses,
        const WDATA_T* __restrict weights,
        const Rescaling_T& __restrict rescaling) const;

    template<int NB_CHANNELS, 
            int CHANNELS_HEIGHT, int CHANNELS_WIDTH,
            int NB_OUTPUTS,
            int OUTPUTS_HEIGHT, int OUTPUTS_WIDTH,
            // Memory mapping: inputs
            int INPUT_MEM_CONT_OFFSET,
            int INPUT_MEM_CONT_SIZE,
            int INPUT_MEM_WRAP_OFFSET,
            int INPUT_MEM_WRAP_SIZE,
            int INPUT_MEM_STRIDE,
            // Memory mapping: outputs
            int OUTPUT_MEM_CONT_OFFSET,
            int OUTPUT_MEM_CONT_SIZE,
            int OUTPUT_MEM_WRAP_OFFSET,
            int OUTPUT_MEM_WRAP_SIZE,
            int OUTPUT_MEM_STRIDE,
            typename Input_T, typename Output_T>
    N2D2_ALWAYS_INLINE void resizeNearestNeighborPropagate(
        const Input_T* __restrict inputs,
        Output_T* __restrict outputs) const;

    template<int NB_CHANNELS, 
            int CHANNELS_HEIGHT, int CHANNELS_WIDTH,
            int NB_OUTPUTS,
            int OUTPUTS_HEIGHT, int OUTPUTS_WIDTH,
            // Memory mapping: inputs
            int INPUT_MEM_CONT_OFFSET,
            int INPUT_MEM_CONT_SIZE,
            int INPUT_MEM_WRAP_OFFSET,
            int INPUT_MEM_WRAP_SIZE,
            int INPUT_MEM_STRIDE,
            // Memory mapping: outputs
            int OUTPUT_MEM_CONT_OFFSET,
            int OUTPUT_MEM_CONT_SIZE,
            int OUTPUT_MEM_WRAP_OFFSET,
            int OUTPUT_MEM_WRAP_SIZE,
            int OUTPUT_MEM_STRIDE,
            typename Input_T, typename Output_T,
            typename Rescaling_T>
    N2D2_ALWAYS_INLINE void scalingPropagate(
        const Input_T* __restrict inputs,
        Output_T* __restrict outputs,
        const Rescaling_T& __restrict rescaling) const;

    template<int NB_CHANNELS,
            int INPUTS_HEIGHT, int INPUTS_WIDTH,
            // Memory mapping: outputs
            int INPUT_MEM_CONT_OFFSET,
            int INPUT_MEM_CONT_SIZE,
            int INPUT_MEM_WRAP_OFFSET,
            int INPUT_MEM_WRAP_SIZE,
            int INPUT_MEM_STRIDE,
            typename Input_T>
    N2D2_ALWAYS_INLINE void maxPropagate(
        const Input_T* __restrict inputs,
        int32_t* __restrict outputs) const;

    template<typename Output_T>
    void saveOutputs(
        int NB_OUTPUTS,
        int OUTPUTS_HEIGHT, int OUTPUTS_WIDTH,
        int OUTPUT_MEM_CONT_OFFSET,
        int OUTPUT_MEM_CONT_SIZE,
        int OUTPUT_MEM_WRAP_OFFSET,
        int OUTPUT_MEM_WRAP_SIZE,
        int OUTPUT_MEM_STRIDE,
        const Output_T* __restrict outputs,
        std::ostream& ostream,
        Format format) const;

private:
    template<typename Output_T>
    N2D2_ALWAYS_INLINE void concatenate(
        Output_T* __restrict /*outputs*/,
        int /*pos*/);

    template<// For first input
            int INPUT_NB_CHANNELS,
            int INPUT_MEM_CONT_OFFSET,
            int INPUT_MEM_CONT_SIZE,
            int INPUT_MEM_WRAP_OFFSET,
            int INPUT_MEM_WRAP_SIZE,
            int INPUT_MEM_STRIDE,
            // For next inputs
            int... ARGS,
            typename... INPUTS,
            // Types
            typename Input_T, typename Output_T>
    N2D2_ALWAYS_INLINE void concatenate(
        Output_T* __restrict outputs,
        int pos,
        const Input_T* __restrict firstInputs,
        INPUTS... inputs);

    template<typename Output_T>
    N2D2_ALWAYS_INLINE void elemWise(
        int /*pos*/,
        int /*ch*/);

    template<ElemWiseOp ELEM_OP,
            // For first input
            int INPUT_NB_CHANNELS,
            int INPUT_MEM_CONT_OFFSET,
            int INPUT_MEM_CONT_SIZE,
            int INPUT_MEM_WRAP_OFFSET,
            int INPUT_MEM_WRAP_SIZE,
            int INPUT_MEM_STRIDE,
            // For next inputs
            int... ARGS,
            typename... INPUTS,
            // Types
            typename Input_T, typename Output_T>
    N2D2_ALWAYS_INLINE void elemWise(
        int pos,
        int ch,
        const Input_T* __restrict firstInputs,
        INPUTS... inputs);

    template<typename T>
    N2D2_ALWAYS_INLINE static T clamp(T v, T lo, T hi) {
        if(v < lo) {
            return lo;
        }
        else if(v > hi) {
            return hi;
        }
        else {
            return v;
        }
    }
    
    template<typename T>
    N2D2_ALWAYS_INLINE static T max(T lhs, T rhs) {
        return (lhs >= rhs)?lhs:rhs;
    }

    template<typename Output_T, typename Rescaling_T>
    N2D2_ALWAYS_INLINE static Output_T sat(SUM_T weightedSum, int output, 
                                           ActivationFunction_T func, 
                                           const Rescaling_T& __restrict rescaling) 
    {
        switch(func) {
            case Linear:
            case Saturation: {
                break;
            }
            case Rectifier: {
                if(weightedSum <= 0) weightedSum = 0;
                break;
            }
            default:
                N2D2_THROW_OR_ABORT(std::runtime_error, "Unsupported activation function.");
        }

        return saturate<Output_T>(rescaling(weightedSum, output), NB_BITS);
    }

    template<typename Input_T>
    N2D2_ALWAYS_INLINE static SUM_T dualMac(const Input_T* __restrict inputs, 
                                            const WDATA_T* __restrict weights, 
                                            SUM_T weightedSum,
                                            int inputsIncrement = 1,
                                            int weightsIncrement = 1) 
    {
        weightedSum += inputs[0] * weights[0];
        weightedSum += inputs[inputsIncrement] * weights[weightsIncrement];

        return weightedSum;
    }

    template<typename Input_T, 
             typename std::enable_if<std::is_floating_point<Input_T>::value>::type* = nullptr>
    N2D2_ALWAYS_INLINE static SUM_T quadMac(const Input_T* __restrict inputs, 
                                            const WDATA_T* __restrict weights, 
                                            SUM_T weightedSum,
                                            int inputsIncrement = 1,
                                            int weightsIncrement = 1) 
    {
        weightedSum += inputs[0*inputsIncrement] * weights[0*weightsIncrement];
        weightedSum += inputs[1*inputsIncrement] * weights[1*weightsIncrement];
        weightedSum += inputs[2*inputsIncrement] * weights[2*weightsIncrement];
        weightedSum += inputs[3*inputsIncrement] * weights[3*weightsIncrement];

        return weightedSum;
    }
#if NB_BITS > 0
    template<typename Input_T, 
             typename std::enable_if<!std::is_floating_point<Input_T>::value>::type* = nullptr>
    N2D2_ALWAYS_INLINE static SUM_T quadMac(const Input_T* __restrict inputs, 
                                            const WDATA_T* __restrict weights, 
                                            SUM_T weightedSum,
                                            int inputsIncrement = 1,
                                            int weightsIncrement = 1) 
    {
        weightedSum += inputs[0*inputsIncrement] * weights[0*weightsIncrement];
        weightedSum += inputs[1*inputsIncrement] * weights[1*weightsIncrement];
        weightedSum += inputs[2*inputsIncrement] * weights[2*weightsIncrement];
        weightedSum += inputs[3*inputsIncrement] * weights[3*weightsIncrement];

        return weightedSum;
    }
#endif
    template<typename Output_T, typename T,  
             typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr>
    N2D2_ALWAYS_INLINE static Output_T saturate(T value, std::int32_t sat) {
        return value;
    }

    template<typename Output_T, typename T,  
             typename std::enable_if<!std::is_floating_point<T>::value>::type* = nullptr>
    N2D2_ALWAYS_INLINE static Output_T saturate(T value, std::uint32_t sat) {
        return std::is_unsigned<Output_T>::value?clamp(value, T(0), (T(1) << sat) - 1):
                                                 clamp(value, -(T(1) << (sat - 1)), 
                                                               (T(1) << (sat - 1)) - 1);
    }

    /**
     * Multiply-accumulate the values in inputs and weights for NB_ITERATIONS.
     * Using an explicit unrolling as unrolling a for-loop with '#pragma GCC unroll NB_ITERATIONS'
     * is slower (TODO check why).
     */
    template<int NB_ITERATIONS, class Input_T, 
             typename std::enable_if<(NB_ITERATIONS == 0)>::type* = nullptr>
    N2D2_ALWAYS_INLINE static void macsOnRange(const Input_T* __restrict /*inputs*/, 
                                               const WDATA_T* __restrict /*weights*/, 
                                               SUM_T& __restrict /*weightedSum*/,
                                               int /*inputsIncrement*/ = 1, 
                                               int /*weightsIncrement*/ = 1) 
    {
    }

    template<int NB_ITERATIONS, class Input_T, 
             typename std::enable_if<(NB_ITERATIONS == 1)>::type* = nullptr>
    N2D2_ALWAYS_INLINE static void macsOnRange(const Input_T* __restrict inputs, 
                                               const WDATA_T* __restrict weights, 
                                               SUM_T& __restrict weightedSum,
                                               int /*inputsIncrement*/ = 1, 
                                               int /*weightsIncrement*/ = 1) 
    {
        weightedSum += (*weights) * (*inputs);
    }

    template<int NB_ITERATIONS, class Input_T, 
             typename std::enable_if<(NB_ITERATIONS >= 2 && NB_ITERATIONS < 4)>::type* = nullptr>
    N2D2_ALWAYS_INLINE static void macsOnRange(const Input_T* __restrict inputs, 
                                               const WDATA_T* __restrict weights, 
                                               SUM_T& __restrict weightedSum,
                                               int inputsIncrement = 1, 
                                               int weightsIncrement = 1) 
    {
        weightedSum = dualMac(inputs, weights, weightedSum, inputsIncrement, weightsIncrement);
        macsOnRange<NB_ITERATIONS - 2>(inputs + 2*inputsIncrement, 
                                       weights + 2*weightsIncrement, weightedSum);
    }

    template<int NB_ITERATIONS, class Input_T, 
             typename std::enable_if<(NB_ITERATIONS >= 4)>::type* = nullptr>
    N2D2_ALWAYS_INLINE static void macsOnRange(const Input_T* __restrict inputs, 
                                               const WDATA_T* __restrict weights, 
                                               SUM_T& __restrict weightedSum,
                                               int inputsIncrement = 1, 
                                               int weightsIncrement = 1) 
    {
        weightedSum = quadMac(inputs, weights, weightedSum, inputsIncrement, weightsIncrement);
        macsOnRange<NB_ITERATIONS - 4>(inputs + 4*inputsIncrement, 
                                       weights + 4*weightsIncrement, weightedSum);
    }

    N2D2_ALWAYS_INLINE inline Tick_T tick();
    N2D2_ALWAYS_INLINE inline void benchmark(const char* name,
                                             const Tick_T& start,
                                             const Tick_T& end,
                                             RunningMean_T& timing);
};
}

template<typename Output_T>
N2D2_ALWAYS_INLINE inline void N2D2::Network::concatenate(
    Output_T* __restrict /*outputs*/,
    int /*pos*/) {}

template<// For first input
         int INPUT_NB_CHANNELS,
         int INPUT_MEM_CONT_OFFSET,
         int INPUT_MEM_CONT_SIZE,
         int INPUT_MEM_WRAP_OFFSET,
         int INPUT_MEM_WRAP_SIZE,
         int INPUT_MEM_STRIDE,
         // For next inputs
         int... ARGS,
         typename... INPUTS,
         // Types
         typename Input_T, typename Output_T>
N2D2_ALWAYS_INLINE inline void N2D2::Network::concatenate(
    Output_T* __restrict outputs,
    int pos,
    const Input_T* __restrict firstInputs,
    INPUTS... inputs)
{
    int iOffset = INPUT_MEM_STRIDE * pos;

    if (INPUT_MEM_WRAP_SIZE > 0 && iOffset >= INPUT_MEM_CONT_SIZE) {
        iOffset += INPUT_MEM_WRAP_OFFSET - INPUT_MEM_CONT_OFFSET
                    - INPUT_MEM_CONT_SIZE;
    }

    for (int ch = 0; ch < INPUT_NB_CHANNELS; ++ch)
        outputs[ch] = firstInputs[iOffset + ch];

    concatenate<ARGS...>(outputs + INPUT_NB_CHANNELS, pos, inputs...);
}

template<// For all inputs
         int NB_INPUTS,
         int CHANNELS_HEIGHT, int CHANNELS_WIDTH,
         int NB_OUTPUTS,
         int OUTPUTS_HEIGHT, int OUTPUTS_WIDTH,
         int OUTPUT_MEM_CONT_OFFSET,
         int OUTPUT_MEM_CONT_SIZE,
         int OUTPUT_MEM_WRAP_OFFSET,
         int OUTPUT_MEM_WRAP_SIZE,
         int OUTPUT_MEM_STRIDE,
         // For first input
         int INPUT_NB_CHANNELS,
         int INPUT_MEM_CONT_OFFSET,
         int INPUT_MEM_CONT_SIZE,
         int INPUT_MEM_WRAP_OFFSET,
         int INPUT_MEM_WRAP_SIZE,
         int INPUT_MEM_STRIDE,
         // For next inputs
         int... ARGS,
         typename... INPUTS,
         // Types
         typename Input_T, typename Output_T>
N2D2_ALWAYS_INLINE inline void N2D2::Network::concatenatePropagate(
    Output_T* __restrict outputs,
    const Input_T* __restrict firstInputs,
    INPUTS... inputs)
{
    for (int oy = 0; oy < OUTPUTS_HEIGHT; oy++) {
        for (int ox = 0; ox < OUTPUTS_WIDTH; ox++) {
            const int pos = (ox + OUTPUTS_WIDTH * oy);
            int oOffset = OUTPUT_MEM_STRIDE * pos;

            if (OUTPUT_MEM_WRAP_SIZE > 0 && oOffset >= OUTPUT_MEM_CONT_SIZE) {
                oOffset += OUTPUT_MEM_WRAP_OFFSET - OUTPUT_MEM_CONT_OFFSET
                            - OUTPUT_MEM_CONT_SIZE;
            }

            concatenate<INPUT_NB_CHANNELS,
                        INPUT_MEM_CONT_OFFSET,
                        INPUT_MEM_CONT_SIZE,
                        INPUT_MEM_WRAP_OFFSET,
                        INPUT_MEM_WRAP_SIZE,
                        INPUT_MEM_STRIDE,
                        ARGS...>(outputs + oOffset, pos, firstInputs, inputs...);
        }
    }
}

template<typename Output_T>
N2D2_ALWAYS_INLINE inline void N2D2::Network::elemWise(
    int /*pos*/,
    int /*ch*/) {}

template<N2D2::Network::ElemWiseOp ELEM_OP,
         // For first input
         int INPUT_NB_CHANNELS,
         int INPUT_MEM_CONT_OFFSET,
         int INPUT_MEM_CONT_SIZE,
         int INPUT_MEM_WRAP_OFFSET,
         int INPUT_MEM_WRAP_SIZE,
         int INPUT_MEM_STRIDE,
         // For next inputs
         int... ARGS,
         typename... INPUTS,
         // Types
         typename Input_T, typename Output_T>
N2D2_ALWAYS_INLINE inline void N2D2::Network::elemWise(
    int pos,
    int ch,
    const Input_T* __restrict firstInputs,
    INPUTS... inputs)
{
    int iOffset = INPUT_MEM_STRIDE * pos;

    if (INPUT_MEM_WRAP_SIZE > 0 && iOffset >= INPUT_MEM_CONT_SIZE) {
        iOffset += INPUT_MEM_WRAP_OFFSET - INPUT_MEM_CONT_OFFSET
                    - INPUT_MEM_CONT_SIZE;
    }

    return firstInputs[iOffset + ch]
                + elemWise<ARGS...>(pos, ch, inputs...);
}

template<// For all inputs
         int NB_INPUTS,
         int CHANNELS_HEIGHT, int CHANNELS_WIDTH,
         int NB_OUTPUTS,
         int OUTPUTS_HEIGHT, int OUTPUTS_WIDTH,
         N2D2::Network::ElemWiseOp ELEM_OP,
         ActivationFunction_T ACTIVATION,
         int OUTPUT_MEM_CONT_OFFSET,
         int OUTPUT_MEM_CONT_SIZE,
         int OUTPUT_MEM_WRAP_OFFSET,
         int OUTPUT_MEM_WRAP_SIZE,
         int OUTPUT_MEM_STRIDE,
         // For first input
         int INPUT_NB_CHANNELS,
         int INPUT_MEM_CONT_OFFSET,
         int INPUT_MEM_CONT_SIZE,
         int INPUT_MEM_WRAP_OFFSET,
         int INPUT_MEM_WRAP_SIZE,
         int INPUT_MEM_STRIDE,
         // For next inputs
         int... ARGS,
         typename... INPUTS,
         // Types
         typename Input_T, typename Output_T,
         typename Rescaling_T>
N2D2_ALWAYS_INLINE inline void N2D2::Network::elemWisePropagate(
    Output_T* __restrict outputs,
    const Input_T* __restrict firstInputs,
    INPUTS... inputs,
    const Rescaling_T& __restrict rescaling)
{
    static_assert(NB_INPUTS > 0, "Number of inputs must be > 0");
    static_assert(ELEM_OP == Sum, "Only Sum is supported");
    static_assert(INPUT_NB_CHANNELS == NB_OUTPUTS,
        "Number of channels and number of outputs must match");

    for (int oy = 0; oy < OUTPUTS_HEIGHT; oy++) {
        for (int ox = 0; ox < OUTPUTS_WIDTH; ox++) {
            const int pos = (ox + OUTPUTS_WIDTH * oy);
            int oOffset = OUTPUT_MEM_STRIDE * pos;

            if (OUTPUT_MEM_WRAP_SIZE > 0 && oOffset >= OUTPUT_MEM_CONT_SIZE) {
                oOffset += OUTPUT_MEM_WRAP_OFFSET - OUTPUT_MEM_CONT_OFFSET
                            - OUTPUT_MEM_CONT_SIZE;
            }

            for (int ch = 0; ch < NB_OUTPUTS; ++ch) {
                const SUM_T val = elemWise<ELEM_OP,
                                        INPUT_NB_CHANNELS,
                                        INPUT_MEM_CONT_OFFSET,
                                        INPUT_MEM_CONT_SIZE,
                                        INPUT_MEM_WRAP_OFFSET,
                                        INPUT_MEM_WRAP_SIZE,
                                        INPUT_MEM_STRIDE,
                                        ARGS...>(pos, ch, firstInputs, inputs...);

                outputs[oOffset + ch]
                    = sat<Output_T>(val, ch, ACTIVATION, rescaling);
            }
        }
    }
}

template<int NB_CHANNELS, 
         int CHANNELS_HEIGHT, int CHANNELS_WIDTH,
         int NB_OUTPUTS,
         int OUTPUTS_HEIGHT, int OUTPUTS_WIDTH,
         int PADDING_Y, int PADDING_X,
         int STRIDE_Y, int STRIDE_X,
         int KERNEL_HEIGHT, int KERNEL_WIDTH,
         ActivationFunction_T ACTIVATION,
         // Memory mapping: inputs
         int INPUT_MEM_CONT_OFFSET,
         int INPUT_MEM_CONT_SIZE,
         int INPUT_MEM_WRAP_OFFSET,
         int INPUT_MEM_WRAP_SIZE,
         int INPUT_MEM_STRIDE,
         // Memory mapping: outputs
         int OUTPUT_MEM_CONT_OFFSET,
         int OUTPUT_MEM_CONT_SIZE,
         int OUTPUT_MEM_WRAP_OFFSET,
         int OUTPUT_MEM_WRAP_SIZE,
         int OUTPUT_MEM_STRIDE,
         typename Input_T, typename Output_T,
         typename Rescaling_T>
N2D2_ALWAYS_INLINE inline void N2D2::Network::convcellPropagate(
    const Input_T* __restrict inputs,
    Output_T* __restrict outputs,
    const BDATA_T* __restrict biasses,
    const WDATA_T* __restrict weights,
    const Rescaling_T& __restrict rescaling) const
{
    for (int oy = 0; oy < OUTPUTS_HEIGHT; ++oy) {
        const int syMin = (PADDING_Y == 0) ? 0
            : max(PADDING_Y - (oy * STRIDE_Y), 0);
        const int syMax = (PADDING_Y == 0) ? KERNEL_HEIGHT
            : clamp(CHANNELS_HEIGHT + PADDING_Y - (oy * STRIDE_Y), 
                    0, KERNEL_HEIGHT);
        const int iy = (oy * STRIDE_Y) - PADDING_Y;

        for (int ox = 0; ox < OUTPUTS_WIDTH; ++ox) {
            const int sxMin = (PADDING_X == 0) ? 0
                : max(PADDING_X - (ox * STRIDE_X), 0);
            const int sxMax = (PADDING_X == 0) ? KERNEL_WIDTH
                : clamp(CHANNELS_WIDTH + PADDING_X - (ox * STRIDE_X), 
                        0, KERNEL_WIDTH);
            const int ix = (ox * STRIDE_X) - PADDING_X;

            const int oPos = (ox + OUTPUTS_WIDTH * oy);
            int oOffset = OUTPUT_MEM_STRIDE * oPos;

            if (OUTPUT_MEM_WRAP_SIZE > 0 && oOffset >= OUTPUT_MEM_CONT_SIZE) {
                oOffset += OUTPUT_MEM_WRAP_OFFSET - OUTPUT_MEM_CONT_OFFSET
                            - OUTPUT_MEM_CONT_SIZE;
            }

            for (int output = 0; output < NB_OUTPUTS; ++output) {
                SUM_T weightedSum = biasses[output];

                for (int sy = 0; sy < KERNEL_HEIGHT; ++sy) {
                    if (PADDING_Y != 0 && sy >= syMax - syMin)
                        break;

                    const int iPos = ((sxMin + ix)
                                        + CHANNELS_WIDTH * (iy + syMin + sy));
                    int iOffset = INPUT_MEM_STRIDE * iPos;

                    if (INPUT_MEM_WRAP_SIZE > 0
                        && iOffset >= INPUT_MEM_CONT_SIZE)
                    {
                        iOffset += INPUT_MEM_WRAP_OFFSET - INPUT_MEM_CONT_OFFSET
                                    - INPUT_MEM_CONT_SIZE;
                    }

                    const int wOffset = NB_CHANNELS * (sxMin
                        + KERNEL_WIDTH * (syMin + sy + KERNEL_HEIGHT * output));

                    if (NB_CHANNELS == INPUT_MEM_STRIDE
                        && (PADDING_X == 0 || sxMax - sxMin == KERNEL_WIDTH))
                    {
                        macsOnRange<KERNEL_WIDTH * NB_CHANNELS>(
                            inputs + iOffset, 
                            weights + wOffset, 
                            weightedSum);
                    }
                    else {
                        for (int sx = 0; sx < KERNEL_WIDTH; ++sx) {
                            if (PADDING_X != 0 && sx >= sxMax - sxMin)
                                break;

                            macsOnRange<NB_CHANNELS>(
                                // same input line so no wrapping can occur
                                inputs + iOffset + sx * INPUT_MEM_STRIDE, 
                                weights + wOffset + sx * NB_CHANNELS, 
                                weightedSum);
                        }
                    }
                }

                outputs[oOffset + output]
                    = sat<Output_T>(weightedSum, output, ACTIVATION, rescaling);
            }
        }
    }
}

template<int NB_CHANNELS, 
         int CHANNELS_HEIGHT, int CHANNELS_WIDTH,
         int NB_OUTPUTS,
         int OUTPUTS_HEIGHT, int OUTPUTS_WIDTH,
         int PADDING_Y, int PADDING_X,
         int STRIDE_Y, int STRIDE_X,
         int KERNEL_HEIGHT, int KERNEL_WIDTH,
         ActivationFunction_T ACTIVATION,
         // Memory mapping: inputs
         int INPUT_MEM_CONT_OFFSET,
         int INPUT_MEM_CONT_SIZE,
         int INPUT_MEM_WRAP_OFFSET,
         int INPUT_MEM_WRAP_SIZE,
         int INPUT_MEM_STRIDE,
         // Memory mapping: outputs
         int OUTPUT_MEM_CONT_OFFSET,
         int OUTPUT_MEM_CONT_SIZE,
         int OUTPUT_MEM_WRAP_OFFSET,
         int OUTPUT_MEM_WRAP_SIZE,
         int OUTPUT_MEM_STRIDE,
         typename Input_T, typename Output_T,
         typename Rescaling_T>
N2D2_ALWAYS_INLINE inline void N2D2::Network::convcellDWPropagate(
    const Input_T* __restrict inputs,
    Output_T* __restrict outputs,
    const BDATA_T* __restrict biasses,
    const WDATA_T* __restrict weights,
    const Rescaling_T& __restrict rescaling) const
{
    static_assert(NB_CHANNELS == NB_OUTPUTS,
        "NB_CHANNELS should be equal to NB_OUTPUTS.");

    for (int oy = 0; oy < OUTPUTS_HEIGHT; ++oy) {
        const int syMin = (PADDING_Y == 0) ? 0
            : max(PADDING_Y - (oy * STRIDE_Y), 0);
        const int syMax = (PADDING_Y == 0) ? KERNEL_HEIGHT
            : clamp(CHANNELS_HEIGHT + PADDING_Y - (oy * STRIDE_Y), 
                    0, KERNEL_HEIGHT);
        const int iy = (oy * STRIDE_Y) - PADDING_Y;

        for (int ox = 0; ox < OUTPUTS_WIDTH; ++ox) {
            const int sxMin = (PADDING_X == 0) ? 0
                : max(PADDING_X - (ox * STRIDE_X), 0);
            const int sxMax = (PADDING_X == 0) ? KERNEL_WIDTH
                : clamp(CHANNELS_WIDTH + PADDING_X - (ox * STRIDE_X), 
                        0, KERNEL_WIDTH);
            const int ix = (ox * STRIDE_X) - PADDING_X;

            const int oPos = (ox + OUTPUTS_WIDTH * oy);
            int oOffset = OUTPUT_MEM_STRIDE * oPos;

            if (OUTPUT_MEM_WRAP_SIZE > 0 && oOffset >= OUTPUT_MEM_CONT_SIZE) {
                oOffset += OUTPUT_MEM_WRAP_OFFSET - OUTPUT_MEM_CONT_OFFSET
                            - OUTPUT_MEM_CONT_SIZE;
            }

            for (int output = 0; output < NB_OUTPUTS; ++output) {
                SUM_T weightedSum = biasses[output];

                for (int sy = 0; sy < KERNEL_HEIGHT; ++sy) {
                    if (PADDING_Y != 0 && sy >= syMax - syMin)
                        break;

                    const int iPos = ((sxMin + ix)
                                        + CHANNELS_WIDTH * (iy + syMin + sy));
                    int iOffset = INPUT_MEM_STRIDE * iPos;

                    if (INPUT_MEM_WRAP_SIZE > 0
                        && iOffset >= INPUT_MEM_CONT_SIZE)
                    {
                        iOffset += INPUT_MEM_WRAP_OFFSET - INPUT_MEM_CONT_OFFSET
                                    - INPUT_MEM_CONT_SIZE;
                    }

                    const int wOffset = (sxMin
                        + KERNEL_WIDTH * (syMin + sy + KERNEL_HEIGHT * output));

                    if (PADDING_X == 0 || sxMax - sxMin == KERNEL_WIDTH) {
                        macsOnRange<KERNEL_WIDTH>(
                            inputs + iOffset + output, 
                            weights + wOffset, 
                            weightedSum, INPUT_MEM_STRIDE, 1);
                    }
                    else {
                        for (int sx = 0; sx < KERNEL_WIDTH; ++sx) {
                            if (PADDING_X != 0 && sx >= sxMax - sxMin)
                                break;

                            weightedSum += inputs[iOffset + output
                                                    + sx * INPUT_MEM_STRIDE]
                                * weights[wOffset + sx];
                        }
                    }
                }

                outputs[oOffset + output]
                    = sat<Output_T>(weightedSum, output, ACTIVATION, rescaling);
            }
        }
    }
}

template<int NB_CHANNELS, 
         int CHANNELS_HEIGHT, int CHANNELS_WIDTH,
         int NB_OUTPUTS,
         int OUTPUTS_HEIGHT, int OUTPUTS_WIDTH,
         int PADDING_Y, int PADDING_X,
         int STRIDE_Y, int STRIDE_X,
         int POOL_HEIGHT, int POOL_WIDTH,
         Pooling_T POOLING_TYPE,
         ActivationFunction_T ACTIVATION,
         // Memory mapping: inputs
         int INPUT_MEM_CONT_OFFSET,
         int INPUT_MEM_CONT_SIZE,
         int INPUT_MEM_WRAP_OFFSET,
         int INPUT_MEM_WRAP_SIZE,
         int INPUT_MEM_STRIDE,
         // Memory mapping: outputs
         int OUTPUT_MEM_CONT_OFFSET,
         int OUTPUT_MEM_CONT_SIZE,
         int OUTPUT_MEM_WRAP_OFFSET,
         int OUTPUT_MEM_WRAP_SIZE,
         int OUTPUT_MEM_STRIDE,
         typename Input_T, typename Output_T>
N2D2_ALWAYS_INLINE inline void N2D2::Network::poolcellPropagate(
    const Input_T* __restrict inputs,
    Output_T* __restrict outputs) const
{
    static_assert(std::is_same<Input_T, Output_T>::value,
        "Input_T and Output_T must be the same.");
    static_assert(NB_CHANNELS == NB_OUTPUTS,
        "NB_CHANNELS should be equal to NB_OUTPUTS.");
    static_assert(POOLING_TYPE == Max || POOLING_TYPE == Average,
        "The export only supports Max and Average pooling.");
    static_assert(ACTIVATION == Linear,
        "The export only supports a Linear activation.");

    for (int oy = 0; oy < OUTPUTS_HEIGHT; ++oy) {
        const int syMin = (PADDING_Y == 0) ? 0
            : max(PADDING_Y - (oy * STRIDE_Y), 0);
        const int syMax = (PADDING_Y == 0) ? POOL_HEIGHT
            : clamp(CHANNELS_HEIGHT + PADDING_Y - (oy * STRIDE_Y), 
                    0, POOL_HEIGHT);
        const int iy = (oy * STRIDE_Y) - PADDING_Y;

        for (int ox = 0; ox < OUTPUTS_WIDTH; ++ox) {
            const int sxMin = (PADDING_X == 0) ? 0
                : max(PADDING_X - (ox * STRIDE_X), 0);
            const int sxMax = (PADDING_X == 0) ? POOL_WIDTH
                : clamp(CHANNELS_WIDTH + PADDING_X - (ox * STRIDE_X), 
                        0, POOL_WIDTH);
            const int ix = (ox * STRIDE_X) - PADDING_X;

            const int oPos = (ox + OUTPUTS_WIDTH * oy);
            int oOffset = OUTPUT_MEM_STRIDE * oPos;

            if (OUTPUT_MEM_WRAP_SIZE > 0 && oOffset >= OUTPUT_MEM_CONT_SIZE) {
                oOffset += OUTPUT_MEM_WRAP_OFFSET - OUTPUT_MEM_CONT_OFFSET
                            - OUTPUT_MEM_CONT_SIZE;
            }

            for (int output = 0; output < NB_OUTPUTS; ++output) {
                if (POOLING_TYPE == Max) {
                    Input_T maxVal = std::numeric_limits<Input_T>::lowest();

                    for (int sy = 0; sy < POOL_HEIGHT; ++sy) {
                        if (PADDING_Y != 0 && sy >= syMax - syMin)
                            break;

                        const int iPos = ((sxMin + ix)
                                            + CHANNELS_WIDTH * (iy + syMin + sy));
                        int iOffset = INPUT_MEM_STRIDE * iPos;

                        if (INPUT_MEM_WRAP_SIZE > 0
                            && iOffset >= INPUT_MEM_CONT_SIZE)
                        {
                            iOffset += INPUT_MEM_WRAP_OFFSET
                                - INPUT_MEM_CONT_OFFSET - INPUT_MEM_CONT_SIZE;
                        }

                        for (int sx = 0; sx < POOL_WIDTH; ++sx) {
                            if (PADDING_X != 0 && sx >= sxMax - sxMin)
                                break;

                            if (inputs[iOffset + output + sx * INPUT_MEM_STRIDE]
                                > maxVal)
                            {
                                maxVal = inputs[iOffset + output
                                            + sx * INPUT_MEM_STRIDE];
                            }
                        }
                    }

                    outputs[oOffset + output] = maxVal;
                }
                else if (POOLING_TYPE == Average) {
                    SUM_T sum = 0;

                    for (int sy = 0; sy < POOL_HEIGHT; ++sy) {
                        if (PADDING_Y != 0 && sy >= syMax - syMin)
                            break;

                        const int iPos = ((sxMin + ix)
                                            + CHANNELS_WIDTH * (iy + syMin + sy));
                        int iOffset = INPUT_MEM_STRIDE * iPos;

                        if (INPUT_MEM_WRAP_SIZE > 0
                            && iOffset >= INPUT_MEM_CONT_SIZE)
                        {
                            iOffset += INPUT_MEM_WRAP_OFFSET
                                - INPUT_MEM_CONT_OFFSET - INPUT_MEM_CONT_SIZE;
                        }

                        for (int sx = 0; sx < POOL_WIDTH; ++sx) {
                            if (PADDING_X != 0 && sx >= sxMax - sxMin)
                                break;

                            sum += inputs[iOffset + output
                                    + sx * INPUT_MEM_STRIDE];
                        }
                    }

                    outputs[oOffset + output] = (Output_T) (sum
                        / (POOL_HEIGHT * POOL_WIDTH));
                }
                else {
                    N2D2_THROW_OR_ABORT(std::runtime_error,
                        "The export only supports Max and Average pooling.");
                }
            }
        }
    }
}

template<int NB_CHANNELS, 
         int CHANNELS_HEIGHT, int CHANNELS_WIDTH,
         int NB_OUTPUTS,
         int OUTPUTS_HEIGHT, int OUTPUTS_WIDTH,
         ActivationFunction_T ACTIVATION,
         // Memory mapping: inputs
         int INPUT_MEM_CONT_OFFSET,
         int INPUT_MEM_CONT_SIZE,
         int INPUT_MEM_WRAP_OFFSET,
         int INPUT_MEM_WRAP_SIZE,
         int INPUT_MEM_STRIDE,
         // Memory mapping: outputs
         int OUTPUT_MEM_CONT_OFFSET,
         int OUTPUT_MEM_CONT_SIZE,
         int OUTPUT_MEM_WRAP_OFFSET,
         int OUTPUT_MEM_WRAP_SIZE,
         int OUTPUT_MEM_STRIDE,
         typename Input_T, typename Output_T,
         typename Rescaling_T>
N2D2_ALWAYS_INLINE inline void N2D2::Network::fccellPropagate(
    const Input_T* __restrict inputs,
    Output_T* __restrict outputs,
    const BDATA_T* __restrict biasses,
    const WDATA_T* __restrict weights,
    const Rescaling_T& __restrict rescaling) const
{
    static_assert(OUTPUTS_HEIGHT == 1, "Outputs height should be 1");
    static_assert(OUTPUTS_WIDTH == 1, "Outputs width should be 1");
    static_assert(OUTPUT_MEM_WRAP_SIZE == 0, "Output wrapping not supported");

    for (int och = 0; och < NB_OUTPUTS; och++) {
        SUM_T weightedSum = biasses[och];

        for (int iy = 0; iy < CHANNELS_HEIGHT; ++iy) {
            const int iPos = (CHANNELS_WIDTH * iy);
            int iOffset = INPUT_MEM_STRIDE * iPos;

            if (INPUT_MEM_WRAP_SIZE > 0 && iOffset >= INPUT_MEM_CONT_SIZE) {
                iOffset += INPUT_MEM_WRAP_OFFSET - INPUT_MEM_CONT_OFFSET
                            - INPUT_MEM_CONT_SIZE;
            }

            const int wOffset = NB_CHANNELS * CHANNELS_WIDTH
                                    * (iy + CHANNELS_HEIGHT * och);

            if (INPUT_MEM_STRIDE == NB_CHANNELS) {
                macsOnRange<NB_CHANNELS * CHANNELS_WIDTH>(
                    inputs + iOffset, 
                    weights + wOffset, 
                    weightedSum);
            }
            else {
                for (int ix = 0; ix < CHANNELS_WIDTH; ++ix) {
                    macsOnRange<NB_CHANNELS>(
                        inputs + iOffset + ix * INPUT_MEM_STRIDE, 
                        weights + wOffset + ix * NB_CHANNELS, 
                        weightedSum);
                }
            }
        }

        outputs[och] = sat<Output_T>(weightedSum, och, ACTIVATION, rescaling);
    }
}

template<typename Output_T>
inline void N2D2::Network::saveOutputs(
    int NB_OUTPUTS,
    int OUTPUTS_HEIGHT, int OUTPUTS_WIDTH,
    int OUTPUT_MEM_CONT_OFFSET,
    int OUTPUT_MEM_CONT_SIZE,
    int OUTPUT_MEM_WRAP_OFFSET,
    int OUTPUT_MEM_WRAP_SIZE,
    int OUTPUT_MEM_STRIDE,
    const Output_T* __restrict outputs,
    std::ostream& ostream,
    Format format) const
{
    if (format == Format::HWC) {
        ostream << "(";
        for(int oy = 0; oy < OUTPUTS_HEIGHT; oy++) {
            ostream << "(";

            for(int ox = 0; ox < OUTPUTS_WIDTH; ox++) {
                ostream << "(";

                const int oPos = (ox + OUTPUTS_WIDTH * oy);
                int oOffset = OUTPUT_MEM_STRIDE * oPos;

                if (OUTPUT_MEM_WRAP_SIZE > 0
                    && oOffset >= OUTPUT_MEM_CONT_SIZE)
                {
                    oOffset += OUTPUT_MEM_WRAP_OFFSET - OUTPUT_MEM_CONT_OFFSET
                                - OUTPUT_MEM_CONT_SIZE;
                }

                for (int output = 0; output < NB_OUTPUTS; output++) {
                    ostream.operator<<(outputs[oOffset + output]);
                    ostream << ", ";
                }

                ostream << "), \n";
            }

            ostream << "), \n";
        }

        ostream << ")\n";
    }
    else if (format == Format::CHW) {
        ostream << "(";
        for(int output = 0; output < NB_OUTPUTS; output++) {
            ostream << "(";

            for(int oy = 0; oy < OUTPUTS_HEIGHT; oy++) {
                ostream << "(";

                for(int ox = 0; ox < OUTPUTS_WIDTH; ox++) {
                    const int oPos = (ox + OUTPUTS_WIDTH * oy);
                    int oOffset = OUTPUT_MEM_STRIDE * oPos;

                    if (OUTPUT_MEM_WRAP_SIZE > 0
                        && oOffset >= OUTPUT_MEM_CONT_SIZE)
                    {
                        oOffset += OUTPUT_MEM_WRAP_OFFSET
                            - OUTPUT_MEM_CONT_OFFSET - OUTPUT_MEM_CONT_SIZE;
                    }

                    ostream.operator<<(outputs[oOffset + output]);
                    ostream << ", ";
                }

                ostream << "), \n";
            }

            ostream << "), \n";
        }

        ostream << ")\n";
    }
    else {
        N2D2_THROW_OR_ABORT(std::runtime_error, "Unknown format.");
    }
}

template<int NB_CHANNELS,
         int INPUTS_HEIGHT, int INPUTS_WIDTH,
         // Memory mapping: outputs
         int INPUT_MEM_CONT_OFFSET,
         int INPUT_MEM_CONT_SIZE,
         int INPUT_MEM_WRAP_OFFSET,
         int INPUT_MEM_WRAP_SIZE,
         int INPUT_MEM_STRIDE,
         typename Input_T>
N2D2_ALWAYS_INLINE inline void N2D2::Network::maxPropagate(
    const Input_T* __restrict inputs,
    int32_t* __restrict outputs) const
{
    int iMaxInput = 0;
    Input_T maxInput = std::numeric_limits<Input_T>::lowest();

    for (int iy = 0; iy < INPUTS_HEIGHT; ++iy) {
        for (int ix = 0; ix < INPUTS_WIDTH; ++ix) {
            const int oPos = (ix + INPUTS_WIDTH * iy);
            int iOffset = INPUT_MEM_STRIDE * oPos;

            if (INPUT_MEM_WRAP_SIZE > 0 && iOffset >= INPUT_MEM_CONT_SIZE) {
                iOffset += INPUT_MEM_WRAP_OFFSET - INPUT_MEM_CONT_OFFSET
                            - INPUT_MEM_CONT_SIZE;
            }

            for (int ch = 0; ch < NB_CHANNELS; ++ch) {
                if (inputs[iOffset + ch] > maxInput) {
                    iMaxInput = ch;
                    maxInput = inputs[iOffset + ch];
                }
            }

            outputs[oPos] = static_cast<int32_t>(iMaxInput);
        }
    }
}

template<int NB_CHANNELS, 
         int CHANNELS_HEIGHT, int CHANNELS_WIDTH,
         int NB_OUTPUTS,
         int OUTPUTS_HEIGHT, int OUTPUTS_WIDTH,
         // Memory mapping: inputs
         int INPUT_MEM_CONT_OFFSET,
         int INPUT_MEM_CONT_SIZE,
         int INPUT_MEM_WRAP_OFFSET,
         int INPUT_MEM_WRAP_SIZE,
         int INPUT_MEM_STRIDE,
         // Memory mapping: outputs
         int OUTPUT_MEM_CONT_OFFSET,
         int OUTPUT_MEM_CONT_SIZE,
         int OUTPUT_MEM_WRAP_OFFSET,
         int OUTPUT_MEM_WRAP_SIZE,
         int OUTPUT_MEM_STRIDE,
         typename Input_T, typename Output_T>
N2D2_ALWAYS_INLINE inline void N2D2::Network::resizeNearestNeighborPropagate(
    const Input_T* __restrict inputs,
    Output_T* __restrict outputs) const
{
    static_assert(NB_CHANNELS == NB_OUTPUTS,
        "NB_CHANNELS should be equal to NB_OUTPUTS.");
    static_assert(OUTPUTS_HEIGHT % CHANNELS_HEIGHT == 0,
        "Output height must be a multiple of input height.");
    static_assert(OUTPUTS_WIDTH % CHANNELS_WIDTH == 0,
        "Output width must be a multiple of input width.");

    for (int oy = 0; oy < OUTPUTS_HEIGHT; ++oy) {
        for (int ox = 0; ox < OUTPUTS_WIDTH; ++ox) {
            const int oPos = (ox + OUTPUTS_WIDTH * oy);
            int oOffset = OUTPUT_MEM_STRIDE * oPos;

            if (OUTPUT_MEM_WRAP_SIZE > 0 && oOffset >= OUTPUT_MEM_CONT_SIZE) {
                oOffset += OUTPUT_MEM_WRAP_OFFSET - OUTPUT_MEM_CONT_OFFSET
                            - OUTPUT_MEM_CONT_SIZE;
            }

            const int ix = ox * CHANNELS_WIDTH / OUTPUTS_WIDTH;
            const int iy = oy * CHANNELS_HEIGHT / OUTPUTS_HEIGHT;

            const int iPos = (ix + CHANNELS_WIDTH * iy);
            int iOffset = INPUT_MEM_STRIDE * iPos;

            if (INPUT_MEM_WRAP_SIZE > 0 && iOffset >= INPUT_MEM_CONT_SIZE) {
                iOffset += INPUT_MEM_WRAP_OFFSET - INPUT_MEM_CONT_OFFSET
                            - INPUT_MEM_CONT_SIZE;
            }

            for (int output = 0; output < NB_OUTPUTS; ++output) {
                outputs[oOffset + output] = inputs[iOffset + output];
            }
        }
    }
}

template<int NB_CHANNELS, 
         int CHANNELS_HEIGHT, int CHANNELS_WIDTH,
         int NB_OUTPUTS,
         int OUTPUTS_HEIGHT, int OUTPUTS_WIDTH,
         // Memory mapping: inputs
         int INPUT_MEM_CONT_OFFSET,
         int INPUT_MEM_CONT_SIZE,
         int INPUT_MEM_WRAP_OFFSET,
         int INPUT_MEM_WRAP_SIZE,
         int INPUT_MEM_STRIDE,
         // Memory mapping: outputs
         int OUTPUT_MEM_CONT_OFFSET,
         int OUTPUT_MEM_CONT_SIZE,
         int OUTPUT_MEM_WRAP_OFFSET,
         int OUTPUT_MEM_WRAP_SIZE,
         int OUTPUT_MEM_STRIDE,
         typename Input_T, typename Output_T,
         typename Rescaling_T>
N2D2_ALWAYS_INLINE inline void N2D2::Network::scalingPropagate(
    const Input_T* __restrict inputs,
    Output_T* __restrict outputs,
    const Rescaling_T& __restrict rescaling) const
{
    static_assert(NB_CHANNELS == NB_OUTPUTS,
        "NB_CHANNELS should be equal to NB_OUTPUTS.");
    static_assert(CHANNELS_HEIGHT == OUTPUTS_HEIGHT,
        "CHANNELS_HEIGHT should be equal to OUTPUTS_HEIGHT.");
    static_assert(CHANNELS_WIDTH == OUTPUTS_WIDTH,
        "CHANNELS_WIDTH should be equal to OUTPUTS_WIDTH.");

    for (int oy = 0; oy < OUTPUTS_HEIGHT; ++oy) {
        for (int ox = 0; ox < OUTPUTS_WIDTH; ++ox) {
            const int pos = (ox + OUTPUTS_WIDTH * oy);
            int oOffset = OUTPUT_MEM_STRIDE * pos;

            if (OUTPUT_MEM_WRAP_SIZE > 0 && oOffset >= OUTPUT_MEM_CONT_SIZE) {
                oOffset += OUTPUT_MEM_WRAP_OFFSET - OUTPUT_MEM_CONT_OFFSET
                            - OUTPUT_MEM_CONT_SIZE;
            }

            int iOffset = INPUT_MEM_STRIDE * pos;

            if (INPUT_MEM_WRAP_SIZE > 0 && iOffset >= INPUT_MEM_CONT_SIZE) {
                iOffset += INPUT_MEM_WRAP_OFFSET - INPUT_MEM_CONT_OFFSET
                            - INPUT_MEM_CONT_SIZE;
            }

            for (int ch = 0; ch < NB_OUTPUTS; ++ch) {
                outputs[oOffset + ch]
                    = sat<Output_T>(inputs[iOffset + ch], ch, Linear, rescaling);
            }
        }
    }
}

N2D2_ALWAYS_INLINE inline N2D2::Network::Tick_T N2D2::Network::tick() {
    return std::chrono::high_resolution_clock::now();
}

N2D2_ALWAYS_INLINE inline void N2D2::Network::benchmark(const char* name,
                                                        const Tick_T& start,
                                                        const Tick_T& end,
                                                        RunningMean_T& timing)
{
    auto duration = std::chrono::duration_cast
                        <std::chrono::microseconds>(end - start).count();
    timing.mean = (timing.mean * timing.count + duration)
                    / (timing.count + 1.0);
    ++timing.count;

    std::cout << name << " timing = " << timing.mean << " us" << std::endl;
}

#endif