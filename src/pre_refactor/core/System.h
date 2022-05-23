//
// Copyright (c) 2021 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "pch.h"

namespace tri {

    class TRI_API System {
    public:
        System() {}
        virtual ~System() {}
        virtual void startup() {};
        virtual void update() {};
        virtual void shutdown() {};
    };

}
