
import numpy as np
import tysoc_bindings
import pytysoc
import time

NUM_BOXES = 5
NUM_SPHERES = 5
NUM_CYLINDERS = 5
NUM_CAPSULES = 5
NUM_MESHES = 0

def createFloor() :
    _collisionData = tysoc_bindings.PyCollisionData()
    _collisionData.type = tysoc_bindings.eShapeType.BOX
    _collisionData.size = [ 10., 10., 1. ]

    _visualData = tysoc_bindings.PyVisualData()
    _visualData.type = tysoc_bindings.eShapeType.BOX
    _visualData.size = [ 10., 10., 1. ]
    _visualData.ambient = [ 0.3, 0.5, 0.7 ]
    _visualData.diffuse = [ 0.3, 0.5, 0.7 ]
    _visualData.specular = [ 0.3, 0.5, 0.7 ]
    _visualData.shininess = 50.0

    _bodyData = tysoc_bindings.PyBodyData()
    _bodyData.dyntype = tysoc_bindings.eDynamicsType.STATIC
    _bodyData.addCollision( _collisionData )
    _bodyData.addVisual( _visualData )

    _floor = tysoc_bindings.PyBody( "floor", _bodyData, [ 0., 0., -0.5 ], [ 0., 0., 0. ] )

    return _floor

def createSingleBody( name, shape ) :
    _collisionData = tysoc_bindings.PyCollisionData()
    _visualData = tysoc_bindings.PyVisualData()
    _bodyData = tysoc_bindings.PyBodyData()

    if shape == 'box' :
        _collisionData.type = tysoc_bindings.eShapeType.BOX
        _collisionData.size = [ 0.2, 0.2, 0.2 ]

        _visualData.type = tysoc_bindings.eShapeType.BOX
        _visualData.size = [ 0.2, 0.2, 0.2 ]

    elif shape == 'sphere' :
        _collisionData.type = tysoc_bindings.eShapeType.SPHERE
        _collisionData.size = [ 0.1, 0.1, 0.1 ]

        _visualData.type = tysoc_bindings.eShapeType.SPHERE
        _visualData.size = [ 0.1, 0.1, 0.1 ]

    elif shape == 'cylinder' :
        _collisionData.type = tysoc_bindings.eShapeType.CYLINDER
        _collisionData.size = [ 0.1, 0.2, 0.1 ]

        _visualData.type = tysoc_bindings.eShapeType.CYLINDER
        _visualData.size = [ 0.1, 0.2, 0.1 ]

    elif shape == 'capsule' :
        _collisionData.type = tysoc_bindings.eShapeType.CAPSULE
        _collisionData.size = [ 0.1, 0.2, 0.1 ]

        _visualData.type = tysoc_bindings.eShapeType.CAPSULE
        _visualData.size = [ 0.1, 0.2, 0.1 ]

    elif shape == 'mesh' :
        _collisionData.type = tysoc_bindings.eShapeType.MESH
        _collisionData.size = [ 0.2, 0.2, 0.2 ]
        _collisionData.filename = pytysoc.PATHS.RESOURCES_DIR + "meshes/monkey.stl"

        _visualData.type = tysoc_bindings.eShapeType.MESH
        _visualData.size = [ 0.2, 0.2, 0.2 ]
        _visualData.filename = pytysoc.PATHS.RESOURCES_DIR + "meshes/monkey.stl"

    _visualData.ambient = [ 0.7, 0.5, 0.3 ]
    _visualData.diffuse = [ 0.7, 0.5, 0.3 ]
    _visualData.specular = [ 0.7, 0.5, 0.3 ]
    _visualData.shininess = 50.0;

    _bodyData = tysoc_bindings.PyBodyData()
    _bodyData.dyntype = tysoc_bindings.eDynamicsType.DYNAMIC
    _bodyData.addCollision( _collisionData )
    _bodyData.addVisual( _visualData )
    
    _position = 4.0 * ( np.random.rand( 3 ) - 0.5 )
    _position[2] = 3.0
    _rotation = np.pi * ( np.random.rand( 3 ) - 0.5 )

    _body = tysoc_bindings.PyBody( name,
                                   _bodyData,
                                   _position,
                                   _rotation )

    return _body

if __name__ == '__main__' :
    _scenario = tysoc_bindings.PyScenario()

    ## _terrainGen = tysoc_bindings.PyStaticTerrainGen( 'terrainGen0' ) 
    ## _terrainGen.createPrimitive( 'plane',
    ##                              [ 10.0, 10.0, 0.2 ],
    ##                              [ 0.0, 0.0, 0.0 ],
    ##                              [ 0.0, 0.0, 0.0 ],
    ##                              [ 0.2, 0.3, 0.4 ],
    ##                              'built_in_chessboard' )
    ## _scenario.addTerrainGen( _terrainGen )

    _scenario.addBody( createFloor() )
    
    for i in range( NUM_BOXES ) :
        _scenario.addBody( createSingleBody( 'box_' + str( i ), 'box' ) )

    for i in range( NUM_SPHERES ) :
        _scenario.addBody( createSingleBody( 'sphere_' + str( i ), 'sphere' ) )

    for i in range( NUM_CYLINDERS ) :
        _scenario.addBody( createSingleBody( 'cylinder_' + str( i ), 'cylinder' ) )

    for i in range( NUM_CAPSULES ) :
        _scenario.addBody( createSingleBody( 'capsule_' + str( i ), 'capsule' ) )

    for i in range( NUM_MESHES ) :
        _scenario.addBody( createSingleBody( 'mesh_' + str( i ), 'mesh' ) )
    
    _runtime = pytysoc.createRuntime( physicsBackend = pytysoc.BACKENDS.PHYSICS.BULLET,
                                      renderingBackend = pytysoc.BACKENDS.RENDERING.GLVIZ )

    _simulation = _runtime.createSimulation( _scenario )
    _visualizer = _runtime.createVisualizer( _scenario )
    
    _simulation.initialize()
    _visualizer.initialize()
    
    _simulation.step()
    _visualizer.render()

    _running = False
    
    while _visualizer.isActive() :
    
        if _visualizer.checkSingleKeyPress( tysoc_bindings.KEY_P ) :
            _running = not _running
        elif _visualizer.checkSingleKeyPress( tysoc_bindings.KEY_R ) :
            _simulation.reset()
        elif _visualizer.checkSingleKeyPress( tysoc_bindings.KEY_ESCAPE ) :
            break

        start = time.time()
        if _running :
            _simulation.step()
    
        _visualizer.render()

        duration = time.time() - start
        print( "step-time: {} ||| fps: {}".format( duration, 1.0 / duration ) )