/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

ResponseCurveComponent::ResponseCurveComponent(BasicEQAudioProcessor& p) : audioProcessor(p)
{
    const auto& params =audioProcessor.getParameters();
    for (auto param : params)
    {
        param->addListener(this);
    }
    
    startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    const auto& params =audioProcessor.getParameters();
    for (auto param : params)
    {
        param->removeListener(this);
    }
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void ResponseCurveComponent::timerCallback()
{
    if (parametersChanged.compareAndSetBool(false, true))
    {
        //update monochain, then repaint
        auto chainSettings = getChainSettings(audioProcessor.apvts);
        auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
        updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
        
        auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
        updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);
        
        auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
        updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);
        
        repaint();
    }
}

void ResponseCurveComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    using namespace juce;
    g.fillAll(Colours::black);
    
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(0.33 * bounds.getHeight());
    
    auto w = responseArea.getWidth();
    
    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();
    
    auto sampleRate = audioProcessor.getSampleRate();
    
    std::vector<double> mags;
    mags.resize(w);
    
    for (int i = 0; i < w; ++i)
    {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);
        
        if (!monoChain.isBypassed<ChainPositions::Peak>())
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        if (!lowcut.isBypassed<0>())
            mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<1>())
            mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<2>())
            mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<3>())
            mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        if (!highcut.isBypassed<0>())
            mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<1>())
            mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<2>())
            mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<3>())
            mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        mags[i] = Decibels::gainToDecibels(mag);
    };
    
    Path responseCurve;
    
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input)
    {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };
    
    responseCurve.startNewSubPath(responseArea.getX(), mags.front());
    
    for (size_t i = 0; i < mags.size(); ++i)
    {
        responseCurve.lineTo(responseArea.getX()+i, map(mags[i]));
    }
    
    g.setColour(Colours::orange);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);
    
    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));
    
};

//==============================================================================
BasicEQAudioProcessorEditor::BasicEQAudioProcessorEditor (BasicEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
responseCurveComponent(audioProcessor),
peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
peakQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }

//    const auto& params =audioProcessor.getParameters();
//    for (auto param : params)
//    {
//        param->addListener(this);
//    }
//
//    startTimerHz(60);
    
    setSize (600, 400);
};

BasicEQAudioProcessorEditor::~BasicEQAudioProcessorEditor()
{
//    const auto& params =audioProcessor.getParameters();
//    for (auto param : params)
//    {
//        param->removeListener(this);
//    }
};

//==============================================================================
void BasicEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    using namespace juce;
    g.fillAll(Colours::black);
//
//    auto bounds = getLocalBounds();
//    auto responseArea = bounds.removeFromTop(0.33 * bounds.getHeight());
//
//    auto w = responseArea.getWidth();
//
//    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
//    auto& peak = monoChain.get<ChainPositions::Peak>();
//    auto& highcut = monoChain.get<ChainPositions::HighCut>();
//
//    auto sampleRate = audioProcessor.getSampleRate();
//
//    std::vector<double> mags;
//    mags.resize(w);
//
//    for (int i = 0; i < w; ++i)
//    {
//        double mag = 1.f;
//        auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);
//
//        if (!monoChain.isBypassed<ChainPositions::Peak>())
//            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
//
//        if (!lowcut.isBypassed<0>())
//            mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
//        if (!lowcut.isBypassed<1>())
//            mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
//        if (!lowcut.isBypassed<2>())
//            mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
//        if (!lowcut.isBypassed<3>())
//            mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
//
//        if (!highcut.isBypassed<0>())
//            mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
//        if (!highcut.isBypassed<1>())
//            mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
//        if (!highcut.isBypassed<2>())
//            mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
//        if (!highcut.isBypassed<3>())
//            mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
//
//        mags[i] = Decibels::gainToDecibels(mag);
//    };
//
//    Path responseCurve;
//
//    const double outputMin = responseArea.getBottom();
//    const double outputMax = responseArea.getY();
//    auto map = [outputMin, outputMax](double input)
//    {
//        return jmap(input, -24.0, 24.0, outputMin, outputMax);
//    };
//
//    responseCurve.startNewSubPath(responseArea.getX(), mags.front());
//
//    for (size_t i = 0; i < mags.size(); ++i)
//    {
//        responseCurve.lineTo(responseArea.getX()+i, map(mags[i]));
//    }
//
//    g.setColour(Colours::orange);
//    g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);
//
//    g.setColour(Colours::white);
//    g.strokePath(responseCurve, PathStrokeType(2.f));
//
};

void BasicEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(0.33 * bounds.getHeight());
    
    responseCurveComponent.setBounds(responseArea);
    
    auto lowCutArea = bounds.removeFromLeft(0.33 * bounds.getWidth());
    auto highCutArea = bounds.removeFromRight(0.5 * bounds.getWidth());
    
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(0.5 * lowCutArea.getHeight()));
    lowCutSlopeSlider.setBounds(lowCutArea);
    
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(0.5 * highCutArea.getHeight()));
    highCutSlopeSlider.setBounds(highCutArea);
    
    peakFreqSlider.setBounds(bounds.removeFromTop(0.33 * bounds.getHeight()));
    peakGainSlider.setBounds(bounds.removeFromTop(0.5 * bounds.getHeight()));
    peakQualitySlider.setBounds(bounds);
};

//void BasicEQAudioProcessorEditor::parameterValueChanged(int parameterIndex, float newValue)
//{
//    parametersChanged.set(true);
//}
//
//void BasicEQAudioProcessorEditor::timerCallback()
//{
//    if (parametersChanged.compareAndSetBool(false, true))
//    {
//        //update monochain, then repaint
//        auto chainSettings = getChainSettings(audioProcessor.apvts);
//        auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
//        updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
//
//        auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
//        updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);
//
//        auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
//        updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);
//
//        repaint();
//    }
//}

std::vector<juce::Component*> BasicEQAudioProcessorEditor::getComps()
{
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider,
        &responseCurveComponent
    };
};
