
#include <bullet_agent_wrapper.h>

namespace tysoc {
namespace bullet {

//     /***************************************************************************
//     *                        TBtJointWrapper Implementation                    *
//     ****************************************************************************/

//     TBtJointWrapper::TBtJointWrapper( btMultiBody* btMultiBodyPtr,
//                                       TKinTreeJoint* jointPtr,
//                                       int linkIndx )
//     {
//         m_btMultiBodyPtr = btMultiBodyPtr;
//         m_kinTreeJointPtr = jointPtr;
//         m_linkIndx = linkIndx;

//         m_nqpos = jointPtr->nqpos;
//         m_nqvel = jointPtr->nqvel;
//     }

//     TBtJointWrapper::~TBtJointWrapper()
//     {
//         m_btMultiBodyPtr = NULL;
//         m_kinTreeJointPtr = NULL;
//     }

//     void TBtJointWrapper::setQpos( const std::vector< TScalar >& qpos )
//     {
//         assert( qpos.size() == m_nqpos );

//         for ( int i = 0; i < qpos.size(); i++ )
//             m_btMultiBodyPtr->setJointPosMultiDof( m_linkIndx, qpos.data() );
//     }

//     void TBtJointWrapper::setQvel( const std::vector< TScalar >& qvel )
//     {
//         assert( qvel.size() == m_nqvel );

//         for ( int i = 0; i < qvel.size(); i++ )
//             m_btMultiBodyPtr->setJointVelMultiDof( m_linkIndx, qvel.data() );
//     }

//     bool TBtJointWrapper::isRootJoint()
//     {
//         if ( !m_kinTreeJointPtr->parentBodyPtr )
//         {
//             std::cout << "ERROR> this joint should have a parent body" << std::endl;
//             return false;
//         }

//         if ( m_kinTreeJointPtr->parentBodyPtr->parentBodyPtr )
//             return false;

//         return true;
//     }

//     /***************************************************************************
//     *                        TBtMultiBodyLink Implementation                   *
//     ****************************************************************************/

//     TBtMultiBodyLink::TBtMultiBodyLink( int indx,
//                                         TBtMultiBodyLink* parentLinkPtr,
//                                         btMultiBody* btMultiBodyPtr,
//                                         btMultiBodyDynamicsWorld* btWorldPtr )
//     {
//         m_btLinkIndx            = indx;
//         m_parentLinkPtr         = parentLinkPtr;
//         m_btMultiBodyPtr        = btMultiBodyPtr;
//         m_btWorldPtr            = btWorldPtr;
//         m_btCollisionShapePtr   = NULL;
//         m_btLinkColliderPtr     = NULL;

//         m_btJointMotor              = NULL;
//         m_btSphericalJointMotor     = NULL;
//         m_btJointLimitConstraint    = NULL;

//         m_density = TYSOC_DEFAULT_DENSITY;
//         m_armature = 0.0f;
//         m_friction = { 3, { 1., 0.005, 0.0001 } };

//         m_mass = 0.0f;
//         m_inertiaDiag = { 0.0f, 0.0f, 0.0f };
//     }

//     TBtMultiBodyLink::~TBtMultiBodyLink()
//     {
//         m_parentLinkPtr         = NULL;
//         m_btMultiBodyPtr        = NULL;
//         m_btWorldPtr            = NULL;
//         m_btCollisionShapePtr   = NULL;
//         m_btLinkColliderPtr     = NULL;

//         m_btJointMotor              = NULL;
//         m_btSphericalJointMotor     = NULL;
//         m_btJointLimitConstraint    = NULL;
//     }

//     void TBtMultiBodyLink::setupLinkInMultiBody( const std::string& shapeType,
//                                                  const TVec3& shapeSize,
//                                                  const TMat4& worldTransform,
//                                                  const TMat4& localTransform,
//                                                  const std::string& jointType,
//                                                  const TVec3& jointAxis,
//                                                  const TVec3& jointPivot,
//                                                  float lowerLimit,
//                                                  float upperLimit,
//                                                  bool useMotor )
//     {
//         // grab parent index for latex usage 
//         int _linkParentIndx = ( m_parentLinkPtr != NULL ) ? 
//                                     m_parentLinkPtr->getIndx() : -1;

//         // first just create the shape required for the link
//         m_btCollisionShapePtr = utils::createCollisionShape( shapeType, shapeSize );
//         if ( m_btCollisionShapePtr )
//             m_btCollisionShapePtr->setMargin( 0.1 * TVec3::length( shapeSize ) );// @TODO: set amrgin according to size of the shape

//         // compute inertial properties from this shape
//         btScalar _linkMass = ( shapeType != "none" ) ? utils::computeVolumeFromShape( m_btCollisionShapePtr ) * m_density : 0.0f;
//         btVector3 _linkInertia = btVector3( 0., 0., 0. );
//         if ( _linkMass != 0.0f )
//             m_btCollisionShapePtr->calculateLocalInertia( _linkMass, _linkInertia );

//         m_mass = _linkMass;
//         m_inertiaDiag = utils::fromBtVec3( _linkInertia );

//         // transform everything to bullet data types
//         btTransform _btWorldTransform = utils::toBtTransform( worldTransform );
//         btTransform _btLocalTransform = utils::toBtTransform( localTransform );
//         btVector3 _btJointPivot = utils::toBtVec3( jointPivot );
//         btVector3 _btJointAxis  = utils::toBtVec3( jointAxis );

//         // grab localPos and localRot from localTransform. Recall this=local, ...
//         // which is respect to the parent link. Also, get the inverse of these ...
//         // quantities, as the setupXYZ methods require these quantities
//         auto _this2parent_quat  = _btLocalTransform.getRotation();
//         auto _this2parent_pos   = _btLocalTransform.getOrigin();
//         auto _parent2this_quat  = _this2parent_quat.inverse();
//         auto _parent2this_pos   = -quatRotate( _parent2this_quat, _this2parent_pos );

//         // compute pivot w.r.t. parent COM
//         auto _pivot2parent_pos = _this2parent_pos - quatRotate( _this2parent_quat, -_btJointPivot );

//         // add armature (if given by the model) to stabilize simulation
//         _linkInertia += btVector3( m_armature, m_armature, m_armature );

//         // setup the constraint for this link
//         if ( jointType == "hinge" || jointType == "continuous" || jointType == "revolute" )
//         {
//             // @HACK: Add a small amount of inertia to avoid erratic behaviour
//             if ( shapeType == "none" && !useMotor )
//                 _linkInertia = { 0.001, 0.001, 0.001 };

//             m_btMultiBodyPtr->setupRevolute( m_btLinkIndx, 
//                                              _linkMass, 
//                                              _linkInertia,
//                                              _linkParentIndx,
//                                              _parent2this_quat,
//                                              _btJointAxis,
//                                              _pivot2parent_pos,
//                                              -_btJointPivot,
//                                              true );

//             utils::TBtLogger::log( std::string( "-------- revolute ----------" ) );
//             utils::TBtLogger::log( std::string( "linkIndex: " ) + std::to_string( m_btLinkIndx ) );
//             utils::TBtLogger::log( std::string( "mass: " ) + std::to_string( _linkMass ) );
//             utils::TBtLogger::log( std::string( "localinertia: " ) + utils::to_string( _linkInertia ) );
//             utils::TBtLogger::log( std::string( "parentIndex: " ) + std::to_string( _linkParentIndx ) );
//             utils::TBtLogger::log( std::string( "rotParentToThis: " ) + utils::to_string( _parent2this_quat ) );
//             utils::TBtLogger::log( std::string( "jointAxis: " ) + utils::to_string( _btJointAxis ) );
//             utils::TBtLogger::log( std::string( "parentComToThisPivotOffset: " ) + utils::to_string( _pivot2parent_pos ) );
//             utils::TBtLogger::log( std::string( "thisPivotToThisComOffset: " ) + utils::to_string( -_btJointPivot ) );
//             utils::TBtLogger::log( std::string( "disableParentCollision: " ) + std::to_string( true ) );
//             utils::TBtLogger::log( std::string( "lowerLimit: " ) + std::to_string( lowerLimit ) );
//             utils::TBtLogger::log( std::string( "upperLimit: " ) + std::to_string( upperLimit ) );
//             utils::TBtLogger::log( std::string( "shape-type: " ) + shapeType );
//             utils::TBtLogger::log( std::string( "shape-size: " ) + TVec3::toString( shapeSize ) );

//             m_btMultiBodyPtr->getLink( m_btLinkIndx ).m_jointLowerLimit = lowerLimit;
//             m_btMultiBodyPtr->getLink( m_btLinkIndx ).m_jointUpperLimit = upperLimit;

//             if ( lowerLimit <= upperLimit )
//             {
//                 m_btJointLimitConstraint = new btMultiBodyJointLimitConstraint( 
//                                                         m_btMultiBodyPtr, 
//                                                         m_btLinkIndx, 
//                                                         lowerLimit, 
//                                                         upperLimit );

//                 m_btWorldPtr->addMultiBodyConstraint( m_btJointLimitConstraint );
//                 m_btJointLimitConstraint->finalizeMultiDof();
                
//                 if ( useMotor )
//                 {
//                     m_btJointMotor = new btMultiBodyJointMotor( m_btMultiBodyPtr, 
//                                                                 m_btLinkIndx, 
//                                                                 0, 0, 5. );
//                     m_btJointMotor->setErp( 0.1 );
//                     m_btJointMotor->setPositionTarget( 0.0 );

//                     m_btWorldPtr->addMultiBodyConstraint( m_btJointMotor );
//                     m_btJointMotor->finalizeMultiDof();
//                 }
//             }
//         }
//         else if ( jointType == "slider" || jointType == "slide" || jointType == "prismatic" )
//         {
//             m_btMultiBodyPtr->setupPrismatic( m_btLinkIndx,
//                                               _linkMass,
//                                               _linkInertia,
//                                               _linkParentIndx,
//                                               _parent2this_quat,
//                                               _btJointAxis,
//                                               _pivot2parent_pos,
//                                               -_btJointPivot,
//                                               true );

//             utils::TBtLogger::log( std::string( "-------- prismatic ----------" ) );
//             utils::TBtLogger::log( std::string( "linkIndex: " ) + std::to_string( m_btLinkIndx ) );
//             utils::TBtLogger::log( std::string( "mass: " ) + std::to_string( _linkMass ) );
//             utils::TBtLogger::log( std::string( "localinertia: " ) + utils::to_string( _linkInertia ) );
//             utils::TBtLogger::log( std::string( "parentIndex: " ) + std::to_string( _linkParentIndx ) );
//             utils::TBtLogger::log( std::string( "rotParentToThis: " ) + utils::to_string( _parent2this_quat ) );
//             utils::TBtLogger::log( std::string( "jointAxis: " ) + utils::to_string( _btJointAxis ) );
//             utils::TBtLogger::log( std::string( "parentComToThisPivotOffset: " ) + utils::to_string( _pivot2parent_pos ) );
//             utils::TBtLogger::log( std::string( "thisPivotToThisComOffset: " ) + utils::to_string( -_btJointPivot ) );
//             utils::TBtLogger::log( std::string( "disableParentCollision: " ) + std::to_string( true ) );
//             utils::TBtLogger::log( std::string( "lowerLimit: " ) + std::to_string( lowerLimit ) );
//             utils::TBtLogger::log( std::string( "upperLimit: " ) + std::to_string( upperLimit ) );
//             utils::TBtLogger::log( std::string( "shape-type: " ) + shapeType );
//             utils::TBtLogger::log( std::string( "shape-size: " ) + TVec3::toString( shapeSize ) );

//             m_btMultiBodyPtr->getLink( m_btLinkIndx ).m_jointLowerLimit = lowerLimit;
//             m_btMultiBodyPtr->getLink( m_btLinkIndx ).m_jointUpperLimit = upperLimit;

//             if ( lowerLimit <= upperLimit )
//             {
//                 if ( useMotor )
//                 {
//                     m_btJointMotor = new btMultiBodyJointMotor( m_btMultiBodyPtr, 
//                                                                 m_btLinkIndx, 
//                                                                 0, 0, 5. );
//                     m_btJointMotor->setErp( 0.1 );
//                     m_btJointMotor->setPositionTarget( 0. );

//                     m_btWorldPtr->addMultiBodyConstraint( m_btJointMotor );
//                 }
//                 else
//                 {
//                     m_btJointLimitConstraint = new btMultiBodyJointLimitConstraint( 
//                                                             m_btMultiBodyPtr, 
//                                                             m_btLinkIndx, 
//                                                             lowerLimit, 
//                                                             upperLimit );

//                     m_btWorldPtr->addMultiBodyConstraint( m_btJointLimitConstraint );
//                     m_btJointLimitConstraint->finalizeMultiDof();
//                 }
//             }

//         }
//         else if ( jointType == "ball" || jointType == "spheric" || jointType == "spherical" )
//         {
//             m_btMultiBodyPtr->setupSpherical( m_btLinkIndx,
//                                               _linkMass,
//                                               _linkInertia,
//                                               _linkParentIndx,
//                                               _parent2this_quat,
//                                               _pivot2parent_pos,
//                                               -_btJointPivot,
//                                               true );

//             utils::TBtLogger::log( std::string( "-------- spherical ----------" ) );
//             utils::TBtLogger::log( std::string( "linkIndex: " ) + std::to_string( m_btLinkIndx ) );
//             utils::TBtLogger::log( std::string( "mass: " ) + std::to_string( _linkMass ) );
//             utils::TBtLogger::log( std::string( "localinertia: " ) + utils::to_string( _linkInertia ) );
//             utils::TBtLogger::log( std::string( "parentIndex: " ) + std::to_string( _linkParentIndx ) );
//             utils::TBtLogger::log( std::string( "rotParentToThis: " ) + utils::to_string( _parent2this_quat ) );
//             utils::TBtLogger::log( std::string( "parentComToThisPivotOffset: " ) + utils::to_string( _pivot2parent_pos ) );
//             utils::TBtLogger::log( std::string( "thisPivotToThisComOffset: " ) + utils::to_string( -_btJointPivot ) );
//             utils::TBtLogger::log( std::string( "disableParentCollision: " ) + std::to_string( true ) );
//             utils::TBtLogger::log( std::string( "lowerLimit: " ) + std::to_string( lowerLimit ) );
//             utils::TBtLogger::log( std::string( "upperLimit: " ) + std::to_string( upperLimit ) );
//             utils::TBtLogger::log( std::string( "shape-type: " ) + shapeType );
//             utils::TBtLogger::log( std::string( "shape-size: " ) + TVec3::toString( shapeSize ) );

//             m_btMultiBodyPtr->getLink( m_btLinkIndx ).m_jointLowerLimit = lowerLimit;
//             m_btMultiBodyPtr->getLink( m_btLinkIndx ).m_jointUpperLimit = upperLimit;

//             if ( lowerLimit <= upperLimit )
//             {
//                 if ( useMotor )
//                 {
//                     m_btSphericalJointMotor = new btMultiBodySphericalJointMotor( 
//                                                             m_btMultiBodyPtr, 
//                                                             m_btLinkIndx, 
//                                                             5. );
//                     m_btSphericalJointMotor->setErp( 0.1 );
//                     m_btSphericalJointMotor->setPositionTarget( { 0., 0., 0., 1. } );

//                     m_btWorldPtr->addMultiBodyConstraint( m_btSphericalJointMotor );
//                     m_btSphericalJointMotor->finalizeMultiDof();
//                 }
//             }
//         }
//         else if ( jointType == "fixed" || jointType == "free" )
//         {
//             m_btMultiBodyPtr->setupFixed( m_btLinkIndx,
//                                           _linkMass,
//                                           _linkInertia,
//                                           _linkParentIndx,
//                                           _parent2this_quat,
//                                           _pivot2parent_pos,
//                                           -_btJointPivot,
//                                           true );

//             utils::TBtLogger::log( std::string( "-------- fixed ----------" ) );
//             utils::TBtLogger::log( std::string( "linkIndex: " ) + std::to_string( m_btLinkIndx ) );
//             utils::TBtLogger::log( std::string( "mass: " ) + std::to_string( _linkMass ) );
//             utils::TBtLogger::log( std::string( "localinertia: " ) + utils::to_string( _linkInertia ) );
//             utils::TBtLogger::log( std::string( "parentIndex: " ) + std::to_string( _linkParentIndx ) );
//             utils::TBtLogger::log( std::string( "rotParentToThis: " ) + utils::to_string( _parent2this_quat ) );
//             utils::TBtLogger::log( std::string( "parentComToThisPivotOffset: " ) + utils::to_string( _pivot2parent_pos ) );
//             utils::TBtLogger::log( std::string( "thisPivotToThisComOffset: " ) + utils::to_string( -_btJointPivot ) );
//             utils::TBtLogger::log( std::string( "disableParentCollision: " ) + std::to_string( true ) );
//             utils::TBtLogger::log( std::string( "shape-type: " ) + shapeType );
//             utils::TBtLogger::log( std::string( "shape-size: " ) + TVec3::toString( shapeSize ) );
//         }
//         else
//         {
//             std::cout << "ERROR> joint type: " << jointType << " not supported" << std::endl;
//         }

//         // collider managment
//         m_btLinkColliderPtr = new btMultiBodyLinkCollider( m_btMultiBodyPtr, m_btLinkIndx );
//         m_btLinkColliderPtr->setCollisionShape( m_btCollisionShapePtr );
//         m_btMultiBodyPtr->getLink( m_btLinkIndx ).m_collider = m_btLinkColliderPtr;

//         // if ( shapeType != "none" )
//         //     m_btLinkColliderPtr->setContactStiffnessAndDamping( 1e18, 1. );

//         // // set the friction of this collider
//         m_btLinkColliderPtr->setFriction( 0.8 );
//         m_btLinkColliderPtr->setSpinningFriction( 0.1 );

//         // initialize worldTransform from parent
//         m_btLinkColliderPtr->setWorldTransform( utils::toBtTransform( worldTransform ) );

//         // add collider to the world
//         m_btWorldPtr->addCollisionObject( m_btLinkColliderPtr, 1, -1 );
//     }

//     TMat4 TBtMultiBodyLink::getWorldTransform()
//     {
//         auto _linkPos = m_btMultiBodyPtr->localPosToWorld( m_btLinkIndx, { 0., 0., 0. } );
//         auto _linkRot = m_btMultiBodyPtr->localFrameToWorld( m_btLinkIndx, btMatrix3x3::getIdentity() );
//         auto _linkWorldTransform = btTransform( _linkRot, _linkPos );

//         return utils::fromBtTransform( _linkWorldTransform );
//     }

//     int TBtMultiBodyLink::getIndx()
//     {
//         return m_btLinkIndx;
//     }

//     void TBtMultiBodyLink::setDensity( const TScalar& density )
//     {
//         m_density = density;
//     }

//     void TBtMultiBodyLink::setFriction( const TSizef& friction )
//     {
//         m_friction = friction;
//     }

//     void TBtMultiBodyLink::setArmature( const TScalar& armature )
//     {
//         m_armature = armature;
//     }

//     /***************************************************************************
//     *                        TBodyCompound Implementation                      *
//     ****************************************************************************/

//     TBodyCompound::TBodyCompound( TKinTreeBody* kinTreeBodyPtr,
//                                   btMultiBody* btMultiBodyPtr,
//                                   btMultiBodyDynamicsWorld* btWorldPtr,
//                                   TBodyCompound* parent,
//                                   int linkIndx )
//     {
//         m_kinTreeBodyPtr    = kinTreeBodyPtr;
//         m_btMultiBodyPtr    = btMultiBodyPtr;
//         m_btWorldPtr        = btWorldPtr;
//         m_parent            = parent;
//         m_linkBaseIndx      = linkIndx;
//         m_hasMultidof       = false;

//         if ( kinTreeBodyPtr )
//         {
//             m_hasMultidof = kinTreeBodyPtr->childJoints.size() > 1;
//             _constructLinksInCompound();
//         }
//     }

//     TBodyCompound::~TBodyCompound()
//     {
//         m_kinTreeBodyPtr    = NULL;
//         m_btMultiBodyPtr    = NULL;
//         m_parent            = NULL;

//         m_children.clear();

//         for ( size_t q = 0; q < m_linksInChain.size(); q++ )
//         {
//             delete m_linksInChain[q];
//             m_linksInChain[q] = NULL;
//         }
//         m_linksInChain.clear();
//     }

//     void TBodyCompound::_constructLinksInCompound()
//     {
//         // Grab the parent link
//         TBtMultiBodyLink* _parentLink = m_parent->lastLink();

//         // if the parent link is NULL, then the parent compound was the base
//         bool _isParentBase = ( _parentLink == NULL );

//         // grab some resources we will need
//         auto _joints        = m_kinTreeBodyPtr->childJoints;
//         auto _collisions    = m_kinTreeBodyPtr->childCollisions;

//         // declare some transforms we will need (will define them later appropriately)
//         TMat4 _trThisCompoundToParentCompound;
//         TMat4 _trParentEndLinkToParentCompound;
//         TMat4 _trParentEndLinkToWorld;

//         // define these transforms appropriately
//         if ( _isParentBase )
//         {
//             _trThisCompoundToParentCompound.setIdentity();
//             _trParentEndLinkToParentCompound.setIdentity();
//             _trParentEndLinkToWorld = m_parent->getWorldTransform();
//         }
//         else
//         {
//             _trThisCompoundToParentCompound = m_kinTreeBodyPtr->relTransform;
//             _trParentEndLinkToParentCompound = m_parent->lastToBaseTransform();
//             _trParentEndLinkToWorld = _parentLink->getWorldTransform();
//         }

//         // If dealing with a multidof case, create dummies accordingly
//         if ( m_hasMultidof )
//         {
//             for ( size_t q = 0; q < _joints.size(); q++ )
//             {
//                 /* Compute the transforms required to setup the link **********/
//                 TMat4 _trThisLinkToParentLink;
//                 TMat4 _trThisLinkToWorld;

//                 if ( q == 0 )
//                 {
//                     _trThisLinkToParentLink = _trParentEndLinkToParentCompound.inverse() *
//                                               _trThisCompoundToParentCompound *
//                                               _joints[q]->relTransform;
//                     _trThisLinkToWorld = _trParentEndLinkToWorld * _trThisLinkToParentLink;

//                     // save the first-to-base transform (first-link w.r.t. this compound)
//                     m_firstLinkToBaseTransform = _joints[q]->relTransform;
//                     m_baseToFirstLinkTransform = m_firstLinkToBaseTransform.inverse();
//                 }
//                 else
//                 {
//                     _trThisLinkToParentLink = _joints[q-1]->relTransform.inverse() *
//                                               _joints[q]->relTransform;
//                     _trThisLinkToWorld = _parentLink->getWorldTransform() * 
//                                          _trThisLinkToParentLink;
//                 }
//                 /**************************************************************/

//                 // The pivot coincides with th origin of the link's frame
//                 TVec3 _jointPivotInThisLink = { 0., 0., 0. };
//                 // Recall, for this case joint-frame = link-frame
//                 TVec3 _jointAxisInThisLink = _joints[q]->axis;

//                 // Create the dummy link
//                 auto _link = new TBtMultiBodyLink( m_linkBaseIndx + m_linksInChain.size(),
//                                                    _parentLink,
//                                                    m_btMultiBodyPtr,
//                                                    m_btWorldPtr );
//                 _link->setArmature( _joints[q]->armature );
//                 _link->setupLinkInMultiBody( "none",
//                                              { 0., 0., 0. },
//                                              _trThisLinkToWorld,
//                                              _trThisLinkToParentLink,
//                                              _joints[q]->type,
//                                              _jointAxisInThisLink,
//                                              _jointPivotInThisLink,
//                                              _joints[q]->lowerLimit,
//                                              _joints[q]->upperLimit );

//                 // cache this new link in the chain of this compound
//                 m_linksInChain.push_back( _link );

//                 // book keeping for next iteration
//                 _parentLink = _link;

//                 // keep the name of the joint linked to its link-id
//                 m_jointsNamesToLinksIdMap[_joints[q]->name] = m_linksInChain.back()->getIndx();
//             }
//         }
//         else
//         {
//             /* 
//             *   Create first link from the joint dof and the collider, others ...
//             *   (if any) are created later with fixed constraints
//             */

//             // if no joints, then we'll assume a fixed joint instead
//             bool _noJoints = ( _joints.size() == 0 );

//             /* Compute the transforms required to setup this link *************/
//             TMat4 _trThisLinkToParentLink;
//             TMat4 _trThisLinkToWorld;
//             TMat4 _trJointFrameToThisLink;

//             _trThisLinkToParentLink = _trParentEndLinkToParentCompound.inverse() *
//                                       _trThisCompoundToParentCompound *
//                                       _collisions.front()->relTransform;

//             _trThisLinkToWorld = _trParentEndLinkToWorld * _trThisLinkToParentLink;

//             if ( _noJoints )
//                 _trJointFrameToThisLink.setIdentity();
//             else
//                 _trJointFrameToThisLink = _collisions.front()->relTransform.inverse() *
//                                           _joints.front()->relTransform;

//             /******************************************************************/

//             /* Define joint information ***************************************/
//             TVec3 _jointPivotInThisLink;
//             TVec3 _jointAxisInThisLink;
//             std::string _jointType;
//             TScalar _jointLowerLimit;
//             TScalar _jointUpperLimit;

//             // The pivot coincides with th origin of the joint-frame
//             _jointPivotInThisLink = _trJointFrameToThisLink.getPosition();
//             // Transform axis given in joint-frame to link-frame
//             if ( _noJoints )
//             {
//                 _jointAxisInThisLink = { 1.0, 0.0, 0.0 };
//                 _jointType = "fixed";
//                 _jointLowerLimit = 1.0f;
//                 _jointUpperLimit = -1.0f;
//             }
//             else
//             {
//                 _jointAxisInThisLink = _trJointFrameToThisLink.getRotation() * 
//                                        _joints.front()->axis;
//                 _jointType = _joints.front()->type;
//                 _jointLowerLimit = _joints.front()->lowerLimit;
//                 _jointUpperLimit = _joints.front()->upperLimit;
//             }

//             /******************************************************************/

//             // armature to be added (depends on the joint)
//             TScalar _armature = ( _noJoints ) ? 0.0 : _joints.front()->armature;

//             // Create the link
//             auto _link = new TBtMultiBodyLink( m_linkBaseIndx + m_linksInChain.size(),
//                                                _parentLink,
//                                                m_btMultiBodyPtr,
//                                                m_btWorldPtr );
//             _link->setDensity( _collisions.front()->density );
//             _link->setFriction( _collisions.front()->friction );
//             _link->setArmature( _armature );
//             _link->setupLinkInMultiBody( _collisions.front()->geometry.type,
//                                          _collisions.front()->geometry.size,
//                                          _trThisLinkToWorld,
//                                          _trThisLinkToParentLink,
//                                          _jointType,
//                                          _jointAxisInThisLink,
//                                          _jointPivotInThisLink,
//                                          _jointLowerLimit,
//                                          _jointUpperLimit );

//             // cache this new link in the chain of this compound
//             m_linksInChain.push_back( _link );

//             // book keeping for next iterations
//             _parentLink = _link;

//             if ( !_noJoints )
//             {
//                 // keep the name of the joint linked to its link-id
//                 m_jointsNamesToLinksIdMap[_joints.front()->name] = m_linksInChain.back()->getIndx();
//             }

//             // save the first-to-base transform (first-link w.r.t. this compound)
//             m_firstLinkToBaseTransform = _collisions.front()->relTransform;
//             m_baseToFirstLinkTransform = m_firstLinkToBaseTransform.inverse();

//             // just in case this is the last link as well, save the last-link transform
//             if ( _collisions.size() == 1 )
//             {
//                 m_lastLinkToBaseTransform = _collisions.front()->relTransform;
//                 m_baseToLastLinkTransform = m_lastLinkToBaseTransform.inverse();
//             }

//             // save the mass for this collision
//             m_masses[_collisions.front()->name] = _link->mass();
//             m_inertiasDiags[_collisions.front()->name] = _link->inertiDiag();
//         }

//         // Create remaining links from collisions. The first one might already ...
//         // have been created. If not, create it w.r.t. previous dummy link. If ...
//         // already created, just skip its construction. All others are fixed ...
//         // one w.r.t another
//         for ( size_t q = 0; q < _collisions.size(); q++ )
//         {
//             // Check if already created first link
//             if ( q == 0 && !m_hasMultidof )
//                 continue;

//             /* Compute the transforms required to setup this link *************/
//             TMat4 _trThisLinkToParentLink;
//             TMat4 _trThisLinkToWorld;

//             if ( q == 0 && m_hasMultidof )
//             {
//                 _trThisLinkToParentLink = _joints.back()->relTransform.inverse() *
//                                           _collisions[q]->relTransform;
//                 _trThisLinkToWorld = _parentLink->getWorldTransform() * 
//                                      _trThisLinkToParentLink;
//             }
//             else
//             {
//                 _trThisLinkToParentLink = _collisions[q-1]->relTransform.inverse() *
//                                         _collisions[q]->relTransform;
//                 _trThisLinkToWorld = _parentLink->getWorldTransform() * 
//                                     _trThisLinkToParentLink;
//             }
//             /******************************************************************/

//             // Create the link
//             auto _link = new TBtMultiBodyLink( m_linkBaseIndx + m_linksInChain.size(),
//                                                _parentLink,
//                                                m_btMultiBodyPtr,
//                                                m_btWorldPtr );
//             _link->setDensity( _collisions[q]->density );
//             _link->setFriction( _collisions[q]->friction );
//             _link->setupLinkInMultiBody( _collisions[q]->geometry.type,
//                                          _collisions[q]->geometry.size,
//                                          _trThisLinkToWorld,
//                                          _trThisLinkToParentLink,
//                                          "fixed",
//                                          { 1., 0., 0. },
//                                          { 0., 0., 0. } );

//             // cache this new link in the chain of this compound
//             m_linksInChain.push_back( _link );

//             // book keeping for next iteration
//             _parentLink = _link;

//             // just in case this is the last link, save the last-link transform
//             if ( q == ( _collisions.size() - 1 ) )
//             {
//                 m_lastLinkToBaseTransform = _collisions[q]->relTransform;
//                 m_baseToLastLinkTransform = m_lastLinkToBaseTransform.inverse();
//             }

//             // save the mass for this collision
//             m_masses[_collisions[q]->name] = _link->mass();
//             m_inertiasDiags[_collisions[q]->name] = _link->inertiDiag();
//         }
//     }

//     void TBodyCompound::addChild( TBodyCompound* childCompound )
//     {
//         m_children.push_back( childCompound );
//     }

//     std::vector< TBodyCompound* > TBodyCompound::children()
//     {
//         return m_children;
//     }

//     void TBodyCompound::setWorldTransform( const TMat4& transform )
//     {
//         m_worldTransform = transform;
//     }

//     TMat4 TBodyCompound::getWorldTransform()
//     {
//         return m_worldTransform;
//     }

//     void TBodyCompound::updateTransforms()
//     {
//         m_kinTreeBodyPtr->worldTransform = m_linksInChain.back()->getWorldTransform() *
//                                            m_baseToLastLinkTransform;
//     }

//     size_t TBodyCompound::getNumLinks()
//     {
//         return m_linksInChain.size();
//     }

//     TBtMultiBodyLink* TBodyCompound::firstLink()
//     {
//         if ( m_linksInChain.size () > 0 )
//             return m_linksInChain.front();
        
//         return NULL;
//     }

//     TBtMultiBodyLink* TBodyCompound::lastLink()
//     {
//         if ( m_linksInChain.size () > 0 )
//             return m_linksInChain.back();

//         return NULL;
//     }

//     TMat4 TBodyCompound::firstToBaseTransform()
//     {
//         return m_firstLinkToBaseTransform;
//     }

//     TMat4 TBodyCompound::baseToFirstTransform()
//     {
//         return m_baseToFirstLinkTransform;
//     }

//     TMat4 TBodyCompound::lastToBaseTransform()
//     {
//         return m_lastLinkToBaseTransform;
//     }

//     TMat4 TBodyCompound::baseToLastTransform()
//     {
//         return m_baseToLastLinkTransform;
//     }

//     std::map< std::string, int > TBodyCompound::getJointsNamesToLinksIdMap()
//     {
//         return m_jointsNamesToLinksIdMap;
//     }

//     std::map< std::string, TScalar > TBodyCompound::getMasses()
//     {
//         return m_masses;
//     }

//     std::map< std::string, TVec3 > TBodyCompound::getInertiasDiags()
//     {
//         return m_inertiasDiags;
//     }

//     /***************************************************************************
//     *                   TBtKinTreeAgentWrapper Implementation                  *
//     ****************************************************************************/

//     TBtKinTreeAgentWrapper::TBtKinTreeAgentWrapper( TAgent* kinTreeAgentPtr,
//                                                     const std::string& workingDir )
//         : TAgentWrapper( kinTreeAgentPtr, workingDir )
//     {
//         m_btWorldPtr        = NULL;
//         m_btMultiBodyPtr    = NULL;
//         m_rootCompound      = NULL;
//         m_baseCompound      = NULL;
//         m_currentLinkIndx   = 0;
//     }

//     TBtKinTreeAgentWrapper::~TBtKinTreeAgentWrapper()
//     {
//         if ( m_baseCompound )
//             delete m_baseCompound;

//         for ( size_t q = 0; q < m_bodyCompoundsArray.size(); q++ )
//             delete m_bodyCompoundsArray[q];

//         m_btWorldPtr    = NULL;
//         m_rootCompound  = NULL;
//         m_baseCompound  = NULL;

//         m_bodyCompoundsMap.clear();
//         m_bodyCompoundsArray.clear();

//         // @TODO|@CHECK: Define the proper functionality for the destructor
//     }

//     void TBtKinTreeAgentWrapper::setBtWorld( btMultiBodyDynamicsWorld* btWorldPtr )
//     {
//         m_btWorldPtr = btWorldPtr;
//     }

//     void TBtKinTreeAgentWrapper::_initializeInternal()
//     {
//         _createBtResourcesFromKinTree();
//     }

//     void TBtKinTreeAgentWrapper::_resetInternal()
//     {
//         if ( !m_agentPtr )
//             return;

//         // clear all external-forces, constraints-forces and velocities prior to the reset
//         m_btMultiBodyPtr->clearForcesAndTorques();
//         m_btMultiBodyPtr->clearConstraintForces();
//         m_btMultiBodyPtr->clearVelocities();

//         // set the qpos values set by the user (left in the abstract agent)
//         for ( size_t i = 0; i < m_jointWrappers.size(); i++ )
//         {
//             // joint being wrapped
//             auto _jointPtr = m_jointWrappers[i].jointPtr();

//             // buffer for the q-values
//             std::vector< TScalar > _qposs;
//             std::vector< TScalar > _qvels;

//             if ( m_jointWrappers[i].isRootJoint() )
//             {
//                 if ( _jointPtr->type == "free" || _jointPtr->type == "ball" || 
//                      _jointPtr->type == "spheric" || _jointPtr->type == "spherical" )
//                 {
//                     auto _position = m_agentPtr->getStartPosition();
//                     auto _rotEuler = m_agentPtr->getStartRotation();
//                     auto _transform = tysoc::TMat4( _position, _rotEuler );

//                     // for this case, we must use the base directly :(
//                     m_btMultiBodyPtr->setBasePos( utils::toBtVec3( _position ) );
//                     m_btMultiBodyPtr->setBaseWorldTransform( utils::toBtTransform( _transform ) );

//                     m_btMultiBodyPtr->setBaseVel( utils::toBtVec3( { 0., 0., 0. } ) );
//                     m_btMultiBodyPtr->setBaseOmega( utils::toBtVec3( { 0., 0., 0. } ) );

//                     // skip setting the q-values for this joint
//                     continue;
//                 }
//                 else if ( _jointPtr->type == "slider" || _jointPtr->type == "slide" || _jointPtr->type == "prismatic" )
//                 {
//                     auto _position = m_agentPtr->getStartPosition();

//                     if ( _jointPtr->axis == TVec3( 1., 0., 0. ) )
//                     {
//                         _qposs = { _position.x };
//                         _qvels = { 0. };
//                     }
//                     else if ( _jointPtr->axis == TVec3( 0., 1., 0. ) )
//                     {
//                         _qposs = { _position.y };
//                         _qvels = { 0. };
//                     }
//                     else if ( _jointPtr->axis == TVec3( 0., 0., 1. ) )
//                     {
//                         _qposs = { _position.z };
//                         _qvels = { 0. };
//                     }
//                 }
//                 else if ( _jointPtr->type == "hinge" || _jointPtr->type == "revolute" || _jointPtr->type == "continuous" )
//                 {
//                     auto _rotation = m_agentPtr->getStartRotation();

//                     if ( _jointPtr->axis == TVec3( 1., 0., 0. ) )
//                     {
//                         _qposs = { _rotation.x };
//                         _qvels = { 0. };
//                     }
//                     else if ( _jointPtr->axis == TVec3( 0., 1., 0. ) )
//                     {
//                         _qposs = { _rotation.y };
//                         _qvels = { 0. };
//                     }
//                     else if ( _jointPtr->axis == TVec3( 0., 0., 1. ) )
//                     {
//                         _qposs = { _rotation.z };
//                         _qvels = { 0. };
//                     }
//                 }
//                 else
//                 {
//                     std::cout << "WARNING> reset functionality not implemented for joint of type: "
//                               << _jointPtr->type << ", found in joint: " << _jointPtr->name << std::endl;
//                 }
//             }
//             else
//             {
//                 // collect qpos from kintree-joint
//                 for ( int j = 0; j < _jointPtr->nqpos; j++ )
//                     _qposs.push_back( _jointPtr->qpos0[j] );

//                 // set qvels to zeros
//                 for ( int j = 0; j < _jointPtr->nqvel; j++ )
//                     _qvels.push_back( _jointPtr->qvel0[j] );
//             }

//             // and set the qposs and qvels into the backend through the wrapper
//             m_jointWrappers[i].setQpos( _qposs );
//             m_jointWrappers[i].setQvel( _qvels );

//             // @TODO: In case there're some motors, we should set their initial target position as well
//         }

//         // reset the internal high-level resources of the kintree
//         m_agentPtr->reset();
//     }

//     void TBtKinTreeAgentWrapper::_preStepInternal()
//     {
//         if ( !m_btMultiBodyPtr )
//             return;

//         // Clear all previous forces and torques
//         m_btMultiBodyPtr->clearForcesAndTorques();
//         m_btMultiBodyPtr->clearConstraintForces();

//         auto _kinActuators = m_agentPtr->actuators;

//         for ( size_t q = 0; q < _kinActuators.size(); q++ )
//         {
//             if ( !_kinActuators[q]->jointPtr )
//             {
//                 std::cout << "WARNING> actuator: " << _kinActuators[q]->name 
//                           << " has no joint attached" << std::endl;
//                 continue;
//             }

//             auto _jointName = _kinActuators[q]->jointPtr->name;
            
//             if ( m_jointToLinkIdMap.find( _jointName ) ==
//                  m_jointToLinkIdMap.end() )
//             {
//                 std::cout << "WARNING> joint: " << _jointName 
//                           << " not mapped to any link";
//                 continue;
//             }

//             auto _linkId = m_jointToLinkIdMap[_jointName];
//             auto _ctrlValue = _kinActuators[q]->gear.buff[0] * 
//                               _kinActuators[q]->ctrlValue;

//             // if ( _kinActuators[q]->ctrlValue > 0.25 || _kinActuators[q]->ctrlValue < -0.25 )
//             //     std::cout << "foooooooo" << std::endl;

//             for ( int dof = 0; dof < m_btMultiBodyPtr->getLink( _linkId ).m_dofCount; dof++ )
//             {
//                 // _ctrlValue -= 1. * m_btMultiBodyPtr->getJointVelMultiDof( _linkId )[dof];
//                 m_btMultiBodyPtr->addJointTorqueMultiDof( _linkId, dof, _ctrlValue );
//             }

//             // m_btMultiBodyPtr->addJointTorque( _linkId, _ctrlValue );
//             // m_btMultiBodyPtr->setJointPos( _linkId, _kinActuators[q]->ctrlValue );

//             // add to summary
//             TGenericParams& _summary = m_agentPtr->summary();
//             _summary.set( "act_" + _jointName, _ctrlValue );
//         }
//     }

//     void TBtKinTreeAgentWrapper::_postStepInternal()
//     {
//         // Update each body world transform from its compound wrapper
//         for ( size_t q = 0; q < m_bodyCompoundsArray.size(); q++ )
//             m_bodyCompoundsArray[q]->updateTransforms();

//         // Update the qpos values of the joints
//         auto _kinJoints = m_agentPtr->joints;
//         for ( size_t q = 0; q < _kinJoints.size(); q++ )
//         {
//             auto _jointName = _kinJoints[q]->name;
//             if ( m_jointToLinkIdMap.find( _jointName ) == m_jointToLinkIdMap.end() )
//                 continue;

//             if ( _kinJoints[q]->type != "revolute" && 
//                  _kinJoints[q]->type != "hinge" )
//                  continue;

//             auto _linkId = m_jointToLinkIdMap[_jointName];
//             _kinJoints[q]->qpos[0] =  m_btMultiBodyPtr->getJointPosMultiDof( _linkId )[0];
//         }
//     }

//     void TBtKinTreeAgentWrapper::_createBtResourcesFromKinTree()
//     {
//         if ( !m_agentPtr )
//             return;

//         // define some required data for the multibody creation
//         auto _numLinks      = utils::calculateNumOfLinksForMultibody( m_agentPtr );
//         auto _isBaseFixed   = utils::shouldBaseBeFixed( m_agentPtr );
//         auto _canSleep      = false;
//         auto _baseMass      = 0.f;
//         auto _baseInertia   = btVector3( 0., 0., 0. );

//         // Create the btMultiBody that will represent the simulated kintree
//         m_btMultiBodyPtr = new btMultiBody( _numLinks,
//                                             _baseMass,
//                                             _baseInertia,
//                                             _isBaseFixed,
//                                             _canSleep );

//         // create a SimMultibodyLink for the base
//         auto _bCollider = new btMultiBodyLinkCollider( m_btMultiBodyPtr, -1 );
//         _bCollider->setCollisionShape( utils::createCollisionShape( "none", { 0., 0., 0. } ) );
//         _bCollider->getWorldTransform().setOrigin( utils::toBtVec3( m_agentPtr->getPosition() ) );

//         m_btWorldPtr->addCollisionObject( _bCollider, 1, -1 );

//         m_btMultiBodyPtr->setBaseCollider( _bCollider );
//         m_btMultiBodyPtr->setBasePos( utils::toBtVec3( m_agentPtr->getPosition() ) );

//         // Create a dummy compound to represent the base (to propagate the starting position)
//         m_baseCompound = new TBodyCompound( NULL,
//                                             m_btMultiBodyPtr,
//                                             m_btWorldPtr,
//                                             NULL,
//                                             -1 );

//         // Recall we are using this dummy compound to propagate the starting ...
//         // position and orientation, so grab and set this info from the kintree
//         m_baseCompound->setWorldTransform( TMat4( m_agentPtr->getPosition(),
//                                                   m_agentPtr->getRotation() ) );

//         m_rootCompound = _createBodyCompoundFromBodyNode( m_agentPtr->getRootBody(), 
//                                                           m_baseCompound );

//         m_btMultiBodyPtr->finalizeMultiDof();
//         m_btMultiBodyPtr->setHasSelfCollision( true );
        
//         // m_btMultiBodyPtr->setLinearDamping( 0.1f );
//         // m_btMultiBodyPtr->setAngularDamping( 0.1f );

//         m_btWorldPtr->addMultiBody( m_btMultiBodyPtr );

// 		for (int i = 0; i < m_btWorldPtr->getNumMultiBodyConstraints(); i++)
// 		{
// 			m_btWorldPtr->getMultiBodyConstraint(i)->finalizeMultiDof();
// 		}

//         // assemble all jointsNames-linksIds mappings into one dictionary
//         for ( size_t q = 0; q < m_bodyCompoundsArray.size(); q++ )
//         {
//             auto _jointsNamesToLinksIdsMap = m_bodyCompoundsArray[q]->getJointsNamesToLinksIdMap();

//             // copy each entry in the dictionary into our global dictionary
//             for ( auto _it = _jointsNamesToLinksIdsMap.begin();
//                        _it != _jointsNamesToLinksIdsMap.end();
//                        _it++ )
//             {
//                 m_jointToLinkIdMap[_it->first] = _it->second;
//             }
//         }
//         utils::TBtLogger::log( "----------------------" );
//         utils::TBtLogger::log( std::string( "num-dofs: " ) + std::to_string( m_btMultiBodyPtr->getNumDofs() ) );
//         utils::TBtLogger::log( std::string( "num-qpos: " ) + std::to_string( m_btMultiBodyPtr->getNumPosVars() ) );
//         utils::TBtLogger::log( std::string( "num-links: " ) + std::to_string( m_btMultiBodyPtr->getNumLinks() ) );

//         utils::TBtLogger::log( "------------- MULTIBODY-INTERNALS ---------------" );
//         utils::TBtLogger::log( "------------- Links ---------------" );

//         _numLinks = m_btMultiBodyPtr->getNumLinks();
//         for ( int linkId = 0; linkId < _numLinks; linkId++ )
//         {
//             auto _linkCollider = m_btMultiBodyPtr->getLinkCollider( linkId );
//             auto _linkColliderType = _linkCollider->getInternalType();
//             auto _linkColliderShape = _linkCollider->getCollisionShape();
            
//             utils::TBtLogger::log( std::string( "collider-indx: " ) + std::to_string( linkId ) );
//             utils::TBtLogger::log( std::string( "collider-AnisotropicFriction: " ) + utils::to_string( _linkCollider->getAnisotropicFriction() ) );
//             utils::TBtLogger::log( std::string( "collider-CollisionFlags: " ) + std::to_string( _linkCollider->getCollisionFlags() ) );
//             utils::TBtLogger::log( std::string( "collider-ContactDamping: " ) + std::to_string( _linkCollider->getContactDamping() ) );
//             utils::TBtLogger::log( std::string( "collider-ContactStiffness: " ) + std::to_string( _linkCollider->getContactStiffness() ) );
//             utils::TBtLogger::log( std::string( "collider-Friction: " ) + std::to_string( _linkCollider->getFriction() ) );
//             utils::TBtLogger::log( std::string( "collider-Restitution: " ) + std::to_string( _linkCollider->getRestitution() ) );
//             utils::TBtLogger::log( std::string( "collider-RollingFriction: " ) + std::to_string( _linkCollider->getRollingFriction() ) );
//             utils::TBtLogger::log( std::string( "collider-SpinningFriction: " ) + std::to_string( _linkCollider->getSpinningFriction() ) );

//             if ( _linkColliderShape )
//                 utils::logShapeInfo( _linkColliderShape );
//         }

//         utils::TBtLogger::log( "\n\r\n\r" );


//         utils::TBtLogger::save(  "/home/gregor/Documents/repos/tysoc_bullet_workspace/tysoc_bullet/model_info.txt"  );
        
//         /* Generate summary information *******************************************/
//         TGenericParams& _summary = m_agentPtr->summary();

//         // collect inertia properties
//         TScalar _totalMass = 0.0f;
//         for ( size_t q = 0; q < m_bodyCompoundsArray.size(); q++ )
//         {
//             auto _masses = m_bodyCompoundsArray[q]->getMasses();
//             for ( auto _it = _masses.begin(); _it != _masses.end(); _it++ )
//             {
//                 _summary.set( "mass-" + _it->first, _it->second );
//                 _totalMass += _it->second;
//             }

//             auto _inertias = m_bodyCompoundsArray[q]->getInertiasDiags();
//             for ( auto _it = _inertias.begin(); _it != _inertias.end(); _it++ )
//                 _summary.set( "inertia-" + _it->first, _it->second );
//         }

//         _summary.set( "total-mass", _totalMass );

//         /**************************************************************************/
//     }

//     TBodyCompound* TBtKinTreeAgentWrapper::_createBodyCompoundFromBodyNode( TKinTreeBody* kinTreeBodyPtr,
//                                                                             TBodyCompound* parentBodyCompound )
//     {
//         auto _bodyCompound = new TBodyCompound( kinTreeBodyPtr,
//                                                 m_btMultiBodyPtr,
//                                                 m_btWorldPtr,
//                                                 parentBodyCompound,
//                                                 m_currentLinkIndx );

//         // save it in the storage structures appropriately
//         m_bodyCompoundsArray.push_back( _bodyCompound );
//         m_bodyCompoundsMap[ kinTreeBodyPtr->name ] = _bodyCompound;

//         // Be nice to other compounds and update the current link index
//         m_currentLinkIndx += _bodyCompound->getNumLinks();

//         auto _childBodies = kinTreeBodyPtr->childBodies;
//         for ( size_t q = 0; q < _childBodies.size(); q++ )
//         {
//             auto _childCompound = _createBodyCompoundFromBodyNode( _childBodies[q],
//                                                                    _bodyCompound );

//             _bodyCompound->addChild( _childCompound );
//         }

//         return _bodyCompound;
//     }

//     void TBtKinTreeAgentWrapper::finishedCreatingResources()
//     {
//         for ( size_t i = 0; i < m_agentPtr->bodies.size(); i++ )
//             _cacheBodyProperties( m_agentPtr->bodies[i] );

//         for ( size_t i = 0; i < m_agentPtr->joints.size(); i++ )
//             _cacheJointProperties( m_agentPtr->joints[i] );
//     }

//     void TBtKinTreeAgentWrapper::_cacheBodyProperties( TKinTreeBody* kinTreeBodyPtr )
//     {
//         // @TODO: Implement body wrappers as well, to lease the code-burden on the agent wrapper
//     }

//     void TBtKinTreeAgentWrapper::_cacheJointProperties( TKinTreeJoint* kinTreeJointPtr )
//     {
//         if ( m_jointToLinkIdMap.find( kinTreeJointPtr->name ) == m_jointToLinkIdMap.end() )
//             return;

//         int _linkIndx = m_jointToLinkIdMap[ kinTreeJointPtr->name ];
//         m_jointWrappers.push_back( TBtJointWrapper( m_btMultiBodyPtr, kinTreeJointPtr, _linkIndx ) );
//     }

//     extern "C" TAgentWrapper* agent_createFromAbstract( TAgent* kinTreeAgentPtr,
//                                                                const std::string& workingDir )
//     {
//         return new TBtKinTreeAgentWrapper( kinTreeAgentPtr, workingDir );
//     }

//     extern "C" TAgentWrapper* agent_createFromFile( const std::string& name,
//                                                            const std::string& filename,
//                                                            const std::string& workingDir )
//     {
//         // @WIP
//         return NULL;
//     }

//     extern "C" TAgentWrapper* agent_createFromId( const std::string& name,
//                                                          const std::string& format,
//                                                          const std::string& id,
//                                                          const std::string& workingDir )
//     {
//         // @WIP
//         return NULL;
//     }

    extern "C" TAgentWrapper* agent_createFromAbstract( TAgent* agentPtr )
    {
        return nullptr;
    }

}}