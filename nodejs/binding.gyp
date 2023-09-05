{
    "targets":[
        {
            "target_name":"path_svr", 
            "sources":[
                "../src/apis.cc", 

                "../ext/utils/UserLog.cpp",
                "../ext/utils/Strings.cpp",
                "../ext/utils/GeoTools.cpp",
                "../ext/utils/Vector2D.cpp",
                "../ext/utils/CatmullRom.cpp",
                "../ext/utils/convexhull.cpp",
                "../ext/libjson/cjson/cJSON.c",

                "../ext/shp/ShpObj.cpp",
                "../ext/shp/ShpReader.cpp",
                "../ext/shp/ShpWriter.cpp",

                "../ext/mst/prim.cpp",
                "../ext/mst/tsp_ga.cpp",
                "../ext/tms/GnKmeansClassfy.cpp",
                "../ext/route/MMPoint.hpp",
                "../ext/route/MapBase.cpp",
                "../ext/route/MapName.cpp",
                "../ext/route/MapMesh.cpp",
                "../ext/route/MapNode.cpp",
                "../ext/route/MapLink.cpp",
                "../ext/route/MapPolygon.cpp",
                "../ext/route/MapTraffic.cpp",
                "../ext/route/DataManager.cpp",
                "../ext/route/TrafficManager.cpp",
                "../ext/route/RoutePlan.cpp",
                "../ext/route/RouteManager.cpp",
                "../ext/route/RoutePackage.cpp",

                "../ext/cvt/FileBase.cpp",
                "../ext/cvt/FileManager.cpp",
                "../ext/cvt/FileName.cpp",
                "../ext/cvt/FileMesh.cpp",
                "../ext/cvt/FilePedestrian.cpp",
                "../ext/cvt/FileTrekking.cpp",
                "../ext/cvt/FileVehicle.cpp",
                "../ext/cvt/FileBuilding.cpp",
                "../ext/cvt/FileComplex.cpp",
                "../ext/cvt/FileEntrance.cpp",
                "../ext/cvt/FileTraffic.cpp"
            ],
            "conditions":[
                ['OS=="linux"', {
                    "cflags":['-fexceptions',
                        '-unused-result',
                        '-fopenmp',
                    ],
                    "cflags_cc":['-fexceptions',
                        '-unused-result',
                        '-fopenmp',
                    ],
                    "ldflags": [
#                      '-fopenmp',
                   ],
                   "libraries": [
                        '-fopenmp',
                   ],

                }],
                ['OS=="win"', {
                    'msvs_settings': {
                        'VCLinkerTool': {
                            'AdditionalOptions': [ '/NODEFAULTLIB:library' ],
                            'IgnoreDefaultLibraryNames': [
                                'nafxcw.lib', 'libcmtd.lib'
                        ]},
                        'VCCLCompilerTool': {
                            'AdditionalOptions': ['/MT', '/openmp'],
                        },
                    },
                    # 'include_dirs': ["../ext/libjson/include"],
                    'libraries': [
                            '-lnafxcw.lib', '-llibcmtd.lib',
                            # '-L<some library directory>'
                    ],
                }],
            ],
            "defines":[
                # "USE_TECKING_DATA",
                # "USE_PEDESTRIAN_DATA",
            ],
        }
    ]
}
