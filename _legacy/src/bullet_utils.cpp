
#include <bullet_utils.h>

namespace tysoc {
namespace bullet {
namespace utils {


    btVector3 toBtVec3( const TVec3& vec )
    {
        return btVector3( vec.x, vec.y, vec.z );
    }

    TVec3 fromBtVec3( const btVector3& vec )
    {
        return TVec3( vec.x(), vec.y(), vec.z() );
    }

    btMatrix3x3 toBtMat3( const TMat3& mat )
    {
        btMatrix3x3 _res;

        for ( size_t i = 0; i < 3; i++ )
            for ( size_t j = 0; j < 3; j++ )
                _res[i][j] = mat.buff[i + 3 * j];

        return _res;
    }

    TMat3 fromBtMat3( const btMatrix3x3& mat )
    {
        TMat3 _res;

        for ( size_t i = 0; i < 3; i++ )
            for ( size_t j = 0; j < 3; j++ )
                _res.buff[i + 3 * j] = mat[i][j];

        return _res;
    }

    TMat4 fromBtTransform( const btTransform& tf )
    {
        auto _pos = fromBtVec3( tf.getOrigin() );
        auto _rot = fromBtMat3( tf.getBasis() );

        return TMat4::fromPositionAndRotation( _pos, _rot );
    }

    btTransform toBtTransform( const TMat4& mat )
    {
        auto _origin = toBtVec3( mat.getPosition() );
        auto _basis = toBtMat3( mat.getRotation() );

        return btTransform( _basis, _origin );
    }

    btCollisionShape* createCollisionShape( const std::string& type, const TVec3& size )
    {
        btCollisionShape* _colShape = nullptr;

        if ( type == "box" )
            _colShape = new btBoxShape( btVector3( 0.5 * size.x, 0.5 * size.y, 0.5 * size.z ) );
        else if ( type == "sphere" )
            _colShape = new btSphereShape( size.x );
        else if ( type == "capsule" || type == "capsulez" )
            _colShape = new btCapsuleShapeZ( size.x, size.y );
        else if ( type == "capsulex" )
            _colShape = new btCapsuleShapeX( size.x, size.y );
        else if ( type == "capsuley" )
            _colShape = new btCapsuleShape( size.x, size.y );
        else if ( type == "cylinder" )
            _colShape = new btCylinderShapeZ( btVector3( size.x, size.x, size.y ) );
        else if ( type == "none" )
            _colShape = new btCompoundShape();
        else
            TYSOC_CORE_ERROR( "Shape {0} not supported yet", type );

        if ( !_colShape )
            return nullptr;

        _colShape->setMargin( 0.0f );

        return _colShape;
    }

