#ifndef ITEMTYPES_HPP
#define ITEMTYPES_HPP

#include <iostream>

// keys
#define KEY_1 0
#define KEY_2 1
#define KEY_3 2
#define KEY_4 3
#define KEY_5 4
#define KEY_6 5
#define KEY_7 6
#define KEY_8 7
#define KEY_9 8
#define KEY_10 9


namespace itemType{

    namespace Adrenaline{
        inline const unsigned int durationTime = 10;
        inline const float runningSpeed = 15.0f;
        inline const float headShake = 13.0f;
    }

    namespace Cross{
        inline const unsigned int durationTime = 10;
        inline const float radius = 50.0f;
    }

    namespace NightVision{
        inline const unsigned int durationTime = 20;
    }

    namespace Battery{
        inline const unsigned int timePlus = 20;
    }

    namespace Teleporter{
        inline const unsigned int uses = 10;
        inline const float radius = 50.0f;
    }

    namespace CrystalExposer{
        inline const float radius = 50.0f;
        inline const unsigned int uses = 1;
        inline const unsigned int durationTime = 10;
    }

    namespace mobStunner{
        inline const float radius = 50.0f;
        inline const unsigned int uses = 1;
        inline const unsigned int durationTime = 8;
    }

    [[nodiscard]] inline bool checkType(std::string const &type){
        if(type.empty()) return false;
        return (type == "adrenaline") || (type == "cross") || (type == "nightVision") || (type == "battery") || (type == "teleporter") || 
        (type == "crystalExposer") || (type == "mobStunner");
    }
    

}






#endif