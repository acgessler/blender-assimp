
SET( HEADER_PATH include/assimp )

SET( Core_SRCS
	code/Assimp.cpp
	code/AssimpPCH.cpp
	code/AssimpPCH.h
)

SET( Boost_SRCS
	code/BoostWorkaround/boost/math/common_factor_rt.hpp
	code/BoostWorkaround/boost/foreach.hpp
	code/BoostWorkaround/boost/format.hpp
	code/BoostWorkaround/boost/scoped_array.hpp
	code/BoostWorkaround/boost/scoped_ptr.hpp
	code/BoostWorkaround/boost/shared_array.hpp
	code/BoostWorkaround/boost/shared_ptr.hpp
	code/BoostWorkaround/boost/make_shared.hpp
	code/BoostWorkaround/boost/static_assert.hpp
	code/BoostWorkaround/boost/tuple/tuple.hpp
)

SET( Logging_SRCS
	${HEADER_PATH}/DefaultLogger.hpp
	${HEADER_PATH}/IOStream.hpp
	${HEADER_PATH}/LogStream.hpp
	${HEADER_PATH}/Logger.hpp
	${HEADER_PATH}/NullLogger.hpp
	code/Win32DebugLogStream.h
	code/DefaultLogger.cpp
	code/FileLogStream.h
)

SET( Common_SRCS
	code/fast_atof.h
	code/qnan.h
	code/BaseImporter.cpp
	code/BaseImporter.h
	code/BaseProcess.cpp
	code/BaseProcess.h
	code/Importer.h
	code/ScenePrivate.h
	code/PostStepRegistry.cpp
	code/ImporterRegistry.cpp
	code/ByteSwap.h
	code/DefaultProgressHandler.h
	code/DefaultIOStream.cpp
	code/DefaultIOStream.h
	code/DefaultIOSystem.cpp
	code/DefaultIOSystem.h
	code/CInterfaceIOWrapper.h
	code/Hash.h
	code/Importer.cpp
	code/IFF.h
	code/ParsingUtils.h
	code/StdOStreamLogStream.h
	code/StreamReader.h
	code/StringComparison.h
	code/SGSpatialSort.cpp
	code/SGSpatialSort.h
	code/VertexTriangleAdjacency.cpp
	code/VertexTriangleAdjacency.h
	code/GenericProperty.h
	code/SpatialSort.cpp
	code/SpatialSort.h
	code/SceneCombiner.cpp
	code/SceneCombiner.h
	code/ScenePreprocessor.cpp
	code/ScenePreprocessor.h
	code/SkeletonMeshBuilder.cpp
	code/SkeletonMeshBuilder.h
	code/SplitByBoneCountProcess.cpp
	code/SplitByBoneCountProcess.h
	code/SmoothingGroups.h
	code/StandardShapes.cpp
	code/StandardShapes.h
	code/TargetAnimation.cpp
	code/TargetAnimation.h
	code/RemoveComments.cpp
	code/RemoveComments.h
	code/Subdivision.cpp
	code/Subdivision.h
	code/Vertex.h
	code/LineSplitter.h
	code/TinyFormatter.h
	code/Profiler.h
	code/LogAux.h
)

SET( 3DS_SRCS
	code/3DSConverter.cpp
	code/3DSHelper.h
	code/3DSLoader.cpp
	code/3DSLoader.h
)

SET( AC_SRCS
	code/ACLoader.cpp
	code/ACLoader.h
)

SET( ASE_SRCS
	code/ASELoader.cpp
	code/ASELoader.h
	code/ASEParser.cpp
	code/ASEParser.h
)

SET( B3D_SRCS
	code/B3DImporter.cpp
	code/B3DImporter.h
)

SET( BVH_SRCS
	code/BVHLoader.cpp
	code/BVHLoader.h
)

SET( DXF_SRCS
	code/DXFLoader.cpp
	code/DXFLoader.h
	code/DXFHelper.h
)

SET( CSM_SRCS
	code/CSMLoader.cpp
	code/CSMLoader.h
)

