/*
Games Console Development COMP10037 - Assignment 1
 B00268411
*/

#include <sampleutil.h>
#include <iostream>
#include "sound_manager.h"
 extern "C" size_t sceLibcHeapSize = 256 * 1024 * 1024; // 256 MiB

#define APP_DIR "/app0/data/"

namespace vecmath = sce::Vectormath::Simd::Aos;
namespace ssg = sce::SampleUtil::Graphics;
namespace ssa = sce::SampleUtil::Audio;




float m_yaw = 0.0f;
float m_pitch = (float)(0.0 / 180.0 * M_PI);


class Camera
{
	float m_distance;
	vecmath::Vector3 m_target; /* Gaze target of camera */

public:
	Camera()
	{
		m_distance = 500.0f;
		m_target = vecmath::Vector3(0.0f, 80.0f, 0.0f);
	}

	/* Return the camera position */
	vecmath::Vector3 getPosition()
	{
		vecmath::Vector3 z = vecmath::Vector3(0.0f, 0.0f, -1.0f);
		vecmath::Vector3 orientation =
			(vecmath::Matrix4::rotationY(-m_yaw)
				* vecmath::Matrix4::rotationX(-m_pitch)
				* vecmath::Vector4(z, 1.0)).getXYZ();
		return m_target - m_distance * orientation;
	}

	/* Return the view matrix of this camera */
	vecmath::Matrix4 getViewMatrix()
	{
		return vecmath::Matrix4::lookAt(
			vecmath::Point3(getPosition()),
			vecmath::Point3(m_target),
			vecmath::Vector3(0.0f, 1.0f, 0.0f));
	}
};

class Character
{
	ssg::Collada::ColladaData* m_collada;
	ssg::Collada::InstanceVisualScene* m_instanceVisualScene;
	ssg::Collada::AnimationPlayer* m_animPlayer;
public:
	vecmath::Vector3 m_pos;
	float            m_rotY;
	float            m_animationTime;

	//Set new parameters for character to be given values later
	int initialize(ssg::Collada::ColladaLoader* loader, const char* path, float animStartTime, vecmath::Vector3 position)
	{
		loader->load(&m_collada, path);
		ssg::Collada::createInstanceVisualScene(&m_instanceVisualScene, m_collada->getVisualScene());
		ssg::Collada::createAnimationPlayer(&m_animPlayer, m_instanceVisualScene, m_collada->getAnimation());

		m_animPlayer->setTime(0.0f);

		m_pos = position;
		m_animationTime = animStartTime;
		return SCE_OK;
	}

	int finalize()
	{
		sce::SampleUtil::destroy(m_animPlayer);
		sce::SampleUtil::destroy(m_instanceVisualScene);
		sce::SampleUtil::destroy(m_collada);
		return SCE_OK;
	}

	void update()
	{
		m_animPlayer->setTime(m_animationTime);
		m_animationTime += 0.01f;
		if (m_animationTime >= m_animPlayer->getAnimation()->getEndTime()) {
			m_animationTime = 0.0f;
		}

	}

	void draw(ssg::GraphicsContext* context)
	{
		vecmath::Matrix4 m = vecmath::Matrix4::translation(m_pos)
			* vecmath::Matrix4::rotationZYX(vecmath::Vector3(0.0, m_rotY, 0.0f));
		m_instanceVisualScene->draw(context, m);
	}
};

class Application : public sce::SampleUtil::SampleSkeleton
{
	ssg::Collada::ColladaLoader* m_loader;
	vecmath::Matrix4             m_projectionMatrix;
	Character                    m_boy;
	Character                    m_boy1;
	Character                    m_boy2;
	Character                    m_boy3;
	Camera                       m_camera;
private:

	ssg::SpriteRenderer* m_sprite;
	uint32_t m_numFrames;
	vecmath::Vector3 lightPos;
	vecmath::Vector3 lightColour;
	int colourTimer;

	SoundPlayer m_audio;
public:
	virtual int initialize()
	{
		int ret;
		(void)ret;

		m_numFrames = 0;

		SceUserServiceUserId userId;

		//flags to inform the program what it should be looking for
		ret = initializeUtil(kFunctionFlagGraphics | kFunctionFlagUserIdManager | kFunctionFlagSpriteRenderer | kFunctionFlagAudio);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
 
		sceUserServiceGetInitialUser(&userId);

		sce::SampleUtil::Graphics::RenderTarget* render_target = getGraphicsContext()->getNextRenderTarget();

		ssg::Collada::createColladaLoader(&m_loader, getGraphicsContext());


		const float fov = (float)(M_PI / 6.0f);
		const float aspect = (float)render_target->getWidth() / (float)render_target->getHeight();
		const float _near = 0.1f;
		const float _far = 10000.0f;
		m_projectionMatrix = vecmath::Matrix4::perspective(fov, aspect, _near, _far);

		lightPos = vecmath::Vector3(0, 1200, 1000);
		lightColour = vecmath::Vector3(1.0, 1.0, 1.0);
		//initialising light
		m_loader->getDefaultParams()->setLight(lightPos, lightColour);

		//initialise characters
		m_boy.initialize(m_loader, APP_DIR "/graphics/model/boy/astroBoy_walk.dae", 0.0f, vecmath::Vector3(-180.0, -50.0, -100));
		m_boy1.initialize(m_loader, APP_DIR "/graphics/model/boy/astroBoy_walk.dae", 0.5f, vecmath::Vector3(180.0, 100.0, -150));
		m_boy2.initialize(m_loader, APP_DIR "/graphics/model/boy/astroBoy_walk.dae", 1.0f, vecmath::Vector3(25.0, -50.0, 20));
 
		//initialising controller
		scePadInit();
		m_handle = scePadOpen(userId, SCE_PAD_PORT_TYPE_STANDARD, 0, NULL);

		//initialising audio
	   ret =  m_audio.initialize(getAudioContext(), APP_DIR);
	   SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		return SCE_OK;
	}

