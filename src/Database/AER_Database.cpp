/*
    (C) Copyright 2017 CEA LIST. All Rights Reserved.
    Contributor(s): Johannes THIELE (johannes.thiele@cea.fr)
                    Olivier BICHLER (olivier.bichler@cea.fr)

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

#include "Database/AER_Database.hpp"



N2D2::AerReadEvent::AerReadEvent(unsigned int x_,
                                 unsigned int y_,
                                 unsigned int channel_,
                                 unsigned int batch_,
                                 int value_,
                                 Time_T time_)
    : x(x_),
    y(y_),
    channel(channel_),
    batch(batch_),
    value(value_),
    time(time_)
{
    // ctor
}


N2D2::AER_Database::AER_Database(bool loadDataInMemory)
    : Database(loadDataInMemory)
{
    // ctor
}