SET( HMP_SRCS
	code/HMPFileData.h
	code/HMPLoader.cpp
	code/HMPLoader.h
	code/HalfLifeFileData.h
)

SET( Irr_SRCS
	code/IRRLoader.cpp
	code/IRRLoader.h
	code/IRRMeshLoader.cpp
	code/IRRMeshLoader.h
	code/IRRShared.cpp
	code/IRRShared.h
)

SET( LWO_SRCS
	code/LWOAnimation.cpp
	code/LWOAnimation.h
	code/LWOBLoader.cpp
	code/LWOFileData.h
	code/LWOLoader.cpp
	code/LWOLoader.h
	code/LWOMaterial.cpp
)

SET( LWS_SRCS
	code/LWSLoader.cpp
	code/LWSLoader.h
)

SET ( M3_SRCS
	code/M3Importer.cpp
	code/M3Importer.h
)

SET( MD2_SRCS
	code/MD2FileData.h
	code/MD2Loader.cpp
	code/MD2Loader.h
	code/MD2NormalTable.h
)

SET( MD3_SRCS
	code/MD3FileData.h
	code/MD3Loader.cpp
	code/MD3Loader.h
)

SET( MD5_SRCS
	code/MD5Loader.cpp
	code/MD5Loader.h
	code/MD5Parser.cpp
	code/MD5Parser.h
)

SET( MDC_SRCS
	code/MDCFileData.h
	code/MDCLoader.cpp
	code/MDCLoader.h
	code/MDCNormalTable.h
)

SET( MDL_SRCS
	code/MDLDefaultColorMap.h
	code/MDLFileData.h
	code/MDLLoader.cpp
	code/MDLLoader.h
	code/MDLMaterialLoader.cpp
)

SET( MaterialSystem_SRCS
	code/MaterialSystem.cpp
	code/MaterialSystem.h
)

SET( NFF_SRCS
	code/NFFLoader.cpp
	code/NFFLoader.h
)

SET( NDO_SRCS
	code/NDOLoader.cpp
	code/NDOLoader.h
)

SET( OFFFormat_SRCS
	code/OFFLoader.cpp
	code/OFFLoader.h
)

SET( Obj_SRCS
	code/ObjFileData.h
	code/ObjFileImporter.cpp
	code/ObjFileImporter.h
	code/ObjFileMtlImporter.cpp
	code/ObjFileMtlImporter.h
	code/ObjFileParser.cpp
	code/ObjFileParser.h
	code/ObjTools.h
	
	code/ObjExporter.h
	code/ObjExporter.cpp
)

SET( Ogre_SRCS
	code/OgreImporter.hpp
	code/OgreXmlHelper.hpp
	code/OgreImporter.cpp
	code/OgreMaterial.cpp
	code/OgreMesh.cpp
	code/OgreSkeleton.cpp
)

SET( Ply_SRCS
	code/PlyLoader.cpp
	code/PlyLoader.h
	code/PlyParser.cpp
	code/PlyParser.h
	code/PlyExporter.cpp
	code/PlyExporter.h
)

SET(MS3D_SRCS
	code/MS3DLoader.cpp
	code/MS3DLoader.h
)

SET(COB_SRCS
	code/COBLoader.cpp
	code/COBLoader.h
	code/COBScene.h
)

SET(IFC_SRCS
	code/IFCLoader.cpp
	code/IFCLoader.h
	code/IFCReaderGen.cpp
	code/IFCReaderGen.h
	code/IFCUtil.h
	code/IFCUtil.cpp
	code/IFCGeometry.cpp
	code/IFCMaterial.cpp
	code/IFCProfile.cpp
	code/IFCCurve.cpp
	code/STEPFile.h
	code/STEPFileReader.h
	code/STEPFileReader.cpp
)

SET( XGL_SRCS
	code/XGLLoader.cpp
	code/XGLLoader.h
)

