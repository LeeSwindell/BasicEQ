/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicEQAudioProcessorEditor::BasicEQAudioProcessorEditor (BasicEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }
    
    setSize (600, 400);
}

BasicEQAudioProcessorEditor::~BasicEQAudioProcessorEditor()
{
}

//==============================================================================
void BasicEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void BasicEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(0.33 * bounds.getHeight());
    
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
        &highCutSlopeSlider
    };
};
