
#include <primitives/loco_single_body_collider_adapter_bullet.h>

namespace loco {
namespace primitives {

    TBulletSingleBodyColliderAdapter::TBulletSingleBodyColliderAdapter( TSingleBodyCollider* collision_ref )
        : TISingleBodyColliderAdapter( collision_ref )
    {
        LOCO_CORE_ASSERT( collision_ref, "TBulletSingleBodyColliderAdapter >>> expected non-null collision-obj reference" );

        m_BulletCollisionShape = nullptr;
        m_BulletRigidBodyRef = nullptr;
        m_BulletWorldRef = nullptr;

        m_Size  = m_ColliderRef->size();
        m_Size0 = m_ColliderRef->size();
        m_Scale = { 1.0f, 1.0f, 1.0f };

    #if defined( LOCO_CORE_USE_TRACK_ALLOCS )
        const std::string name = ( m_ColliderRef ) ? m_ColliderRef->name() : "undefined";
        if ( tinyutils::Logger::IsActive() )
            LOCO_CORE_TRACE( "Loco::Allocs: Created TBulletSingleBodyColliderAdapter {0} @ {1}", name, tinyutils::PointerToHexAddress( this ) );
        else
            std::cout << "Loco::Allocs: Created TBulletSingleBodyColliderAdapter " << name << " @ " << tinyutils::PointerToHexAddress( this ) << std::endl;
    #endif
    }

    TBulletSingleBodyColliderAdapter::~TBulletSingleBodyColliderAdapter()
    {
        m_BulletCollisionShape = nullptr;
        m_BulletRigidBodyRef = nullptr;
        m_BulletWorldRef = nullptr;

    #if defined( LOCO_CORE_USE_TRACK_ALLOCS )
        const std::string name = ( m_ColliderRef ) ? m_ColliderRef->name() : "undefined";
        if ( tinyutils::Logger::IsActive() )
            LOCO_CORE_TRACE( "Loco::Allocs: Destroyed TBulletSingleBodyColliderAdapter {0} @ {1}", name, tinyutils::PointerToHexAddress( this ) );
        else
            std::cout << "Loco::Allocs: Destroyed TBulletSingleBodyColliderAdapter " << name << " @ " << tinyutils::PointerToHexAddress( this ) << std::endl;
    #endif
    }

    void TBulletSingleBodyColliderAdapter::Build()
    {
        m_BulletCollisionShape = bullet::CreateCollisionShape( m_ColliderRef->data() );
        m_BulletCollisionShape->setMargin( 0.001 );

        m_Size  = m_ColliderRef->size();
        m_Size0 = m_ColliderRef->size();
        m_Scale = { 1.0f, 1.0f, 1.0f };
    }