SET(FBX_SRCS
	code/FBXCompileConfig.h
	code/FBXConverter.cpp
	code/FBXConverter.h
	code/FBXDocument.cpp
	code/FBXDocument.h
	code/FBXImporter.cpp
	code/FBXImporter.h
	code/FBXImportSettings.h
	code/FBXParser.cpp
	code/FBXParser.h
	code/FBXTokenizer.cpp
	code/FBXTokenizer.h
	code/FBXUtil.cpp
	code/FBXUtil.h
	code/FBXDocumentUtil.h
	code/FBXMaterial.cpp
	code/FBXMeshGeometry.cpp
	code/FBXModel.cpp
	code/FBXProperties.cpp
	code/FBXProperties.h
	code/FBXNodeAttribute.cpp
	code/FBXDeformer.cpp
	code/FBXAnimation.cpp
	code/FBXBinaryTokenizer.cpp
	code/FBXDocumentUtil.cpp
)


SET( PostProcessing_SRCS
	code/CalcTangentsProcess.cpp
	code/CalcTangentsProcess.h
	code/ComputeUVMappingProcess.cpp
	code/ComputeUVMappingProcess.h
	code/ConvertToLHProcess.cpp
	code/ConvertToLHProcess.h
	code/FindDegenerates.cpp
	code/FindDegenerates.h
	code/FindInstancesProcess.cpp
	code/FindInstancesProcess.h
	code/FindInvalidDataProcess.cpp
	code/FindInvalidDataProcess.h
	code/FixNormalsStep.cpp
	code/FixNormalsStep.h
	code/GenFaceNormalsProcess.cpp
	code/GenFaceNormalsProcess.h
	code/GenVertexNormalsProcess.cpp
	code/GenVertexNormalsProcess.h
	code/PretransformVertices.cpp
	code/PretransformVertices.h
	code/ImproveCacheLocality.cpp
	code/ImproveCacheLocality.h
	code/JoinVerticesProcess.cpp
	code/JoinVerticesProcess.h
	code/LimitBoneWeightsProcess.cpp
	code/LimitBoneWeightsProcess.h
	code/RemoveRedundantMaterials.cpp
	code/RemoveRedundantMaterials.h
	code/RemoveVCProcess.cpp
	code/RemoveVCProcess.h
	code/SortByPTypeProcess.cpp
	code/SortByPTypeProcess.h
	code/SplitLargeMeshes.cpp
	code/SplitLargeMeshes.h
	code/TerragenLoader.cpp
	code/TerragenLoader.h
	code/TextureTransform.cpp
	code/TextureTransform.h
	code/TriangulateProcess.cpp
	code/TriangulateProcess.h
	code/ValidateDataStructure.cpp
	code/ValidateDataStructure.h
	code/OptimizeGraph.cpp
	code/OptimizeGraph.h
	code/OptimizeMeshes.cpp
	code/OptimizeMeshes.h
	code/DeboneProcess.cpp
	code/DeboneProcess.h
	code/ProcessHelper.h
	code/ProcessHelper.cpp
	code/PolyTools.h
	code/MakeVerboseFormat.cpp
	code/MakeVerboseFormat.h
)

SET( Q3D_SRCS
	code/Q3DLoader.cpp
	code/Q3DLoader.h
)

SET( Q3BSP_SRCS
	code/Q3BSPFileData.h
	code/Q3BSPFileParser.h
	code/Q3BSPFileParser.cpp
	code/Q3BSPFileImporter.h
	code/Q3BSPFileImporter.cpp
	code/Q3BSPZipArchive.h
	code/Q3BSPZipArchive.cpp
)

SET( Raw_SRCS
	code/RawLoader.cpp
	code/RawLoader.h
)

SET( SMD_SRCS
	code/SMDLoader.cpp
	code/SMDLoader.h
)

SET( STL_SRCS
	code/STLLoader.cpp
	code/STLLoader.h
	code/STLExporter.h
	code/STLExporter.cpp
)

SET( Unreal_SRCS
	code/UnrealLoader.cpp
	code/UnrealLoader.h
)

SET( XFile_SRCS
	code/XFileHelper.h
	code/XFileImporter.cpp
	code/XFileImporter.h
	code/XFileParser.cpp
	code/XFileParser.h
)

SET( Exporter_SRCS
	code/Exporter.cpp
	code/AssimpCExport.cpp
	code/BlobIOSystem.h
)

