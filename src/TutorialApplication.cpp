#include "TutorialApplication.h"


TutorialApplication::TutorialApplication()
    : mRoot(0),
    mResourcesCfg(Ogre::StringUtil::BLANK),
    mPluginsCfg(Ogre::StringUtil::BLANK)
{
}

TutorialApplication::~TutorialApplication()
{
    /* Remove ourself as a Window listener */
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
    delete mRoot;
}

bool TutorialApplication::go()
{
#ifdef _DEBUG
    mResourcesCfg = "resources_d.cfg";
    mPluginsCfg = "plugins_d.cfg";
#else
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";
#endif

    mRoot = new Ogre::Root(mPluginsCfg);

    /* Load resource configuration */
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);
    Ogre::String name, locType;
    Ogre::ConfigFile::SectionIterator secIt = cf.getSectionIterator();
    while (secIt.hasMoreElements())
    {
        Ogre::ConfigFile::SettingsMultiMap* settings = secIt.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator it;
        for (it = settings->begin(); it != settings->end(); ++it)
        {
            locType = it->first;
            name = it->second;
            Ogre::ResourceGroupManager::getSingleton()
                .addResourceLocation(name, locType);
        }
    }
    /* Alt: Manually Configure the rendering system. */
    //RenderSystem* rs = mRoot->getRenderSystemByName("Direct3D9 Rendering Subsystem");
    // 
    //mRoot->setRenderSystem(rs);
    //rs->setConfigOption("Full Screen", "No");
    //rs->setConfigOption("Video Mode", "800 x 600 @ 32-bit colour");
   
    /* Create rendering window using OGRE. */
    if(!(mRoot->restoreConfig() || mRoot->showConfigDialog()))
        return false; // should throw exception and delete ogre.cfg
    mWindow = mRoot->initialise(true, "TutorialApplication Render Window");

    /* Alt: Create rendering window using Win32 API. */
    //mRoot->initialise(false);

    //HWND hWnd = 0;

    //// Retrieve the HWND for the window we want to render in.
    //// This step depends entirely on the windowing system you are using.

    //NameValuePairList misc;
    //misc["externalWindowHandle"] = StringConverter::toString((int)hWnd);

    //RenderWindow* win = mRoot->createRenderWindow("Main RenderWindow", 800, 600, false, &misc);
   
    /* Define number of mipmaps. */
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
   
    /* Initialize all resources found. */
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    /* Create default scene manager. */
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);

    /* Set up camera. */
    mCamera = mSceneMgr->createCamera("MainCam");
     
    mCamera->setPosition(0, 0, 80);
    mCamera->lookAt(0, 0, -300);
    mCamera->setNearClipDistance(5);

    /* Declare and create a viewport. */
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);

    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

    mCamera->setAspectRatio(
            Ogre::Real(vp->getActualWidth()) / 
            Ogre::Real(vp->getActualHeight()));
    
    /* Set up scene. */
    Ogre::Entity* ogreEntity = mSceneMgr->createEntity("ogrehead.mesh");

    Ogre::SceneNode* ogreNode = mSceneMgr
        ->getRootSceneNode()
        ->createChildSceneNode();
    ogreNode->attachObject(ogreEntity);

    mSceneMgr->setAmbientLight(Ogre::ColourValue(.5, .5, .5));

    Ogre::Light* light = mSceneMgr->createLight("MainLight");
    light->setPosition(20, 80, 50);

    /* Set up Inputmanager */
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;
     
    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
     
    mInputManager = OIS::InputManager::createInputSystem( pl );

    /* I want keyboard and mouse. (false->unbuffered) */
    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager
            ->createInputObject(OIS::OISKeyboard, false));
    mMouse = static_cast<OIS::Mouse*>(mInputManager
            ->createInputObject(OIS::OISMouse, false));

    // Why is this in BaseApplication? TODO: research
    //mMouse->setEventCallback(this);
    //mKeyboard->setEventCallback(this);

    /* Register application as WindowEventListener */
    //Set initial mouse clipping size
    windowResized(mWindow);
    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

    /* Listen to frame events and render. */
    mRoot->addFrameListener(this);
    mRoot->startRendering();

    return true;
}

//Adjust mouse clipping area
void TutorialApplication::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}

//Unattach OIS before window shutdown (very important under Linux)
void TutorialApplication::windowClosed(Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if(rw == mWindow)
    {
        if(mInputManager)
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );

            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}

bool TutorialApplication::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if(mWindow->isClosed())
        return false;

    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();

    if(mKeyboard->isKeyDown(OIS::KC_ESCAPE))
        return false;

    return true;
}


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
        int main(int argc, char *argv[])
#endif
        {
            // Create application object
            TutorialApplication app;

            try {
                app.go();
            } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
                MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
                std::cerr << "An exception has occured: " <<
                    e.getFullDescription().c_str() << std::endl;
#endif
            }

            return 0;
        }

#ifdef __cplusplus
}
#endif
