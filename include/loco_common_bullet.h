#pragma once

#include <loco_common.h>
#include <loco_data.h>
// Main Bullet-API
#include <btBulletDynamicsCommon.h>
// Heightfield-terrain functionality
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
// Multibody bullet functionality
#include <BulletDynamics/Featherstone/btMultiBody.h>
#include <BulletDynamics/Featherstone/btMultiBodyConstraint.h>
#include <BulletDynamics/Featherstone/btMultiBodyConstraintSolver.h>
#include <BulletDynamics/Featherstone/btMultiBodyMLCPConstraintSolver.h>
#include <BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.h>
#include <BulletDynamics/Featherstone/btMultiBodyLinkCollider.h>
#include <BulletDynamics/Featherstone/btMultiBodyLink.h>
#include <BulletDynamics/Featherstone/btMultiBodyJointLimitConstraint.h>
#include <BulletDynamics/Featherstone/btMultiBodyJointMotor.h>
#include <BulletDynamics/Featherstone/btMultiBodySphericalJointMotor.h>
#include <BulletDynamics/Featherstone/btMultiBodyPoint2Point.h>
#include <BulletDynamics/Featherstone/btMultiBodyFixedConstraint.h>
#include <BulletDynamics/Featherstone/btMultiBodySliderConstraint.h>

namespace loco {
namespace bullet {

    // Conversions from and to bullet-math types and tinymath-math types
    btVector3 vec3_to_bt( const TVec3& vec );
    btMatrix3x3 mat3_to_bt( const TMat3& mat );
    btTransform mat4_to_bt( const TMat4& mat );
    TVec3 vec3_from_bt( const btVector3& vec );
    TMat3 mat3_from_bt( const btMatrix3x3& mat );
    TMat4 mat4_from_bt( const btTransform& mat );

    // Converts bullet's shape enum-type to its string representation
    std::string bt_shape_enum_to_str( const BroadphaseNativeTypes& bt_shape_enum );

    // Creates an appropriate bullet collision-shape given user shape data
    std::unique_ptr<btCollisionShape> CreateCollisionShape( const TShapeData& data );

    // Returns the volume of a given btCollisionShape (either primitive or mesh)
    double ComputeVolumeFromBtShape( const btCollisionShape* collision_shape );

    // Constructs a bt-convex-hull shape from a given set of vertices
    std::unique_ptr<btConvexHullShape> CreateConvexHull( const std::vector<TVec3>& mesh_vertices );

    // Constructs a bt-bvh-triangle-mesh-shape from given vertex data
    std::unique_ptr<btBvhTriangleMeshShape> CreateTriangularMesh( const std::vector<float>& mesh_vertices,
                                                                  const std::vector<int>& mesh_faces );

    // Creates vertices of an ellipsoid given user shape data
    std::vector<TVec3> CreateEllipsoidVertices( const TShapeData& data );

    // Creates vertices of a mesh given user shape data
    std::vector<TVec3> CreateConvexMeshVertices( const TShapeData& data );

    // Gets vertices of a given mesh, using a filepath for a mesh model
    void _CollectMeshVerticesFromFile( std::vector<TVec3>& mesh_vertices, const TShapeData& data );

    // Gets vertices of a given mesh, using user vertex-data
    void _CollectMeshVerticesFromUser( std::vector<TVec3>& mesh_vertices, const TShapeData& data );

    // Creates the vertex data of a given tri-mesh given user shape data
    void CreateTriangularMeshData( const TShapeData& data, std::vector<float>& dst_vertices, std::vector<int>& dst_faces );

    // Gets vertex-data of a given trimesh, using a filepath for a trimesm model
    void _CollectTrimeshDataFromFile( std::vector<float>& dst_vertices, std::vector<int>& dst_faces, const TShapeData& data );

    // Gets vertex-data of a given trimesh, using user vertex-data
    void _CollectTrimeshDataFromUser( std::vector<float>& dst_vertices, std::vector<int>& dst_faces, const TShapeData& data );

    // Custom deleter for assimp-scene objects
    struct aiSceneDeleter
    {
        void operator() ( const aiScene* assimp_scene ) const;
    };
}}