    btCollisionShape* createCollisionShape( const TShapeData& data )
    {
        btCollisionShape* _colShape = nullptr;

        if ( data.type == eShapeType::BOX )
        {
            _colShape = new btBoxShape( btVector3( 0.5 * data.size.x,       // half-width
                                                   0.5 * data.size.y,       // half-depth
                                                   0.5 * data.size.z ) );   // half-height
        }
        else if ( data.type == eShapeType::SPHERE )
        {
            _colShape = new btSphereShape( data.size.x ); // sphere-radius
        }
        else if ( data.type == eShapeType::CAPSULE )
        {
            _colShape = new btCapsuleShapeZ( data.size.x,   // capsule-radius
                                             data.size.y ); // capsule-height (cylindrical part)
        }
        else if ( data.type == eShapeType::CYLINDER )
        {
            _colShape = new btCylinderShapeZ( btVector3( data.size.x,           // cylinder half-x extent (radius)
                                                         data.size.x,           // cylinder half-y extent (radius)
                                                         0.5 * data.size.y ) ); // cylinder half-z extent (half-height)
        }
        else if ( data.type == eShapeType::ELLIPSOID )
        {
            std::vector< TVec3 > _vertices; 
            _createEllipsoidVertices( data, _vertices );

            /* create a convex-hull shape, with vertices from the ellipsoid data created above */
            auto _convexHullShape = new btConvexHullShape();
            for ( auto _vertex : _vertices )
                _convexHullShape->addPoint( btVector3( _vertex.x, _vertex.y, _vertex.z ), false );

            _convexHullShape->recalcLocalAabb();
            _convexHullShape->optimizeConvexHull();

            _colShape = _convexHullShape;
        }
        else if ( data.type == eShapeType::MESH )
        {
            std::vector< TVec3 > _vertices;
            _createMeshVertices( data, _vertices );

            if ( _vertices.size() < 1 )
                return nullptr;

            /* create a convex-hull shape, with vertices scaled by data.size (scale factors) */
            auto _convexHullShape = new btConvexHullShape();
            for ( auto _vertex : _vertices )
                _convexHullShape->addPoint( btVector3( data.size.x * _vertex.x,
                                                       data.size.y * _vertex.y,
                                                       data.size.z * _vertex.z ), false );

            _convexHullShape->recalcLocalAabb();
            _convexHullShape->optimizeConvexHull();

            _colShape = _convexHullShape;
        }
        else if ( data.type == eShapeType::HFIELD )
        {
            const int _upAxis = 2; // up = z-axis
            // compute max-height (required to compute aabb)
            float _zMax = *std::max_element( data.hdata.heightData.begin(), data.hdata.heightData.end() );
            //// TYSOC_CORE_TRACE( "zmax: {0}, zscale: {1}", _zMax, data.size.z );
            _colShape = new btHeightfieldTerrainShape( data.hdata.nWidthSamples,
                                                       data.hdata.nDepthSamples,
                                                       (void*)data.hdata.heightData.data(), 
                                                       btScalar( 1.0f ),
                                                       btScalar( 0.0f ),
                                                       btScalar( _zMax * data.size.z ), // max-z (for aabb) is z-max * z-scale
                                                       _upAxis,
                                                       PHY_FLOAT,
                                                       false );
            _colShape->setLocalScaling( toBtVec3( { data.size.x / ( data.hdata.nWidthSamples - 1 ),
                                                    data.size.y / ( data.hdata.nDepthSamples - 1 ),
                                                    data.size.z } ) );
            dynamic_cast< btHeightfieldTerrainShape* >( _colShape )->setFlipTriangleWinding( true );
        }
        else if ( data.type == eShapeType::NONE )
        {
            _colShape = new btCompoundShape();
        }
        else
        {
            TYSOC_CORE_ERROR( "Bullet-utils >>> Shape {0} is not supported", data.type );
        }

        if ( _colShape )
            _colShape->setMargin( 0.001f );

        return _colShape;
    }

    void _createEllipsoidVertices( const TShapeData& data, std::vector< TVec3 >& vertices )
    {

    }

    void _createMeshVertices( const TShapeData& data, std::vector< TVec3 >& vertices )
    {
        auto _assimpScenePtr = aiImportFile( data.filename.c_str(),
                                             aiProcessPreset_TargetRealtime_MaxQuality |
                                             aiProcess_PreTransformVertices );

        if ( !_assimpScenePtr )
        {
            TYSOC_CORE_ERROR( "Bullet-utils >>> Couldn't load mesh from path {0}", data.filename );
            return;
        }

        // recursively copy the data from assimp to our buffer
        _processAssimpNode( _assimpScenePtr, _assimpScenePtr->mRootNode, vertices );

        // make sure we release the assimp resources
        aiReleaseImport( _assimpScenePtr );
    }

    void _processAssimpNode( const aiScene* assimpScenePtr,
                             aiNode* assimpNodePtr,
                             std::vector< TVec3 >& vertices )
    {
        for ( size_t i = 0; i < assimpNodePtr->mNumMeshes; i++ )
        {
            aiMesh* _assimpMeshPtr = assimpScenePtr->mMeshes[ assimpNodePtr->mMeshes[i] ];
            _processAssimpSubmesh( _assimpMeshPtr, vertices );
        }

        for ( size_t i = 0; i < assimpNodePtr->mNumChildren; i++ )
        {
            _processAssimpNode( assimpScenePtr,
                                assimpNodePtr->mChildren[i], 
                                vertices );
        }
    }

    void _processAssimpSubmesh( aiMesh* assimpMeshPtr, std::vector< TVec3 >& vertices )
    {
        // grab all vertices, in spite of the ordering (in faces-tris, quads, ...)
        for ( size_t i = 0; i < assimpMeshPtr->mNumVertices; i++ )
        {
            vertices.push_back( { assimpMeshPtr->mVertices[i].x,
                                  assimpMeshPtr->mVertices[i].y,
                                  assimpMeshPtr->mVertices[i].z } );
        }
    }