    void TBulletSingleBodyColliderAdapter::Initialize()
    {
        LOCO_CORE_ASSERT( m_BulletCollisionShape, "TBulletSingleBodyColliderAdapter::Initialize >>> must have \
                          already created a valid bullet collision shape (missing ->Build call?)" );
    #if defined(LOCO_BULLET_DEBUG)
        LOCO_CORE_TRACE( "Bullet-backend >>> collision-adapter" );
        LOCO_CORE_TRACE( "\tname            : {0}", m_ColliderRef->name() );
        LOCO_CORE_TRACE( "\tcol-size        : {0}", ToString( m_ColliderRef->size() ) );
        LOCO_CORE_TRACE( "\tcol-shape       : {0}", ToString( m_ColliderRef->shape() ) );
        LOCO_CORE_TRACE( "\tcol-group       : {0}", std::to_string( m_ColliderRef->collisionGroup() ) );
        LOCO_CORE_TRACE( "\tcol-mask        : {0}", std::to_string( m_ColliderRef->collisionMask() ) );
        LOCO_CORE_TRACE( "\tbt-geom-shape   : {0}", bullet::bt_shape_enum_to_str( (BroadphaseNativeTypes)m_BulletCollisionShape->getShapeType() ) );
        LOCO_CORE_TRACE( "\tbt-local-scale  : {0}", ToString( bullet::vec3_from_bt( m_BulletCollisionShape->getLocalScaling() ) ) );
        LOCO_CORE_TRACE( "\tbt-margin       : {0}", m_BulletCollisionShape->getMargin() );
        btVector3 aabb_min, aabb_max;
        m_BulletCollisionShape->getAabb( btTransform::getIdentity(), aabb_min, aabb_max );
        btVector3 bsphere_center; btScalar bsphere_radius;
        m_BulletCollisionShape->getBoundingSphere( bsphere_center, bsphere_radius );
        LOCO_CORE_TRACE( "\tbt-aabb (dxyz)  : ({0},{1},{2})", aabb_max.x() - aabb_min.x(), aabb_max.y() - aabb_min.y(), aabb_max.z() - aabb_min.z() );
        LOCO_CORE_TRACE( "\tbt-bsphere-rad  : {0}", bsphere_radius );
        const int bt_shape_type = m_BulletCollisionShape->getShapeType();
        if ( bt_shape_type == COMPOUND_SHAPE_PROXYTYPE )
        {
            auto bt_compound_shape = dynamic_cast<btCompoundShape*>( m_BulletCollisionShape.get() );
            const int bt_num_children = bt_compound_shape->getNumChildShapes();
            LOCO_CORE_TRACE( "\tbt-num-children : {0}", bt_num_children );
            for ( size_t i = 0; i < bt_num_children; i++ )
            {
                LOCO_CORE_TRACE( "\tbt-child-tf     : \n{0}", ToString( bullet::mat4_from_bt( bt_compound_shape->getChildTransform( i ) ) ) );
                if ( auto bt_child_shape = bt_compound_shape->getChildShape( i ) )
                {
                    LOCO_CORE_TRACE( "\tbt-child-type   : {0}", bullet::bt_shape_enum_to_str( (BroadphaseNativeTypes)bt_child_shape->getShapeType() ) );
                    LOCO_CORE_TRACE( "\tbt-child-loc.scl: {0}", ToString( bullet::vec3_from_bt( bt_child_shape->getLocalScaling() ) ) );
                    LOCO_CORE_TRACE( "\tbt-child-margin : {0}", bt_child_shape->getMargin() );
                }

            }
        }
    #endif
        ChangeFriction( m_ColliderRef->data().friction.x() );
    }

    void TBulletSingleBodyColliderAdapter::ChangeSize( const TVec3& newSize )
    {
        // adapted from gazebo set-size functionality, similar to our drawable's resize using scaling: 
        //      url> https://bitbucket.org/osrf/gazebo/src/804410860234af97c5e309896dc007e8cde04ba8/gazebo/physics/bullet/BulletBoxShape.hh#lines-88
        //      comments> I'm not sure if this will work for capsules. For our drawables, when using scaling
        //                we ran into some issues as scaling deformed the capsule. Will have to check (@todo) if
        //                for all collision shapes scaling just increases base dimensions and everything else is
        //                computed accordingly (collision detection, etc.)

        if ( !m_BulletCollisionShape )
            return;

        m_Size = newSize;
        const eShapeType shape = m_ColliderRef->shape();
        switch ( shape )
        {
            case eShapeType::PLANE :
            {
                m_Scale.x() = m_Size.x() / m_Size0.x();
                m_Scale.y() = m_Size.y() / m_Size0.y();
                m_Scale.z() = 1.0f;
                break;
            }
            case eShapeType::BOX :
            {
                m_Scale.x() = m_Size.x() / m_Size0.x();
                m_Scale.y() = m_Size.y() / m_Size0.y();
                m_Scale.z() = m_Size.z() / m_Size0.z();
                break;
            }
            case eShapeType::SPHERE :
            {
                const auto scale = m_Size.x() / m_Size0.x();
                m_Scale.x() = scale;
                m_Scale.y() = scale;
                m_Scale.z() = scale;
                break;
            }
            case eShapeType::CYLINDER :
            {
                const auto scale_radius = m_Size.x() / m_Size0.x();
                const auto scale_height = m_Size.y() / m_Size0.y();
                m_Scale.x() = scale_radius;
                m_Scale.y() = scale_radius;
                m_Scale.z() = scale_height;
                break;
            }
            case eShapeType::CAPSULE :
            {
                const auto scale_radius = m_Size.x() / m_Size0.x();
                const auto scale_height = m_Size.y() / m_Size0.y();
                m_Scale.x() = scale_radius;
                m_Scale.y() = scale_radius;
                m_Scale.z() = scale_height;
                break;
            }
            case eShapeType::CONVEX_MESH :
            case eShapeType::TRIANGULAR_MESH :
            {
                m_Scale.x() = m_Size.x() / m_Size0.x();
                m_Scale.y() = m_Size.y() / m_Size0.y();
                m_Scale.z() = m_Size.z() / m_Size0.z();
                break;
            }
            case eShapeType::HEIGHTFIELD :
            {
                m_Scale.x() = m_Size.x() / m_Size0.x();
                m_Scale.y() = m_Size.y() / m_Size0.y();
                m_Scale.z() = m_Size.z() / m_Size0.z();
                break;
            }
        }

        m_BulletCollisionShape->setLocalScaling( bullet::vec3_to_bt( m_Scale ) );
    }

