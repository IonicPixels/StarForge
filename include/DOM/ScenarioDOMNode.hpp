#pragma once

#include "DOM/DOMNodeBase.hpp"
#include "io/BcsvIO.hpp"

//TODO Load Power Star Type names for UI rendering from JSON and or ini

class SScenarioDOMNode : public SDOMNodeBase {
    unsigned int mScenarioNo;
    unsigned int mPowerStarId; // Bitfield, what power stars exist in scenario. TODO: make this a list that is generated on load?
    unsigned int mPowerStarType;
    std::string mAppearPowerStarObj; // Name of object that spawns the power star associate w/ scenario #
    std::string mComet; // Comet Type
    unsigned int mCometLimitTimer;

    //Unused
    std::string mScenarioName;
    unsigned int mLuigiModeTimer;
    unsigned int mErrorCheck;

    // SMG1 Specific
    unsigned int mIsHidden;


public:
    typedef SDOMNodeBase Super; 

    void RenderHeirarchyUI();
    void RenderDetailsUI();

    SScenarioDOMNode();
    ~SScenarioDOMNode();

    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::Scenario){
            return true;
        }

        return Super::IsNodeType(type);
    }

    void Deserialize(SBcsvIO* bcsv, int entry);
    void Serialize();

};