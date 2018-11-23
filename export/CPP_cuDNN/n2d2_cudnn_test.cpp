/*
    (C) Copyright 2015 CEA LIST. All Rights Reserved.
    Contributor(s): Olivier BICHLER (olivier.bichler@cea.fr)
                    David BRIAND (david.briand@cea.fr)

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

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// For the Windows version of dirent.h (http://www.softagalleria.net/dirent.php)
#undef min
#undef max

#include "n2d2_cudnn_inference.hpp"

int main(int argc, char* argv[])
{
    std::string stimulus = "";
    std::string::size_type sz;   // alias of size_t
    unsigned int device = 0;
    unsigned int batchSize = 1;
    bool profiling = false;
    double total_time = 0.0;
    float successRate = 0.0;
    std::cout << "Binary automatically generated by the N2D2 platform\n "
                 "-Description: cudnn inference binary export for Deep Neural "
                 "Network.\n"
              << " -Command list:\n"
              << "    Stimulus selection: Use the '-stimulus "
                 "path/to/the/stimulus' command to select a specific input "
                 "stimulus (default value: none)\n"
              << "    Batch size: Use the '-batch xx' argument to select the batch size "
                 "(default value: 1) \n\n"
              << "    GPU-Device selection: Use the '-dev xx' argument to select the GPU device to target "
                 "(default value: 0) \n\n"
              << "    Profiling: Use the '-prof' argument to activate layer wise profiling"
                 "\n\n"
              << "This binary  is the exclusive property of the CEA. (C) "
                 "Copyright 2017 CEA LIST\n\n" << std::endl;

    for (int i = 1; i < argc; ++i) {
        const std::string Arg(argv[i]);
        if (Arg.compare("-batch") == 0) {
            std::string::size_type sz; // alias of size_t
            std::string nextArg(argv[i + 1]);
            batchSize = (unsigned)std::stoi(nextArg, &sz);
            std::cout << "Option -batch: Set batch size to " << batchSize
                      << std::endl;
        } else if (Arg.compare("-stimulus") == 0) {
            std::string nextArg(argv[i + 1]);
            stimulus = nextArg;
            std::cout << "Option -stimulus: process the stimulus " << stimulus
                      << std::endl;
        }
        else if (Arg.compare("-dev") == 0) {
            std::string nextArg(argv[i + 1]);
            device = std::stoi(nextArg,&sz);
            std::cout << "Option -dev: process network on the device: "
                      << device
                      << std::endl;
        }
        else if (Arg.compare("-prof") == 0) {
            profiling = true;
            std::cout << "Option -prof: layer wise profiling is activated "
                      << "a loss of performance can occur..."
                      << std::endl;
        }
    }
    std::cout << "Initialization of the cudnn network..." << std::endl;
    n2d2_cudnn_inference net(batchSize, device, profiling);
    std::cout << "Initialization of the cudnn network done!" << std::endl;

    unsigned int inputSize = net.inputDimX()*net.inputDimY()*net.inputDimZ();
    DATA_T* env_data = new DATA_T[inputSize * batchSize];

    unsigned int dimX = 1;
    unsigned int dimY = 1;
    std::vector<unsigned int> outDimX = net.outputDimX();
    std::vector<unsigned int> outDimY = net.outputDimY();
    std::vector<unsigned int> outDimZ = net.outputDimZ();
    std::vector<unsigned int> outTarget = net.outputTarget();

    if (outDimX[0] > 1 || outDimY[0] > 1) {
        dimX = net.inputDimX();
        dimY = net.inputDimY();
    }

    int32_t* outputTargets = new int32_t[dimX * dimY * batchSize * outDimZ.size()];

    uint32_t** outputEstimated;
    outputEstimated = new uint32_t*[outDimZ.size()];
    for(unsigned int t = 0; t < outDimZ.size(); ++t)
            outputEstimated[t] = new uint32_t[outDimX[t]*outDimY[t] * batchSize];

    unsigned int** confusion;
    confusion = new unsigned int*[outDimZ.size()];
    for(unsigned int t = 0; t < outDimZ.size(); ++t)
        confusion[t] = new unsigned int[outTarget[t]*outTarget[t]]{0};


    if (!stimulus.empty()) {

        envRead(stimulus,
                inputSize,
                net.inputDimY(),
                net.inputDimX(),
                env_data,
                dimX * dimY * outDimZ.size(),
                outputTargets);

        net.execute(env_data);

        for(unsigned int targetIdx = 0; targetIdx < outDimZ.size(); ++targetIdx)
        {

            net.estimated(outputEstimated[targetIdx], targetIdx);

            unsigned int nbTargetLabels = (outDimZ[targetIdx] > 1 ? outDimZ[targetIdx] : 2);

            double yRatio = net.inputDimY() / outDimY[targetIdx];
            double xRatio = net.inputDimX() / outDimX[targetIdx];


            unsigned int nbIgnored = 0;
            unsigned int nbHits = 0;

            for (unsigned int oy = 0; oy < outDimY[targetIdx]; ++oy) {
                for (unsigned int ox = 0; ox < outDimX[targetIdx]; ++ox) {
                    int iy = oy;
                    int ix = ox;
                    if (dimX > 1 || dimY > 1) {
                        iy = (int)floor((oy + 0.5) * yRatio);
                        ix = (int)floor((ox + 0.5) * xRatio);
                    }

                    const unsigned int oIdx = ox + oy * outDimX[targetIdx];
                    const unsigned int Idx = ix + iy * dimX
                                                + targetIdx * dimX * dimY;

                    if (outputTargets[Idx] < 0)
                        ++nbIgnored;
                    else {
                        const unsigned int confIdx = outputEstimated[targetIdx][oIdx]
                                                     + outputTargets[Idx]
                                                       * outTarget[targetIdx];
                        confusion[targetIdx][confIdx] += 1;
                        if (outputTargets[Idx] == (int)outputEstimated[targetIdx][oIdx])
                            ++nbHits;

                    }

                }
            }

            const double success
                = (outDimX[targetIdx]*outDimY[targetIdx] > nbIgnored)
                      ? (nbHits / (double)(outDimX[targetIdx]*outDimY[targetIdx] - nbIgnored))
                      : 1.0;
            printf("Success rate = %02f%%\n", 100.0 * success);
            confusion_print(outTarget[targetIdx], confusion[targetIdx]);
        }

    } else {

        std::string fileName;
        std::vector<std::string> filesList = std::vector<std::string>();
        const std::string dirName = "stimuli";
        getFilesList(dirName, filesList);
        double* success = new double[outDimZ.size()]{0};
        unsigned int total = 0;
        double elapsed = 0.0;
        double elapsed_avg = 0.0;
        unsigned int indexFile = 0;
        unsigned int n = batchSize;

        for (std::vector<std::string>::iterator filesIt = filesList.begin(),
                                                itEnd = filesList.end();
             filesIt < itEnd;
             std::advance(filesIt, batchSize)) {

            //-------Extract data from input file---------//
            for (unsigned int i = 0; i < batchSize; i++) {
                if (indexFile >= filesList.size()) {
                    n = i;
                    break;
                }
                fileName = *(filesIt + i);
                envRead(fileName,
                        inputSize,
                        net.inputDimY(),
                        net.inputDimX(),
                        env_data + i * inputSize,
                        dimX * dimY *outDimZ.size(),
                        &outputTargets[i * dimX * dimY * outDimZ.size()]);
                indexFile++;
            }
            //-------Neural Network---------//
            const std::chrono::high_resolution_clock::time_point start
                = std::chrono::high_resolution_clock::now();
            net.execute(env_data);

            for(unsigned int t = 0; t < outDimZ.size(); ++t)
                net.estimated(outputEstimated[t], t);

            elapsed = 1.0e6 * std::chrono::duration_cast
                              <std::chrono::duration<double> >(
                                  std::chrono::high_resolution_clock::now()
                                  - start).count();
            elapsed_avg += elapsed;
            //---------------------------//


            for (unsigned int i = 0; i < n; i++)
            {
                for(unsigned int targetIdx = 0; targetIdx < outDimZ.size();
                        ++targetIdx)
                {
                    unsigned int nbTargetLabels = (outDimZ[targetIdx] > 1 ?
                                                        outDimZ[targetIdx] : 2);

                    double yRatio = net.inputDimY() / outDimY[targetIdx];
                    double xRatio = net.inputDimX() / outDimX[targetIdx];

                    unsigned int nbIgnored = 0;
                    unsigned int nbHits = 0;

                    for (unsigned int oy = 0; oy < outDimY[targetIdx]; ++oy)
                    {
                        for (unsigned int ox = 0; ox < outDimX[targetIdx]; ++ox)
                        {
                            int iy = oy;
                            int ix = ox;
                            if (dimX > 1 || dimY > 1) {
                                iy = (int)floor((oy + 0.5) * yRatio);
                                ix = (int)floor((ox + 0.5) * xRatio);
                            }
                            const unsigned int oIdx
                                = ox + oy * outDimX[targetIdx]
                                  + i * outDimX[targetIdx] * outDimY[targetIdx];

                            const unsigned int Idx = ix + iy * dimX
                                                     + targetIdx * dimX * dimY
                                                     + i * dimX * dimY * outDimZ.size();


                            if (outputTargets[Idx] < 0)
                                ++nbIgnored;
                            else {
                                const unsigned int confIdx
                                    = (unsigned int)outputEstimated[targetIdx][oIdx]
                                      + outputTargets[Idx] * outTarget[targetIdx];
                                confusion[targetIdx][confIdx] += 1;
                                if (outputTargets[Idx]
                                    == (int)outputEstimated[targetIdx][oIdx]) {
                                    ++nbHits;
                                }
                            }
                        }
                    }
                    success[targetIdx]
                        += (outDimX[targetIdx]*outDimY[targetIdx] > nbIgnored)
                        ? (nbHits / (double)(outDimX[targetIdx]*outDimY[targetIdx] - nbIgnored))
                        : 1.0;
                }
                ++total;
#ifndef NRET
                for(unsigned int targetIdx = 0; targetIdx < outDimZ.size(); ++targetIdx)
                {
                    printf(
                        "t(%d) %.02f/%d (avg = %02f%%)   ",
                        targetIdx,
                        success[targetIdx],
                        total,
                        100.0 * success[targetIdx] / (float)total);
                }
                printf( "Host process time = %f µs\n" ,  elapsed / (double)n);
#endif
            }
        }

#ifndef NRET
        for(unsigned int targetIdx = 0; targetIdx < outDimZ.size(); ++targetIdx)
            confusion_print(outTarget[targetIdx], confusion[targetIdx]);

        net.getProfiling(total);
#endif

        std::cout << "---" << std::endl;

        total_time = (elapsed_avg / (double)total);
        std::cout << "---" << std::endl;
        std::cout << ESC_BOLD << "Tested " << total << " stimuli" << ESC_ALL_OFF
                  << "\n"
                  << "Success rate per target: ";
        for(unsigned int t = 0; t < outDimZ.size(); ++t)
                  std::cout << " " << (100.0 * success[t] / (float)total)
                            << "% t(" << t << ")";
        std::cout << "\ncuDNN batch size used: " << batchSize << ESC_ALL_OFF
                  << "\n"
                     "Average elapsed host time per stimulus = " << total_time
                  << " us\n"
                     "Processing frequency = " << std::setprecision(9)
                  << 1 / (total_time * 1.0e-6) << " stimuli/sec" << std::endl;
                  successRate = (100.0 * success[0] / (float)total);

    }

#ifdef OUTXT
    std::ofstream success_result("success_rate.txt");

    if (!success_result.good())
        throw std::runtime_error("Could not create file:  success_rate.txt");
    success_result << successRate;
    success_result.close();
#endif

    delete[] env_data;
    delete[] outputTargets;

    for(unsigned int t = outDimZ.size() - 1; t >-1; --t)
    {
      delete[] outputEstimated[t];
      delete[] confusion[t];
    }

}

