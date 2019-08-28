#include "PacketLossConcealment.hpp"
#include "Fade.hpp"




class PacketLossConcealment_ZeroInsertion : public PacketLossConcealment {
public:
    PacketLossConcealment_ZeroInsertion(){}

    virtual bool predict(std::vector<std::shared_ptr<PcmSegment>> older, std::shared_ptr<PcmSegment> &outputSegment) override {
        // fade out old segments, and append one empty buffer
        FadeExponential fader;
        fader.fade(FadeInOut::FadeOut, older);
        
        std::memset(outputSegment->data(), 0, outputSegment->size());
        return true;
    }
};