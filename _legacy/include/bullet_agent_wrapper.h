#pragma once

#include <bullet_common.h>
#include <bullet_utils.h>

#include <agent_wrapper.h>

namespace tysoc {
namespace bullet {

//     /**
//     *   A wrapper to handle low-level control over the bullet 
//     *   internal generalized coordinates and dofs.
//     */
//     class TBtJointWrapper
//     {
//         private :

//         /* number of generalized coordnates available in this joint */
//         int m_nqpos;

//         /* number of "degrees of freedom" available in this joint */
//         int m_nqvel;

//         /* index to be used for referencing in the multibody */
//         int m_linkIndx;

//         /* A reference to the multibody ptr to handle sets-gets */
//         btMultiBody* m_btMultiBodyPtr;

//         /* kintree joint wrapped by this object */
//         TKinTreeJoint* m_kinTreeJointPtr;

//         public :

//         /* Constructs a wrapper for this joint, to handle low-level qpos-qvel sets */
//         TBtJointWrapper( btMultiBody* btMultiBodyPtr,
//                          TKinTreeJoint* jointPtr,
//                          int linkIndx );

//         /* Releases the references used by this wrapper */
//         ~TBtJointWrapper();

//         /* sets the generalized coordinates to some given values */
//         void setQpos( const std::vector< TScalar >& qpos );

//         /* sets the qvels (degrees of freedom) to some given values */
//         void setQvel( const std::vector< TScalar >& qvel );

//         /* returns a reference to the joint ptr being wrapped */
//         TKinTreeJoint* jointPtr() { return m_kinTreeJointPtr; }

//         /* returns whether or no this joint belongs to the root of the kintree */
//         bool isRootJoint();
//     };

//     /**
//     *   A wrapper for the links in the multibody that represents a ...
//     *   kinematic tree.
//     */
//     class TBtMultiBodyLink
//     {

//         private :

//         // Index of this link in the btMultiBody
//         int m_btLinkIndx;

//         // Reference to the parent link
//         TBtMultiBodyLink* m_parentLinkPtr;

//         // Reference to the btMultiBody it belongs to
//         btMultiBody* m_btMultiBodyPtr;

//         // Reference to the btMultiBodyDynamicsWorld
//         btMultiBodyDynamicsWorld* m_btWorldPtr;

//         // Bullet collision shape for this link
//         btCollisionShape* m_btCollisionShapePtr;

//         // Bullet Multibody collider
//         btMultiBodyLinkCollider* m_btLinkColliderPtr;

//         // Reference to an motor, or constraint (if any)
//         btMultiBodyJointMotor* m_btJointMotor;
//         btMultiBodySphericalJointMotor* m_btSphericalJointMotor;
//         btMultiBodyJointLimitConstraint* m_btJointLimitConstraint;

//         // density
//         TScalar m_density;
//         // friction
//         TSizef m_friction;
//         // armature
//         TScalar m_armature;

//         // mass and inertia, for summary
//         TScalar m_mass;
//         TVec3 m_inertiaDiag;

//         public :

//         TBtMultiBodyLink( int indx,
//                           TBtMultiBodyLink* parentLinkPtr,
//                           btMultiBody* btMultiBodyPtr,
//                           btMultiBodyDynamicsWorld* btWorldPtr );

//         ~TBtMultiBodyLink();

//         /**
//         *   Creates the link in the bullet multibody
//         */
//         void setupLinkInMultiBody( const std::string& shapeType,
//                                    const TVec3& shapeSize,
//                                    const TMat4& worldTransform,
//                                    const TMat4& localTransform,
//                                    const std::string& jointType,
//                                    const TVec3& jointAxis,
//                                    const TVec3& jointPivot,
//                                    float lowerLimit = 1.0f,
//                                    float upperLimit = -1.0f,
//                                    bool useMotor = false );

//         /**
//         *   Returns the transform in worldspace of this link, using the ...
//         *   btMultiBody reference for this computation.
//         */
//         TMat4 getWorldTransform();

//         /**
//         *   Returns the index of the wrapped btMultiBodyLink in the ... 
//         *   btMultiBody it belongs to.
//         */
//         int getIndx();

//         /**
//         *   Sets the density of this link (for mass computation)
//         */
//        void setDensity( const TScalar& density );

//         /**
//         *   Sets the friction of this link
//         */
//        void setFriction( const TSizef& friction );

//         /**
//         *   Sets the armature (extra value added to diagonal of inertia matrix).
//         *   This value is used to stabilize the simulation.
//         */
//        void setArmature( const TScalar& armature );

//         /* Checks to see if this link either has or hasn't some resources */
//         bool hasLimitConstraint() { return m_btJointLimitConstraint != NULL; }
//         bool hasJointMotor() { return m_btJointMotor != NULL; }
//         bool hasSphericalJointMotor() { return m_btSphericalJointMotor != NULL; }
//         bool hasAnyMotors() { return ( hasJointMotor() || hasSphericalJointMotor() ); }

//         /* Getters for some requested resources */
//         btMultiBodyJointLimitConstraint* getLimitConstraint() { return m_btJointLimitConstraint; }
//         btMultiBodyJointMotor* getJointMotor() { return m_btJointMotor; }
//         btMultiBodySphericalJointMotor* getSphericalJointMotor() { return m_btSphericalJointMotor; }

//         /* Some more getters for inertial properties */
//         TScalar mass() { return m_mass; }
//         TVec3 inertiDiag() { return m_inertiaDiag; }
//     };

//     /* 
//     *   This object encloses all collisions and fixed-constraints used while ...
//     *   creating the resources from a kintree body node. Because the dynamics ...
//     *   properties are not recomputed correctly (another approach would be to ...
//     *   take into account each collision body, which actually corresponds to a ...
//     *   dynamics body (if no inertia is given) and compensate the inertial frame ...
//     *   of the actual (and only one) btbody with this correction terms )
//     *
//     *   For this approach we just create a single btMultiBodyLink for each kintree ...
//     *   collision object we have and chain them with fixed constraints sequentially.
//     *
//     *   The DOFs in the body that represents the whole compound are used in the ...
//     *   root of this chain
//     */
//     class TBodyCompound
//     {
//         private :

//         // A reference to the actual kintree body node (that contains all required info)
//         TKinTreeBody* m_kinTreeBodyPtr;

//         // A reference to the btMultiBody that represents the kintree
//         btMultiBody* m_btMultiBodyPtr;

//         // Reference to the btMultiBodyDynamicsWorld
//         btMultiBodyDynamicsWorld* m_btWorldPtr;

//         // Children compounds that this body might contain
//         std::vector< TBodyCompound* > m_children;

//         // A reference to the parent compound (if any)
//         TBodyCompound* m_parent;

//         // Link index to start in the multibody
//         int m_linkBaseIndx;

//         // Link index to the parent-link in the multibody
//         int m_parentLinkIndx;

//         // Whether or not this compound has multidof
//         bool m_hasMultidof;

//         // bodies used for each collision (i) in { 1 ... nb }
//         std::vector< TBtMultiBodyLink* > m_linksInChain;

//         // transforms for each body in the chain, wrt the previous body in the same chain
//         std::vector< TMat4 > m_relTransformsInChain;

//         // the transform from the 'kintree body frame' to the 'start link', fixed w.r.t body frame
//         TMat4 m_firstLinkToBaseTransform;

//         // and the inverse of that, used every simulation step for some calculations
//         TMat4 m_baseToFirstLinkTransform;

//         // the transform from the 'kintree body frame' to the 'end link', fixed w.r.t body frame
//         TMat4 m_lastLinkToBaseTransform;

//         // and the inverse of that, used every simulation step for some calculations
//         TMat4 m_baseToLastLinkTransform;

//         // the world transform of this compound
//         TMat4 m_worldTransform;

//         // A dictionary to map joints (dofs) to its link ids in the multibody
//         std::map< std::string, int > m_jointsNamesToLinksIdMap;

//         // A dictionary to keep the masses for each link (if non-zero)
//         std::map< std::string, TScalar > m_masses;

//         // A dictionary to keep inertia vector (diagonal) for each link (if non-zero)
//         std::map< std::string, TVec3 > m_inertiasDiags;

//         /**
//         *   Initializes the compound by creating all links in the btMultiBody ...
//         *   that will be required, according to the collisions and joints that ...
//         *   the given kinTreeBodyPtr contains
//         */
//         void _constructLinksInCompound();

//         public :

//         TBodyCompound( TKinTreeBody* kinTreeBodyPtr,
//                        btMultiBody* btMultiBodyPtr,
//                        btMultiBodyDynamicsWorld* btWorldPtr,
//                        TBodyCompound* parent,
//                        int linkIndx );

//         ~TBodyCompound();

//         /**
//         *   Adds a child compound to this compound
//         */
//         void addChild( TBodyCompound* childCompound );

//         /**
//         *   Returns all children compounds of this compound
//         */
//         std::vector< TBodyCompound* > children();

//         /**
//         *   Sets the transform of this compound in world-space
//         */
//         void setWorldTransform( const TMat4& transform );

//         /**
//         *   Gets the transform of this compound in world-space
//         */
//         TMat4 getWorldTransform();

//         /**
//         *   Updates the world-transform of the wrapped kinTreeBody based on ...
//         *   the world-transform of the starting link in the compound
//         */
//         void updateTransforms();

//         /**
//         *   Gets the number of links (in the btMultiBody) that this compound ...
//         *   handles (including dummies and non-dummies)
//         */
//         size_t getNumLinks();

//         /**
//         *   Gets the first link in the chain of this compound
//         */
//         TBtMultiBodyLink* firstLink();

//         /**
//         *   Gets the last link in the chain of this compound
//         */
//         TBtMultiBodyLink* lastLink();

//         /**
//         *   Gets the relative transform of the first link of the compound ...
//         *   w.r.t. the reference frame of this compound (kintree-body's frame)
//         */
//         TMat4 firstToBaseTransform();

//         /**
//         *   Gets the relative transform of this compound's reference frame ...
//         *   (kintree-body's frame) w.r.t. the first link in the compound
//         */
//         TMat4 baseToFirstTransform();

//         /**
//         *   Gets the relative transform of the last link of the compound ...
//         *   w.r.t. the reference frame of this compound (kintree-body's frame)
//         */
//         TMat4 lastToBaseTransform();

//         /**
//         *   Gets the relative transform of this compound's reference frame ...
//         *   (kintree-body's frame) w.r.t. the last link in the compound
//         */
//         TMat4 baseToLastTransform();

//         /**
//         *   Gets the mapping from joints names to link-ids
//         */
//         std::map< std::string, int > getJointsNamesToLinksIdMap();

//         /**
//         *   Gets the masses dictionary (link-name to mass)
//         */
//        std::map< std::string, TScalar > getMasses();

//         /**
//         *   Gets the inertiasDiags dictionary (link-name to inertiaDiag)
//         */
//        std::map< std::string, TVec3 > getInertiasDiags();
//     };

//     class TBtKinTreeAgentWrapper : public TAgentWrapper
//     {

//         private :

//         // A reference to the world to add our resources to
//         btMultiBodyDynamicsWorld* m_btWorldPtr;

//         // A bullet multibody object that represents this kintree
//         btMultiBody* m_btMultiBodyPtr;

//         // The root compound for this kintree (recall body-node <> body-compound)
//         TBodyCompound* m_rootCompound;

//         // A dummy compound used as the base (to propagate the start transform)
//         TBodyCompound* m_baseCompound;

//         // A storage for later access to each compound (by name)
//         std::map< std::string, TBodyCompound* > m_bodyCompoundsMap;

//         // A storage for later access to each compound (by index)
//         std::vector< TBodyCompound* > m_bodyCompoundsArray;

//         // A dictionary to map joints (dofs) to its link ids in the multibody
//         std::map< std::string, int > m_jointToLinkIdMap;

//         // A counter for the current link index
//         int m_currentLinkIndx;

//         /* Joint wrappers used for low-level access to joint internals in bullet */
//         std::vector< TBtJointWrapper > m_jointWrappers;

//         /**
//         *   Start the creation process for all resources required by the ...
//         *   kintree to be wrapped.
//         */
//         void _createBtResourcesFromKinTree();

//         /**
//         *   Creates a body compound out of a given kintree body object
//         *
//         *   @param kinTreeBodyPtr       body node from the core kintree
//         *   @param parentBodyCompound   compound corresponding to the parent body
//         */
//         TBodyCompound* _createBodyCompoundFromBodyNode( TKinTreeBody* kinTreeBodyPtr,
//                                                         TBodyCompound* parentBodyCompound );

//         /* Creates a wrapper for a body, used later for low-level access */
//         void _cacheBodyProperties( TKinTreeBody* kinTreeBodyPtr );

//         /* Creates a wrapper for a joint, used later for low-leve access */
//         void _cacheJointProperties( TKinTreeJoint* kinTreeJointPtr );

//         protected :

//         void _initializeInternal() override;
//         void _resetInternal() override;
//         void _preStepInternal() override;
//         void _postStepInternal() override;

//         public :

//         TBtKinTreeAgentWrapper( TAgent* kinTreeAgentPtr,
//                                 const std::string& workingDir );
//         ~TBtKinTreeAgentWrapper();

//         void setBtWorld( btMultiBodyDynamicsWorld* btWorldPtr );

//         void finishedCreatingResources();
//     };

//     extern "C" TAgentWrapper* agent_createFromAbstract( TAgent* kinTreeAgentPtr,
//                                                                const std::string& workingDir );

//     extern "C" TAgentWrapper* agent_createFromFile( const std::string& name,
//                                                            const std::string& filename,
//                                                            const std::string& workingDir );

//     extern "C" TAgentWrapper* agent_createFromId( const std::string& name,
//                                                          const std::string& format,
//                                                          const std::string& id,
//                                                          const std::string& workingDir );

    extern "C" TAgentWrapper* agent_createFromAbstract( TAgent* agentPtr );

}}