	virtual int update(void)
	{
		colourTimer++;
		int ret;
		(void)ret;

		m_numFrames++;

		ret = updateUtil();
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		m_boy.update();
		m_boy1.update();
		m_boy2.update();
 
		//rotate camera up
		scePadRead(m_handle, m_data, sizeof(m_data) / sizeof(ScePadData));
		if (m_data[0].buttons & SCE_PAD_BUTTON_UP)
		{
			m_pitch += 0.05f;
		}
		//rotate camera down
		scePadRead(m_handle, m_data, sizeof(m_data) / sizeof(ScePadData));
		if (m_data[0].buttons & SCE_PAD_BUTTON_DOWN)
		{
			m_pitch -= 0.05f;
		}
		//rotate camera left
		scePadRead(m_handle, m_data, sizeof(m_data) / sizeof(ScePadData));
		if (m_data[0].buttons & SCE_PAD_BUTTON_LEFT)
		{
			m_yaw -= 0.05f;
		}
		//rotate camera right
		scePadRead(m_handle, m_data, sizeof(m_data) / sizeof(ScePadData));
		if (m_data[0].buttons & SCE_PAD_BUTTON_RIGHT)
		{
			m_yaw += 0.05f;
		}
		//play audio
		scePadRead(m_handle, m_data, sizeof(m_data) / sizeof(ScePadData));
		if (m_data[0].buttons & SCE_PAD_BUTTON_R1)
		{
			m_audio.playVoice1();
		}
		//stop audio
		scePadRead(m_handle, m_data, sizeof(m_data) / sizeof(ScePadData));
		if (m_data[0].buttons & SCE_PAD_BUTTON_R2)
		{
			m_audio.killVoice1();
		}

		if (colourTimer < 10) {
			lightColour = vecmath::Vector3(0.0f, 1.0f, 0.0f);
			m_loader->getDefaultParams()->setLight(lightPos, lightColour); //update light colour to green 

		}
		else if (colourTimer > 20 && colourTimer <= 30) {
			lightColour = vecmath::Vector3(1.0, 0.0, 0.0);
			m_loader->getDefaultParams()->setLight(lightPos, lightColour); //update light colour to red
 
		}
		else if (colourTimer > 40 && colourTimer <= 50) {
			lightColour = vecmath::Vector3(1.0, 0.0, 1.0);
			m_loader->getDefaultParams()->setLight(lightPos, lightColour); // update light colour to purple
 
		}
		else if (colourTimer > 60 && colourTimer <= 70) {
			lightColour = vecmath::Vector3(1.0, 1.0, 0.0);
			m_loader->getDefaultParams()->setLight(lightPos, lightColour); // update light colour to yellow

		}
		else if (colourTimer > 80 && colourTimer <= 90) {
			lightColour = vecmath::Vector3(0.0, 0.0, 1.0);
			m_loader->getDefaultParams()->setLight(lightPos, lightColour); // update light colour to blue
 
		}
		if (colourTimer > 100) {
			colourTimer = 0;//set counter back to zero
 
		}
		return SCE_OK;
	}

	virtual void render()
	{
		m_loader->getDefaultParams()->setProjectionMatrix(m_projectionMatrix);
		getGraphicsContext()->beginScene(getGraphicsContext()->getNextRenderTarget(ssg::GraphicsContext::kFrameBufferSideLeft), getGraphicsContext()->getDepthStencilSurface());

		getGraphicsContext()->clearRenderTarget(0x00000000);

		getGraphicsContext()->setDepthWriteEnable(true);
		getGraphicsContext()->setDepthFunc(ssg::kDepthFuncLessEqual);

		m_loader->getDefaultParams()->setViewMatrix(m_camera.getViewMatrix());
		getGraphicsContext()->setCullMode(ssg::kCullNone);

		m_boy.draw(getGraphicsContext());
		m_boy1.draw(getGraphicsContext());
		m_boy2.draw(getGraphicsContext());
 

		getGraphicsContext()->setDepthFunc(ssg::kDepthFuncAlways);

		getGraphicsContext()->endScene();
		getGraphicsContext()->flip(1);
	}

	virtual int finalize()
	{
		int ret;
		(void)ret;

		m_boy.finalize();
		m_boy1.finalize();
		m_boy2.finalize();
 
		m_sprite = NULL;
		m_audio.finalize();


		sce::SampleUtil::destroy(m_loader);
		finalizeUtil();
		return SCE_OK;
	}

private:


	int m_handle = SCE_SAMPLE_UTIL_USER_ID_INVALID;

	ScePadData m_data[8]{};
};

Application g_application;
int main(int argc, char* argv[])
{

	Application app;

	int ret = 0;
	ret = g_application.initialize();
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

	while (1) {
		ret = g_application.update();
		if (ret != SCE_OK) {
			break;
		}
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		g_application.render();
	}
	ret = g_application.finalize();
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

	return 0;
}