    btScalar computeVolumeFromShape( btCollisionShape* colShape )
    {
        if ( !colShape )
        {
            TYSOC_CORE_ERROR( "No collision shape provided for mass calculation, returning 0.0" );
            return btScalar( 0.0f );
        }

        int _type = colShape->getShapeType();

        btScalar _volume;
        btScalar _pi = 3.141592653589793;

        if ( _type == BOX_SHAPE_PROXYTYPE )
        {
            auto _boxShape = reinterpret_cast< btBoxShape* >( colShape );
            auto _boxDimensions = _boxShape->getHalfExtentsWithMargin();

            _volume = 8. * ( _boxDimensions.x() * _boxDimensions.y() * _boxDimensions.z() );
        }
        else if ( _type == SPHERE_SHAPE_PROXYTYPE )
        {
            auto _sphereShape = reinterpret_cast< btSphereShape* >( colShape );
            auto _sphereRadius = _sphereShape->getRadius();

            _volume = (4. / 3.) * _pi * ( _sphereRadius * _sphereRadius * _sphereRadius );
        }
        else if ( _type == CYLINDER_SHAPE_PROXYTYPE )
        {
            auto _cylinderShape = reinterpret_cast< btCylinderShape* >( colShape );
            auto _cylinderUpAxis = _cylinderShape->getUpAxis();
            auto _cylinderRadius = _cylinderShape->getRadius();
            auto _cylinderHalfExtents = _cylinderShape->getHalfExtentsWithoutMargin();
            auto _cylinderHeight = 2.0 * _cylinderHalfExtents[_cylinderUpAxis];

            _volume = ( _pi * _cylinderRadius * _cylinderRadius ) * _cylinderHeight;
        }
        else if ( _type == CAPSULE_SHAPE_PROXYTYPE )
        {
            auto _capsuleShape = reinterpret_cast< btCapsuleShape* >( colShape );
            auto _capsuleUpAxis = _capsuleShape->getUpAxis();
            auto _capsuleRadius = _capsuleShape->getRadius();
            auto _capsuleHeight = 2.0 * _capsuleShape->getHalfHeight();

            _volume = ( _pi * _capsuleRadius * _capsuleRadius ) * _capsuleHeight +
                      (4. / 3.) * ( _pi * _capsuleRadius * _capsuleRadius * _capsuleRadius );
        }
        else
        {
            // compute from aabb
            btVector3 _aabbMin, _aabbMax;

            btTransform _identityTransform;
            _identityTransform.setIdentity();

            colShape->getAabb( _identityTransform, _aabbMin, _aabbMax );

            _volume = btFabs( ( _aabbMax.x() - _aabbMin.x() ) * 
                              ( _aabbMax.y() - _aabbMin.y() ) *
                              ( _aabbMax.z() - _aabbMin.z() ) );
        }

        return _volume;
    }

    size_t calculateNumOfLinksForMultibody( TAgent* kinTreePtr )
    {
        size_t _numLinks = 0;

        std::stack< TKinTreeBody* > _bodiesToConsider;
        _bodiesToConsider.push( kinTreePtr->getRootBody() );

        while ( !_bodiesToConsider.empty() )
        {
            auto _currentBodyPtr = _bodiesToConsider.top();
            _bodiesToConsider.pop();

            // check how many dofs does this body has. Ball and spherical are ...
            // considered as 1, as the setup(TypeOfConstraint) considers ball ...
            // as only one constraint
            auto _numJoints = _currentBodyPtr->joints.size();
            if ( _numJoints > 1 )
            {
                // MultiDof case, so we have to use dummies. We are using ...
                // 1 dummy per joint, and then connect to the actual collider ...
                // via a fixed constraint
                _numLinks += _numJoints;
            }
            else
            {
                // For any other case, this single joint (or none at all, which ...
                // makes it fixed) is taken into account by the links created ...
                // by the colliders
                _numLinks += 0;
            }

            // add as many links as colliders we have
            _numLinks += _currentBodyPtr->collisions.size();

            for ( size_t q = 0; q < _currentBodyPtr->children.size(); q++ )
                _bodiesToConsider.push( _currentBodyPtr->children[q] );
        }

        return _numLinks;
    }

    bool shouldBaseBeFixed( TAgent* kinTreePtr )
    {
        auto _joints = kinTreePtr->getRootBody()->joints;

        if ( _joints.size() == 0 )
        {
            // the root body has no dofs, so just fix the base
            return true;
        }
        else if ( _joints.size() == 1 &&
                 _joints[0]->data.type == eJointType::FREE )
        {
            // for free joints (full 6dofs) just leave the base free
            return false;
        }

        // all other cases have limited dofs, so the next body to the base ...
        // configures the required dofs
        return true;
    }

