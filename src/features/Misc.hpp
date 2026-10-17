#pragma once

#include <windows.h>

namespace Features
{
    class Misc
    {
    public:
        static void Update();

    private:
        static void HandleGodmode();
        static void HandleInfiniteAmmo();
        static void HandleNoRecoil();
        static void HandleWeightLimit();
        static void HandleSuperJump();
        static void HandleSpeedhack();
        static void HandleBunnyhop();
        static void HandleEnvironment();
        static void HandleTrainerHotkeys();
    };
}
