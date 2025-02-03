#pragma once
#include  "Value.h"
#include  "PluginData.h"
#include  "CallFrame.h"
#include  <vector>
#include  <stack>
#include  <array>
namespace Eva
{
    struct CallState {
        std::vector<ValueContainer>& stack;
        PluginTable& pluginTable;
        std::array<CallFrame, callFrameAmount >& callFrames;
        int nextToCurrentCallFrame = -1;
        int argumentCount = -1;


        CallState(std::vector<ValueContainer>& stack,
            PluginTable& pluginTable,
            std::array<CallFrame, callFrameAmount>& callFrames,
            int nextToCurrentCallFrame)
            : stack(stack),
            pluginTable(pluginTable),
            callFrames(callFrames),
            nextToCurrentCallFrame(nextToCurrentCallFrame),
            argumentCount(-1)  // add it
        {

        }

    };
}