    void logShapeInfo( btCollisionShape* colShape, bool isChild )
    {
        if( !isChild )
            TBtLogger::log( "---------------------------------------------------" );
        else
            TBtLogger::log( "********** child **********" );
    
        auto _colliderMargin = colShape->getMargin();
        TBtLogger::log( std::string( "collision-margin: " ) + std::to_string( _colliderMargin ) );
    
        if ( colShape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE )
        {
            TBtLogger::log( "colshape-type: compound" );
    
            // recurse over its children
            auto _colShapeCompound = reinterpret_cast< btCompoundShape* >( colShape );
            auto _numChildren = _colShapeCompound->getNumChildShapes();
    
            for ( int i = 0; i < _numChildren; i++ )
            {
                TBtLogger::log( std::string( "child-transform: " ) + utils::to_string( _colShapeCompound->getChildTransform( i ) ) );
                logShapeInfo( _colShapeCompound->getChildShape( i ), true );
            }
        }
        else if ( colShape->getShapeType() == BOX_SHAPE_PROXYTYPE )
        {
            TBtLogger::log( "colshape-type: box" );
    
            auto _colShapeBox = reinterpret_cast< btBoxShape* >( colShape );
            auto _boxExtents = _colShapeBox->getHalfExtentsWithoutMargin() * 2.0f;
    
            TBtLogger::log( std::string( "box-extents: " ) + utils::to_string( _boxExtents ) );
    
        }
        else if ( colShape->getShapeType() == SPHERE_SHAPE_PROXYTYPE )
        {
            TBtLogger::log( "colshape-type: sphere" );
    
            auto _colShapeSphere = reinterpret_cast< btSphereShape* >( colShape );
            auto _sphereRadius = _colShapeSphere->getRadius();
    
            TBtLogger::log( std::string( "sphere-radius: " ) + std::to_string( _sphereRadius ) );
        }
        else if ( colShape->getShapeType() == CAPSULE_SHAPE_PROXYTYPE )
        {
            TBtLogger::log( "colshape-type: capsule" );
    
            auto _colShapeCapsule = reinterpret_cast< btCapsuleShape* >( colShape );
            auto _capsuleRadius = _colShapeCapsule->getRadius();
            auto _capsuleHeight = _colShapeCapsule->getHalfHeight() * 2.0f;
    
            TBtLogger::log( std::string( "capsule-radius: " ) + std::to_string( _capsuleRadius ) );
            TBtLogger::log( std::string( "capsule-height: " ) + std::to_string( _capsuleHeight ) );
        }
    }

    /**********************************************************
    *          Singleton File-Logger functionality            *
    ***********************************************************/

    TBtLogger* TBtLogger::_instance = nullptr;

    void TBtLogger::log( const std::string& message )
    {
        if ( !TBtLogger::_instance )
            TBtLogger::_instance = new TBtLogger();

        TBtLogger::_instance->_logInternal( message );
    }

    void TBtLogger::save( const std::string& filename )
    {
        if ( !TBtLogger::_instance )
        {
            TYSOC_CORE_WARN( "There's no file-logger to use yet" );
            return;
        }

        TBtLogger::_instance->_saveInternal( filename );
    }

    void TBtLogger::_logInternal( const std::string& message )
    {
        m_logs.push_back( message );
    }

    void TBtLogger::_saveInternal( const std::string& filename )
    {
        std::ofstream _fileHandle;
        _fileHandle.open( filename.c_str(), std::ofstream::out );

        for ( size_t i = 0; i < m_logs.size(); i++ )
            _fileHandle << m_logs[i] << std::endl;

        _fileHandle.close();
    }

    /**********************************************************
    *               Some to-strings for logging               *
    ***********************************************************/

    std::string to_string( const btVector3& vec3 )
    {
        std::string _res;

        _res += "(";
        _res += std::to_string( vec3.x() ); _res += std::string( ", " );
        _res += std::to_string( vec3.y() ); _res += std::string( ", " );
        _res += std::to_string( vec3.z() );
        _res += ")";

        return _res;
    }

    std::string to_string( const btQuaternion& quat )
    {
        std::string _res;
        
        _res += "(";
        _res += std::to_string( quat.x() ); _res += std::string( ", " );
        _res += std::to_string( quat.y() ); _res += std::string( ", " );
        _res += std::to_string( quat.z() ); _res += std::string( ", " );
        _res += std::to_string( quat.w() );
        _res += ")";

        return _res;
    }

