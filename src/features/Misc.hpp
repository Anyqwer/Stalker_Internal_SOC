#pragma once

#include <windows.h>

namespace Features
{
    class Misc
    {
    public:
        static void Update();

        // Third Person Camera Control
        static void SetCameraMode(int camIndex);
        static int GetCurrentCameraMode();

    private:
        static void HandleThirdPerson();
        static void HandleGodmode();
        static void HandleInfiniteAmmo();
        static void HandleNoRecoil();
        static void HandleWeightLimit();
        static void HandleSuperJump();
        static void HandleSpeedhack();
        static void HandleEnvironment();
        static void HandleFovChanger();
        static void HandleTrainerHotkeys();
    };
}