SET( Extra_SRCS
	code/MD4FileData.h
)

SET( IrrXML_SRCS
	code/irrXMLWrapper.h
	contrib/irrXML/CXMLReaderImpl.h
	contrib/irrXML/heapsort.h
	contrib/irrXML/irrArray.h
	contrib/irrXML/irrString.h
	contrib/irrXML/irrTypes.h
	contrib/irrXML/irrXML.cpp
	contrib/irrXML/irrXML.h
)

SET( ConvertUTF_SRCS
	contrib/ConvertUTF/ConvertUTF.h
	contrib/ConvertUTF/ConvertUTF.c
)

SET( Clipper_SRCS 
	contrib/clipper/clipper.hpp
	contrib/clipper/clipper.cpp
)


SET( Poly2Tri_SRCS 
	contrib/poly2tri/poly2tri/common/shapes.cc
	contrib/poly2tri/poly2tri/common/shapes.h
	contrib/poly2tri/poly2tri/common/utils.h
	contrib/poly2tri/poly2tri/sweep/advancing_front.h
	contrib/poly2tri/poly2tri/sweep/advancing_front.cc
	contrib/poly2tri/poly2tri/sweep/cdt.cc
	contrib/poly2tri/poly2tri/sweep/cdt.h
	contrib/poly2tri/poly2tri/sweep/sweep.cc
	contrib/poly2tri/poly2tri/sweep/sweep.h
	contrib/poly2tri/poly2tri/sweep/sweep_context.cc
	contrib/poly2tri/poly2tri/sweep/sweep_context.h
)

SET( unzip_SRCS
	contrib/unzip/crypt.h
	contrib/unzip/ioapi.c
	contrib/unzip/ioapi.h
	contrib/unzip/unzip.c
	contrib/unzip/unzip.h
)


set(INC
	${PUBLIC_HEADERS}
	${COMPILER_HEADERS}
)

set(INC_SYS

)

set(SRC
	# Assimp Files
	${Core_SRCS}
	${Common_SRCS}
	${Logging_SRCS}
	${Exporter_SRCS}
	${PostProcessing_SRCS}

	# Model Support
	${3DS_SRCS}
	${AC_SRCS}
	${ASE_SRCS}
	${B3D_SRCS}
	${BVH_SRCS}
	${DXF_SRCS}
	${CSM_SRCS}
	${HMP_SRCS}
	${Irr_SRCS}
	${LWO_SRCS}
	${LWS_SRCS}
	${M3_SRCS}
	${MD2_SRCS}
	${MD3_SRCS}
	${MD5_SRCS}
	${MDC_SRCS}
	${MDL_SRCS}
	${MaterialSystem_SRCS}
	${NFF_SRCS}
	${OFFFormat_SRCS}
	${Obj_SRCS}
	${Ogre_SRCS}
	${Ply_SRCS}
	${Q3D_SRCS}
	${Q3BSP_SRCS}
	${Raw_SRCS}
	${SMD_SRCS}
	${STL_SRCS}
	${Unreal_SRCS}
	${XFile_SRCS}
	${Extra_SRCS}
	${MS3D_SRCS}
	${COB_SRCS}
	${NDO_SRCS}
	${IFC_SRCS}
	${XGL_SRCS}
	${FBX_SRCS}
	
	# Third-party libraries
	${IrrXML_SRCS}
	${ConvertUTF_SRCS}
	${unzip_SRCS}
	${Poly2Tri_SRCS}
	${Clipper_SRCS}
	${Boost_SRCS}
)

add_definitions(-DASSIMP_BUILD_NO_COLLADA_IMPORTER)
add_definitions(-DASSIMP_BUILD_NO_BLEND_IMPORTER)

if(WITH_BOOST)
	list(APPEND INC
		${BOOST_INCLUDE_DIR}
	)
else()
	add_definitions(-DASSIMP_BUILD_BOOST_WORKAROUND)

	list(APPEND INC
		code/BoostWorkaround/
	)
endif()

blender_add_lib(extern_assimp "${SRC}" "${INC}" "${INC_SYS}")