    void TBulletSingleBodyColliderAdapter::ChangeVertexData( const std::vector<float>& vertices, const std::vector<int>& faces )
    {
        if ( !m_BulletCollisionShape )
            return;


        if ( m_ColliderRef->shape() != eShapeType::CONVEX_MESH )
        {
            LOCO_CORE_WARN( "TBulletSingleBodyColliderAdapter::ChangeVertexData >>> tried to set vertex-data \
                             to the non-mesh collider {0}", m_ColliderRef->name() );
            return;
        }

        const ssize_t num_vertices = vertices.size() / 3;
        const ssize_t num_faces = faces.size() / 3;
        const bool recalc_local_aabb = false;
        const TVec3 scale = m_ColliderRef->data().size;

        if ( auto convex_hull_shape = dynamic_cast<btConvexHullShape*>( m_BulletCollisionShape.get() ) )
        {
            convex_hull_shape->reset();
            for ( ssize_t f = 0; f < num_faces; f++ )
                for ( ssize_t v = 0; v < 3; v++ )
                    convex_hull_shape->addPoint( btVector3( scale.x() * vertices[3 * faces[3 * f + v] + 0],
                                                            scale.y() * vertices[3 * faces[3 * f + v] + 1],
                                                            scale.z() * vertices[3 * faces[3 * f + v] + 2] ),
                                                 recalc_local_aabb );
            convex_hull_shape->recalcLocalAabb();
            convex_hull_shape->optimizeConvexHull();
        }
    }

    void TBulletSingleBodyColliderAdapter::ChangeElevationData( const std::vector<float>& heights )
    {
        // Nothing extra here. Adaptee already updates heights buffer using memcpy to keep the
        // pointer to heights-data for bullet (recall we have to maintain that pointer valid and alive)

        // @todo: Check if scale-z does the trick to update the max-height. If not, then would have
        //        to create a fork of bullet and make the required functionality for that case
    }

    void TBulletSingleBodyColliderAdapter::ChangeCollisionGroup( int collisionGroup )
    {
        if ( !m_BulletRigidBodyRef )
        {
            LOCO_CORE_ERROR( "TBulletSingleBodyColliderAdapter::ChangeCollisionGroup >>> collider {0} doesn't \
                              have a handle to its associated bullet rigid-body owner", m_ColliderRef->name() );
            return;
        }

        m_BulletRigidBodyRef->getBroadphaseHandle()->m_collisionFilterGroup = collisionGroup;
    }

    void TBulletSingleBodyColliderAdapter::ChangeCollisionMask( int collisionMask )
    {
        if ( !m_BulletRigidBodyRef )
        {
            LOCO_CORE_ERROR( "TBulletSingleBodyColliderAdapter::ChangeCollisionMask >>> collider {0} doesn't \
                              have a handle to its associated bullet rigid-body owner", m_ColliderRef->name() );
            return;
        }

        m_BulletRigidBodyRef->getBroadphaseHandle()->m_collisionFilterMask = collisionMask;
    }

    void TBulletSingleBodyColliderAdapter::ChangeFriction( const TScalar& friction )
    {
        if ( !m_BulletRigidBodyRef )
        {
            LOCO_CORE_ERROR( "TBulletSingleBodyColliderAdapter::ChangeFriction >>> collider {0} doesn't \
                              have a handle to its associated bullet rigid-body owner", m_ColliderRef->name() );
            return;
        }

        m_BulletRigidBodyRef->setFriction( friction );
    }

}}