    std::string to_string( const btMatrix3x3& mat3 )
    {
        std::string _res;
        
        _res += "(";
        _res += std::to_string( mat3[0].x() ); _res += std::string( ", " );
        _res += std::to_string( mat3[0].y() ); _res += std::string( ", " );
        _res += std::to_string( mat3[0].z() ); _res += std::string( ", " );

        _res += std::to_string( mat3[1].x() ); _res += std::string( ", " );
        _res += std::to_string( mat3[1].y() ); _res += std::string( ", " );
        _res += std::to_string( mat3[1].z() ); _res += std::string( ", " );

        _res += std::to_string( mat3[2].x() ); _res += std::string( ", " );
        _res += std::to_string( mat3[2].y() ); _res += std::string( ", " );
        _res += std::to_string( mat3[2].z() );

        _res += ")";

        return _res;
    }

    std::string to_string( const btTransform& transform )
    {
        std::string _res;
        
        auto _pos = transform.getOrigin();
        auto _quat = transform.getRotation();

        _res += "(";
        _res += std::to_string( _pos.x() ); _res += std::string( ", " );
        _res += std::to_string( _pos.y() ); _res += std::string( ", " );
        _res += std::to_string( _pos.z() ); _res += std::string( ", " );

        _res += std::to_string( _pos.x() ); _res += std::string( ", " );
        _res += std::to_string( _pos.y() ); _res += std::string( ", " );
        _res += std::to_string( _pos.z() ); _res += std::string( ", " );
        _res += std::to_string( _pos.w() );

        _res += ")";

        return _res;
    }   

    /**********************************************************
    *                       Debug drawer                      *
    ***********************************************************/

    TBtDebugDrawer::TBtDebugDrawer()
    {
        m_visualizerPtr = nullptr;

        // m_debugMode = btIDebugDraw::DBG_DrawWireframe | 
        //               btIDebugDraw::DBG_DrawAabb |
        //               btIDebugDraw::DBG_DrawFrames |
        //               btIDebugDraw::DBG_DrawConstraints;

        m_debugMode = btIDebugDraw::DBG_DrawWireframe | 
                      btIDebugDraw::DBG_DrawConstraints;

        // m_debugMode = btIDebugDraw::DBG_DrawWireframe;
    }

    TBtDebugDrawer::~TBtDebugDrawer()
    {
        m_visualizerPtr = nullptr;
    }

    void TBtDebugDrawer::setVisualizer( TIVisualizer* visualizerPtr )
    {
        m_visualizerPtr = visualizerPtr;
    }

    void TBtDebugDrawer::drawLine( const btVector3& from, const btVector3& to, const btVector3& color )
    {
        if ( !m_visualizerPtr )
            return;

        m_visualizerPtr->drawLine( fromBtVec3( from ), fromBtVec3( to ), fromBtVec3( color ) );
    }

    void TBtDebugDrawer::drawContactPoint( const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color )
    {
        if ( !m_visualizerPtr )
            return;

        m_visualizerPtr->drawLine( fromBtVec3( PointOnB ), 
                                   fromBtVec3( PointOnB + normalOnB * distance ), 
                                   fromBtVec3( color ) );

        m_visualizerPtr->drawLine( fromBtVec3( PointOnB ), 
                                   fromBtVec3( PointOnB + normalOnB * 0.01 ), 
                                   { 0.2, 0.4, 0.5 } );
    }

    void TBtDebugDrawer::reportErrorWarning( const char* warningString )
    {
        TYSOC_CORE_ERROR( "BtDebugDrawer says: {0}", warningString );
    }

    void TBtDebugDrawer::draw3dText( const btVector3& location, const char* textString )
    {
        // do nothing
    }

    void TBtDebugDrawer::setDebugMode( int debugMode )
    {
        m_debugMode = debugMode;
    }

    int TBtDebugDrawer::getDebugMode() const
    {
        return m_debugMode;
    }

    /**********************************************************
    *                Custom filter callback                   *
    ***********************************************************/

    bool TBtOverlapFilterCallback::needBroadphaseCollision( btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1 ) const 
    {
        // TYSOC_LOG( "Testing broadphase collision checking" );

        bool _proxy0Affinity1 = ( proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask ) != 0;
        bool _proxy1Affinity0 = ( proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask ) != 0;

        return _proxy0Affinity1 || _proxy1Affinity0;
    }


}}}