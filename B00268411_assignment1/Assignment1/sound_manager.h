 /* SIE CONFIDENTIAL
* PlayStation(R)4 Programmer Tool Runtime Library Release 07.008.001
* Copyright (C) 2013 Sony Interactive Entertainment Inc.

Games Console Development COMP10037 - Assignment 1
 B00268411

*/


#ifndef _SOUND_MANAGER_H
#define _SOUND_MANAGER_H

#include <sampleutil.h>

class SoundPlayer
{
public:
	SoundPlayer(void)
	{
		m_audioContext = NULL;
		m_voice1 = NULL;
		m_voiceData1 = NULL;
		m_voice2 = NULL;
		m_voiceData2 = NULL;
	}
	int initialize(sce::SampleUtil::Audio::AudioContext* audioContext, const char* dataDir)
	{
		int ret;
		(void)ret;
		m_audioContext = audioContext;

		std::string voidData1Path = "/ app0/" + std::string("audio_video/sound/wave/bgm.wav");
		//std::string voidData2Path = std::string(dataDir) + std::string("snd_explosion1.wav");

		ret = sce::SampleUtil::Audio::createVoiceDataFromFile(&m_voiceData1, voidData1Path.c_str(), true);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		/*ret = sce::SampleUtil::Audio::createVoiceDataFromFile(&m_voiceData2, voidData2Path.c_str(), false);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
*/
		ret = m_audioContext->createVoice(&m_voice1, m_voiceData1);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		/*ret = m_audioContext->createVoice(&m_voice2, m_voiceData2);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);*/
		return SCE_OK;
	}

	void playVoice1(void)
	{
		m_voice1->play();
	}
	void killVoice1(void)
	{
		m_voice1->kill();
	}
	void playVoice2(void)
	{
		m_voice2->play();
	}
	void killVoice2(void)
	{
		m_voice2->kill();
	}

	int finalize(void)
	{
		if (m_voice1 != NULL) {
			m_voice1->kill();
			sce::SampleUtil::destroy(m_voice1);
			m_voice1 = NULL;
		}
		if (m_voiceData1 != NULL) {
			sce::SampleUtil::destroy(m_voiceData1);
			m_voiceData1 = NULL;
		}
		if (m_voice2 != NULL) {
			m_voice2->kill();
			sce::SampleUtil::destroy(m_voice2);
			m_voice2 = NULL;
		}
		if (m_voiceData2 != NULL) {
			sce::SampleUtil::destroy(m_voiceData2);
			m_voiceData2 = NULL;
		}
		return SCE_OK;
	}
private:
	sce::SampleUtil::Audio::AudioContext* m_audioContext;
	sce::SampleUtil::Audio::Voice* m_voice1;
	sce::SampleUtil::Audio::VoiceData* m_voiceData1;
	sce::SampleUtil::Audio::Voice* m_voice2;
	sce::SampleUtil::Audio::VoiceData* m_voiceData2;
};



#endif /* _SOUND_MANAGER_H */