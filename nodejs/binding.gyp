{
    "targets":[
        {
            "target_name":"path_svr", 
            "sources":[
                "../src/apis.cc", 

                "../ext/tsp/indi.cpp",
                "../ext/tsp/randomize.cpp",
                "../ext/tsp/sort.cpp",
                "../ext/tsp/evaluator.cpp",
                "../ext/tsp/kopt.cpp",
                "../ext/tsp/cross.cpp",
                "../ext/tsp/environment.cpp",

                "../ext/tms/prim.cpp",
                "../ext/tms/tsp_ga.cpp",
                "../ext/tms/GnKmeansClassfy.cpp",
                "../ext/tms/TMSManager.cpp",

                "../ext/utils/UserLog.cpp",
                "../ext/utils/Strings.cpp",
                "../ext/utils/GeoTools.cpp",
                "../ext/utils/Vector2D.cpp",
                "../ext/utils/CatmullRom.cpp",
                "../ext/utils/convexhull.cpp",

                # "../ext/libjson/json_reader.cpp",
                # "../ext/libjson/json_writer.cpp",
                # "../ext/libjson/json_value.cpp",
                # "../ext/libjson/json_valueiterator.inl",       
                "../ext/libjson/cjson/cJSON.c",

                "../ext/thlib/mapInfo.cpp",
                "../ext/thlib/shw_mapinfol.cpp",
                "../ext/thlib/sti_cache_s.cpp",
                "../ext/thlib/sti_ctrl_s.cpp",
                "../ext/thlib/sti_index_ctrl.cpp",
                "../ext/thlib/sti_reader_s.cpp",
                "../ext/thlib/thfile.cpp",
                "../ext/thlib/thholiday.cpp",
                #"../ext/thlib/thlog.cpp",
                "../ext/thlib/thmemalloc.cpp",
                "../ext/thlib/thmemfile.cpp",
                "../ext/thlib/thstring.cpp",
                "../ext/thlib/thtime.cpp",
                "../ext/thlib/thwinfile.cpp",
                "../ext/thlib/ttl_mapinfo_r.cpp",
                "../ext/thlib/ttl_mapreader_r.cpp",
                "../ext/thlib/ReadTTLSpd.cpp",

                "../ext/shp/ShpObj.cpp",
                "../ext/shp/ShpReader.cpp",
                "../ext/shp/ShpWriter.cpp",

                "../ext/route/MMPoint.hpp",
                "../ext/route/MapBase.cpp",
                "../ext/route/MapName.cpp",
                "../ext/route/MapMesh.cpp",
                "../ext/route/MapNode.cpp",
                "../ext/route/MapLink.cpp",
                "../ext/route/MapPolygon.cpp",
                "../ext/route/MapTraffic.cpp",
                "../ext/route/MapCourse.cpp",
                "../ext/route/MapExtend.cpp",
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
                "../ext/cvt/FileForest.cpp",
                "../ext/cvt/FileVehicle.cpp",
                "../ext/cvt/FileVehicleEx.cpp",
                "../ext/cvt/FileExtend.cpp",
                "../ext/cvt/FileBuilding.cpp",
                "../ext/cvt/FileComplex.cpp",
                "../ext/cvt/FileEntrance.cpp",
                "../ext/cvt/FileMountain.cpp",
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
                        '-lnafxcw.lib', '-llibcmtd.lib', '-lC:/__Work/Trekking/Trekking/libs/libz/lib/zlib.lib', '-lC:/__Work/Trekking/Trekking/libs/libz/lib/zlib_x64.lib', '-LC:/__Work/Trekking/Trekking/libs/libz/lib',
                        # '-L<some library directory>'
                    ],
                    # "include_dirs": [
                    #     "<(module_root_dir)/tobii/include"
                    # ],
                    # "copies": [
                    # {
                    #     "destination": "<(module_root_dir)/build/Release/",
                    #     "files": [
                    #     "<(module_root_dir)/tobii/lib/x64/tobii_interaction_lib.dll",
                    #     "<(module_root_dir)/tobii/lib/x64/tobii_stream_engine.dll"
                    #     ]
                    # }],
                    "defines":[
                        # "USE_TECKING_DATA",
                        # "USE_PEDESTRIAN_DATA",
                        "WIN32", "STL_USE_MFC",
                    ],
                }],
            ],
            # "defines":[
            #     # "USE_TECKING_DATA",
            #     # "USE_PEDESTRIAN_DATA",
            # ],
        }
    ]
}
