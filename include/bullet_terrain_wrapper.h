
#pragma once

#include <bullet_common.h>
#include <bullet_utils.h>


#include <terrain_wrapper.h>


namespace tysoc {
namespace bullet {





    extern "C" TTerrainGenWrapper* terrain_createFromAbstract( terrain::TITerrainGenerator* terrainGenPtr,
                                                               const std::string& workingDir );

    extern "C" TTerrainGenWrapper* terrain_createFromParams( const std::string& name,
                                                             const TGenericParams& params,
                                                             const std::string& workingDir );


}}