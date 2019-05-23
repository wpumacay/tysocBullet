
#include <test_interface.h>

class AppExample : public bullet::MultibodyTestApplication
{
    private :

    bullet::SimMultibody* _createHumanoidShoulder( const btVector3& position );

    protected :

    void _initScenario() override;

    public :

    AppExample();
    ~AppExample();
};

AppExample::AppExample()
    : bullet::MultibodyTestApplication()
{
    // nothing here
}

AppExample::~AppExample()
{
    // nothing here
}

void AppExample::_initScenario()
{
    // Create the base
    auto _base = createBody( "box",
                             btVector3( 10, 10, 1 ),
                             btVector3( 0, 0, -2.5 ),
                             btVector3( 0, 0, 0 ),
                             btVector3( 0.3, 0.3, 0.3 ),
                             0.0f );

    // Create a testing capsule
    auto _capsule = createBody( "capsule",
                                btVector3( 0.1, 0.4, 0 ),
                                btVector3( 1, 1, 1 ),
                                btVector3( 0, 0, 0 ),
                                btVector3( 0.7, 0.5, 0.3 ),
                                0.5f );

    auto _humanoidShoulder = _createHumanoidShoulder( { 0, 0, 1. } );

    addSimMultibody( _humanoidShoulder );
}

bullet::SimMultibody* AppExample::_createHumanoidShoulder( const btVector3& position )
{
    auto _simbody = new bullet::SimMultibody( (2 + 1) + 1, // 3 for the multidof, 1 for the singlejoint dof
                                              position,
                                              "box",
                                              { 0.1, 0.4, 0.1 },
                                              1.,
                                              true );

    // define the components of the shoulder
    std::vector< std::string > _jointsTypes = { "revolute", "revolute" };
    std::vector< btVector3 > _jointsAxes    = { { -1., -1., 1. }, { 1., -1., 1. } };
    std::vector< btVector3 > _jointsPivots  = { { 0., 0.15, 0. }, { 0., 0.15, 0. } };

    // define the transform of the shoulder w.r.t. the base
    btTransform _trShoulderToBase;
    _trShoulderToBase.setIdentity();
    _trShoulderToBase.setOrigin( { 0., -0.2 - 0.15, 0. } );

    auto _shoulderLinks = _simbody->setupLinkMultiDof( 0, 
                                                       "box",
                                                       { 0.05, 0.3, 0.05 },
                                                       _trShoulderToBase,
                                                       _simbody->ptrRootLink(),
                                                       _jointsTypes,
                                                       _jointsAxes,
                                                       _jointsPivots );

    btTransform _trArmToShoulder;
    _trArmToShoulder.setIdentity();
    _trArmToShoulder.setOrigin( { 0., -0.15 - 0.2, 0. } );

    auto _arm = _simbody->setupLinkSingleJoint( 0 + _shoulderLinks.size(),
                                                "box",
                                                { 0.05, 0.4, 0.05 },
                                                _trArmToShoulder,
                                                _shoulderLinks.back(),
                                                "revolute",
                                                { 1., 0., 0. },
                                                { 0., 0.2, 0. } );

    _simbody->ptrBtMultibody()->setHasSelfCollision( false );
    _simbody->ptrBtMultibody()->finalizeMultiDof();

    return _simbody;
}

int main()
{
    auto _app = new AppExample();

    _app->start();
    _app->togglePause();

    while ( true )
    {
        if ( engine::InputSystem::checkSingleKeyPress( GLFW_KEY_P ) )
            _app->togglePause();

        if ( engine::InputSystem::checkSingleKeyPress( GLFW_KEY_ESCAPE ) )
            break;

        _app->step();
    }

    return 0;
}